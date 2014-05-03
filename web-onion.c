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


onion *mom_onion = NULL;
onion_url *mom_onion_root = NULL;
static pthread_mutex_t mtx_onion = PTHREAD_MUTEX_INITIALIZER;
static long nb_weq_requests;

extern momit_dictionnary_t *mom_item__web_dictionnary;

#define WEB_MAGIC 0x11b63c99	/* web magic 297155737 */
struct mom_web_info_st
{
  unsigned web_magic;
  onion_connection_status web_stat;
  unsigned long web_num;
  double web_time;
  onion_request *web_requ;
  onion_response *web_resp;
};




static
  void *mom_really_process_request (struct GC_stack_base *sb, void *data)
  __attribute__ ((noinline));


static onion_connection_status
process_request (void *ignore, onion_request * req, onion_response * res)
{
  long webnum = 0;
  double webtim = monimelt_clock_time (CLOCK_REALTIME);
  {
    char thnambuf[32];
    memset (thnambuf, 0, sizeof (thnambuf));
    pthread_mutex_lock (&mtx_onion);
    nb_weq_requests++;
    webnum = nb_weq_requests;
    snprintf (thnambuf, sizeof (thnambuf), "mom-web%05ld", webnum);
    pthread_setname_np (pthread_self (), thnambuf);
    pthread_mutex_unlock (&mtx_onion);
  }
  struct mom_web_info_st webinf;
  memset (&webinf, 0, sizeof (webinf));
  webinf.web_magic = WEB_MAGIC;
  webinf.web_stat = OCS_NOT_PROCESSED;
  webinf.web_num = webnum;
  webinf.web_time = webtim;
  webinf.web_requ = req;
  webinf.web_resp = res;
  MOMGC_CALL_WITH_STACK_BASE (mom_really_process_request, &webinf);
  MONIMELT_INFORM ("process request webnum#%ld return %d",
		   webnum, webinf.web_stat);
  return webinf.web_stat;
}


void
mom_start_web (const char *webhost)
{
  char webuf[128];
  memset (webuf, 0, sizeof (webuf));
  if (strlen (webhost) + 2 >= sizeof (webuf))
    MONIMELT_FATAL ("too long webhost %s", webhost);
  strncpy (webuf, webhost, sizeof (webuf) - 1);
  mom_onion = onion_new (O_THREADED | O_DETACH_LISTEN);
  char *lastcolon = strchr (webuf, ':');
  if (lastcolon && isdigit (lastcolon[1]))
    {
      *lastcolon = (char) 0;
      onion_set_port (mom_onion, lastcolon + 1);
    }
  if (webuf[0])
    onion_set_hostname (mom_onion, webuf);
  mom_onion_root = onion_root_url (mom_onion);
  onion_handler *hdlr = onion_handler_new ((onion_handler_handler)
					   process_request, NULL, NULL);
  onion_url_add_handler (mom_onion_root, "status", onion_internal_status ());
  onion_url_add_handler (mom_onion_root, "^" MONIMELT_WEB_DIRECTORY,
			 onion_handler_export_local_new
			 (MONIMELT_WEB_DIRECTORY));

  onion_url_add_handler (mom_onion_root, "^.*", hdlr);
  MONIMELT_INFORM ("before listening web host %s", webhost);
  onion_listen (mom_onion);
  MONIMELT_INFORM ("after listening web host %s", webhost);
}



struct post_dict_st
{
  unsigned post_len;
  unsigned post_count;
  struct mom_jsonentry_st post_pairtab[];
};

static void
dict_add (void *data, const char *key, const void *value, int flags)
{
  struct post_dict_st *pd = data;
  unsigned cnt = pd->post_count;
  assert (cnt < pd->post_len);
  pd->post_pairtab[cnt].je_name = (momval_t) mom_make_string (key);
  pd->post_pairtab[cnt].je_attr = (momval_t) mom_make_string (value);
  pd->post_count = cnt + 1;
}


#define WEBRUNNER_MAGIC 0x1158fcab	/* webrunner magic 291044523 */
struct webrunner_st
{
  unsigned webrunner_magic;
  const momclosure_t *webrunner_closhandler;
  momit_webrequest_t *webrunner_webitm;
  void *webrunner__gap;
};

static void
really_webrun (struct GC_stack_base *sb, void *data)
{
  MOMGC_REGISTER_MY_THREAD (sb);
  struct webrunner_st *wr = data;
  assert (wr && wr->webrunner_magic == WEBRUNNER_MAGIC);
  assert (wr->webrunner_webitm != NULL
	  && wr->webrunner_webitm->iweb_item.typnum == momty_webrequestitem);
  unsigned long webnum = wr->webrunner_webitm->iweb_webnum;
  MONIMELT_DEBUG (web, "really_webrun webnum#%ld before running closure",
		  webnum);
  (void) mom_run_closure ((momval_t) (const momclosure_t *)
			  wr->webrunner_closhandler, MOMPFR_VALUE,
			  (momval_t) wr->webrunner_webitm, MOMPFR_END);
  MONIMELT_DEBUG (web, "really_webrun webnum#%ld after running closure",
		  webnum);
  usleep (100);
  MOMGC_UNREGISTER_MY_THREAD ();
}

static void *
webrun_cb (void *data)
{
  char thrname[24];
  struct webrunner_st *wr = data;
  memset (thrname, 0, sizeof (thrname));
  assert (wr && wr->webrunner_magic == WEBRUNNER_MAGIC);
  assert (wr->webrunner_webitm != NULL
	  && wr->webrunner_webitm->iweb_item.typnum == momty_webrequestitem);
  unsigned long webnum = wr->webrunner_webitm->iweb_webnum;
  snprintf (thrname, sizeof (thrname), "mom-wru%05ld", webnum);
  pthread_setname_np (pthread_self (), thrname);
  MONIMELT_DEBUG (web, "webrun before running closhandler webnum#%ld",
		  webnum);
  MOMGC_CALL_WITH_STACK_BASE (really_webrun, wr);
  usleep (100);
  MONIMELT_DEBUG (web, "webrun after running closhandler webnum#%ld", webnum);
  return wr;
}

#define WEB_REPLY_TIMEOUT 2.4	/*seconds web reply timeout */
static void *
mom_really_process_request (struct GC_stack_base *sb, void *data)
{
  MOMGC_REGISTER_MY_THREAD (sb);
  struct mom_web_info_st *pwebinf = data;
  assert (pwebinf->web_magic == WEB_MAGIC);
  assert (mom_item__web_dictionnary != NULL);
  unsigned long webnum = pwebinf->web_num;
  onion_request *req = pwebinf->web_requ;
  onion_response *res = pwebinf->web_resp;
  double wtim = pwebinf->web_time;
  double wend = wtim + WEB_REPLY_TIMEOUT;
  const char *fullpath = onion_request_get_fullpath (req);
  const char *path = onion_request_get_path (req);
  unsigned flags = onion_request_get_flags (req);
  mom_anyitem_t *methoditm = NULL;
  const char *method = NULL;
  MONIMELT_DEBUG (web,
		  "really_process_request start webnum#%ld wtim=%f wend=%f",
		  webnum, wtim, wend);
  switch (flags & OR_METHODS)
    {
    case OR_GET:
      method = "GET";
      methoditm = (mom_anyitem_t *) mom_item__GET;
      break;
    case OR_POST:
      method = "POST";
      methoditm = (mom_anyitem_t *) mom_item__POST;
      break;
    case OR_PUT:
      method = "PUT";
      methoditm = (mom_anyitem_t *) mom_item__PUT;
      break;
    case OR_HEAD:
      method = "HEAD";
      methoditm = (mom_anyitem_t *) mom_item__HEAD;
      break;
    case OR_OPTIONS:
      method = "OPTIONS";
      methoditm = (mom_anyitem_t *) mom_item__OPTIONS;
      break;
    default:
      MONIMELT_WARNING ("unexpected req#%ld fullpath=%s flags %#x",
			webnum, fullpath, flags);
    }
  unsigned fullpathlen = fullpath ? strlen (fullpath) : 0;
  momval_t closhandler = MONIMELT_NULLV;
  momval_t jpost = MONIMELT_NULLV;
  momval_t jquery = MONIMELT_NULLV;
  MONIMELT_INFORM ("got web request #%ld fullpath=%s path=%s method=%s",
		   webnum, fullpath, path, method ? method : "??");
  if (fullpathlen > 2 && isalpha (fullpath[1])
      && (closhandler =
	  mom_item_dictionnary_get_cstr ((momval_t) mom_item__web_dictionnary,
					 fullpath + 1)).ptr != NULL
      && *closhandler.ptype == momty_closure)
    {
      mom_dbg_value (web, "closhandler", closhandler);
      momval_t pathv = (momval_t) mom_make_string (fullpath + 1);
      if (methoditm == (mom_anyitem_t *) mom_item__POST)
	{
	  const onion_dict *odicpost = onion_request_get_post_dict (req);
	  int cntdicpost = onion_dict_count (odicpost);
	  struct post_dict_st *pdic =
	    GC_MALLOC (sizeof (struct post_dict_st) +
		       cntdicpost * sizeof (struct mom_jsonentry_st));
	  if (MONIMELT_UNLIKELY (!pdic))
	    MONIMELT_FATAL
	      ("failed to allocate for %d pairs for POST request",
	       cntdicpost);
	  memset (pdic, 0,
		  sizeof (struct post_dict_st) +
		  cntdicpost * sizeof (struct mom_jsonentry_st));
	  pdic->post_len = cntdicpost;
	  onion_dict_preorder (odicpost, dict_add, pdic);
	  jpost = (momval_t) mom_make_json_object
	    (MOMJSON_COUNTED_ENTRIES, pdic->post_count, pdic->post_pairtab,
	     MOMJSON_END);
	  GC_FREE (pdic);
	}
      {
	const onion_dict *odicquery = onion_request_get_post_dict (req);
	int cntdicquery = onion_dict_count (odicquery);
	struct post_dict_st *pdic =
	  GC_MALLOC (sizeof (struct post_dict_st) +
		     cntdicquery * sizeof (struct mom_jsonentry_st));
	if (MONIMELT_UNLIKELY (!pdic))
	  MONIMELT_FATAL ("failed to allocate for %d pairs of query",
			  cntdicquery);
	memset (pdic, 0,
		sizeof (struct post_dict_st) +
		cntdicquery * sizeof (struct mom_jsonentry_st));
	pdic->post_len = cntdicquery;
	onion_dict_preorder (odicquery, dict_add, pdic);
	jquery = (momval_t) mom_make_json_object
	  (MOMJSON_COUNTED_ENTRIES, pdic->post_count, pdic->post_pairtab,
	   MOMJSON_END);
	GC_FREE (pdic);
      }
      momit_webrequest_t *webitm = mom_allocate_item (momty_webrequestitem,
						      sizeof
						      (momit_webrequest_t),
						      MONIMELT_SPACE_NONE);
      webitm->iweb_request = req;
      webitm->iweb_response = res;
      webitm->iweb_webnum = webnum;
      webitm->iweb_time = wtim;
      webitm->iweb_methoditm = methoditm;
      webitm->iweb_postjsob = jpost;
      webitm->iweb_queryjsob = jquery;
      webitm->iweb_path = pathv;
      pthread_cond_init (&webitm->iweb_cond, NULL);
      MONIMELT_DEBUG (web,
		      "really_process_request made webitm@%p webnum#%ld wtim=%f wend=%f",
		      webitm, webnum, wtim, wend);
      mom_dbg_item (web, "really_process_request webitm",
		    (const mom_anyitem_t *) webitm);
      mom_dbg_value (web, "really_process_request closhandler", closhandler);
      MONIMELT_DEBUG (web, "before running webrunthread");
      pthread_t webrunthread = 0;
      struct webrunner_st webrun = { };
      memset (&webrun, 0, sizeof (webrun));
      webrun.webrunner_closhandler = closhandler.pclosure;
      webrun.webrunner_webitm = webitm;
      webrun.webrunner_magic = WEBRUNNER_MAGIC;
      if (pthread_create (&webrunthread, NULL, webrun_cb, &webrun))
	MONIMELT_FATAL ("failed to create webrunner for webnum#%ld", webnum);
      usleep (1000);
      void *ret = NULL;
      MONIMELT_DEBUG (web, "created webrunthread=%#lx for webnum#%ld",
		      (long) webrunthread, webnum);
      sched_yield ();
      /* we should loop on lock the webitm's mutex and
         pthread_cond_timedwait webitm->iweb_cond, so we should define
         a protocol to reply to a request */
      bool gotreply = false;
      do
	{
	  int waiterr = 0;
	  gotreply = false;
	  pthread_mutex_lock (&webitm->iweb_item.i_mtx);
	  struct timespec endts = monimelt_timespec (wend);
	  waiterr =
	    pthread_cond_timedwait (&webitm->iweb_cond,
				    &webitm->iweb_item.i_mtx, &endts);
	  gotreply = webitm->iweb_response == NULL;
	  MONIMELT_DEBUG (web, "waiterr=%d (%s), gotreply=%d", waiterr,
			  strerror (waiterr), (int) gotreply);
	  if (!gotreply && monimelt_clock_time (CLOCK_REALTIME) > wend + 0.01)
	    {
	      onion_response_free (webitm->iweb_response);
	      webitm->iweb_response =
		onion_response_new (webitm->iweb_request);
	      onion_response_printf (webitm->iweb_response,
				     "<html><head><title>Monimelt timeout</title></head>\n"
				     "<body><h1>Monimelt timeout webnum#%ld</h1>\n"
				     "<p>For %s of <tt>",
				     (long) webitm->iweb_webnum, method);
	      onion_response_write_html_safe (webitm->iweb_response,
					      fullpath);
	      {
		char timebuf[80];
		struct tm timetm = { };
		time_t wt = (time_t) wtim;
		memset (timebuf, 0, sizeof (timebuf));
		localtime_r (&wt, &timetm);
		strftime (timebuf, sizeof (timebuf), "%Y-%b-%d %T %Z",
			  &timetm);
		onion_response_printf (webitm->iweb_response,
				       "</tt> at <i>%s</i></p>", timebuf);
	      }
	      onion_response_write0 (webitm->iweb_response,
				     "</body></html>\n");
	      onion_response_set_code (webitm->iweb_response,
				       HTTP_SERVICE_UNAVALIABLE);
	      onion_response_flush (webitm->iweb_response);
	      webitm->iweb_response = NULL;
	    }
	  pthread_mutex_unlock (&webitm->iweb_item.i_mtx);
	  sched_yield ();
	}
      while (!gotreply && monimelt_clock_time (CLOCK_REALTIME) <= wend);
      MONIMELT_DEBUG (web, "before joining webrunthread");
      if (pthread_join (webrunthread, &ret))
	MONIMELT_FATAL
	  ("failed to join successfully webrunner for webnum#%ld", webnum);
      if (ret != NULL && ret != &webrun)
	MONIMELT_FATAL
	  ("bad return @%p from webrunner for webnum#%ld", ret, webnum);
      MONIMELT_DEBUG (web, "joined webrunthread=%#lx", (long) webrunthread);
      memset (&webrun, 0, sizeof (webrun));
      sched_yield ();
      pwebinf->web_stat = OCS_PROCESSED;
    }
  MOMGC_UNREGISTER_MY_THREAD ();
  return NULL;

}

void
mom_item_webrequest_reply (momval_t vweb, const char *mimetype, int code)
{
  if (!vweb.ptr || *vweb.ptype != momty_webrequestitem || !mimetype)
    return;
  if (!code)
    code = HTTP_OK;
  momit_webrequest_t *webitm = vweb.pwebrequestitem;
  MONIMELT_DEBUG (web, "webrequest_reply mimetype=%s code=%d webnum#%ld",
		  mimetype, code, (long) webitm->iweb_webnum);
  mom_dbg_item (web, "webrequest_reply webitm",
		(const mom_anyitem_t *) webitm);
  bool webdone = false;
  pthread_mutex_lock (&vweb.panyitem->i_mtx);
  if (webitm->iweb_response)
    {
      onion_response_set_header (webitm->iweb_response, "Content-Type",
				 mimetype);
      if (code > 0)
	onion_response_set_code (webitm->iweb_response, code);
      onion_response_flush (webitm->iweb_response);
      webitm->iweb_response = NULL;
      webdone = true;
      MONIMELT_DEBUG (web,
		      "webrequest_reply clearing response webnum#%ld",
		      (long) webitm->iweb_webnum);
    }
  pthread_mutex_unlock (&vweb.panyitem->i_mtx);
  if (webdone)
    {
      MONIMELT_DEBUG (web, "webrequest_reply webnum#%ld webdone broadcast",
		      (long) webitm->iweb_webnum);
      int brerr = pthread_cond_broadcast (&webitm->iweb_cond);
      MONIMELT_DEBUG (web, "webrequest_reply webnum#%ld brerr#%d (%s)",
		      webitm->iweb_webnum, brerr, strerror (brerr));

    }
  MONIMELT_DEBUG (web, "webrequest_reply webnum#%ld end",
		  (long) webitm->iweb_webnum);
  sched_yield ();
}

extern void
mom_webrequest_destroy (mom_anyitem_t * itm)
{
  assert (itm && itm->typnum == momty_webrequestitem);
  momit_webrequest_t *webitm = (momit_webrequest_t *) itm;
  assert (!webitm->iweb_request);
  assert (!webitm->iweb_response);
  pthread_cond_destroy (&webitm->iweb_cond);
  memset (&webitm->iweb_cond, 0, sizeof (pthread_cond_t));
}
