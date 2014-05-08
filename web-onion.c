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




static void *mom_really_process_request (struct GC_stack_base *sb,
					 void *data);


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
  const char *fullpath = onion_request_get_fullpath (req);
  MONIMELT_DEBUG (web, "process_request webnum#%ld webtim=%.3f fullpath=%s",
		  webnum, webtim, fullpath);
  /// hack to deliver local files in MONIMELT_WEB_DIRECTORY
  {
    char bufpath[128];
    struct stat stpath = { };
    memset (bufpath, 0, sizeof (bufpath));
    memset (&stpath, 0, sizeof (stpath));
    if (fullpath && fullpath[0] == '/' && isalpha (fullpath[1])
	&& !strstr (fullpath, "..")
	&& strlen (fullpath) + sizeof (MONIMELT_WEB_DIRECTORY) + 3
	< sizeof (bufpath))
      {
	strcpy (bufpath, MONIMELT_WEB_DIRECTORY);
	strcat (bufpath, fullpath);
	assert (strlen (bufpath) < sizeof (bufpath) - 1);
	MONIMELT_DEBUG (web, "testing for access of bufpath=%s", bufpath);
	if (!access (bufpath, R_OK) && !stat (bufpath, &stpath)
	    && S_ISREG (stpath.st_mode))
	  {
	    MONIMELT_DEBUG (web, "shortcutting readable file bufpath=%s",
			    bufpath);
	    return onion_shortcut_response_file (bufpath, req, res);
	  }
      }
    MONIMELT_DEBUG (web, "fullpath %s not static file", fullpath);
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
  char *portstr = NULL;
  char *hoststr = NULL;
  if (lastcolon && isdigit (lastcolon[1]))
    {
      *lastcolon = (char) 0;
      portstr = lastcolon + 1;
    }
  if (webuf[0])
    hoststr = webuf;
  if (hoststr)
    onion_set_hostname (mom_onion, hoststr);
  if (portstr)
    onion_set_port (mom_onion, portstr);
  MONIMELT_INFORM ("start web hoststr=%s portstr=%s", hoststr, portstr);
  mom_onion_root = onion_root_url (mom_onion);
  onion_handler *hdlr = onion_handler_new ((onion_handler_handler)
					   process_request, NULL, NULL);
  onion_url_add_handler (mom_onion_root, "^", hdlr);
  onion_url_add_handler (mom_onion_root, "status", onion_internal_status ());
  onion_url_add_handler (mom_onion_root, "^",
			 onion_handler_export_local_new
			 (MONIMELT_WEB_DIRECTORY));

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




#define WEB_INITIAL_REPLY_SIZE 256
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
      MOM_DBG_VALUE (web, "closhandler", closhandler);
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
	};
      {
	const onion_dict *odicquery = onion_request_get_query_dict (req);
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
      {
	char *wbuf = GC_MALLOC_ATOMIC (WEB_INITIAL_REPLY_SIZE);
	if (MONIMELT_UNLIKELY (!wbuf))
	  MONIMELT_FATAL ("cannot allocate web buffer of %d",
			  WEB_INITIAL_REPLY_SIZE);
	memset (wbuf, 0, WEB_INITIAL_REPLY_SIZE);
	webitm->iweb_replybuf = wbuf;
	webitm->iweb_replysize = WEB_INITIAL_REPLY_SIZE;
	webitm->iweb_replylength = 0;
      }
      pthread_cond_init (&webitm->iweb_cond, NULL);
      MONIMELT_DEBUG (web,
		      "really_process_request made webitm@%p webnum#%ld wtim=%f wend=%f",
		      webitm, webnum, wtim, wend);
      MOM_DBG_ITEM (web, "really_process_request webitm",
		    (const mom_anyitem_t *) webitm);
      MOM_DBG_VALUE (web, "really_process_request closhandler", closhandler);
      {
	momit_tasklet_t *webtasklet =
	  mom_make_item_tasklet (MONIMELT_SPACE_NONE);
	mom_tasklet_push_frame ((momval_t) webtasklet, (momval_t) closhandler,
				MOMPFR_VALUE, webitm, MOMPFR_END);
	MOM_DBG_ITEM (web, "new webtasklet=",
		      (const mom_anyitem_t *) webtasklet);
	mom_agenda_add_tasklet_front ((momval_t) webtasklet);
      }
      sched_yield ();
      // we sleep for 25 milliseconds, to avoid replying too quickly
      // to the browser; this might slow down some attacks...
      usleep (25000);
      /* we should loop on lock the webitm's mutex and
         pthread_cond_timedwait webitm->iweb_cond, so we should define
         a protocol to reply to a request */
      bool gotreply = false;
      bool repeatloop = true;
#define INITIAL_WAIT_DELAY 0.001
      double waitdelay = INITIAL_WAIT_DELAY;
      while (repeatloop)
	{
	  int waiterr = 0;
	  double tnow = monimelt_clock_time (CLOCK_REALTIME);
	  gotreply = false;
	  MONIMELT_DEBUG (web,
			  "before waiting for webreply webnum#%ld wend=%.4f tnow=%.4f",
			  (long) webitm->iweb_webnum, wend, tnow);
	  pthread_mutex_lock (&webitm->iweb_item.i_mtx);
	  double twait = tnow + waitdelay;
	  struct timespec waitts = monimelt_timespec (twait);
	  MONIMELT_DEBUG (web,
			  "waiting for webreply webnum#%ld twait=%.4f waitdelay=%g",
			  webnum, twait, waitdelay);
	  waiterr =
	    pthread_cond_timedwait (&webitm->iweb_cond,
				    &webitm->iweb_item.i_mtx, &waitts);
	  gotreply = webitm->iweb_replycode > 0;
	  MONIMELT_DEBUG (web, "waiterr=%d (%s), gotreply=%d, replycode %d",
			  waiterr, strerror (waiterr), (int) gotreply,
			  webitm->iweb_replycode);
	  /// got a reply
	  if (gotreply)
	    {
	      MONIMELT_DEBUG (web,
			      "gotreply webnum#%ld mime %s length %u code %d",
			      webnum, webitm->iweb_replymime,
			      webitm->iweb_replylength,
			      webitm->iweb_replycode);
	      if (webitm->iweb_replymime
		  && !strncmp (webitm->iweb_replymime, "text/", 5)
		  && webitm->iweb_replybuf)
		{
		  if (webitm->iweb_replylength + 1 < webitm->iweb_replysize)
		    webitm->iweb_replybuf[webitm->iweb_replylength] =
		      (char) 0;
		  MONIMELT_DEBUG (web, "webnum#%ld buffer:%s\n",
				  webnum, webitm->iweb_replybuf);
		}
	      assert (webitm->iweb_response != NULL);
	      onion_response_set_code (webitm->iweb_response,
				       webitm->iweb_replycode);
	      onion_response_set_length (webitm->iweb_response,
					 webitm->iweb_replylength);
	      if (webitm->iweb_replymime)
		onion_response_set_header (webitm->iweb_response,
					   "content-type",
					   webitm->iweb_replymime);
	      else
		MONIMELT_WARNING ("web request #%ld has no mime type",
				  webitm->iweb_webnum);
	      if (webitm->iweb_replylength > 0)
		onion_response_write (webitm->iweb_response,
				      webitm->iweb_replybuf,
				      webitm->iweb_replylength);
	      onion_response_flush (webitm->iweb_response);
	      MONIMELT_DEBUG (web, "webnum#%ld flushed response",
			      webitm->iweb_webnum);
	      webitm->iweb_response = NULL;
	      repeatloop = false;
	    }
	  // timedout
	  else if (!gotreply && tnow > wend + 0.01)
	    {
	      MONIMELT_DEBUG (web, "timedout webnum#%ld", webnum);
	      onion_response_free (webitm->iweb_response);
	      webitm->iweb_response =
		onion_response_new (webitm->iweb_request);
	      onion_response_set_header (webitm->iweb_response,
					 "Content-Type",
					 "text/html; charset=utf-8");
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
	      repeatloop = false;
	    }
	  else
	    {
	      if (waitdelay < 0.2)
		waitdelay = waitdelay * 1.4 + 0.002;
	      MONIMELT_DEBUG (web,
			      "patiently rewaiting for webnum#%ld waitdelay=%g",
			      webnum, waitdelay);
	      repeatloop = true;
	    }
	  pthread_mutex_unlock (&webitm->iweb_item.i_mtx);
	  sched_yield ();
	  MONIMELT_DEBUG (web, "webnum#%ld endloop repeatloop=%d", webnum,
			  (int) repeatloop);
	};			// end while repeatloop
      sched_yield ();
      pwebinf->web_stat = OCS_PROCESSED;
      MONIMELT_DEBUG (web, "processed webnum#%ld", webnum);
    }
  else
    {
      pwebinf->web_stat = OCS_NOT_PROCESSED;
      MONIMELT_DEBUG (web, "notprocessed webnum#%ld", webnum);
    }
  MONIMELT_DEBUG (web, "end really_process_request webnum %ld web_stat %d",
		  webnum, (int) pwebinf->web_stat);
  MOMGC_UNREGISTER_MY_THREAD ();
  return NULL;

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

momval_t
mom_item_webrequest_post_arg (momval_t val, const char *argname)
{
  momval_t res = MONIMELT_NULLV;
  if (!val.ptr || *val.ptype != momty_webrequestitem || !argname
      || !argname[0])
    return MONIMELT_NULLV;
  pthread_mutex_lock (&val.panyitem->i_mtx);
  res = mom_jsonob_getstr (val.pwebrequestitem->iweb_postjsob, argname);
  pthread_mutex_unlock (&val.panyitem->i_mtx);
  return res;
}

momval_t
mom_item_webrequest_jsob_post (momval_t val)
{
  momval_t res = MONIMELT_NULLV;
  if (!val.ptr || *val.ptype != momty_webrequestitem)
    return MONIMELT_NULLV;
  pthread_mutex_lock (&val.panyitem->i_mtx);
  res = val.pwebrequestitem->iweb_postjsob;
  pthread_mutex_unlock (&val.panyitem->i_mtx);
  return res;
}

momval_t
mom_item_webrequest_query_arg (momval_t val, const char *argname)
{
  momval_t res = MONIMELT_NULLV;
  if (!val.ptr || *val.ptype != momty_webrequestitem || !argname
      || !argname[0])
    return MONIMELT_NULLV;
  pthread_mutex_lock (&val.panyitem->i_mtx);
  res = mom_jsonob_getstr (val.pwebrequestitem->iweb_queryjsob, argname);
  pthread_mutex_unlock (&val.panyitem->i_mtx);
  return res;
}

momval_t
mom_item_webrequest_jsob_query (momval_t val)
{
  momval_t res = MONIMELT_NULLV;
  if (!val.ptr || *val.ptype != momty_webrequestitem)
    return MONIMELT_NULLV;
  pthread_mutex_lock (&val.panyitem->i_mtx);
  res = val.pwebrequestitem->iweb_queryjsob;
  pthread_mutex_unlock (&val.panyitem->i_mtx);
  return res;
}


momval_t
mom_item_webrequest_method (momval_t val)
{
  momval_t res = MONIMELT_NULLV;
  if (!val.ptr || *val.ptype != momty_webrequestitem)
    return MONIMELT_NULLV;
  pthread_mutex_lock (&val.panyitem->i_mtx);
  res = (momval_t) (val.pwebrequestitem->iweb_methoditm);
  pthread_mutex_unlock (&val.panyitem->i_mtx);
  return res;
}




////////////////


static inline void
webrequest_reserve (momit_webrequest_t * webitm, unsigned more)
{
  if (webitm->iweb_replylength + more + 2 >= webitm->iweb_replysize)
    {
      unsigned newsize =
	((5 * webitm->iweb_replylength / 4 + more + 100) | 0xff) + 1;
      char *newbuf = GC_MALLOC_ATOMIC (newsize);
      if (MONIMELT_UNLIKELY (!newbuf))
	MONIMELT_FATAL ("failed to grow webrequest reply to %d",
			(int) newsize);
      memset (newbuf, 0, newsize);
      memcpy (newbuf, webitm->iweb_replybuf, webitm->iweb_replylength);
      GC_FREE (webitm->iweb_replybuf);
      webitm->iweb_replybuf = newbuf;
    }
}

#define ADDWEBSTR(Webitm,Str) do { const char*str = (Str);		\
      int slen=strlen (str);						\
      webrequest_reserve ((Webitm), slen+1);				\
      memcpy ((Webitm)->iweb_replybuf+(Webitm)->iweb_replylength,	\
	      str, slen);						\
      (Webitm)->iweb_replylength += slen;				\
  } while(0)

static void
webrequest_addhtml (momit_webrequest_t * webitm, const char *htmlstr)
{
  unsigned len = strlen (htmlstr);
  const gchar *end = NULL;
  if (MONIMELT_UNLIKELY (!g_utf8_validate (htmlstr, len, &end)))
    MONIMELT_FATAL ("invalid UTF-8 string %s", htmlstr);
  webrequest_reserve (webitm, 9 * len / 8 + 2);
  for (const gchar * pc = htmlstr; *pc && pc < htmlstr + len;
       pc = g_utf8_next_char (pc))
    {
      gunichar c = g_utf8_get_char (pc);
      switch (c)
	{
	case '&':
	  ADDWEBSTR (webitm, "&amp;");
	  break;
	case '<':
	  ADDWEBSTR (webitm, "&lt;");
	  break;
	case '>':
	  ADDWEBSTR (webitm, "&gt;");
	  break;
	case '"':
	  ADDWEBSTR (webitm, "&quot;");
	  break;
	case '\'':
	  ADDWEBSTR (webitm, "&apos;");
	  break;
	case 160:
	  ADDWEBSTR (webitm, "&nbsp;");
	  break;
	default:
	  if (c >= 127)
	    {
	      char ebuf[16];
	      memset (ebuf, 0, sizeof (ebuf));
	      snprintf (ebuf, sizeof (ebuf), "&#%d;", (int) c);
	      ADDWEBSTR (webitm, ebuf);
	    }
	  else
	    {
	      webrequest_reserve (webitm, 4);
	      webitm->iweb_replybuf[webitm->iweb_replylength++] = (char) c;
	    }
	  break;
	}
    }
}


void
mom_item_webrequest_add (momval_t val, ...)
{
  int replycode = -1;
  if (!val.ptr || *val.ptype != momty_webrequestitem)
    return;
  momit_webrequest_t *webitm = val.pwebrequestitem;
  va_list args;
  enum mom_webreplydirective_en wdir = 0;
  pthread_mutex_lock (&val.panyitem->i_mtx);
  va_start (args, val);
  while ((wdir = va_arg (args, enum mom_webreplydirective_en)) != MOMWEB__END)
    {
      bool wanthtmlencoding = false;
      switch (wdir)
	{
	case MOMWEB__END:
	  break;
	case MOMWEB_LIT_STRING:
	  {
	    const char *litstr = va_arg (args, const char *);
	    if (litstr && litstr[0])
	      ADDWEBSTR (webitm, litstr);
	  }
	  break;
	case MOMWEB_HTML_STRING:
	  {
	    const char *htmlstr = va_arg (args, const char *);
	    if (htmlstr && htmlstr[0])
	      webrequest_addhtml (webitm, htmlstr);
	  }
	  break;
	case MOMWEB_HTML_VALUE:
	  wanthtmlencoding = true;
	  // failthru
	case MOMWEB_VALUE:
	  {
	    momval_t val = va_arg (args, momval_t);
	    unsigned typn = 0;
	    if (val.ptr)
	      switch ((typn = (*val.ptype)))
		{
		case momty_string:
		  if (wanthtmlencoding)
		    webrequest_addhtml (webitm, val.pstring->cstr);
		  else
		    ADDWEBSTR (webitm, val.pstring->cstr);
		  break;
		case momty_int:
		  {
		    char ibuf[32];
		    memset (ibuf, 0, sizeof (ibuf));
		    snprintf (ibuf, sizeof (ibuf), "%ld",
			      (long) val.pint->intval);
		    ADDWEBSTR (webitm, ibuf);
		  }
		  break;
		case momty_float:
		  {
		    char fbuf[48];
		    memset (fbuf, 0, sizeof (fbuf));
		    snprintf (fbuf, sizeof (fbuf), "%g", val.pfloat->floval);
		    ADDWEBSTR (webitm, fbuf);
		  }
		  break;
		case momty_jsonarray:
		case momty_jsonobject:
		case momty_jsonitem:
		case momty_booleanitem:
		  {
		    char *bufj = NULL;
		    size_t sizj = 0;
		    struct jsonoutput_st outj = { };
		    FILE *foutj = open_memstream (&bufj, &sizj);
		    if (!foutj)
		      MONIMELT_FATAL ("failed to open stream for JSON");
		    mom_json_output_initialize (&outj, foutj, NULL,
						jsof_flush);
		    mom_output_json (&outj, val);
		    putc ('\n', foutj);
		    fflush (foutj);
		    mom_json_output_close (&outj);
		    ADDWEBSTR (webitm, bufj);
		    free (bufj), bufj = NULL;
		  }
		  break;
		case momty_bufferitem:
		  {
		    pthread_mutex_lock (&val.panyitem->i_mtx);
		    momit_buffer_t *bufitm = val.pbufferitem;
		    assert (bufitm->itu_buf
			    && bufitm->itu_end < bufitm->itu_size);
		    if (bufitm->itu_begin < bufitm->itu_end)
		      {
			bufitm->itu_buf[bufitm->itu_end] = (char) 0;
			if (wanthtmlencoding)
			  webrequest_addhtml (webitm,
					      bufitm->itu_buf +
					      bufitm->itu_begin);
			else
			  ADDWEBSTR (webitm,
				     bufitm->itu_buf + bufitm->itu_begin);
		      };
		    pthread_mutex_unlock (&val.panyitem->i_mtx);
		  }
		  break;
		default:
		  if (typn > momty__itemlowtype)
		    {
		      const char *iname =
			mom_string_cstr ((momval_t)
					 mom_name_of_item (val.panyitem));
		      if (iname)
			{
			  if (wanthtmlencoding)
			    webrequest_addhtml (webitm, iname);
			  else
			    ADDWEBSTR (webitm, iname);
			}
		      else
			{
			  char ustr[UUID_PARSED_LEN + 4];
			  memset (ustr, 0, sizeof (ustr));
			  ustr[0] = '$';
			  mom_underscore_item_uuid (val.panyitem, ustr + 1);
			  ADDWEBSTR (webitm, ustr);
			}
		    }
		  break;
		}
	    else		// null value
	      {
	      }
	  }			// end case MOMWEB_VALUE, MOMWEB_HTML_VALUE
	  break;
	case MOMWEB_JSON_VALUE:
	  {
	    momval_t val = va_arg (args, momval_t);
	    if (mom_is_jsonable (val))
	      {
		char *bufj = NULL;
		size_t sizj = 0;
		struct jsonoutput_st outj = { };
		FILE *foutj = open_memstream (&bufj, &sizj);
		if (!foutj)
		  MONIMELT_FATAL ("failed to open stream for JSON");
		mom_json_output_initialize (&outj, foutj, NULL, jsof_flush);
		mom_output_json (&outj, val);
		putc ('\n', foutj);
		fflush (foutj);
		mom_json_output_close (&outj);
		ADDWEBSTR (webitm, bufj);
		free (bufj), bufj = NULL;
	      }
	  }			// end case MOMWEB_JSON_VALUE
	  break;
	case MOMWEB_DEC_INT:
	  {
	    int num = va_arg (args, int);
	    char ibuf[32];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%d", num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_DEC_UNSIGNED:
	  {
	    unsigned num = va_arg (args, int);
	    char ibuf[32];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%u", num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_HEX_INT:
	  {
	    int num = va_arg (args, int);
	    char ibuf[32];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%#x", num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_DEC_LONG:
	  {
	    long num = va_arg (args, long);
	    char ibuf[48];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%ld", num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_HEX_LONG:
	  {
	    long num = va_arg (args, long);
	    char ibuf[48];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%#lx", num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_DEC_INT64:
	  {
	    int64_t num = va_arg (args, int64_t);
	    char ibuf[48];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%lld", (long long) num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_HEX_INT64:
	  {
	    int64_t num = va_arg (args, int64_t);
	    char ibuf[48];
	    memset (ibuf, 0, sizeof (ibuf));
	    snprintf (ibuf, sizeof (ibuf), "%#llx", (long long) num);
	    ADDWEBSTR (webitm, ibuf);
	  }
	  break;
	case MOMWEB_DOUBLE:
	  {
	    double x = va_arg (args, double);
	    char dbuf[48];
	    memset (dbuf, 0, sizeof (dbuf));
	    snprintf (dbuf, sizeof (dbuf), "%g", x);
	    ADDWEBSTR (webitm, dbuf);
	  }
	  break;
	case MOMWEB_FIXED_DOUBLE:
	  {
	    double x = va_arg (args, double);
	    char dbuf[48];
	    memset (dbuf, 0, sizeof (dbuf));
	    snprintf (dbuf, sizeof (dbuf), "%.15f", x);
	    ADDWEBSTR (webitm, dbuf);
	  }
	  break;
	case MOMWEB_RESERVE:
	  {
	    unsigned more = va_arg (args, unsigned);
	    webrequest_reserve (webitm, more + 1);
	  }
	  break;
	case MOMWEB_CLEAR_BUFFER:
	  {
	    webitm->iweb_replylength = 0;
	    memset (webitm->iweb_replybuf, 0, webitm->iweb_replysize);
	    webitm->iweb_replymime = NULL;
	  }
	  break;
	case MOMWEB_SET_MIME:
	  {
	    const char *mime = va_arg (args, const char *);
	    if (mime && mime[0])
	      {
		// for common mime types, give a constant literal to avoid the GC_STRDUP
#define COMMON_MIME_PREFIX_UTF8(Pref) else if (!strncmp (mime, Pref, sizeof(Pref)-1)) \
		webitm->iweb_replymime = Pref "; charset=utf-8"
#define COMMON_MIME_TYPE(Mime) else if (!strcmp (mime, Mime)) \
		webitm->iweb_replymime = Mime
		// to start the if ... else if
		if (0)
		  mime = "???";
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
		else
		{
		  webitm->iweb_replymime = GC_STRDUP (mime);
		  if (MONIMELT_UNLIKELY (!webitm->iweb_replymime))
		    MONIMELT_FATAL ("failed to strdup mime type %s", mime);
		}
		/* *INDENT-ON* */
#undef COMMON_MIME_PREFIX_UTF8
#undef COMMON_MIME_TYPE
	      }
	  }
	  break;
	case MOMWEB_STDIO_FILE_HTML_CONTENT:
	  wanthtmlencoding = true;
	  // failthru
	case MOMWEB_STDIO_FILE_CONTENT:
	  {
	    FILE *filc = va_arg (args, FILE *);
	    char buff[1024];
	    if (filc)
	      {
		do
		  {
		    memset (buff, 0, sizeof (buff));
		    size_t len = fread (buff, 1, sizeof (buff) - 1, filc);
		    if (len > 0)
		      {
			if (wanthtmlencoding)
			  ADDWEBSTR (webitm, buff);
			else
			  {
			    webrequest_reserve (webitm, len + 1);
			    memcpy (webitm->iweb_replybuf +
				    webitm->iweb_replylength, buff, len);
			    webitm->iweb_replylength += len;
			  }
		      }
		  }
		while (!feof (filc));
	      }
	  }			// end case MOMWEB_STDIO_FILE_HTML_CONTENT & MOMWEB_STDIO_FILE_CONTENT
	  break;
	case MOMWEB_REPLY_CODE:
	  {
	    if (replycode > 0)
	      MONIMELT_FATAL ("already given reply code %d", replycode);
	    int rcode = va_arg (args, int);
	    if (rcode > 0)
	      {
		replycode = rcode;
		webitm->iweb_replycode = rcode;
	      }
	  }			// end case MOMWEB_SEND_REPLY_CODE
	  break;
	default:
	  MONIMELT_FATAL ("unexpected web reply directive %d", (int) wdir);
	}
    }
  va_end (args);
  pthread_mutex_unlock (&val.panyitem->i_mtx);
  if (replycode > 0)
    pthread_cond_broadcast (&webitm->iweb_cond);
}

#undef ADDWEBSTR
