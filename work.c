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
#define SESSION_TIMEOUT_MOM 4000	/* a bit more than one hour of inactivity */
static volatile atomic_bool stop_work_loop_mom;
static volatile atomic_long webcount_mom;
static struct momhashdict_st *websessiondict_mom;
static pthread_mutex_t webmtx_mom = PTHREAD_MUTEX_INITIALIZER;
static char web_host_mom[80];

uint32_t webloginrandom_mom;

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
	{
	  MOM_WARNPRINTF ("agenda_step failed count#%ld", count);
	  usleep (95000 + mom_random_nonzero_32_here () % 65536);
	};
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
  // reject paths starting with a dot
  if (reqfupath[0] == '.' || reqfupath[1] == '.')
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
  return OCS_NOT_PROCESSED;
}				/* end of web_doc_root_mom */


#define LOGIN_QUERY_DATA_MAGIC 373530929	/* login_query_data_magic 0x1643a131 */
struct login_query_data_mom_st
{
  unsigned logqd_magic;		/* always LOGIN_QUERY_DATA_MAGIC */
  unsigned logqd_count;
  FILE *logqd_outf;
  long logqd__gap;
};

static void
weblogin_querydictpreorder_mom (void *data, const char *key,
				const void *valuep, int flags)
{
  struct login_query_data_mom_st *plqd =
    (struct login_query_data_mom_st *) data;
  const char *sval = (const char *) valuep;
  if (!plqd || plqd->logqd_magic != LOGIN_QUERY_DATA_MAGIC)
    MOM_FATAPRINTF ("weblogin_querydictpreorder_mom bad plqd@%p for key=%s",
		    plqd, key);
  assert (plqd->logqd_outf != NULL);
  MOM_DEBUGPRINTF (web,
		   "weblogin_querydictpreorder key=%s count=%d sval=%s flags=%d",
		   key, plqd->logqd_count, sval, flags);
  if (plqd->logqd_count > 0)
    putc ('&', plqd->logqd_outf);
  plqd->logqd_count++;
  fprintf (plqd->logqd_outf, "%s=", key);
  if (sval)
    for (const char *ps = sval; *ps != '\0'; ps++)
      {
	if (*ps == ' ')
	  putc ('+', plqd->logqd_outf);
	else if (isalnum (*ps))
	  putc (*ps, plqd->logqd_outf);
	else
	  fprintf (plqd->logqd_outf, "%%%02x", (unsigned) (*ps) & 0xff);
      };
}				/* end of weblogin_querydictpreorder_mom */


static onion_connection_status
web_login_template_mom (long reqcnt, const char *reqfupath,
			const momitem_t *reqmethitm, onion_request *requ,
			onion_response *resp)
{
  if (MOM_UNLIKELY (webloginrandom_mom == 0))
    webloginrandom_mom = (mom_random_nonzero_32_here () & 0x3fffffff);
  MOM_DEBUGPRINTF (web,
		   "web_login_template start request #%ld reqfupath '%s' reqmethitm %s loginrandom=%u",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm),
		   (unsigned) webloginrandom_mom);
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

  size_t outsiz = 2048;
  char *outbuf = malloc (outsiz);
  size_t linsiz = 128;
  ssize_t linlen = -1;
  char *linbuf = malloc (linsiz);
  FILE *outf = NULL;
  int linecount = 0;
  int nbwlhidinp = 0;
  if (!outbuf || !linbuf || !(outf = open_memstream (&outbuf, &outsiz)))
    MOM_FATAPRINTF ("web_login_template request #%ld full path '%s' failed"
		    " to allocate output buffer of %zd bytes or line of %zd bytes",
		    reqcnt, reqfupath, outsiz, linsiz);
  memset (outbuf, 0, outsiz);
  char timbuf[80];
  memset (timbuf, 0, sizeof (timbuf));
  mom_now_strftime_bufcenti (timbuf, "%Y %b %d, %H:%M:%S.__ %Z");
  while ((linlen = getline (&linbuf, &linsiz, wlf)) >= 0)
    {
      const char *pc = linbuf;
      size_t pilen = 0;
      linecount++;
      while (pc && *pc && isspace (*pc))
	pc++;
      if (!strncmp (pc, MOM_WEBLOGIN_HIDDEN_INPUT_PI,
		    (pilen = sizeof (MOM_WEBLOGIN_HIDDEN_INPUT_PI) - 1))
	  || !strncmp (pc, MOM_WEBLOGIN_HIDDEN_INPUT_COMM,
		       (pilen = sizeof (MOM_WEBLOGIN_HIDDEN_INPUT_COMM) - 1)))
	{
	  nbwlhidinp++;
	  if (pc > linbuf)
	    fwrite (linbuf, pc - linbuf, 1, outf);
	  fprintf (outf, " <!-- WEBLOGIN GENERATED at %s -->\n", timbuf);
	  fprintf (outf,
		   "<input type='hidden' name='mom_web_full_path' value='%s'/>\n",
		   reqfupath);
	  fprintf (outf,
		   "<input type='hidden' name='mom_login_random', value='%d'/>\n",
		   (int) webloginrandom_mom);
	  const onion_dict *querydict = onion_request_get_query_dict (requ);
	  if (querydict && onion_dict_count (querydict) > 0)
	    {
	      struct login_query_data_mom_st lqd;
	      memset (&lqd, 0, sizeof (lqd));
	      lqd.logqd_magic = LOGIN_QUERY_DATA_MAGIC;
	      lqd.logqd_outf = outf;
	      lqd.logqd_count = 0;
	      fputs ("<input type='hidden' name='mom_web_query' value='",
		     outf);
	      onion_dict_preorder (querydict, weblogin_querydictpreorder_mom,
				   &lqd);
	      memset (&lqd, 0, sizeof (lqd));
	      fputs ("'/>\n", outf);
	    }
	  if (pc + pilen < linbuf + linlen)
	    fputs (pc + pilen, outf);
	}
      else
	if (!strncmp
	    (pc, MOM_WEBLOGIN_TIMESTAMP_PI,
	     (pilen = sizeof (MOM_WEBLOGIN_TIMESTAMP_PI) - 1)))
	{
	  if (pc > linbuf)
	    fwrite (linbuf, pc - linbuf, 1, outf);
	  fputs (timbuf, outf);
	  if (pc + pilen < linbuf + linlen)
	    fputs (pc + pilen, outf);
	}
      else
	{
	  char *piptr = strstr (linbuf, "<?mom_web_login");
	  if (MOM_UNLIKELY (piptr != NULL) && piptr > linbuf)
	    MOM_WARNPRINTF ("in weblogin file %s line #%d got unexpected"
			    " HTML processing instruction %s\n for request #%ld full path '%s'",
			    wlogintpath, linecount, piptr, reqcnt, reqfupath);
	  fputs (linbuf, outf);
	}
    };				/* end while ... getline ... */
  if (nbwlhidinp != 1)
    MOM_FATAPRINTF
      ("invalid login template file %s; it should contain exactly one occurrence of "
       MOM_WEBLOGIN_HIDDEN_INPUT_PI " or " MOM_WEBLOGIN_HIDDEN_INPUT_COMM
       " but got %d", wlogintpath, nbwlhidinp);
  fclose (wlf), wlf = NULL;
  long outbuflen = ftell (outf);
  if (MOM_UNLIKELY (fflush (outf)))
    MOM_FATAPRINTF
      ("web_login_template_mom failed to flush output buffer (of %ld bytes) for request#%ld to full path %s",
       outbuflen, reqcnt, reqfupath);
  assert (outbuflen > 0);
  if (outbuflen < (long) outsiz)
    outbuf[outbuflen] = (char) 0;
  MOM_DEBUGPRINTF (web,
		   "web_login_template for request#%ld to full path %s: output buffer of %ld bytes:\n%s\n\n",
		   reqcnt, reqfupath, outbuflen, outbuf);
  onion_response_set_length (resp, outbuflen);
  onion_response_set_code (resp, HTTP_OK);
  onion_response_set_header (resp, "Content-Type",
			     "text/html; charset=utf-8");
  onion_response_write (resp, outbuf, outbuflen);
  return OCS_PROCESSED;
}				/* end web_login_template_mom */


static onion_connection_status
web_login_post_mom (long reqcnt, const char *reqfupath,
		    onion_request *requ, onion_response *resp)
{
  assert (requ != NULL);
  assert (resp != NULL);
  const char *postrandomstr =
    onion_request_get_post (requ, "mom_login_random");
  const char *postfullpathstr =
    onion_request_get_post (requ, "mom_web_full_path");
  const char *postquerystr = onion_request_get_post (requ, "mom_web_query");
  const char *postuserstr = onion_request_get_post (requ, "mom_login_user");
  const char *postpasswordstr =
    onion_request_get_post (requ, "mom_login_password");
  const char *postdologinstr = onion_request_get_post (requ, "mom_do_login");
  MOM_DEBUGPRINTF (web,
		   "web_login_post for request#%ld to full path %s;\n .."
		   "random=%s fullpath=%s query=%s user=%s password=%s dologin=%s",
		   reqcnt, reqfupath,
		   postrandomstr, postfullpathstr, postquerystr,
		   postuserstr, postpasswordstr, postdologinstr);
  if (!postrandomstr || atoi (postrandomstr) != (int) webloginrandom_mom)
    {
      MOM_WARNPRINTF
	("web login POST request#%ld fullpath=%s longinrandom mismatch expecting %d",
	 reqcnt, reqfupath, webloginrandom_mom);
      return OCS_FORBIDDEN;
    };
  if (!postuserstr || !postpasswordstr || !isalnum (postuserstr[0])
      || strlen (postpasswordstr) < MOM_MIN_PASSWD_LEN)
    {
      MOM_WARNPRINTF
	("web login POST request#%ld fullpath=%s with bad user (%s) or password",
	 reqcnt, reqfupath, postuserstr);
      return OCS_FORBIDDEN;
    }
  if (postfullpathstr)
    {
      for (const char *fp = postfullpathstr; *fp; fp++)
	{
	  if (isalnum (*fp) || *fp == '+' || *fp == '-' || *fp == '/'
	      || (*fp == '.' && fp > postfullpathstr && isalnum (fp[-1])))
	    continue;
	  else
	    {
	      MOM_WARNPRINTF
		("web_login_post for request#%ld  invalid postfullpathstr %s",
		 reqcnt, postfullpathstr);
	      return OCS_FORBIDDEN;
	    };
	}
    }
  else
    postfullpathstr = "/";
  if (postquerystr)
    {
      for (const char *fq = postquerystr; *fq; fq++)
	{
	  if (isalnum (*fq) || *fq == '%' || *fq == '&' || *fq == '+'
	      || *fq == '.' || *fq == '-' || *fq == '_' || *fq == '/')
	    continue;
	  else
	    {
	      MOM_WARNPRINTF
		("web_login_post for request#%ld invalid postquerystr %s",
		 reqcnt, postquerystr);
	      return OCS_FORBIDDEN;
	    };
	}
    };
  if (!mom_web_authentificator)
    {
      MOM_WARNPRINTF
	("web login POST request#%ld fullpath=%s without authentificator routine",
	 reqcnt, reqfupath);
      usleep (100 + mom_random_nonzero_32_here () % 4096);
      return OCS_FORBIDDEN;
    }
  if ((*mom_web_authentificator) (postuserstr, postpasswordstr))
    {
      MOM_DEBUGPRINTF (web,
		       "web_login_post for request#%ld authentified user %s",
		       reqcnt, postuserstr);
      usleep (100 + mom_random_nonzero_32_here () % 4096);
      if (!postfullpathstr || postfullpathstr[0] == '\0')
	postfullpathstr = "/";
      const char *newfullurl = NULL;
      char fullurlbuf[MOM_PATH_MAX];
      const momstring_t *fullurlstr = NULL;
      memset (fullurlbuf, 0, sizeof (fullurlbuf));
      if (postquerystr && postquerystr[0])
	{
	  if (strlen (postquerystr) + strlen (postfullpathstr) + 10 <
	      sizeof (fullurlbuf))
	    {
	      snprintf (fullurlbuf, sizeof (fullurlbuf), "%s?%s",
			postfullpathstr, postquerystr);
	      newfullurl = fullurlbuf;
	    }
	  else
	    {
	      fullurlstr =
		mom_make_string_sprintf ("%s?%s", postfullpathstr,
					 postquerystr);
	      newfullurl = fullurlstr->cstr;
	    };
	}
      else
	newfullurl = postfullpathstr;
      MOM_DEBUGPRINTF (web,
		       "web_login_post for request#%ld for authentified user %s redirect newfullurl %s",
		       reqcnt, postuserstr, newfullurl);
      momitem_t *websessitm = mom_make_anonymous_item ();
      websessitm->itm_kind = MOM_PREDEFINED_NAMED (web_session);
      websessitm->itm_space = momspa_transient;
      momvalue_t valuser = mom_stringv_cstr (postuserstr);
      mom_item_unsync_put_attribute (websessitm, MOM_PREDEFINED_NAMED (user),
				     valuser);
      char cookiebuf[80];
      memset (cookiebuf, 0, sizeof (cookiebuf));
      snprintf (cookiebuf, sizeof (cookiebuf), "%s/%08d",
		mom_item_cstring (websessitm),
		mom_random_nonzero_32_here () % 100000000);
      long obstime =
	(long) mom_elapsed_real_time () + 3 * SESSION_TIMEOUT_MOM / 4;
      momvalue_t webval =
	mom_nodev_new (MOM_PREDEFINED_NAMED (web_session), 3,
		       mom_itemv (websessitm),
		       mom_intv (obstime),
		       valuser);
      mom_valueptr_set_transient (&webval, true);
      MOM_DEBUGPRINTF (web,
		       "web_login_post for request#%ld user %s has sessionval %s for cookie %s",
		       reqcnt, postuserstr, mom_output_gcstring (webval),
		       cookiebuf);
      websessiondict_mom =
	mom_hashdict_put (websessiondict_mom,
			  mom_make_string_cstr (cookiebuf), webval);
      onion_response_add_cookie (resp, SESSION_COOKIE_MOM, cookiebuf,
				 SESSION_TIMEOUT_MOM, "/",
				 web_host_mom[0] ? web_host_mom : NULL, 0);
      const char *newfullurlhtml =
	onion_html_quote (newfullurl) ? : newfullurl;
      const momstring_t *redirstr =	//
	mom_make_string_sprintf
	("<!doctype html>\n"
	 "<html>\n"
	 "<head><title>MELTmon redirect</title>\n"
	 "<meta http-equiv=\"refresh\" content='1; URL=%s'/>"
	 "</head>\n"
	 "<body><h1>redirection to <a href='%s'>%s</a></h1></body>\n"
	 "</html>\n",
	 newfullurl, newfullurlhtml, newfullurl);
      MOM_DEBUGPRINTF (web,
		       "web_login_post for request#%ld user %s redirstr=%s",
		       reqcnt, postuserstr, mom_string_cstr (redirstr));
      assert (redirstr != NULL);
      onion_response_set_header (resp, "Location", newfullurl);
      onion_response_set_length (resp, redirstr->slen);
      onion_response_set_header (resp, "Content-Type",
				 "text/html; charset=utf-8");
      onion_response_set_code (resp, HTTP_REDIRECT);
      onion_response_write0 (resp, redirstr->cstr);
      return OCS_PROCESSED;
    }
  else
    {
      MOM_WARNPRINTF
	("web login POST request#%ld fullpath=%s postfullpathstr=%s authentification for user %s failed",
	 reqcnt, reqfupath, postfullpathstr, postuserstr);
      return OCS_FORBIDDEN;
    }
}				/* end of web_login_post_mom */



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
  else if (!strcmp (reqpath, "/favicon.ico"))
    {				// favicon.ico deserve a special redirection because it is often
      // requested by browsers/
      MOM_DEBUGPRINTF (web,
		       "handle_web request #%ld reqpath '%s' WEB /favicon.ico",
		       reqcnt, reqpath);
      return web_doc_root_mom ("/" MOM_WEB_DOC_ROOT_PREFIX "/favicon.ico",
			       reqcnt, requ, resp);
    }
  if (!strcmp (reqfupath, "/" MOM_WEBLOGIN_ACTION))
    {
      MOM_DEBUGPRINTF (web,
		       "handle_web request #%ld reqfupath '%s' LOGIN ACTION",
		       reqcnt, reqfupath);
      // we use some random usleep-s to slight bother the malicious attacker....
      if (reqmethitm == MOM_PREDEFINED_NAMED (http_POST))
	{
	  usleep (6000 + getpid () % 1024 +
		  mom_random_nonzero_32_here () % 32768);
	  pthread_mutex_lock (&webmtx_mom);
	  onion_connection_status ocs =	//
	    web_login_post_mom (reqcnt, reqfupath, requ, resp);
	  pthread_mutex_lock (&webmtx_mom);
	  return ocs;
	}
      else
	{
	  usleep (4000 + getpid () % 1024 +
		  mom_random_nonzero_32_here () % 4096);
	  pthread_mutex_lock (&webmtx_mom);
	  onion_connection_status ocs =	//
	    web_login_template_mom (reqcnt, reqfupath, reqmethitm, requ,
				    resp);
	  pthread_mutex_unlock (&webmtx_mom);
	  return ocs;
	}
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
      pthread_mutex_lock (&webmtx_mom);
      onion_connection_status ocs =	//
	web_login_template_mom (reqcnt, reqfupath, reqmethitm, requ, resp);
      pthread_mutex_unlock (&webmtx_mom);
      return ocs;
    };
  const momnode_t *sessnod = mom_value_to_node (sessval);
  assert (sessnod != NULL);
#warning handle_web_mom should do something here
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld  reqfupath %s reqmethitm %s NOT PROCESSED !!!",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm));
  return OCS_NOT_PROCESSED;
}				/* end of handle_web_mom */

static bool
default_web_authentificator_mom (const char *user, const char *passwd)
{
  bool good = false;
  assert (user != NULL);
  assert (passwd != NULL);
  FILE *fp = fopen (mom_webpasswdfile, "r");
  if (!fp)
    {
      MOM_WARNPRINTF ("failed to open default web password %s - %m",
		      mom_webpasswdfile);
      return false;
    };
  if (!isalpha (user[0]) || !isalnum (user[1]))
    return false;
  for (const char *pu = user; *pu; pu++)
    if (!isalnum (*pu) && *pu != '_' && *pu != '.' && *pu != '@')
      return false;
  int userlen = strlen (user);
  int passlen = strlen (passwd);
  if (!userlen || !passlen)
    return false;
  char linbuf[256];
  do
    {
      memset (linbuf, 0, sizeof (linbuf));
      if (!fgets (linbuf, sizeof (linbuf) - 2, fp))
	break;
      if (linbuf[0] == '#')
	continue;
      if (strncmp (linbuf, user, userlen))
	continue;
      if (linbuf[userlen] != ':')
	continue;
      if (!strcmp
	  (crypt (passwd, linbuf + userlen + 1), linbuf + userlen + 1))
	{
	  MOM_DEBUGPRINTF (web, "authentifying user %s with %s", user,
			   linbuf);
	  good = true;
	}
    }
  while (!feof (fp));
  fclose (fp);
  MOM_DEBUGPRINTF (web, "web authentification of user %s is %s",
		   user, good ? "good" : "bad");
  return good;
}				/* end of default_web_authentificator_mom */

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
      strncpy (web_host_mom, mom_web_host, sizeof (web_host_mom) - 1);
      MOM_DEBUGPRINTF (web, "start_web_onion hostname %s", mom_web_host);
      *whcolon = ':';
    }
  if (whcolon && isdigit (whcolon[1]))
    {
      onion_set_port (onion_mom, whcolon + 1);
      MOM_DEBUGPRINTF (web, "start_web_onion port %s", whcolon + 1);
    };
  if (!mom_webpasswdfile && !access (MOM_DEFAULT_WEBPASSWD, R_OK))
    mom_webpasswdfile = MOM_DEFAULT_WEBPASSWD;
  if (mom_webpasswdfile)
    mom_web_authentificator = default_web_authentificator_mom;
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
