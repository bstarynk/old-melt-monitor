// file work.c - working loop, web & socket

/**   Copyright (C)  2015 Free Software Foundation, Inc.
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "monimelt.h"

static volatile atomic_bool stop_work_loop_mom;
static volatile atomic_long webcount_mom;

_Thread_local int mom_worker_num;

static pthread_t worker_threads_mom[MOM_MAX_WORKERS + 2];

static onion *onion_mom;

bool
mom_should_stop (void)
{
  return atomic_load (&stop_work_loop_mom);
}

void
mom_stop_work (void)
{
  atomic_store (&stop_work_loop_mom, true);
  MOM_DEBUGPRINTF (run, "stop working");
}

static void *
work_run_mom (void *p)
{
  char wbuf[16];
  memset (wbuf, 0, sizeof (wbuf));
  intptr_t ix = (intptr_t) p;
  assert (ix > 0 && ix <= mom_nb_workers);
  snprintf (wbuf, sizeof (wbuf), "momwork#%d", (int) ix);
  pthread_setname_np (pthread_self (), wbuf);
  long count = 0;
  mom_worker_num = ix;
  usleep (ix * 1024);
  while (!mom_should_stop ())
    {
      count++;
      MOM_DEBUGPRINTF (run, "work_run count#%ld before step", count);
      if (MOM_UNLIKELY (!momhook_agenda_step ()))
	MOM_WARNPRINTF ("agenda_step failed count#%ld", count);
      if (MOM_UNLIKELY ((count + MOM_MAX_WORKERS) % 2048 == ix))
	usleep (20);
    }
  MOM_DEBUGPRINTF (run, "work_run count#%ld ending", count);
  return NULL;
}				/* end work_run_mom */



static void
start_workers_mom (void)
{
  MOM_DEBUGPRINTF (run, "start_workers nbworkers=%d", mom_nb_workers);
  for (unsigned ix = 1; ix <= (unsigned) mom_nb_workers; ix++)
    {
      int err = 0;
      if ((err =
	   pthread_create (&worker_threads_mom[ix], (pthread_attr_t *) NULL,
			   work_run_mom, (void *) ((intptr_t) ix))) != 0)
	MOM_FATAPRINTF ("pthread_create for worker#%d failed (%d = %s)", ix,
			err, strerror (err));
      MOM_DEBUGPRINTF (run, "start_workers worker#%u thread = %#lx", ix,
		       (long) worker_threads_mom[ix]);
      usleep (1000);
    }
}				/* end of start_workers_mom */


static void
join_workers_mom (void)
{
  MOM_DEBUGPRINTF (run, "join_workers nbworkers=%d", mom_nb_workers);
  for (unsigned ix = 1; ix <= (unsigned) mom_nb_workers; ix++)
    {
      int err = 0;
      void *ret = NULL;
      MOM_DEBUGPRINTF (run, "join_workers before joining worker#%d",
		       (int) ix);
      if ((err = pthread_join (worker_threads_mom[ix], &ret)) != 0)
	MOM_FATAPRINTF ("pthread_join for worker#%d failed (%d = %s)",
			ix, err, strerror (err));
    };
}				/* end of join_workers_mom */


static onion_connection_status
handle_web_mom (void *data, onion_request *requ, onion_response *resp)
{
  assert (data == NULL);
  long reqcnt = atomic_fetch_add (&webcount_mom, 1) + 1;
  const char *reqfupath = onion_request_get_fullpath (requ);
  const char *reqpath = onion_request_get_path (requ);
  unsigned reqflags = onion_request_get_flags (requ);
  momitem_t *reqmethitm = NULL;
  if ((reqflags & OR_METHODS) == OR_HEAD)
    reqmethitm = MOM_PREDEFINED_NAMED (http_HEAD);
  else if ((reqflags & OR_METHODS) == OR_GET)
    reqmethitm = MOM_PREDEFINED_NAMED (http_GET);
  else if ((reqflags & OR_METHODS) == OR_POST)
    reqmethitm = MOM_PREDEFINED_NAMED (http_POST);
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld reqfupath %s reqpath %s reqflags/%x reqmethitm %s",
		   reqcnt, reqfupath, reqpath, reqflags,
		   mom_item_cstring (reqmethitm));
  if (MOM_UNLIKELY (reqmethitm == NULL))
    return OCS_NOT_IMPLEMENTED;
  if (MOM_UNLIKELY (mom_should_stop ()))
    {
      MOM_WARNPRINTF ("handle_web aborting request #%ld full path %s", reqcnt,
		      reqfupath);
      return OCS_INTERNAL_ERROR;
    }
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld  reqfupath %s reqmethitm %s NOT PROCESSED !!!",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm));
  return OCS_NOT_PROCESSED;
}				/* end of handle_web_mom */


static void
start_web_onion_mom (void)
{
  MOM_INFORMPRINTF ("start web services using webhost %s", mom_web_host);
  onion_mom = onion_new (O_THREADED);
  onion_set_max_threads (onion_mom, mom_nb_workers);
  char *whcolon = strchr (mom_web_host, ':');
  if (whcolon && whcolon > mom_web_host)
    {
      *whcolon = 0;
      onion_set_hostname (onion_mom, mom_web_host);
      MOM_DEBUGPRINTF (web, "start_web_onion hostname %s", mom_web_host);
      *whcolon = ':';
    }
  if (whcolon && isdigit (whcolon[1]))
    {
      onion_set_port (onion_mom, whcolon + 1);
      MOM_DEBUGPRINTF (web, "start_web_onion port %s", whcolon + 1);
    };
  {
    onion_url *ourl = onion_root_url (onion_mom);
    int onerr = 0;
    for (int rix = 0; rix < MOM_MAX_WEBDOCROOT; rix++)
      {
	const char *curdocroot = mom_webdocroot[rix];
	if (!curdocroot)
	  break;
	onerr = 0;
	MOM_DEBUGPRINTF (web, "start_web_onion rix#%d curdocroot %s", rix,
			 curdocroot);
	if ((onerr = onion_url_add_handler (ourl, "^" MOM_WEB_DOC_ROOT_PREFIX,
					    onion_handler_export_local_new
					    (curdocroot))) != 0)
	  MOM_FATAPRINTF ("failed to add to  ^" MOM_WEB_DOC_ROOT_PREFIX
			  " export-local handler #%d for %s (onionerr#%d)",
			  rix, curdocroot, onerr);
	MOM_INFORMPRINTF ("will serve the files in %s/ from http://%s/%s",
			  curdocroot, mom_web_host, MOM_WEB_DOC_ROOT_PREFIX);
      };
    struct stat wrstat;
    memset (&wrstat, 0, sizeof (wrstat));
    if (!stat (MOM_WEBDOCROOT_DIRECTORY, &wrstat)
	&& (wrstat.st_mode & S_IFMT) == S_IFDIR)
      {
	onerr = 0;
	MOM_DEBUGPRINTF (web, "start_web_onion %s is directory",
			 MOM_WEBDOCROOT_DIRECTORY);
	if ((onerr = onion_url_add_handler (ourl, "^" MOM_WEB_DOC_ROOT_PREFIX,
					    onion_handler_export_local_new
					    (MOM_WEBDOCROOT_DIRECTORY))) != 0)
	  MOM_FATAPRINTF ("failed to add to  ^" MOM_WEB_DOC_ROOT_PREFIX
			  " export-local handler for webdocroot %s (onionerr#%d)",
			  MOM_WEBDOCROOT_DIRECTORY, onerr);
	MOM_INFORMPRINTF ("will serve the files in %s/ from http://%s/%s",
			  MOM_WEBDOCROOT_DIRECTORY, mom_web_host,
			  MOM_WEB_DOC_ROOT_PREFIX);
      };
    onerr = 0;
    if ((onerr =
	 onion_url_add_handler (ourl, "",
				onion_handler_new (handle_web_mom, NULL,
						   NULL))) != 0)
      MOM_FATAPRINTF ("failed to add generic webhandler (onionerr#%d", onerr);
  };
  MOM_DEBUGPRINTF (web, "start_web_onion before listening @%p", onion_mom);
  onion_listen (onion_mom);
}				/* end start_web_onion_mom */


void
mom_run_workers (void)
{
  MOM_INFORMPRINTF ("start run %d workers (webhost %s, socket %s)",
		    mom_nb_workers, mom_web_host, mom_socket);
  if (MOM_UNLIKELY (mom_nb_workers < MOM_MIN_WORKERS
		    || mom_nb_workers > MOM_MAX_WORKERS))
    MOM_FATAPRINTF ("invalid mom_nb_workers %d", mom_nb_workers);
  start_workers_mom ();
  if (mom_web_host && mom_web_host[0])
    start_web_onion_mom ();
  sched_yield ();
#warning should start the webservice, the socketservice, handle signals & timers
  join_workers_mom ();
  if (onion_mom)
    onion_listen_stop (onion_mom);
  assert (mom_should_stop ());
  MOM_INFORMPRINTF ("done running %d workers", mom_nb_workers);
}				/* end of mom_run_workers */
