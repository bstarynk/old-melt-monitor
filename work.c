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

#define SESSION_COOKIE_MOM "MOMSESSION"
static volatile atomic_bool stop_work_loop_mom;
static volatile atomic_long webcount_mom;
static struct momhashdict_st *websessiondict_mom;

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
web_doc_root_mom (const char *reqfupath, long reqcnt, onion_request *requ,
		  onion_response *resp)
{
  assert (!strncmp (reqfupath, MOM_WEB_DOC_ROOT_PREFIX,
		    strlen (MOM_WEB_DOC_ROOT_PREFIX)));
  const char *restpath = reqfupath + strlen (MOM_WEB_DOC_ROOT_PREFIX);
  MOM_DEBUGPRINTF (web, "web_doc_root request#%ld restpath=%s",
		   reqcnt, restpath);
  // reject path with .. inside, so reject URL going outside the web doc root
  if (strstr (reqfupath, ".."))
    return OCS_NOT_PROCESSED;
  // reject too long paths
  if (strlen (reqfupath) >= MOM_PATH_MAX - 16)
    return OCS_NOT_PROCESSED;
  char fpath[MOM_PATH_MAX];
#define CHECK_PATH_WEB_MOM(Dir)   do {					\
    memset (fpath, 0, sizeof(fpath));					\
    if (snprintf(fpath, sizeof(fpath), "%s/%s",				\
		 (Dir), restpath)					\
	<(int)sizeof(fpath)						\
	&& !access(fpath, R_OK)) {					\
      MOM_DEBUGPRINTF(web, "web_doc_root request#%ld got fpath %s",	\
		      reqcnt, fpath);					\
      return onion_shortcut_response_file(fpath, requ, resp);		\
    }									\
  } while(0)
  for (int rix = 0; rix < MOM_MAX_WEBDOCROOT; rix++)
    {
      const char *curdocroot = mom_webdocroot[rix];
      if (!curdocroot)
	break;
      CHECK_PATH_WEB_MOM (curdocroot);
    };
  CHECK_PATH_WEB_MOM (MOM_WEBDOCROOT_DIRECTORY);
#undef CHECK_PATH_WEB_MOM
}				/* end of web_doc_root_mom */


static onion_connection_status
web_login_template_mom (long reqcnt, const char *reqfupath,
			const momitem_t *reqmethitm, onion_request *requ,
			onion_response *resp)
{
  MOM_DEBUGPRINTF (web,
		   "web_login_template start request #%ld reqfupath '%s' reqmethitm %s",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm));
  FILE *wlf = NULL;
  char wlogintpath[MOM_PATH_MAX];
  // see the MOM_WEBLOGIN_TEMPLATE_FILE in every webdocroot
  for (unsigned ix = 0; ix < MOM_MAX_WEBDOCROOT && !wlf; ix++)
    {
      memset (wlogintpath, 0, sizeof (wlogintpath));
      if (!mom_webdocroot[ix])
	break;
      if (snprintf (wlogintpath, sizeof (wlogintpath),
		    "%s/%s", mom_webdocroot[ix], MOM_WEBLOGIN_TEMPLATE_FILE)
	  >= (int) sizeof (wlogintpath) - 2)
	MOM_FATAPRINTF ("too long wlogintpath %s for webdocroot#%d %s",
			wlogintpath, ix, mom_webdocroot[ix]);
      errno = 0;
      wlf = fopen (wlogintpath, "r");
      MOM_DEBUGPRINTF (web, "web_login_template ix#%d wlogintpath %s - %s",
		       ix, wlogintpath,
		       wlf ? "got login file" : strerror (errno));
    };
  if (!wlf)
    {
      memset (wlogintpath, 0, sizeof (wlogintpath));
      if (snprintf (wlogintpath, sizeof (wlogintpath),
		    "%s/%s", MOM_WEBDOCROOT_DIRECTORY,
		    MOM_WEBLOGIN_TEMPLATE_FILE) >=
	  (int) sizeof (wlogintpath) - 2)
	// this cannot happen, but still....
	MOM_FATAPRINTF
	  ("too long wlogintpath %s for builtin webdocroot directory",
	   wlogintpath);
      errno = 0;
      wlf = fopen (wlogintpath, "r");
      MOM_DEBUGPRINTF (web, "web_login_template builtin wlogintpath %s - %s",
		       wlogintpath,
		       wlf ? "got login file" : strerror (errno));
    };
  if (!wlf)
    {
      MOM_WARNPRINTF
	("failed to get web login template file %s for webrequest #%ld to full path %s",
	 MOM_WEBLOGIN_TEMPLATE_FILE, reqcnt, reqfupath);
      return OCS_NOT_PROCESSED;
    };
#warning web_login_template_mom incomplete
  MOM_FATAPRINTF
    ("web_login_template_mom incomplete request #%ld reqfupath '%s' reqmethitm %s",
     reqcnt, reqfupath, mom_item_cstring (reqmethitm));
}				/* end web_login_template_mom */

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
		   "handle_web request #%ld reqfupath '%s' reqpath '%s' reqflags/%x reqmethitm %s",
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
  if (!strncmp
      (reqpath, MOM_WEB_DOC_ROOT_PREFIX, strlen (MOM_WEB_DOC_ROOT_PREFIX))
      && (reqmethitm == MOM_PREDEFINED_NAMED (http_GET)
	  || reqmethitm == MOM_PREDEFINED_NAMED (http_HEAD)))
    {
      MOM_DEBUGPRINTF (web,
		       "handle_web request #%ld reqpath '%s' WEB_DOC_ROOT",
		       reqcnt, reqpath);
      return web_doc_root_mom (reqpath, reqcnt, requ, resp);
    }
  const char *sesscookie
    = onion_request_get_cookie (requ, SESSION_COOKIE_MOM);
  momvalue_t sessval = mom_hashdict_getcstr (websessiondict_mom, sesscookie);
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld  reqfupath %s reqmethitm %s sesscookie=%s sessval=%s",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm),
		   sesscookie, mom_output_gcstring (sessval));
  if (sessval.typnum == momty_null)
    {
      if (reqmethitm == MOM_PREDEFINED_NAMED (http_POST))
	return OCS_FORBIDDEN;
      return web_login_template_mom (reqcnt, reqfupath, reqmethitm, requ,
				     resp);
    };
#warning handle_web_mom should do something here
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
  onion_url *ourl = onion_root_url (onion_mom);
  {
    int onerr = 0;
    if ((onerr =
	 onion_url_add_handler (ourl, "^",
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
