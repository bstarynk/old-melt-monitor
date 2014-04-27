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
  unsigned long web_rand;
  onion_request *web_requ;
  onion_response *web_resp;
};


static struct random_data web_randata;

static
  void *mom_really_process_request (struct GC_stack_base *sb, void *data)
  __attribute__ ((noinline));


static onion_connection_status
process_request (void *ignore, onion_request * req, onion_response * res)
{
  long webnum = 0;
  int32_t randnum = 0;
  {
    char thnambuf[32];
    memset (thnambuf, 0, sizeof (thnambuf));
    pthread_mutex_lock (&mtx_onion);
    nb_weq_requests++;
    webnum = nb_weq_requests;
    random_r (&web_randata, &randnum);
    snprintf (thnambuf, sizeof (thnambuf), "monimelt-w%04ld", webnum);
    pthread_setname_np (pthread_self (), thnambuf);
    pthread_mutex_unlock (&mtx_onion);
  }
  struct mom_web_info_st webinf;
  memset (&webinf, 0, sizeof (webinf));
  webinf.web_magic = WEB_MAGIC;
  webinf.web_stat = OCS_NOT_PROCESSED;
  webinf.web_num = webnum;
  webinf.web_rand = (randnum * 100049) ^ (1039 * getpid () + time (NULL));
  webinf.web_requ = req;
  webinf.web_resp = res;
  GC_call_with_stack_base (mom_really_process_request, &webinf);
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
  memset (&web_randata, 0, sizeof (web_randata));
  {
    unsigned seed = getpid ();
    FILE *fr = fopen ("/dev/urandom", "r");
    if (!fr)
      MONIMELT_FATAL ("cannot open /dev/urandom");
    fread (&seed, sizeof (seed), 1, fr);
    fclose (fr);
    srandom_r (seed, &web_randata);
  }
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
  onion_url_add_handler (mom_onion_root, "",
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

static void *
mom_really_process_request (struct GC_stack_base *sb, void *data)
{
  GC_register_my_thread (sb);
  struct mom_web_info_st *pwebinf = data;
  assert (pwebinf->web_magic == WEB_MAGIC);
  assert (mom_item__web_dictionnary != NULL);
  unsigned long webnum = pwebinf->web_num;
  onion_request *req = pwebinf->web_requ;
  onion_response *res = pwebinf->web_resp;
  const char *fullpath = onion_request_get_fullpath (req);
  const char *path = onion_request_get_path (req);
  unsigned flags = onion_request_get_flags (req);
  mom_anyitem_t *methoditm = NULL;
  const char *method = NULL;
  extern momit_box_t *mom_item__GET;
  extern momit_box_t *mom_item__PUT;
  extern momit_box_t *mom_item__POST;
  extern momit_box_t *mom_item__OPTIONS;
  extern momit_box_t *mom_item__HEAD;
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
  if (fullpathlen > 2 && isalpha (fullpath[1])
      && (closhandler =
	  mom_item_dictionnary_get_cstr ((momval_t) mom_item__web_dictionnary,
					 fullpath + 1)).ptr != NULL
      && *closhandler.ptype == momty_closure)
    {
      momval_t jpost = MONIMELT_NULLV;
      momval_t jquery = MONIMELT_NULLV;
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
    }
#warning incomplete mom_really_process_request
end:
  MONIMELT_INFORM ("request #%ld fullpath=%s path=%s method=%s",
		   webnum, fullpath, path, method ? method : "??");
  GC_unregister_my_thread ();
  return NULL;
}
