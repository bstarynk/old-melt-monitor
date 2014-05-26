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
  if (wxd->webx_requ)
    onion_request_free (wxd->webx_requ), wxd->webx_requ = NULL;
  if (wxd->webx_resp)
    onion_response_free (wxd->webx_resp), wxd->webx_resp = NULL;
}



#define WEB_ANSWER_DELAY_MOM ((MOM_IS_DEBUGGING(web))?7.0:4.0)
static onion_connection_status
process_web_exchange_mom (void *ignore __attribute__ ((unused)),
			  onion_request * req, onion_response * resp)
{
  double webtim = mom_clock_time (CLOCK_REALTIME);
  double endtim = webtim + WEB_ANSWER_DELAY_MOM;
  momitem_t *methoditm = NULL;
  const char *method = NULL;
  int webnum = 0;
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
    snprintf (thrname, sizeof (thrname), "mom-web%05x", webnum);
    pthread_setname_np (pthread_self (), thrname);
  }
  const char *fullpath = onion_request_get_fullpath (req);
  const char *path = onion_request_get_path (req);
  unsigned flags = onion_request_get_flags (req);
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
	  MOM_FATAL ("cannot open web root page %s", bufpath);
	return onion_shortcut_response_file (bufpath, req, resp);
      }
    if (fullpath && fullpath[0] == '/' && isalpha (fullpath[1])
	&& !strstr (fullpath, "..")
	&& strlen (fullpath) + sizeof (MOM_WEB_DIRECTORY) + 3
	< sizeof (bufpath))
      {
	strcpy (bufpath, MOM_WEB_DIRECTORY);
	strcat (bufpath, fullpath);
	assert (strlen (bufpath) < sizeof (bufpath) - 1);
	MOM_DEBUG (web, "testing for access of bufpath=%s", bufpath);
	if (!access (bufpath, R_OK) && !stat (bufpath, &stpath)
	    && S_ISREG (stpath.st_mode))
	  {
	    MOM_DEBUGPRINTF (web, "shortcutting readable file bufpath=%s",
			     bufpath);
	    return onion_shortcut_response_file (bufpath, req, resp);
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
	     HTTP_METHOD_NOT_ALLOWED, req, resp,
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
		       "request #%d fullpath %s method %s good namidpath %s pos %d",
		       webnum, fullpath, method, namidpath, pos);
      assert (namiditm && namiditm->i_typnum == momty_item
	      && namiditm->i_magic == MOM_ITEM_MAGIC);
      {
	mom_lock_item (namiditm);
	wclosv =
	  mom_get_attribute (namiditm->i_attrs, mom_named__web_handler);
	mom_unlock_item (namiditm);
      }
      MOM_DEBUG (web, "request #", MOMOUT_DEC_INT (webnum),
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
	  struct mom_webexchange_data_st *wxd = MOM_GC_ALLOC ("webexchange",
							      sizeof (struct
								      mom_webexchange_data_st));
	  wxd->webx_magic = MOM_WEBX_MAGIC;
	  wxd->webx_num = webnum;
	  wxd->webx_time = webtim;
	  wxd->webx_requ = req;
	  wxd->webx_resp = resp;
	  FILE *webf = open_memstream (&wxd->webx_obuf, &wxd->webx_osize);
	  if (!webf)
	    MOM_FATAPRINTF ("failed to open memstream for webreq#%d", webnum);
	  mom_initialize_output (&wxd->webx_out, webf, 0);
	  // we don't need to lock the webxitm, nobody know it yet!
	  webxitm->i_payload = wxd;
	  webxitm->i_paylkind = mompayk_webexchange;
	  MOM_DEBUG (web, "webrequest #", MOMOUT_DEC_INT (webnum),
		     MOMOUT_LITERAL (" webxitm:"),
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
	}
    }
}
