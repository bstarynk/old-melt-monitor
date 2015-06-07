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

// maximal reply delay for a web request - in seconds
#define REPLY_TIMEOUT_MOM 3.5	/* reply timeout in seconds */

static volatile atomic_bool stop_work_loop_mom;
static volatile atomic_long webcount_mom;
static struct momhashdict_st *websessiondict_mom;
static pthread_mutex_t webmtx_mom = PTHREAD_MUTEX_INITIALIZER;
static char web_host_mom[80];

#define WEBEXCHANGE_MAGIC_MOM 815064971	/* webexchange magic 0x3094e78b */
struct webexchange_mom_st
{				// it is the itm_data1 of `web_exchange` items
  unsigned webx_magic;		// always WEBEXCHANGE_MAGIC_MOM
  double webx_time;
  long webx_reqcnt;
  momitem_t *webx_methitm;
  const momstring_t *webx_fupath;
  onion_request *webx_requ;
  onion_response *webx_resp;
  momvalue_t webx_sessionv;
  char *webx_outbuf;
  size_t webx_outsiz;
  FILE *webx_outfil;
  char webx_mimetype[48];
  int webx_code;
  pthread_cond_t webx_donecond;
  long webx__spare;
};				/* end of struct webexchange_mom_st */



#define WEBSESSION_MAGIC_MOM 659863763	/* websession magic 659863763 */
/// the websocket is expecting JSON messages on read, each message being newline-terminated
struct websession_mom_st
{
  // it is the itm_data1 of `web_session` items
  unsigned wbss_magic;		/* always WEBSESSION_MAGIC_MOM  */
  onion_websocket *wbss_websock;
  char *wbss_inbuf;		/* GC-scalar-allocated input buffer */
  unsigned wbss_insiz;		/* size of buffer */
  unsigned wbss_inoff;		/* used offset or length */
};				/* end of struct websession_mom_st */

static onion_connection_status websocketcb_mom (void *data,
						onion_websocket * ws,
						ssize_t datalen);

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
      MOM_DEBUGPRINTF (run, "work_run count#%ld after step", count);
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
      MOM_DEBUGPRINTF (web,
		       "web_login_post for request#%ld bad postuserstr=%s postpasswordstr=%s minpasslen=%d",
		       reqcnt, postuserstr, postpasswordstr,
		       MOM_MIN_PASSWD_LEN);
      MOM_WARNPRINTF
	("web login POST request#%ld fullpath=%s with bad user (%s) or password too short (%d minimal length)",
	 reqcnt, reqfupath, postuserstr, MOM_MIN_PASSWD_LEN);
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
	      || *fq == '=' || *fq == '.' || *fq == '-' || *fq == '_'
	      || *fq == '/')
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
  MOM_DEBUGPRINTF (web,
		   "web_login_post for request#%ld before authentificator user=%s password=%s",
		   reqcnt, postuserstr, postpasswordstr);
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
      struct websession_mom_st *wses =	//
	MOM_GC_ALLOC ("websession", sizeof (struct websession_mom_st));
      wses->wbss_magic = WEBSESSION_MAGIC_MOM;
      websessitm->itm_data1 = wses;
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
	 newfullurl, newfullurl, newfullurlhtml);
      MOM_DEBUGPRINTF (web,
		       "web_login_post for request#%ld user %s newfullurlhtml=%s (%s) redirstr=%s",
		       reqcnt, postuserstr,
		       newfullurlhtml,
		       (newfullurlhtml == newfullurl) ? "same" : "quoted",
		       mom_string_cstr (redirstr));
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


static void
unsync_clear_webexitem_mom (momitem_t *wxitm)
{
  assert (wxitm && wxitm != MOM_EMPTY);
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange))
    return;
  struct webexchange_mom_st *webex = wxitm->itm_data1;
  wxitm->itm_kind = NULL;
  wxitm->itm_data1 = NULL;
  assert (webex && webex->webx_magic == WEBEXCHANGE_MAGIC_MOM);
  if (webex->webx_outfil)
    {
      fclose (webex->webx_outfil), webex->webx_outfil = NULL;
    };
  if (webex->webx_outbuf)
    {
      free (webex->webx_outbuf), webex->webx_outbuf = NULL;
      webex->webx_outsiz = 0;
    };
  pthread_cond_destroy (&webex->webx_donecond);
  memset (webex, 0, sizeof (struct webexchange_mom_st));
}				/* end unsync_clear_webexitem_mom */


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
		   "handle_web request #%ld reqfupath '%s' reqpath '%s' reqflags/%#x reqmethitm %s (%d queries, %d posts)",
		   reqcnt, reqfupath, reqpath, reqflags,
		   mom_item_cstring (reqmethitm),
		   onion_dict_count (onion_request_get_query_dict (requ)),
		   onion_dict_count (onion_request_get_post_dict (requ)));
  if (MOM_UNLIKELY (reqmethitm == NULL))
    return OCS_NOT_IMPLEMENTED;
  const char *sesscookie
    = onion_request_get_cookie (requ, SESSION_COOKIE_MOM);
  momvalue_t sessval = mom_hashdict_getcstr (websessiondict_mom, sesscookie);
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld  reqfupath %s reqmethitm %s sesscookie=%s sessval=%s",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm),
		   sesscookie, mom_output_gcstring (sessval));
  const momnode_t *sessnod = mom_value_to_node (sessval);
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
  else if (!strcmp (reqpath, MOM_WEB_SOCKET_FULL_PATH))
    {
      onion_connection_status ocs = OCS_NOT_IMPLEMENTED;
      MOM_DEBUGPRINTF (web,
		       "handle_web request #%ld reqpath '%s' WEB_SOCKET",
		       reqcnt, reqpath);
      if (!sessnod)
	{
	  MOM_WARNPRINTF
	    ("websocket request #%ld reqpath '%s' without session", reqcnt,
	     reqpath);
	  return OCS_NOT_PROCESSED;
	}
      momitem_t *wsessitm = mom_value_to_item (mom_node_nth (sessnod, 0));
      assert (wsessitm != NULL);
      {
	mom_item_lock (wsessitm);
	assert (wsessitm->itm_kind == MOM_PREDEFINED_NAMED (web_session));
	struct websession_mom_st *wsess =
	  (struct websession_mom_st *) wsessitm->itm_data1;
	assert (wsess && wsess->wbss_magic == WEBSESSION_MAGIC_MOM);
	if (wsess->wbss_websock)
	  {
	    MOM_WARNPRINTF
	      ("websocket request #%ld reqpath '%s' already has websocket",
	       reqcnt, reqpath);
	  }
	else
	  {
	    wsess->wbss_websock = onion_websocket_new (requ, resp);
	    onion_websocket_set_userdata (wsess->wbss_websock, wsessitm,
					  NULL);
	    onion_websocket_set_callback (wsess->wbss_websock,
					  websocketcb_mom);
	    MOM_DEBUGPRINTF (web,
			     "handle_web request #%ld reqpath '%s' created websocket@%p in wsessitm=%s",
			     reqcnt, reqpath, wsess->wbss_websock,
			     mom_item_cstring (wsessitm));
	    ocs = OCS_WEBSOCKET;
	  };
	mom_item_unlock (wsessitm);
      }
      return ocs;
    }
  else if (!strcmp (reqpath, "/favicon.ico"))
    {
      // favicon.ico deserves a special redirection because it is often
      // requested by browsers
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
	  pthread_mutex_unlock (&webmtx_mom);
	  MOM_DEBUGPRINTF (web,
			   "handle_web request #%ld after login_post_mom ocs#%d",
			   reqcnt, (int) ocs);
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
  momitem_t *webxitm = mom_make_anonymous_item ();
  webxitm->itm_space = momspa_transient;
  struct webexchange_mom_st *webex =	//
    MOM_GC_ALLOC ("webexchange", sizeof (struct webexchange_mom_st));
  webex->webx_magic = WEBEXCHANGE_MAGIC_MOM;
  webex->webx_time = mom_clock_time (CLOCK_REALTIME);
  webex->webx_reqcnt = reqcnt;
  webex->webx_methitm = reqmethitm;
  webex->webx_fupath = mom_make_string_cstr (reqfupath);
  webex->webx_requ = requ;
  webex->webx_resp = resp;
  webex->webx_sessionv = sessval;
  unsigned respinisiz = 4072;
  webex->webx_outbuf = malloc (respinisiz);
  if (MOM_UNLIKELY (!webex->webx_outbuf))
    MOM_FATAPRINTF
      ("failed to allocate webx_outbuf of %u bytes for request#%ld; %m",
       respinisiz, reqcnt);
  memset (webex->webx_outbuf, 0, respinisiz);
  webex->webx_outsiz = respinisiz;
  pthread_cond_init (&webex->webx_donecond, NULL);
  webex->webx_outfil =
    open_memstream (&webex->webx_outbuf, &webex->webx_outsiz);
  if (MOM_UNLIKELY (webex->webx_outfil == NULL))
    MOM_FATAPRINTF
      ("failed to openmemstream for output of %u bytes for request#%ld; %m",
       respinisiz, reqcnt);
  webxitm->itm_kind = MOM_PREDEFINED_NAMED (web_exchange);
  webxitm->itm_data1 = webex;
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld reqfupath %s reqmethitm %s webxitm %s",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm),
		   mom_item_cstring (webxitm));
  assert (sessnod != NULL);
  assert (mom_node_arity (sessnod) == 3);
  long sessobstime = mom_value_to_int (mom_node_nth (sessnod, 1), -1);
  assert (reqfupath[0] == '/');
  bool foundhandler = false;
  momvalue_t vclos = MOM_NONEV;
  momvalue_t vrestpath = MOM_NONEV;
  {
    const momstring_t *restpathstr = NULL;
    char *reqfucopy =
      strchr (reqfupath + 1, '/') ? MOM_GC_STRDUP ("reqfucopy",
						   reqfupath) : reqfupath;
    assert (MOM_PREDEFINED_NAMED (web_processor) != NULL);
    mom_item_lock (MOM_PREDEFINED_NAMED (web_processor));
    char *lastslash = NULL;
    do
      {
	if (MOM_PREDEFINED_NAMED (web_processor)->itm_kind !=
	    MOM_PREDEFINED_NAMED (hashed_dict))
	  break;
	struct momhashdict_st *hdic =
	  MOM_PREDEFINED_NAMED (web_processor)->itm_data1;
	vclos = mom_hashdict_getcstr (hdic, reqfucopy);
	MOM_DEBUGPRINTF (web,
			 "handle_web request #%ld reqfucopy=%s vclos=%s restpathstr=%s",
			 reqcnt, reqfucopy, mom_output_gcstring (vclos),
			 mom_string_cstr (restpathstr));
	if (vclos.typnum == momty_node)
	  {
	    foundhandler = true;
	    break;
	  }
	else
	  {
	    char *prevlastslash = lastslash;
	    lastslash = strrchr (reqfucopy, '/');
	    if (lastslash && lastslash > reqfucopy)
	      {
		*lastslash = '\0';
		if (prevlastslash)
		  *prevlastslash = '/';
		restpathstr = mom_make_string_cstr (lastslash + 1);
	      }
	    else
	      restpathstr = NULL;
	  };
      }
    while (!foundhandler);
    if (vclos.typnum == momty_node)
      {
	MOM_DEBUGPRINTF (web,
			 "handle_web request #%ld got reqfucopy=%s vclos=%s restpathstr=%s ;"
			 " webxitm=%s sessval=%s",
			 reqcnt, reqfucopy, mom_output_gcstring (vclos),
			 mom_string_cstr (restpathstr),
			 mom_item_cstring (webxitm),
			 mom_output_gcstring (sessval));
      }
    else
      {
	vclos =
	  mom_item_unsync_get_attribute (MOM_PREDEFINED_NAMED (web_processor),
					 MOM_PREDEFINED_NAMED (fail));
	restpathstr = mom_make_string_cstr (reqfupath);
	reqfucopy = NULL;
	MOM_DEBUGPRINTF (web,
			 "handle_web request #%ld failing vclos=%s restpathstr=%s ;"
			 " webxitm=%s sessval=%s",
			 reqcnt, mom_output_gcstring (vclos),
			 mom_string_cstr (restpathstr),
			 mom_item_cstring (webxitm),
			 mom_output_gcstring (sessval));
      }
    vrestpath = mom_stringv (restpathstr);
    mom_item_unlock (MOM_PREDEFINED_NAMED (web_processor));
  }
  /// apply vclos to webxitm & reqmethitm & vrestpath & sessval
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld reqfupath %s applying vclos %s webxitm %s reqmethitm %s vrestpath %s sessval %s",
		   reqcnt, reqfupath, mom_output_gcstring (vclos),
		   mom_item_cstring (webxitm), mom_item_cstring (reqmethitm),
		   mom_output_gcstring (vrestpath),
		   mom_output_gcstring (sessval));
  if (mom_applval_2itm2val_to_void
      (vclos, webxitm, reqmethitm, vrestpath, sessval))
    {
      MOM_DEBUGPRINTF (web,
		       "handle_web request #%ld !!successful application"
		       " reqfupath %s vclos %s webxitm %s reqmethitm %s",
		       reqcnt, reqfupath, mom_output_gcstring (vclos),
		       mom_item_cstring (webxitm),
		       mom_item_cstring (reqmethitm));
      bool waitreply = false;
      do
	{
	  int waitres = -1;
	  waitreply = true;
	  mom_item_lock (webxitm);
	  if (webxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange)
	      || webxitm->itm_data1 != webex)
	    MOM_FATAPRINTF
	      ("handle_web request #%ld reqfupath %s bad webxitm %s after application",
	       reqcnt, reqfupath, mom_item_cstring (webxitm));
	  struct timespec ts =
	    mom_timespec (webex->webx_time + REPLY_TIMEOUT_MOM);
	  MOM_DEBUGPRINTF (web,
			   "handle_web request #%ld reqfupath %s waiting from reply to %s",
			   reqcnt, reqfupath, mom_item_cstring (webxitm));
	  if (webex->webx_code > 0)
	    // the closure might have replied already
	    waitres = 0;
	  else
	    waitres =
	      pthread_cond_timedwait (&webex->webx_donecond,
				      &webxitm->itm_mtx, &ts);
	  MOM_DEBUGPRINTF (web,
			   "handle_web request #%ld reqfupath %s waited from reply to %s waitres %s code %d mimetype %s",
			   reqcnt, reqfupath, mom_item_cstring (webxitm),
			   strerror (waitres), webex->webx_code,
			   webex->webx_mimetype);
	  if (!waitres && webex->webx_code > 0
	      && isalpha (webex->webx_mimetype[0]))
	    {
	      waitreply = false;
	      webxitm->itm_kind = NULL;
	      webxitm->itm_data1 = webxitm->itm_data2 = NULL;
	      long off = ftell (webex->webx_outfil);
	      MOM_DEBUGPRINTF (web,
			       "handle_web request #%ld reqfupath %s reqmethitm %s webxitm %s got reply code %d mimetype %s length %ld",
			       reqcnt, reqfupath,
			       mom_item_cstring (reqmethitm),
			       mom_item_cstring (webxitm),
			       webex->webx_code, webex->webx_mimetype, off);
	      fflush (webex->webx_outfil);
	      if ((long) (mom_elapsed_real_time () + 10.0) >= sessobstime)
		{
		  long newobstime =
		    (long) mom_elapsed_real_time () +
		    3 * SESSION_TIMEOUT_MOM / 4;
		  momvalue_t newwebval =
		    mom_nodev_new (MOM_PREDEFINED_NAMED (web_session), 3,
				   mom_node_nth (sessnod, 0),
				   mom_intv (newobstime),
				   mom_node_nth (sessnod, 2));
		  mom_valueptr_set_transient (&newwebval, true);
		  pthread_mutex_lock (&webmtx_mom);
		  websessiondict_mom =
		    mom_hashdict_put (websessiondict_mom,
				      mom_make_string_cstr (sesscookie),
				      newwebval);
		  pthread_mutex_unlock (&webmtx_mom);
		  MOM_DEBUGPRINTF (web,
				   "handle_web request #%ld refresh cookie %s newwebval %s",
				   reqcnt, sesscookie,
				   mom_output_gcstring (newwebval));
		  onion_response_add_cookie (webex->webx_resp,
					     SESSION_COOKIE_MOM, sesscookie,
					     SESSION_TIMEOUT_MOM, "/",
					     web_host_mom[0] ? web_host_mom :
					     NULL, 0);
		};
	      onion_response_set_code (webex->webx_resp, webex->webx_code);
	      if (!strncmp (webex->webx_mimetype, "text/", 5)
		  && !strstr (webex->webx_mimetype, "charset"))
		{
		  char fullmime[sizeof (webex->webx_mimetype) + 16];
		  memset (fullmime, 0, sizeof (fullmime));
		  snprintf (fullmime, sizeof (fullmime), "%s; charset=UTF-8",
			    webex->webx_mimetype);
		  onion_response_set_header (webex->webx_resp, "Content-Type",
					     fullmime);
		}
	      else
		onion_response_set_header (webex->webx_resp, "Content-Type",
					   webex->webx_mimetype);
	      onion_response_set_length (webex->webx_resp, off);
	      onion_response_write (webex->webx_resp, webex->webx_outbuf,
				    off);
	      webex = NULL;
	      unsync_clear_webexitem_mom (webxitm);
	    }
	  else if (mom_clock_time (CLOCK_REALTIME) >=
		   webex->webx_time + REPLY_TIMEOUT_MOM)
	    {
	      // timeout
	      MOM_WARNPRINTF
		("handle_web request #%ld reqfupath %s, webxitm %s, reqmethitm %s timed out",
		 reqcnt, reqfupath, mom_item_cstring (webxitm),
		 mom_item_cstring (reqmethitm));
	      waitreply = false;
	      char timeoutbuf[64];
	      memset (timeoutbuf, 0, sizeof (timeoutbuf));
	      mom_now_strftime_bufcenti (timeoutbuf,
					 "%Y %b %d, %H:%M:%S.__ %Z");
	      const momstring_t *timeoutstr =	//
		mom_make_string_sprintf
		("<!doctype html>\n"
		 "<html><head><title>MONIMELT timedout</title></head>\n"
		 "<body><h1>MONIMELT request #%ld timedout</h1>\n"
		 "%s request to <tt>%s</tt> timed out with webxitm %s on %s</body></html>\n",
		 reqcnt, mom_item_cstring (reqmethitm), reqfupath,
		 mom_item_cstring (webxitm),
		 timeoutbuf);
	      assert (timeoutstr != NULL);
	      onion_response_set_code (webex->webx_resp, HTTP_INTERNAL_ERROR);
	      onion_response_set_header (webex->webx_resp, "Content-Type",
					 "text/html; charset=UTF-8");
	      onion_response_set_length (webex->webx_resp, timeoutstr->slen);
	      onion_response_write (webex->webx_resp, timeoutstr->cstr,
				    timeoutstr->slen);
	      webex = NULL;
	      unsync_clear_webexitem_mom (webxitm);
	    }
	  mom_item_unlock (webxitm);
	}
      while (waitreply);
      MOM_DEBUGPRINTF (web, "handle_web request #%ld done", reqcnt);
      return OCS_PROCESSED;
    }
  else
    {				/* vclos application failed */
      MOM_DEBUGPRINTF (web,
		       "handle_web request #%ld **failed application"
		       " reqfupath %s vclos %s webxitm %s reqmethitm %s",
		       reqcnt, reqfupath, mom_output_gcstring (vclos),
		       mom_item_cstring (webxitm),
		       mom_item_cstring (reqmethitm));
      mom_item_lock (webxitm);
      webex = NULL;
      unsync_clear_webexitem_mom (webxitm);
      mom_item_unlock (webxitm);
    }
  MOM_DEBUGPRINTF (web,
		   "handle_web request #%ld  reqfupath %s reqmethitm %s NOT PROCESSED !!!",
		   reqcnt, reqfupath, mom_item_cstring (reqmethitm));
  return OCS_NOT_PROCESSED;
}				/* end of handle_web_mom */


void
mom_unsync_webexitem_reply (momitem_t *wxitm, const char *mimetype, int code)
{
  struct webexchange_mom_st *webex = NULL;
  if (!wxitm || wxitm == MOM_EMPTY)
    MOM_FATAPRINTF ("webexitem_reply without wxitm (code %d)", code);
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange)
      || !(webex = wxitm->itm_data1)
      || webex->webx_magic != WEBEXCHANGE_MAGIC_MOM)
    MOM_FATAPRINTF ("webexitem_reply with invalid wxitm %s (code %d)",
		    mom_item_cstring (wxitm), code);
  if (!mimetype || mimetype == MOM_EMPTY || !isalpha (mimetype[0]))
    MOM_FATAPRINTF ("webexitem_reply with bad mimetype (wxitem %s, code %d)",
		    mom_item_cstring (wxitm), code);
  if (code <= 0 || code > 999)
    MOM_FATAPRINTF ("webexitem_reply with bad code %d (wxitem %s)",
		    code, mom_item_cstring (wxitm));
  webex->webx_code = code;
  fflush (webex->webx_outfil);
  memset (webex->webx_mimetype, 0, sizeof (webex->webx_mimetype));
  strncpy (webex->webx_mimetype, mimetype, sizeof (webex->webx_mimetype) - 1);
  MOM_DEBUGPRINTF (web, "webexitem_reply wxitm %s mimetype %s code %d",
		   mom_item_cstring (wxitm), mimetype, code);
  pthread_cond_broadcast (&webex->webx_donecond);
}				/* end of mom_unsync_webexitem_reply  */

FILE *
mom_unsync_webexitem_file (const momitem_t *wxitm)
{
  if (!wxitm || wxitm == MOM_EMPTY)
    return NULL;
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange))
    return NULL;
  struct webexchange_mom_st *webex = wxitm->itm_data1;
  if (!webex || webex == MOM_EMPTY)
    return NULL;
  assert (webex && webex->webx_magic == WEBEXCHANGE_MAGIC_MOM);
  return webex->webx_outfil;
}				/* end mom_unsync_webexitem_file */


int
mom_unsync_webexitem_printf (momitem_t *wxitm, const char *fmt, ...)
{
  int n = -1;
  if (!wxitm || wxitm == MOM_EMPTY)
    return -1;
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange))
    return -1;
  struct webexchange_mom_st *webex = wxitm->itm_data1;
  if (!webex || webex == MOM_EMPTY)
    return -1;
  assert (webex && webex->webx_magic == WEBEXCHANGE_MAGIC_MOM);
  if (!webex->webx_outfil)
    return -1;
  va_list args;
  va_start (args, fmt);
  n = vfprintf (webex->webx_outfil, fmt, args);
  va_end (args);
  return n;
}				/* end mom_unsync_webexitem_printf */

onion_request *
mom_unsync_webexitem_request (momitem_t *wxitm)
{
  if (!wxitm || wxitm == MOM_EMPTY)
    return NULL;
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange))
    return NULL;
  struct webexchange_mom_st *webex = wxitm->itm_data1;
  if (!webex || webex == MOM_EMPTY)
    return NULL;
  assert (webex && webex->webx_magic == WEBEXCHANGE_MAGIC_MOM);
  return webex->webx_requ;
}				/* end of mom_unsync_webexitem_request */


onion_response *
mom_unsync_webexitem_response (momitem_t *wxitm)
{
  if (!wxitm || wxitm == MOM_EMPTY)
    return NULL;
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange))
    return NULL;
  struct webexchange_mom_st *webex = wxitm->itm_data1;
  if (!webex || webex == MOM_EMPTY)
    return NULL;
  assert (webex && webex->webx_magic == WEBEXCHANGE_MAGIC_MOM);
  return webex->webx_resp;
}				/* end of mom_unsync_webexitem_response */


int
mom_unsync_webexitem_fputs (momitem_t *wxitm, const char *str)
{
  int n = -1;
  if (!wxitm || wxitm == MOM_EMPTY)
    return -1;
  if (!str || str == MOM_EMPTY)
    return -1;
  if (wxitm->itm_kind != MOM_PREDEFINED_NAMED (web_exchange))
    return -1;
  struct webexchange_mom_st *webex = wxitm->itm_data1;
  if (!webex || webex == MOM_EMPTY)
    return -1;
  assert (webex && webex->webx_magic == WEBEXCHANGE_MAGIC_MOM);
  if (!webex->webx_outfil)
    return -1;
  n = fputs (str, webex->webx_outfil);
  return n;
}				/* end mom_unsync_webexitem_fputs */


static bool
default_web_authentificator_mom (const char *user, const char *passwd)
{
  bool good = false;
  assert (user != NULL);
  assert (passwd != NULL);
  MOM_DEBUGPRINTF (web, "default_web_authentificator start user=%s passwd=%s",
		   user, passwd);
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
      MOM_DEBUGPRINTF (web, "default_web_authentificator linbuf=%s", linbuf);
      if (linbuf[0] == '#')
	continue;
      if (strncmp (linbuf, user, userlen))
	continue;
      if (linbuf[userlen] != ':')
	continue;
      int linlen = strlen (linbuf);
      if (linlen < userlen + 8)
	continue;
      if (linbuf[linlen - 1] == '\n')
	linbuf[linlen - 1] = (char) 0;
      const char *linpass = linbuf + userlen + 1;
      const char *crypass = crypt (passwd, linpass);
      MOM_DEBUGPRINTF (web,
		       "default_web_authentificator passwd=%s linpass=%s crypass=%s",
		       passwd, linpass, crypass);
      if (!strcmp (crypass, linpass))
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
  onion_mom =
    onion_new (O_THREADED | O_DETACH_LISTEN | O_NO_SIGTERM | O_NO_SIGPIPE);
  onion_set_max_threads (onion_mom, mom_nb_workers + 1);
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
  MOM_DEBUGPRINTF (web, "start_web_onion done");
}				/* end start_web_onion_mom */



static onion_connection_status
websocketcb_mom (void *pridata, onion_websocket * ws, ssize_t datalen)
{
  onion_connection_status ocs = OCS_NOT_PROCESSED;
  momitem_t *wsessitm = (momitem_t *) pridata;
  struct websession_mom_st *wses = NULL;

  assert (ws != NULL);
  assert (wsessitm != NULL);
  mom_item_lock (wsessitm);
  MOM_DEBUGPRINTF (web, "websocketcb start wsessitm=%s datalen=%d",
		   mom_item_cstring (wsessitm), (int) datalen);
  if (wsessitm->itm_kind != MOM_PREDEFINED_NAMED (web_session)
      || !(wses = wsessitm->itm_data1))
    {
      MOM_WARNPRINTF ("websocketcb bad wsessitm=%s",
		      mom_item_cstring (wsessitm));
      goto end;
    };
  assert (wses->wbss_magic == WEBSESSION_MAGIC_MOM);
  assert (!wses->wbss_websock || wses->wbss_websock == ws);
  if (datalen < 0)
    {
      MOM_DEBUGPRINTF (web,
		       "websocketcb start wsessitm=%s closing websocket",
		       mom_item_cstring (wsessitm));
      wses->wbss_websock = NULL;
      ocs = OCS_CLOSE_CONNECTION;
    }
  else if (datalen > 0)
    {
      char *buf = wses->wbss_inbuf;
      if (MOM_UNLIKELY (buf == MOM_EMPTY))
	buf = NULL;
      unsigned bsiz = (buf != NULL) ? wses->wbss_insiz : 0;
      unsigned off = (buf != NULL) ? wses->wbss_inoff : 0;
      if (off + datalen + 4 >= bsiz)
	{
	  unsigned newsiz = ((off + datalen + bsiz / 4 + 100) | 0xff) + 1;
	  char *newbuf = MOM_GC_SCALAR_ALLOC ("websockbuf", newsiz);
	  if (off > 0)
	    memcpy (newbuf, wses->wbss_inbuf, off);
	  wses->wbss_inbuf = newbuf;
	  if (buf)
	    MOM_GC_FREE (buf, bsiz);
	  buf = newbuf;
	  wses->wbss_insiz = bsiz = newsiz;
	};
      buf[off] = '\0';
      int rlen = onion_websocket_read (ws, buf + off, datalen);
      if (rlen >= 0)
	buf[off + rlen] = '\0';
      MOM_DEBUGPRINTF (web,
		       "websocketcb wsessitm=%s read %d bytes offset %d:\n%s\n",
		       mom_item_cstring (wsessitm), rlen, off,
		       (rlen > 0) ? (buf + off) : "");
      const char *nl = NULL;
      do
	{
	  nl = strchr (buf, '\n');
	  if (!nl)
	    break;
	  int nloff = nl - buf;
	  assert (nloff <= (int) off);
	  json_error_t jerr;
	  memset (&jerr, 0, sizeof (jerr));
	  MOM_DEBUGPRINTF (web,
			   "websocketcb wsessitm=%s decoding %d bytes for JSON: %.*s\n",
			   mom_item_cstring (wsessitm), nloff, nloff, buf);
	  json_t *json = json_loadb (buf, nl - buf,
				     JSON_DECODE_ANY | JSON_DISABLE_EOF_CHECK,
				     &jerr);
	  if (!json)
	    MOM_WARNPRINTF
	      ("websocketcb wsessitm=%s json error %s at %d for input:\n%.*s\n",
	       mom_item_cstring (wsessitm), jerr.text, jerr.position, nloff,
	       buf);
	  else
	    {
	      momvalue_t vwebsockhdler = MOM_NONEV;
	      {
		mom_item_lock (MOM_PREDEFINED_NAMED (web_processor));
		vwebsockhdler =	//
		  mom_item_unsync_get_attribute	//
		  (MOM_PREDEFINED_NAMED (web_processor),
		   MOM_PREDEFINED_NAMED (websocket_handler));
		mom_item_unlock (MOM_PREDEFINED_NAMED (web_processor));
	      }
	      momitem_t *jsonitm = mom_make_anonymous_item ();
	      jsonitm->itm_kind = MOM_PREDEFINED_NAMED (json);
	      jsonitm->itm_space = momspa_transient;
	      jsonitm->itm_data1 = json;
	      MOM_DEBUGPRINTF (web, "websocketcb wsessitm=%s vwebsockhdler=%s jsonitm=%s decoded JSON %s", mom_item_cstring (wsessitm), mom_output_gcstring (vwebsockhdler), mom_item_cstring (jsonitm),	//
			       json_dumps (json,
					   JSON_ENSURE_ASCII |
					   JSON_ENCODE_ANY));
	      if (!mom_applval_2itm_to_void
		  (vwebsockhdler, wsessitm, jsonitm))
		MOM_WARNPRINTF
		  ("websocketcb failed to apply vwebsockhdler=%s to wsessitm=%s jsonitm=%s",
		   mom_output_gcstring (vwebsockhdler),
		   mom_item_cstring (wsessitm), mom_item_cstring (jsonitm));
	    };
	  memmove (buf, buf + nloff, off - nloff);
	  buf[off - nloff] = (char) 0;
	  wses->wbss_inoff = off - nloff;
	}
      while (nl);
      ocs = OCS_PROCESSED;
    }
end:
  mom_item_unlock (wsessitm);
  MOM_DEBUGPRINTF (web, "websocketcb end wsessitm=%s ocs#%d",
		   mom_item_cstring (wsessitm), (int) ocs);
  return ocs;
}				/* end websocketcb_mom */





void
mom_run_workers (void)
{
  extern void mom_event_loop (void);	// in file eventloop.c
  MOM_INFORMPRINTF ("start run %d workers (webhost %s, socket %s)",
		    mom_nb_workers, mom_web_host, mom_socket_path);
  if (MOM_UNLIKELY (mom_nb_workers < MOM_MIN_WORKERS
		    || mom_nb_workers > MOM_MAX_WORKERS))
    MOM_FATAPRINTF ("invalid mom_nb_workers %d", mom_nb_workers);
  start_workers_mom ();
  if (mom_web_host && mom_web_host[0])
    start_web_onion_mom ();
  sched_yield ();
  MOM_DEBUGPRINTF (run, "run_workers before event_loop");
  mom_event_loop ();
  join_workers_mom ();
  if (onion_mom)
    onion_listen_stop (onion_mom);
  assert (mom_should_stop ());
  MOM_INFORMPRINTF ("done running %d workers", mom_nb_workers);
}				/* end of mom_run_workers */
