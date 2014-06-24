// file web-onion.c

/**   Copyright (C)  2014 Free Software Foundation, Inc.
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

static onion *onion_mom = NULL;
static onion_url *onion_root_url_mom = NULL;
static pthread_mutex_t onion_mtx_mom = PTHREAD_MUTEX_INITIALIZER;
static int onion_nb_web_requests_mom;


struct webdict_mom_st
{
  unsigned webd_size;
  unsigned webd_count;
  struct mom_jsonentry_st webd_pairtab[];
};


// used thru onion_dict_preorder
static void
webdict_add_mom (void *data, const char *key, const void *value, int flags
		 __attribute__ ((unused)))
{
  struct webdict_mom_st *wd = data;
  unsigned cnt = wd->webd_count;
  assert (cnt < wd->webd_size);
  wd->webd_pairtab[cnt].je_name = (momval_t) mom_make_string (key);
  wd->webd_pairtab[cnt].je_attr = (momval_t) mom_make_string (value);
  wd->webd_count = cnt + 1;
}



//internally used in payload.c
void
mom_paylwebx_finalize (momitem_t *witm, void *wdata)
{
  struct mom_webexchange_data_st *wxd = wdata;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  assert (witm && witm->i_typnum == momty_item);
  MOM_DEBUG (web, MOMOUT_LITERAL ("finalizing witm:"),
	     MOMOUT_ITEM ((const momitem_t *) witm),
	     MOMOUT_LITERAL (" webreq#"), MOMOUT_DEC_INT (wxd->webx_num));
  if (wxd->webx_out.mout_file)
    {
      fclose (wxd->webx_out.mout_file);
      wxd->webx_out.mout_file = NULL;
    };
  memset (&wxd->webx_out, 0, sizeof (wxd->webx_out));
  if (wxd->webx_obuf)
    // we need to free, not GC_free, the webx_obuf, because
    // open_memstream wants it like that...
    free (wxd->webx_obuf), wxd->webx_obuf = NULL;
  /// we don't call onion_request_free or onion_response_free since the
  /// web thread would free them.
  wxd->webx_requ = NULL;
  wxd->webx_resp = NULL;
  pthread_cond_destroy (&wxd->webx_cond);
}



#define WEB_ANSWER_DELAY_MOM ((MOM_IS_DEBUGGING(web))?16.0:4.0)
static onion_connection_status
handle_web_exchange_mom (void *ignore __attribute__ ((unused)),
			 onion_request * requ, onion_response * resp)
{
  double webtim = mom_clock_time (CLOCK_REALTIME);
  double endtim = webtim + WEB_ANSWER_DELAY_MOM;
  momitem_t *methoditm = NULL;
  const char *method = NULL;
  int webnum = 0;
  bool reqishead = false;
  char namidpath[104];		/* should be at least 100 characters,
				   see below */
#define NAMID_FMT_MOM "%100[a-zA-Z0-9_]"
  /*namid fmt should be  compatible with above namidpath */
  memset (namidpath, 0, sizeof (namidpath));
  {
    char thrname[24];
    memset (thrname, 0, sizeof (thrname));
    pthread_mutex_lock (&onion_mtx_mom);
    onion_nb_web_requests_mom++;
    webnum = onion_nb_web_requests_mom;
    pthread_mutex_unlock (&onion_mtx_mom);
    snprintf (thrname, sizeof (thrname), "mom-web%05d", webnum);
    pthread_setname_np (pthread_self (), thrname);
  }
  const char *fullpath = onion_request_get_fullpath (requ);
  const char *path = onion_request_get_path (requ);
  unsigned flags = onion_request_get_flags (requ);
  MOM_DEBUGPRINTF (web,
		   "process_request tid %ld webnum#%d=%#x webtim=%.3f endtim=%.3f fullpath=%s, path %s, flags %#x",
		   (long) mom_gettid (), webnum, webnum, webtim, endtim,
		   fullpath, path, flags);
  /// hack to deliver local files in MOM_WEB_DIRECTORY and the root
  /// document as MOM_WEB_ROOT_PAGE
  {
    char bufpath[MOM_PATH_MAX];
    struct stat stpath = { 0 };
    memset (bufpath, 0, sizeof (bufpath));
    memset (&stpath, 0, sizeof (stpath));
    if (!fullpath[0] || !strcmp (fullpath, "/"))
      {
	MOM_DEBUGPRINTF (web, "servicing the MOM_WEB_ROOT_PAGE %s",
			 MOM_WEB_ROOT_PAGE);
	snprintf (bufpath, sizeof (bufpath), "%s/%s", MOM_WEB_DIRECTORY,
		  MOM_WEB_ROOT_PAGE);
	MOM_DEBUGPRINTF (web, "MOM web root page: %s", bufpath);
	if (access (bufpath, R_OK))
	  MOM_FATAPRINTF ("cannot open web root page %s", bufpath);
	return onion_shortcut_response_file (bufpath, requ, resp);
      }
    if (fullpath && fullpath[0] == '/' && isalpha (fullpath[1])
	&& !strstr (fullpath, "..")
	&& strlen (fullpath) + sizeof (MOM_WEB_DIRECTORY) + 3
	< sizeof (bufpath))
      {
	strcpy (bufpath, MOM_WEB_DIRECTORY);
	strcat (bufpath, fullpath);
	assert (strlen (bufpath) < sizeof (bufpath) - 1);
	MOM_DEBUGPRINTF (web, "testing for access of bufpath=%s", bufpath);
	if (!access (bufpath, R_OK) && !stat (bufpath, &stpath)
	    && S_ISREG (stpath.st_mode))
	  {
	    MOM_DEBUGPRINTF (web, "shortcutting readable file bufpath=%s",
			     bufpath);
	    return onion_shortcut_response_file (bufpath, requ, resp);
	  }
      }
    MOM_DEBUGPRINTF (web,
		     "fullpath %s not static file, path %s, flags %#x",
		     fullpath, path, flags);
    switch (flags & OR_METHODS)
      {
      case OR_GET:
	method = "GET";
	methoditm = (momitem_t *) mom_named__GET;
	break;
      case OR_POST:
	method = "POST";
	methoditm = (momitem_t *) mom_named__POST;
	break;
      case OR_HEAD:
	method = "HEAD";
	methoditm = (momitem_t *) mom_named__HEAD;
	reqishead = true;
	break;
      default:
	{
	  char buf[400];
	  char timbuf[64];
	  char lenbuf[8];
	  memset (buf, 0, sizeof (buf));
	  memset (timbuf, 0, sizeof (timbuf));
	  memset (lenbuf, 0, sizeof (lenbuf));
	  MOM_WARNPRINTF
	    ("web request#%d fullpath %s method#%d not understood", webnum,
	     fullpath, (flags & OR_METHODS));
	  mom_strftime_centi (timbuf, sizeof (timbuf),
			      "%Y-%b-%d %H:%M:%S.__ %Z", webtim);
	  snprintf (buf, sizeof (buf),
		    "<html><head><title>Monimelt bad method</title></head>\n"
		    "<body><h1>Monimelt bad method</h1>\n"
		    "<p>request #%d fullpath <tt>%s</tt> at <i>%s</i> method#%d not understood.</p>"
		    "</body></html>\n", webnum, fullpath, timbuf,
		    (flags & OR_METHODS));
	  snprintf (lenbuf, sizeof (lenbuf), "%d", (int) strlen (buf));
	  return onion_shortcut_response_extra_headers
	    (buf,
	     HTTP_METHOD_NOT_ALLOWED, requ, resp,
	     "Content-Length", lenbuf,
	     "Content-Type", "text/html; charset=utf-8", NULL);
	}
      }
  }
  if (!methoditm)
    return OCS_NOT_PROCESSED;
  int pos = 0;
  momitem_t *namiditm = NULL;
  if ((sscanf (fullpath, "/" NAMID_FMT_MOM "%n", namidpath, &pos) > 0
       || sscanf (fullpath, NAMID_FMT_MOM "%n", namidpath, &pos) > 0)
      && pos > 0 && (isalpha (namidpath[0]) ||
		     (namidpath[0] == '_' && isdigit (namidpath[1])))
      && (fullpath[pos] == (char) 0 || fullpath[pos] == '/')
      && (namiditm = mom_get_item_of_name_or_ident_cstr (namidpath)) != NULL)
    {
      momval_t wclosv = MOM_NULLV;
      const char *restpath = fullpath + pos;
      MOM_DEBUGPRINTF (web,
		       "request #%d=%#x fullpath %s method %s good namidpath %s pos %d",
		       webnum, webnum, fullpath, method, namidpath, pos);
      assert (namiditm && namiditm->i_typnum == momty_item
	      && namiditm->i_magic == MOM_ITEM_MAGIC);
      {
	mom_lock_item (namiditm);
	wclosv =
	  mom_get_attribute (namiditm->i_attrs, mom_named__web_handler);
	mom_unlock_item (namiditm);
      }
      MOM_DEBUG (web, "request #", MOMOUT_DEC_INT (webnum),
		 MOMOUT_LITERAL ("=H"), MOMOUT_HEX_INT (webnum),
		 MOMOUT_LITERAL (" namiditm:"),
		 MOMOUT_ITEM ((const momitem_t *) namiditm),
		 MOMOUT_LITERAL (" methoditm:"),
		 MOMOUT_ITEM ((const momitem_t *) methoditm),
		 MOMOUT_LITERAL (" restpath:"),
		 MOMOUT_LITERALV (restpath),
		 MOMOUT_LITERAL (" wclosv:"),
		 MOMOUT_VALUE ((momval_t) wclosv));
      if (mom_is_node (wclosv))
	{
	  momitem_t *webxitm = mom_make_item ();
	  /// this is the only place where we are starting a webexchange item
	  struct mom_webexchange_data_st *wxd	////
	    = MOM_GC_ALLOC ("webexchange",
			    sizeof (struct mom_webexchange_data_st));
	  wxd->webx_magic = MOM_WEBX_MAGIC;
	  wxd->webx_num = webnum;
	  wxd->webx_time = webtim;
	  wxd->webx_requ = requ;
	  wxd->webx_resp = resp;
	  pthread_cond_init (&wxd->webx_cond, NULL);
	  // set the query arguments
	  {
	    const onion_dict *odicquery = onion_request_get_query_dict (requ);
	    int cntdicquery = onion_dict_count (odicquery);
	    if (cntdicquery > 0)
	      {
		struct webdict_mom_st *pdic =	////
		  MOM_GC_ALLOC ("new query dictionnary",
				sizeof (struct webdict_mom_st)
				+
				cntdicquery *
				sizeof (struct mom_jsonentry_st));
		pdic->webd_size = cntdicquery;
		onion_dict_preorder (odicquery, webdict_add_mom, pdic);
		wxd->webx_jobquery = (momval_t) mom_make_json_object
		  (MOMJSOB_COUNTED_ENTRIES
		   (pdic->webd_count,
		    (struct mom_jsonentry_st *) pdic->webd_pairtab),
		   MOMJSON_END);
		MOM_GC_FREE (pdic);
	      }
	  }
	  /// set the post arguments
	  if (methoditm == (momitem_t *) mom_named__POST)
	    {
	      const onion_dict *odicpost = onion_request_get_post_dict (requ);
	      int cntdicpost = onion_dict_count (odicpost);
	      if (cntdicpost > 0)
		{
		  struct webdict_mom_st *pdic =	////
		    MOM_GC_ALLOC ("new post dictionnary",
				  sizeof (struct webdict_mom_st)
				  +
				  cntdicpost *
				  sizeof (struct mom_jsonentry_st));
		  pdic->webd_size = cntdicpost;
		  onion_dict_preorder (odicpost, webdict_add_mom, pdic);
		  wxd->webx_jobpost = (momval_t) mom_make_json_object
		    (MOMJSOB_COUNTED_ENTRIES
		     (pdic->webd_count,
		      (struct mom_jsonentry_st *) pdic->webd_pairtab),
		     MOMJSON_END);
		  MOM_GC_FREE (pdic);
		}
	    }
	  /// open a memory stream for reply content
	  FILE *webf = open_memstream (&wxd->webx_obuf, &wxd->webx_osize);
	  if (!webf)
	    MOM_FATAPRINTF ("failed to open memstream for webreq#%d", webnum);
	  mom_initialize_output (&wxd->webx_out, webf, 0);
	  // we don't need to lock the webxitm, nobody know it yet!
	  webxitm->i_payload = wxd;
	  webxitm->i_paylkind = mompayk_webexchange;
	  MOM_DEBUG (web, MOMOUT_LITERAL ("webrequest #"),
		     MOMOUT_DEC_INT (webnum), MOMOUT_LITERAL (" webxitm:"),
		     MOMOUT_ITEM ((const momitem_t *) webxitm));
	  momitem_t *wtskitm = mom_make_item ();
	  // dont need to lock the tasklet item, nobody knows it!
	  mom_item_start_tasklet (wtskitm);
	  mom_item_tasklet_push_frame	/////
	    (wtskitm, (momval_t) wclosv,
	     MOMPFR_FOUR_VALUES ((momval_t) webxitm, (momval_t) methoditm,
				 (momval_t) namiditm,
				 (momval_t) mom_make_string (restpath)),
	     MOMPFR_END ());
	  mom_add_tasklet_to_agenda_front (wtskitm);
	  MOM_DEBUG (web, MOMOUT_LITERAL ("webrequest #"),
		     MOMOUT_DEC_INT (webnum),
		     MOMOUT_LITERAL ("; added tasklet wtskitm:"),
		     MOMOUT_ITEM ((const momitem_t *) wtskitm));
	  double minidelay = MOM_IS_DEBUGGING (web) ? 0.25 : 0.002;
	  double curtim = 0.0;
	  bool replied = false;
	  do
	    {
	      usleep (100);
	      curtim = mom_clock_time (CLOCK_REALTIME);
	      mom_lock_item (webxitm);
	      struct timespec ts = mom_timespec (curtim + minidelay);
	      pthread_cond_timedwait (&wxd->webx_cond, &webxitm->i_mtx, &ts);
	      if (webxitm->i_paylkind == mompayk_webexchange)
		{
		  wxd = webxitm->i_payload;
		  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
		}
	      else
		wxd = NULL;
	      if (wxd && wxd->webx_mime != NULL && wxd->webx_httpcode > 0)
		{
		  MOM_DEBUG (web,
			     MOMOUT_LITERAL ("got reply for webrequest #"),
			     MOMOUT_DEC_INT (webnum),
			     MOMOUT_LITERAL (" webxitm:"),
			     MOMOUT_ITEM ((const momitem_t *) webxitm),
			     MOMOUT_LITERAL (" httpcode:"),
			     MOMOUT_DEC_INT (wxd->webx_httpcode));
		  endtim = curtim - 0.001;
		  if (wxd->webx_resp)
		    {
		      bool istext = wxd->webx_mime
			&& (!strncmp (wxd->webx_mime, "text/", 5)
			    || !strcmp (wxd->webx_mime, "application/json"));
		      MOM_DEBUG (web,
				 MOMOUT_LITERAL ("replying to webrequest #"),
				 MOMOUT_DEC_INT (webnum),
				 MOMOUT_LITERAL (" webxitm:"),
				 MOMOUT_ITEM ((const momitem_t *) webxitm),
				 MOMOUT_LITERAL (" fullpath:"),
				 MOMOUT_LITERALV ((const char *) fullpath),
				 MOMOUT_LITERAL (" method:"),
				 MOMOUT_LITERALV ((const char *) method),
				 MOMOUT_LITERAL (" httpcode:"),
				 MOMOUT_LITERALV
				 (onion_response_code_description
				  (wxd->webx_httpcode)),
				 MOMOUT_LITERAL (" mime:"),
				 MOMOUT_LITERALV ((const char
						   *) (wxd->webx_mime)),
				 MOMOUT_LITERAL (" size:"),
				 MOMOUT_DEC_INT ((int) wxd->webx_osize),
				 MOMOUT_LITERALV ((const char *) (istext ?
								  " text:" :
								  " binary")),
				 MOMOUT_NEWLINE (),
				 MOMOUT_LITERALV ((const char *) (istext
								  ?
								  (wxd->webx_obuf)
								  : NULL)),
				 MOMOUT_NEWLINE ());
		      if (wxd->webx_osize > 0)
			onion_response_set_length (wxd->webx_resp,
						   wxd->webx_osize);
		      if (wxd->webx_mime)
			onion_response_set_header (wxd->webx_resp,
						   "Content-Type",
						   wxd->webx_mime);
		      onion_response_set_code (wxd->webx_resp,
					       wxd->webx_httpcode);
		      if (wxd->webx_obuf && wxd->webx_osize > 0 && !reqishead)
			{
			  onion_response_write (wxd->webx_resp,
						wxd->webx_obuf,
						wxd->webx_osize);
			}
		      onion_response_flush (wxd->webx_resp);
		      wxd->webx_resp = NULL;
		      wxd->webx_requ = NULL;
		      replied = true;
		      mom_item_clear_payload (webxitm);
		    }
		  else
		    MOM_DEBUGPRINTF (web, "webrequest#%d nilwebxresp",
				     webnum);
		}
	      else
		{		// timedout 
		  if (minidelay < (MOM_IS_DEBUGGING (web) ? 1.23 : 0.5))
		    minidelay = 1.5 * minidelay + 0.001;
		  MOM_DEBUGPRINTF (web,
				   "waiting again for webrequest #%d minidelay=%.4f, wxd@%p",
				   webnum, minidelay, wxd);
		}
	      mom_unlock_item (webxitm);
	    }
	  while (curtim < endtim && !replied);
	  if (!replied)
	    {			//// timeout
	      mom_lock_item (webxitm);
	      MOM_DEBUG (web, "timedout webrequest #",
			 MOMOUT_DEC_INT (webnum),
			 MOMOUT_LITERAL (" webxitm:"),
			 MOMOUT_ITEM ((const momitem_t *) webxitm),
			 MOMOUT_LITERAL (" fullpath:"),
			 MOMOUT_LITERALV ((const char *) fullpath),
			 MOMOUT_LITERAL (" method:"),
			 MOMOUT_LITERALV ((const char *) method));
	      onion_response_free (resp);
	      wxd->webx_resp = resp = NULL;
	      wxd->webx_requ = NULL;
	      onion_response *timoutresp = onion_response_new (requ);
	      onion_response_set_header (timoutresp, "Content-Type",
					 "text/html; charset=utf-8");
	      onion_response_printf (timoutresp,
				     "<html><head><title>Monimelt timeout</title></head>\n"
				     "<body><h1>Monimelt timeout webnum#%ld</h1>\n"
				     "<p>For %s of <tt>",
				     (long) wxd->webx_num, method);
	      onion_response_write_html_safe (timoutresp, fullpath);
	      onion_response_write0 (timoutresp, "</body></html>\n");
	      onion_response_set_code (timoutresp, HTTP_SERVICE_UNAVALIABLE);
	      onion_response_flush (timoutresp);
	      mom_item_clear_payload (webxitm);
	      mom_unlock_item (webxitm);
	      return OCS_PROCESSED;
	    }
	  else
	    {
	      MOM_DEBUGPRINTF (web, "done webreq#%d", webnum);
	      return OCS_PROCESSED;
	    }
	}			/* end if wclosv is node */
    }
  MOM_DEBUGPRINTF (web, "not processed webreq#%d=%#x fullpath=%s method=%s",
		   webnum, webnum, fullpath, method);
  return OCS_NOT_PROCESSED;
}


void
mom_webx_out_at (const char *sfil, int lin, momitem_t *webitm, ...)
{
  va_list alist;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL ("bad webxout "), MOMOUT_LITERALV (sfil),
		   MOMOUT_LITERAL (":"),
		   MOMOUT_DEC_INT (lin),
		   MOMOUT_LITERAL ("; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm));
      goto end;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  va_start (alist, webitm);
  if (wxd->webx_out.mout_file)
    mom_outva_at (sfil, lin, &wxd->webx_out, alist);
  va_end (alist);
end:;
}


void
mom_webx_reply (momitem_t *webitm, const char *mime, int httpcode)
{
  MOM_DEBUG (web, MOMOUT_LITERAL ("webx_reply webitm="),
	     MOMOUT_ITEM ((const momitem_t *) webitm),
	     MOMOUT_LITERAL ("; mime="),
	     MOMOUT_LITERALV (mime),
	     MOMOUT_LITERAL (" httpcode#"), MOMOUT_DEC_INT (httpcode));
  if (!webitm || webitm->i_typnum != momty_item)
    return;

  if (!mime)
    mime = "text/plain; charset=utf-8";
#define COMMON_MIME_PREFIX_UTF8(Pref) else if (!strncmp (mime, Pref, sizeof(Pref)-1)) \
    mime = Pref "; charset=utf-8"
#define COMMON_MIME_TYPE(Mime) else if (!strcmp (mime, Mime))	\
    mime = Mime
  /* *INDENT-OFF* */
  // our common mime types, by probable decreasing frequencies
  COMMON_MIME_PREFIX_UTF8 ("text/html");
  COMMON_MIME_PREFIX_UTF8 ("text/plain");
  COMMON_MIME_TYPE ("application/json");
  COMMON_MIME_TYPE ("application/javascript");
  COMMON_MIME_TYPE ("application/xhtml+xml");
  COMMON_MIME_PREFIX_UTF8 ("text/x-csrc");
  COMMON_MIME_PREFIX_UTF8 ("text/x-chdr");
  COMMON_MIME_PREFIX_UTF8 ("text/x-c++src");
  COMMON_MIME_PREFIX_UTF8 ("text/x-c++hdr");
  COMMON_MIME_TYPE ("image/jpeg");
  COMMON_MIME_TYPE ("image/png");
  COMMON_MIME_TYPE ("image/gif");
  COMMON_MIME_TYPE ("image/svg+xml");
  COMMON_MIME_PREFIX_UTF8 ("application/xml");
  else mime = MOM_GC_STRDUP("strange mime", mime);
  /* *INDENT-ON* */
#undef COMMON_MIME_PREFIX_UTF8
#undef COMMON_MIME_TYPE
  ///
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL ("bad webxreply; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      goto end;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  assert (wxd->webx_out.mout_magic == MOM_MOUT_MAGIC);
  if (wxd->webx_out.mout_file)
    fflush (wxd->webx_out.mout_file);
  else
    MOM_WARNPRINTF ("webnum#%d no momoutfile", wxd->webx_num);
  bool istext = (!strncmp (mime, "text/", 5)
		 || !strcmp (mime, "application/json"));
  MOM_DEBUG (web, MOMOUT_LITERAL ("webreply webnum#"),
	     MOMOUT_DEC_INT ((int) wxd->webx_num), MOMOUT_LITERAL (" mime:"),
	     MOMOUT_LITERALV ((const char *) mime),
	     MOMOUT_LITERAL (" httpcode:"), MOMOUT_DEC_INT (httpcode),
	     MOMOUT_LITERAL ("="),
	     MOMOUT_LITERALV ((const char *)
			      onion_response_code_description (httpcode)),
	     MOMOUT_LITERAL ("; osize="),
	     MOMOUT_DEC_INT ((int) wxd->webx_osize),
	     MOMOUT_LITERALV ((const char *) (istext ? "; text.obuf=" :
					      "; bin.obuf")),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERALV ((const char *) (istext ? wxd->webx_obuf :
					      NULL)),
	     istext ? MOMOUT_NEWLINE () : MOMOUT_FLUSH (), NULL);
  if (!wxd->webx_resp)
    {
      MOM_WARNING (MOMOUT_LITERAL ("bad webxreply; already replied webnum#"),
		   MOMOUT_DEC_INT ((int) wxd->webx_num),
		   MOMOUT_LITERAL (" webitm:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5), NULL);
      goto end;
    };
  wxd->webx_mime = (char *) mime;
  wxd->webx_httpcode = httpcode;
  pthread_cond_broadcast (&wxd->webx_cond);
  usleep (150);
  MOM_DEBUGPRINTF (web, "webxreply ending webnum#%d mime=%s httpcode=%d",
		   wxd->webx_num, mime, httpcode);
end:
  return;
}



momval_t
mom_webx_jsob_post (momitem_t *webitm)
{
  if (!webitm || webitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL
		   ("bad webxjosbpost; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      return MOM_NULLV;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  return wxd->webx_jobpost;
}


momval_t
mom_webx_post_arg (momitem_t *webitm, const char *argname)
{
  if (!webitm || webitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (!argname || !argname[0])
    return MOM_NULLV;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL
		   ("bad webxpostarg; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      return MOM_NULLV;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  return mom_jsonob_getstr (wxd->webx_jobpost, argname);
}



momval_t
mom_webx_jsob_query (momitem_t *webitm)
{
  if (!webitm || webitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL
		   ("bad webxjsobquery; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      return MOM_NULLV;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  return wxd->webx_jobquery;
}



momval_t
mom_webx_query_arg (momitem_t *webitm, const char *argname)
{
  if (!webitm || webitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (!argname || !argname[0])
    return MOM_NULLV;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL
		   ("bad webxqueryarg; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      return MOM_NULLV;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  return mom_jsonob_getstr (wxd->webx_jobquery, argname);
}


momval_t
mom_webx_fullpath (momitem_t *webitm)
{
  if (!webitm || webitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL
		   ("bad webxfullpath; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      return MOM_NULLV;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  if (wxd->webx_requ)
    return (momval_t)
      mom_make_string (onion_request_get_fullpath (wxd->webx_requ));
  return MOM_NULLV;
}


momval_t
mom_webx_method (momitem_t *webitm)
{
  if (!webitm || webitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (webitm->i_paylkind != mompayk_webexchange)
    {
      MOM_WARNING (MOMOUT_LITERAL
		   ("bad webxfullpath; non webexchange webitem:"),
		   MOMOUT_ITEM ((const momitem_t *) webitm),
		   MOMOUT_LITERAL (" ...from "), MOMOUT_BACKTRACE (5));
      return MOM_NULLV;
    }
  struct mom_webexchange_data_st *wxd = webitm->i_payload;
  assert (wxd && wxd->webx_magic == MOM_WEBX_MAGIC);
  if (wxd->webx_requ)
    {
      unsigned flags = onion_request_get_flags (wxd->webx_requ);
      switch (flags & OR_METHODS)
	{
	case OR_GET:
	  return (momval_t) mom_named__GET;
	case OR_POST:
	  return (momval_t) mom_named__POST;
	case OR_HEAD:
	  return (momval_t) mom_named__HEAD;
	default:
	  break;
	}
    }
  return MOM_NULLV;
}



// called at most once from main
void
mom_start_web (const char *webhost)
{
  char webuf[128];
  memset (webuf, 0, sizeof (webuf));
  assert (webhost != NULL);
  if (strlen (webhost) + 2 >= sizeof (webuf))
    MOM_FATAPRINTF ("too long webhost %s", webhost);
  strncpy (webuf, webhost, sizeof (webuf) - 1);
  onion_mom = onion_new (O_THREADED | O_DETACH_LISTEN);
  if (!onion_mom)
    MOM_FATAPRINTF ("failed to create onion");
  char *lastcolon = strchr (webuf, ':');
  char *portstr = NULL;
  char *hoststr = NULL;
  if (lastcolon && isdigit (lastcolon[1]))
    {
      *lastcolon = (char) 0;
      portstr = lastcolon + 1;
    }
  if (webuf[0])
    hoststr = webuf;
  if (hoststr && hoststr[0])
    onion_set_hostname (onion_mom, hoststr);
  if (portstr && portstr[0])
    onion_set_port (onion_mom, portstr);
  MOM_INFORMPRINTF ("start web hoststr=%s portstr=%s", hoststr, portstr);
  onion_root_url_mom = onion_root_url (onion_mom);
  onion_handler *hdlr = onion_handler_new ((onion_handler_handler)
					   handle_web_exchange_mom, NULL,
					   NULL);
  onion_url_add_handler (onion_root_url_mom, "^", hdlr);
  onion_url_add_handler (onion_root_url_mom, "status",
			 onion_internal_status ());
  onion_url_add_handler (onion_root_url_mom, "^",
			 onion_handler_export_local_new (MOM_WEB_DIRECTORY));
  MOM_INFORM ("before listening web host %s", webhost);
  onion_listen (onion_mom);
  MOM_INFORM ("after listening web host %s", webhost);
}				/* end mom_start_web */
