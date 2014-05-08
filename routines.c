// file routines.c

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

int
momcode_web_form_exit (int state, momit_tasklet_t * tasklet,
		       momclosure_t * closure, momval_t * locvals,
		       intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MONIMELT_DEBUG (web, "momcode_web_form_exit state=%d webnum=%ld nowbuf=%s",
		  state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "web_form_exit tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "web_form_exit webv=", webv);
  MOM_DBG_VALUE (web, "web_form_exit closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "web_form_exit method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MONIMELT_DEBUG (web, "momcode_web_form_exit POST");
      MOM_DBG_VALUE (web, "web_form_exit jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      if (mom_item_webrequest_post_arg (webv, "do_savexit").ptr)
	{
	  MONIMELT_DEBUG (web, "momcode_web_form_exit do_savexit");
	  mom_item_webrequest_add
	    (webv,
	     MOMWEB_SET_MIME, "text/html",
	     MOMWEB_LIT_STRING,
	     "<!doctype html><head><title>dump and exit Monimelt</title></head>\n"
	     "<body><h1>Monimelt dump and exit</h1>\n"
	     "<p>dump to default <tt>" MONIMELT_DEFAULT_STATE_FILE
	     "</tt> reqnum#",
	     MOMWEB_DEC_LONG, (long) mom_item_webrequest_webnum (webv),
	     MOMWEB_LIT_STRING, " at <i>",
	     MOMWEB_HTML_STRING, nowbuf,
	     MOMWEB_LIT_STRING, "</i></p>\n" "</body></html>\n",
	     MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  usleep (25000);
	  MONIMELT_DEBUG (web,
			  "momcode_web_form_exit do_savexit before request stop");
	  mom_request_stop ("momcode_web_form_exit do_savexit", NULL, NULL);
	  usleep (2000);
	  MONIMELT_DEBUG (web,
			  "momcode_web_form_exit do_savexit before fulldump");
	  mom_full_dump ("web save&exit dump", MONIMELT_DEFAULT_STATE_FILE);;
	  MONIMELT_DEBUG (web,
			  "momcode_web_form_exit do_savexit after fulldump");
	}
      else if (mom_item_webrequest_post_arg (webv, "do_quit").ptr)
	{
	  MONIMELT_DEBUG (web, "momcode_web_form_exit do_quit");
	  mom_item_webrequest_add
	    (webv,
	     MOMWEB_SET_MIME, "text/html",
	     MOMWEB_LIT_STRING,
	     "<!doctype html><head><title>Quit Monimelt</title></head>\n"
	     "<body><h1>Monimelt quitting</h1>\n"
	     "<p>quitting reqnum#",
	     MOMWEB_DEC_LONG, (long) mom_item_webrequest_webnum (webv),
	     MOMWEB_LIT_STRING, " at <i>",
	     MOMWEB_HTML_STRING, nowbuf,
	     MOMWEB_LIT_STRING, "</i></p>\n" "</body></html>\n",
	     MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  usleep (2000);
	  MONIMELT_DEBUG (web,
			  "momcode_web_form_exit do_quit before request stop");
	  mom_request_stop ("momcode_web_form_exit do_quit", NULL, NULL);
	}
      else
	MONIMELT_WARNING ("unexpected post query for webnum#%ld",
			  mom_item_webrequest_webnum (webv));
    }
  usleep (5000);
  return routres_pop;
}

const struct momroutinedescr_st momrout_web_form_exit =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "web_form_exit",
  .rout_code = (const momrout_sig_t *) momcode_web_form_exit
};


////////////////////////////////////////////////////////////////

int
momcode_web_form_new_named (int state, momit_tasklet_t * tasklet,
			    momclosure_t * closure, momval_t * locvals,
			    intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MONIMELT_DEBUG (web,
		  "momcode_web_form_new_named state=%d webnum=%ld nowbuf=%s",
		  state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "web_form_new_named tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "web_form_new_named webv=", webv);
  MOM_DBG_VALUE (web, "web_form_new_named closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "web_form_new_named method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MONIMELT_DEBUG (web, "momcode_web_form_new_named POST");
      MOM_DBG_VALUE (web, "web_form_new_named jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      if (mom_item_webrequest_post_arg (webv, "do_create_new_named").ptr)
	{
	  mom_anyitem_t *newitm = NULL;
	  char *errmsg = NULL;
	  MONIMELT_DEBUG (web,
			  "momcode_web_form_new_named do_create_new_named");
	  momval_t namestr = mom_item_webrequest_post_arg (webv, "name_str");
	  MOM_DBG_VALUE (web, "web_form_new_named namestr=", namestr);
	  momval_t typestr = mom_item_webrequest_post_arg (webv, "type_str");
	  MOM_DBG_VALUE (web, "web_form_new_named type=", typestr);
	  momval_t commentstr =
	    mom_item_webrequest_post_arg (webv, "comment_str");
	  MOM_DBG_VALUE (web, "web_form_new_named comment=", commentstr);
	  const char *namec = mom_string_cstr (namestr);
	  if (mom_item_named (namec) != NULL)
	    errmsg = "already existing item";
	  bool goodname = isalpha (namec[0]);
	  for (const char *pc = namec; *pc && goodname; pc++)
	    if (pc[0] == '_')
	      goodname = pc[1] != '_';
	    else if (!isalnum (*pc))
	      goodname = false;
	  if (!goodname)
	    errmsg = "invalid name";
	  else
	    {
	      const char *typec = mom_string_cstr (typestr);
	      if (!typec || !typec[0])
		errmsg = "no type";
	      else if (!strcmp (typec, "assoc"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_assoc (MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "box"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_box (MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "buffer"))
		newitm =
		  (mom_anyitem_t *)
		  mom_make_item_buffer (MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "dictionnary"))
		newitm =
		  (mom_anyitem_t *)
		  mom_make_item_dictionnary (MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "json_name"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_json_name (namec,
							     MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "queue"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_queue (MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "routine"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_routine (namec,
							   MONIMELT_SPACE_ROOT);
	      else if (!strcmp (typec, "vector"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_vector (MONIMELT_SPACE_ROOT,
							  15);
	      else
		errmsg = "bad type";
	      if (!newitm && !errmsg)
		errmsg = "failed to make item";
	      else
		{
		  mom_register_new_name_string (namestr.pstring, newitm);
		  const char *commc = mom_string_cstr (commentstr);
		  if (commc && commc[0])
		    mom_item_put_attr (newitm,
				       (mom_anyitem_t *) mom_item__comment,
				       commentstr);
		}
	    }
	  if (errmsg)
	    mom_item_webrequest_add
	      (webv,
	       MOMWEB_SET_MIME, "text/html",
	       MOMWEB_LIT_STRING,
	       "<!doctype html><head><title>Failed to Create New Named Monimelt</title></head>\n"
	       "<body><h1>Monimelt failed to create a new named</h1>\n"
	       "<p>name=<tt>", MOMWEB_HTML_STRING, mom_string_cstr (namestr),
	       MOMWEB_LIT_STRING, "</tt> reqnum#",
	       MOMWEB_DEC_LONG, (long) mom_item_webrequest_webnum (webv),
	       MOMWEB_LIT_STRING, " at <i>",
	       MOMWEB_HTML_STRING, nowbuf,
	       MOMWEB_LIT_STRING, "</i> because: ",
	       MOMWEB_HTML_STRING, errmsg,
	       MOMWEB_LIT_STRING, "</p>\n" "</body></html>\n",
	       MOMWEB_REPLY_CODE, HTTP_NOT_FOUND, MOMWEB_END);
	  else
	    {
	      char uidstr[UUID_PARSED_LEN];
	      memset (uidstr, 0, sizeof (uidstr));
	      mom_item_webrequest_add
		(webv,
		 MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING,
		 "<!doctype html><head><title>Create New Named Monimelt</title></head>\n"
		 "<body><h1>Monimelt create a new named</h1>\n"
		 "<p>name=<tt>", MOMWEB_HTML_STRING,
		 mom_string_cstr (namestr), MOMWEB_LIT_STRING,
		 "</tt> reqnum#", MOMWEB_DEC_LONG,
		 (long) mom_item_webrequest_webnum (webv), MOMWEB_LIT_STRING,
		 " of uuid <tt>", MOMWEB_LIT_STRING,
		 mom_unparse_item_uuid (newitm, uidstr), MOMWEB_LIT_STRING,
		 "</tt> at <i>", MOMWEB_HTML_STRING, nowbuf,
		 MOMWEB_LIT_STRING, "</i></p>\n" "</body></html>\n",
		 MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	    }

	  MONIMELT_DEBUG (web,
			  "momcode_web_form_new_named do_create_new_named after");
	}
      else
	MONIMELT_WARNING ("unexpected post query for webnum#%ld",
			  mom_item_webrequest_webnum (webv));
    }
  usleep (5000);
  return routres_pop;
}

const struct momroutinedescr_st momrout_web_form_new_named =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "web_form_new_named",
  .rout_code = (const momrout_sig_t *) momcode_web_form_new_named
};



int
momcode_ajax_start (int state, momit_tasklet_t * tasklet,
		    momclosure_t * closure, momval_t * locvals,
		    intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MONIMELT_DEBUG (web,
		  "momcode_ajax_start state=%d webnum=%ld nowbuf=%s",
		  state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "ajax_start tasklet=", (const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "ajax_start webv=", webv);
  MOM_DBG_VALUE (web, "ajax_start closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "ajax_start method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr == ((momval_t) mom_item__GET).ptr)
    {
      char myhostname[64];
      memset (myhostname, 0, sizeof (myhostname));
      gethostname (myhostname, sizeof (myhostname));
      MONIMELT_DEBUG (web, "momcode_ajax_start GET myhostname=%s",
		      myhostname);
      mom_item_webrequest_add
	(webv, MOMWEB_SET_MIME, "text/html",
	 MOMWEB_LIT_STRING,
	 "<!-- ajax_start fragment -->\n",
	 MOMWEB_LIT_STRING, "<b>Monimelt</b> at <tt>",
	 MOMWEB_HTML_STRING, nowbuf, MOMWEB_LIT_STRING,
	 "</tt> pid ", MOMWEB_DEC_LONG,
	 (long) getpid (), MOMWEB_LIT_STRING,
	 " on host <i>", MOMWEB_HTML_STRING, myhostname,
	 MOMWEB_LIT_STRING, "</i>", MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
      MOM_DBG_VALUE (web, "ajax_start replied webv=", webv);
    }
  return routres_pop;
}

const struct momroutinedescr_st momrout_ajax_start =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_start",
  .rout_code = (const momrout_sig_t *) momcode_ajax_start
};


////////////////////////////////////////////////////////////////

int
momcode_ajax_complete_routine_name (int state, momit_tasklet_t * tasklet,
				    momclosure_t * closure,
				    momval_t * locvals, intptr_t * locnums,
				    double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MONIMELT_DEBUG (web,
		  "momcode_ajax_complete_routine_name state=%d webnum=%ld nowbuf=%s",
		  state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "ajax_complete_routine_name tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "ajax_complete_routine_name webv=", webv);
  MOM_DBG_VALUE (web, "ajax_complete_routine_name closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "ajax_complete_routine_name method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr == ((momval_t) mom_item__GET).ptr)
    {
      momval_t qtermv = mom_item_webrequest_query_arg (webv, "term");
      MOM_DBG_VALUE (web, "ajax_complete_routine_name qtermv=", qtermv);
      const char *qtermstr = mom_string_cstr (qtermv);
      assert (qtermstr != NULL);
      momval_t nodev =
	mom_node_sorted_names_prefixed ((const mom_anyitem_t *)
					mom_item__dictionnary, qtermstr);
      unsigned nbnames = mom_node_arity (nodev);
      momval_t jres = MONIMELT_NULLV;
      if (nbnames > 0)
	{
	  momval_t *goodnames = GC_MALLOC (nbnames * sizeof (momval_t));
	  if (!goodnames)
	    MONIMELT_FATAL ("failed to allocate %d names", nbnames);
	  memset (goodnames, 0, nbnames * sizeof (momval_t));
	  unsigned goodcount = 0;
	  for (unsigned ix = 0; ix < nbnames; ix++)
	    {
	      assert (goodcount < nbnames);
	      momval_t curname = mom_node_nth (nodev, ix);
	      const char *curstr = mom_string_cstr (curname);
	      if (!curstr)
		continue;
	      const mom_anyitem_t *curitm = mom_item_named (curstr);
	      if (!curitm || curitm->typnum != momty_routineitem)
		continue;
	      goodnames[goodcount++] = curname;
	    }
	  jres =
	    (momval_t) ((goodcount > 0)
			? mom_make_json_array_count (goodcount, goodnames)
			: NULL);
	}
      MOM_DBG_VALUE (web, "ajax_complete_routine_name jres=", jres);
      mom_item_webrequest_add
	(webv,
	 MOMWEB_SET_MIME, "application/json",
	 MOMWEB_JSON_VALUE, jres, MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
      MOM_DBG_VALUE (web, "ajax_complete_routine_name replied webv=", webv);
    }
  return routres_pop;
}

const struct momroutinedescr_st momrout_ajax_complete_routine_name =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_complete_routine_name",
  .rout_code = (const momrout_sig_t *) momcode_ajax_complete_routine_name
};


////////////////////////////////////////////////////////////////
static inline const char *
c_name_suffix (mom_anyitem_t * itm)
{
  const char *cn = NULL;
  if (!itm)
    return NULL;
  cn = mom_string_cstr ((momval_t) mom_name_of_item (itm));
  if (!cn)
    {
      char cbuf[UUID_PARSED_LEN + 4];
      memset (cbuf, 0, sizeof (cbuf));
      cbuf[0] = '0';
      cbuf[1] = 'u';
      mom_underscore_item_uuid (itm, cbuf + 2);
      cn = GC_STRDUP (cbuf);
      if (!cn)
	MONIMELT_FATAL ("failed to make c_name_suffix");
    }
  return cn;
}

#define GENERATED_BASE_NAME MONIMELT_SHARED_MODULE_PREFIX "first"
#define GENERATED_SOURCE_FILE_NAME GENERATED_BASE_NAME ".c"
#define GENERATED_SHAROB_FILE_NAME GENERATED_BASE_NAME ".so"

static pthread_mutex_t backup_mtx = PTHREAD_MUTEX_INITIALIZER;
static int backup_count;
static void
backup_old_shared_object ()
{
  pthread_mutex_lock (&backup_mtx);
  if (!access (GENERATED_SHAROB_FILE_NAME, R_OK))
    {
      char pathbuf[128];
      memset (pathbuf, 0, sizeof (pathbuf));
      backup_count++;
      snprintf (pathbuf, sizeof (pathbuf), "%s+p%d_c%d~",
		GENERATED_SHAROB_FILE_NAME, (int) getpid (), backup_count);
      rename (GENERATED_SHAROB_FILE_NAME, pathbuf);
      MONIMELT_INFORM ("backup of old shared object to %s", pathbuf);
    }
  pthread_mutex_unlock (&backup_mtx);
}

enum web_form_compile_values_en
{
  wfcv_argres,
  wfcv_web,
  wfcv_module,
  wfcv_routines,
  wfcv_dashboard,
  wfcv_buffer,
  wfcv_curout,
  wfcv_curprep,
  wfcv_routdata,
  wfcv_routemp,
  wfcv_curemit,
  wfcv_compilproc,
  wfcv__lastval
};

enum web_form_compile_closure_en
{
  wfcc_aftercompilation,
  wfcc__lastclos
};

enum web_form_compile_numbers_en
{
  wfcn_ix,
  wfcn__lastnum
};

int
momcode_web_form_compile (int state, momit_tasklet_t * tasklet,
			  momclosure_t * closure, momval_t * locvals,
			  intptr_t * locnums, double *locdbls)
{
  enum web_form_compile_state_en
  {
    wfcs_start,
    wfcs_compute_routines,
    wfcs_got_routines,
    wfcs_begin_emission,
    wfcs_preparation_loop,
    wfcs_prepare_routine,
    wfcs_emission_loop,
    wfcs_got_preparation,
    wfcs_declare_routine,
    wfcs_emit_routine,
    wfcs_got_emitter,
    wfcs_run_compiler,
    wfcs__last
  };
#define SET_STATE(St) do {						\
    MONIMELT_DEBUG (web,						\
		    "momcode_web_form_compile setstate " #St " = %d",	\
		    (int)wfcs_##St);					\
    return wfcs_##St; } while(0)
#define l_argres locvals[wfcv_argres]
#define l_web locvals[wfcv_web]
#define l_module locvals[wfcv_module]
#define l_routines locvals[wfcv_routines]
#define l_dashboard locvals[wfcv_dashboard]
#define l_buffer locvals[wfcv_buffer]
#define l_curout locvals[wfcv_curout]
#define l_curprep locvals[wfcv_curprep]
#define l_routdata locvals[wfcv_routdata]
#define l_routemp locvals[wfcv_routemp]
#define l_curemit locvals[wfcv_curemit]
#define l_compilproc locvals[wfcv_compilproc]
#define c_aftercompilation closure->sontab[wfcc_aftercompilation]
#define n_ix locnums[wfcn_ix]
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MONIMELT_DEBUG (web,
		  "momcode_web_form_compile state=%d webnum=%ld nowbuf=%s",
		  state, mom_item_webrequest_webnum (l_web), nowbuf);
  MOM_DBG_ITEM (web, "web_form_compile tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "web_form_compile l_web=", l_web);
  MOM_DBG_VALUE (web, "web_form_compile closure=",
		 (momval_t) (const momclosure_t *) closure);
  bool goodstate = false;
  //// state machine
  switch ((enum web_form_compile_state_en) state)
    {
      ////////////////
    case wfcs_start:		////================ start
      {
	goodstate = true;
	l_web = l_argres;
	MONIMELT_DEBUG (web, "momcode_web_form_compile start webnum=%ld",
			mom_item_webrequest_webnum (l_web));
	MOM_DBG_VALUE (web, "web_form_compile l_web=", l_web);
	l_module = (momval_t) mom_item__first_module;
	MOM_DBG_VALUE (web, "web_form_compile l_module=", l_module);
	l_routines =
	  (momval_t) mom_item_get_attr (mom_value_as_item (l_module),
					(mom_anyitem_t *) mom_item__routines);
	MOM_DBG_VALUE (web, "web_form_compile l_routines=", l_routines);
	if (mom_type (l_routines) == momty_closure)
	  SET_STATE (compute_routines);
	else if (mom_type (l_routines) == momty_set)
	  SET_STATE (begin_emission);
	else
	  {
	    MONIMELT_WARNING ("no routines in module");
	    l_routines = MONIMELT_NULLV;
	    SET_STATE (begin_emission);
	  }
	MONIMELT_FATAL
	  ("momcode_web_form_compile unimplemented routine form at start");
      }
      break;
      ////////////////
    case wfcs_compute_routines:	////================ compute routines
      {
	MONIMELT_DEBUG (web,
			"momcode_web_form_compile compute_routines webnum=%ld",
			mom_item_webrequest_webnum (l_web));
	goodstate = true;
	mom_tasklet_push_frame ((momval_t) tasklet, (momval_t) l_routines,
				MOMPFR_VALUE, l_module, MOMPFR_END);
	SET_STATE (got_routines);
      }
      break;
      ////////////////
    case wfcs_got_routines:	////================ got routines
      {
	goodstate = true;
	l_routines = l_argres;
	MOM_DBG_VALUE (web, "web_form_compile got routines=", l_routines);
	if (mom_type (l_routines) != momty_set)
	  {
	    MONIMELT_WARNING ("got no routines in module");
	    l_routines = MONIMELT_NULLV;
	  }
	SET_STATE (begin_emission);
      }
      break;
      ////////////////
    case wfcs_begin_emission:	////================ begin emissions
      {
	goodstate = true;
	l_dashboard = (momval_t) mom_make_item_assoc (MONIMELT_SPACE_NONE);
	l_buffer = (momval_t) mom_make_item_buffer (MONIMELT_SPACE_NONE);
	MOM_DBG_VALUE (web, "web_form_compile l_dashboard=", l_dashboard);
	MOM_DBG_VALUE (web, "web_form_compile l_buffer=", l_buffer);
	mom_item_buffer_puts
	  (l_buffer,
	   "// generated file " GENERATED_SOURCE_FILE_NAME
	   " *** DO NOT EDIT ***\n");
	time_t nowt = 0;
	time (&nowt);
	struct tm nowtm = { };
	char nowyear[16];
	char nowdate[72];
	memset (nowyear, 0, sizeof (nowyear));
	memset (nowdate, 0, sizeof (nowdate));
	localtime_r (&nowt, &nowtm);
	strftime (nowyear, sizeof (nowyear), "%Y", &nowtm);
	strftime (nowdate, sizeof (nowdate), "%Y %b %d [%F]", &nowtm);
	mom_item_buffer_printf (l_buffer, "// generated on %s\n\n", nowdate);
	mom_item_buffer_printf
	  (l_buffer,
	   "/**   Copyright (C) %s Free Software Foundation, Inc.\n"
	   " MONIMELT is a monitor for MELT - see http://gcc-melt.org/\n"
	   " This generated file is part of GCC.\n"
	   "\n"
	   " GCC is free software; you can redistribute it and/or modify\n"
	   " it under the terms of the GNU General Public License as published by\n"
	   " the Free Software Foundation; either version 3, or (at your option)\n"
	   " any later version.\n"
	   "\n"
	   " GCC is distributed in the hope that it will be useful,\n"
	   " but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	   " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	   " GNU General Public License for more details.\n"
	   " You should have received a copy of the GNU General Public License\n"
	   " along with GCC; see the file COPYING3.   If not see\n"
	   " <http://www.gnu.org/licenses/>.\n"
	   "**/\n\n" "#" "include \"monimelt.h\"\n\n", nowyear);
	n_ix = 0;
	SET_STATE (preparation_loop);
      }
      break;
      ////////////////
    case wfcs_preparation_loop:	////================ preparation loop
      {
	goodstate = true;
	MONIMELT_DEBUG (web,
			"momcode_web_form_compile preparation_loop n_ix=%ld",
			(long) n_ix);
	if (n_ix > (long) mom_set_cardinal (l_routines))
	  {
	    mom_item_buffer_printf
	      (l_buffer, "\n\n"
	       "// prepared %ld routines\n",
	       (long) mom_set_cardinal (l_routines));
	    n_ix = 0;
	    SET_STATE (emission_loop);
	  }
	l_curout = (momval_t) mom_set_nth_item (l_routines, n_ix);
	MOM_DBG_VALUE (web, "web_form_compile l_curout=", l_curout);
	n_ix++;
	if (mom_value_as_item (l_curout) != NULL)
	  SET_STATE (prepare_routine);
	else
	  SET_STATE (preparation_loop);
      }
      break;
      ////////////////
    case wfcs_prepare_routine:	////================ prepare routine
      {
	goodstate = true;
	MONIMELT_DEBUG (web,
			"momcode_web_form_compile prepare_routine n_ix=%ld",
			(long) n_ix);
	// get the preparator in the routine, or else in the module
	l_curprep =
	  (momval_t) mom_item_get_attr (mom_value_as_item (l_curout),
					(mom_anyitem_t *)
					mom_item__routine_preparator);
	if (mom_type (l_curprep) != momty_closure)
	  l_curprep =
	    (momval_t) mom_item_get_attr (mom_value_as_item (l_module),
					  (mom_anyitem_t *)
					  mom_item__routine_preparator);
	if (mom_type (l_curprep) == momty_closure)
	  {
	    MOM_DBG_VALUE (web, "web_form_compile l_curprep=", l_curprep);
	    mom_tasklet_push_frame ((momval_t) tasklet, (momval_t) l_curprep,
				    MOMPFR_FOUR_VALUES, l_curout, l_dashboard,
				    l_buffer, l_module, MOMPFR_END);
	    SET_STATE (got_preparation);
	  }
	else
	  {
	    SET_STATE (declare_routine);
	  }
      }
      break;
      ////////////////
    case wfcs_got_preparation:	////================ got preparation
      {
	MOM_DBG_VALUE (web, "web_form_compile got preparation l_argres=",
		       l_argres);
	if (!l_argres.ptr)
	  SET_STATE (declare_routine);
	else
	  SET_STATE (preparation_loop);
      }
      break;
      ////////////////
    case wfcs_declare_routine:	////================ declare_routine
      {
	// should emit the declaration of the routine
	const char *cnam = c_name_suffix (mom_value_as_item (l_curout));
	assert (cnam != NULL);
	l_routdata = mom_item_assoc_get1 (l_dashboard, l_curout);
	if (!l_routdata.ptr)
	  l_routdata = l_curout;
	mom_item_buffer_printf
	  (l_buffer, "\n"
	   "// declaration of code for %s\n"
	   "int momcode_%s (int, momit_tasklet_t *, momclosure_t *,\n"
	   "       momval_t *,intptr_t *, double *);\n", cnam, cnam);
	momval_t statev = MONIMELT_NULLV, closurev = MONIMELT_NULLV;
	momval_t valuesv = MONIMELT_NULLV, numbersv = MONIMELT_NULLV;
	momval_t doublesv = MONIMELT_NULLV;
	mom_item_get_several_attrs (mom_value_as_item (l_routdata),
				    mom_item__state, &statev,
				    mom_item__closure, &closurev,
				    mom_item__values, &valuesv,
				    mom_item__numbers, &numbersv,
				    mom_item__doubles, &doublesv, NULL);
	mom_item_buffer_printf (l_buffer,
				"\n" "// routine descriptor for %s\n"
				"const struct momroutinedescr_st momrout_%s = { .rout_magic = ROUTINE_MAGIC, \n"
				" .rout_minclosize = %d,\n"
				" .rout_frame_nbval = %d,\n"
				" .rout_frame_nbnum = %d,\n"
				" .rout_frame_nbdbl = %d,\n"
				" .rout_name = \"%s\",\n"
				" .rout_code = (const momrout_sig_t *) momcode_%s\n"
				"};\n", cnam, cnam,
				mom_seqitem_length (closurev),
				mom_seqitem_length (valuesv),
				mom_seqitem_length (numbersv),
				mom_seqitem_length (doublesv), cnam, cnam);
	SET_STATE (preparation_loop);
      }
      break;
      ////////////////
    case wfcs_emission_loop:	////================ emission loop
      {
	goodstate = true;
	MONIMELT_DEBUG (web,
			"momcode_web_form_compile emission_loop n_ix=%ld",
			(long) n_ix);
	if (n_ix > (long) mom_set_cardinal (l_routines))
	  {
	    mom_item_buffer_printf
	      (l_buffer, "\n\n"
	       "// emitted %ld routines\n",
	       (long) mom_set_cardinal (l_routines));
	    n_ix = 0;
	    SET_STATE (run_compiler);
	  }
	l_curout = (momval_t) mom_set_nth_item (l_routines, n_ix);
	MOM_DBG_VALUE (web, "web_form_compile emission loop l_curout=",
		       l_curout);
	n_ix++;
	if (mom_value_as_item (l_curout) != NULL)
	  SET_STATE (emit_routine);
	else
	  SET_STATE (emission_loop);
      }
      break;
      ////////////////
    case wfcs_emit_routine:	////================ emit routine
      {
	const char *cnam = c_name_suffix (mom_value_as_item (l_curout));
	MONIMELT_DEBUG (web, "web_form_compile emit routine %s", cnam);
	goodstate = true;
	MOM_DBG_VALUE (web, "web_form_compile emit routine l_curout=",
		       l_curout);
	l_routemp = (momval_t) mom_make_item_assoc (MONIMELT_SPACE_NONE);
	// get the emitter in the routine, or else in the module
	l_curemit =
	  (momval_t) mom_item_get_attr (mom_value_as_item (l_curout),
					(mom_anyitem_t *)
					mom_item__routine_emitter);
	if (mom_type (l_curemit) != momty_closure)
	  l_curemit =
	    (momval_t) mom_item_get_attr (mom_value_as_item (l_module),
					  (mom_anyitem_t *)
					  mom_item__routine_emitter);
	MOM_DBG_VALUE (web, "web_form_compile l_curemit=", l_curemit);
	if (mom_type (l_curemit) == momty_closure)
	  {
	    mom_tasklet_push_frame
	      ((momval_t) tasklet, (momval_t) l_curemit,
	       MOMPFR_FOUR_VALUES, l_curout, l_routdata, l_routemp,
	       l_dashboard, MOMPFR_TWO_VALUES, l_buffer, l_module,
	       MOMPFR_END);
	    SET_STATE (got_emitter);
	  }
	else
	  {
	    MONIMELT_WARNING ("no routine emitter for %s", cnam);
	    return routres_pop;
	  }
      }
      break;
      ////////////////
    case wfcs_got_emitter:	////================ got emitter
      {
	// essentially a no-op
	goodstate = true;
	const char *cnam = c_name_suffix (mom_value_as_item (l_curout));
	MONIMELT_DEBUG (web, "web_form_compile got emitter routine %s", cnam);
	SET_STATE (emission_loop);
      }
      break;
      //////////////// 
    case wfcs_run_compiler:	////================ run compiler
      {
	goodstate = true;
	mom_item_buffer_printf (l_buffer, "\n\n///// end of %d routines \n\n"
				"/*** eof " GENERATED_SOURCE_FILE_NAME
				" ****/\n", (int) n_ix);
	MONIMELT_DEBUG (web,
			"web_form_compile run compiler buffer of %d bytes",
			(int) mom_item_buffer_length (l_buffer));
	/// write the buffer
	rename (GENERATED_SOURCE_FILE_NAME, GENERATED_SOURCE_FILE_NAME "~");
	FILE *fout = fopen (GENERATED_SOURCE_FILE_NAME, "w");
	if (MONIMELT_UNLIKELY (!fout))
	  MONIMELT_FATAL ("failed to open file " GENERATED_SOURCE_FILE_NAME);
	unsigned blen = mom_item_buffer_length (l_buffer);
	if (mom_item_buffer_output_content_to_file (l_buffer, fout)
	    != (int) blen)
	  MONIMELT_WARNING ("failed to output all %d bytes to "
			    GENERATED_SOURCE_FILE_NAME, (int) blen);
	fclose (fout), fout = NULL;
	MONIMELT_INFORM ("wrote %d bytes of code into %s", (int) blen,
			 GENERATED_SOURCE_FILE_NAME);
	backup_old_shared_object ();
	/// create and start the compilation process
	l_compilproc = (momval_t)
	  mom_make_item_process_argvals ((momval_t) mom_make_string ("make"),
					 (momval_t)
					 mom_make_string
					 (GENERATED_SHAROB_FILE_NAME), NULL);
	MOM_DBG_VALUE (web, "web_form_compile l_compilproc=", l_compilproc);
	MOM_DBG_VALUE (web, "web_form_compile c_aftercompilation=",
		       c_aftercompilation);
	mom_item_process_start (l_compilproc, c_aftercompilation);
	MONIMELT_DEBUG (web, "after compilation start");
	char timbuf[64] = { };
	memset (timbuf, 0, sizeof (timbuf));
	struct tm curtm = { };
	time_t now = 0;
	time (&now);
	strftime (timbuf, sizeof (timbuf), "%c", localtime_r (&now, &curtm));
	/// answer the web request
	mom_item_webrequest_add
	  (l_web,
	   MOMWEB_SET_MIME, "text/html",
	   MOMWEB_LIT_STRING,
	   "<!doctype html><head><title>Monimelt generates code</title></head>\n"
	   "<body><h1>Monimelt generates &amp; compiles code</h1>\n"
	   "<p>Wrote <tt>" GENERATED_SHAROB_FILE_NAME "</tt> code file with ",
	   MOMWEB_DEC_LONG, (long) n_ix,
	   MOMWEB_LIT_STRING, " routines and ",
	   MOMWEB_DEC_LONG, (long) blen,
	   MOMWEB_LIT_STRING, " bytes <small>at ",
	   MOMWEB_LIT_STRING, timbuf,
	   MOMWEB_LIT_STRING, "</small>.</p>\n" "</body></html>\n",
	   MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	return routres_pop;
      }
      break;
      ////////////////
    case wfcs__last:
      {
	MONIMELT_FATAL ("momcode_web_form_compile unexpected last");
      }
    }
  if (!goodstate)
    MONIMELT_FATAL ("momcode_web_form_compile invalid state %d", state);
  MONIMELT_DEBUG (web, "momcode_web_form_compile ending state %d", state);
  return routres_pop;
#undef SET_STATE
#undef l_argres
#undef l_web
#undef l_module
#undef l_routines
#undef l_dashboard
#undef l_buffer
#undef l_curout
#undef l_curprep
#undef l_routdata
#undef l_routemp
#undef l_curemit
#undef l_compilproc
#undef c_aftercompilation
#undef n_ix
}

const struct momroutinedescr_st momrout_web_form_compile =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = (unsigned) wfcc__lastclos,
  .rout_frame_nbval = (unsigned) wfcv__lastval,
  .rout_frame_nbnum = (unsigned) wfcn__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "web_form_compile",
  .rout_code = (const momrout_sig_t *) momcode_web_form_compile
};



/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

enum proc_compilation_values_en
{
  pcov_argres,
  pcov_outstr,
  pcov_endreason,
  pcov_proc,
  pcov__lastval
};

enum proc_compilation_closure_en
{
  pcoc__lastclosure
};

enum proc_compilation_numbers_en
{
  pcon_procstatus,
  pcon__lastnum
};


int
momcode_proc_compilation (int state, momit_tasklet_t * tasklet,
			  momclosure_t * closure, momval_t * locvals,
			  intptr_t * locnums, double *locdbls)
{
  enum proc_compilation_state_en
  {
    pcos_start,
    pcos_loadnewmodule,
    pcos__laststate
  };
#define l_argres locvals[pcov_argres]
#define l_proc locvals[pcov_proc]
#define l_outstr locvals[pcov_outstr]
#define l_endreason locvals[pcov_endreason]
#define n_procstatus locnums[pcon_procstatus]
#define SET_STATE(St) do {						\
    MONIMELT_DEBUG (run,						\
		    "momcode_proc_compilation setstate " #St " = %d",	\
		    (int)pcos_##St);					\
    return pcos_##St; } while(0)
  MONIMELT_DEBUG (run, "begin momcode_proc_compilation state=%d", state);
  MOM_DBG_VALUE (run, "proc_compilation closure=",
		 (momval_t) (const momclosure_t *) closure);
  bool goodstate = false;
  //// state machine
  switch ((enum proc_compilation_state_en) state)
    {
      ////////////////
    case pcos_start:		////================ start
      {
	goodstate = true;
	/// incoming values: process, output_string, endstatus (exited|terminated)
	/// incoming numbers: exit-status|termination-signal
	l_proc = l_argres;
	MOM_DBG_VALUE (run, "momcode_proc_compilation start l_proc=", l_proc);
	MOM_DBG_VALUE (run, "momcode_proc_compilation start l_outstr=",
		       l_outstr);
	MOM_DBG_VALUE (run, "momcode_proc_compilation start l_endreason=",
		       l_endreason);
	MONIMELT_DEBUG (run,
			"momcode_proc_compilation start n_procstatus=%ld",
			(long) n_procstatus);
	if (l_endreason.panyitem == (mom_anyitem_t *) mom_item__exited
	    && n_procstatus == 0)
	  {
	    MONIMELT_DEBUG (run,
			    "momcode_proc_compilation make process succeeded");
	    SET_STATE (loadnewmodule);
	  }
	else if (l_endreason.panyitem == (mom_anyitem_t *) mom_item__exited
		 && n_procstatus != 0)
	  {
	    MONIMELT_WARNING
	      ("momcode_proc_compilation make process failed, exit code=%ld",
	       (long) n_procstatus);
	    return routres_pop;
	  }
	else if (l_endreason.panyitem ==
		 (mom_anyitem_t *) mom_item__terminated)
	  {
	    MONIMELT_WARNING
	      ("momcode_proc_compilation make process terminated with signal#%d = %s",
	       (int) n_procstatus, strsignal (n_procstatus));
	    return routres_pop;
	  }
	else
	  MONIMELT_FATAL ("momcode_proc_compilation bad endreason");
      }
      break;
      ////////////////
    case pcos_loadnewmodule:	////================ load new module
      {
	goodstate = true;
	MONIMELT_DEBUG (run, "momcode_proc_compilation loadnewmodule");
	mom_request_stop ("proc_compilation loadnewmodule",
			  (mom_post_runner_sig_t *) mom_load_code_post_runner,
			  GENERATED_BASE_NAME);
	MONIMELT_DEBUG (run,
			"momcode_proc_compilation stop to load "
			GENERATED_BASE_NAME);
	return routres_pop;
      }
      break;
      ////////////////
    case pcos__laststate:
      {
	MONIMELT_FATAL ("momcode_proc_compilation unexpected last");
      }
    }
  if (!goodstate)
    MONIMELT_FATAL ("momcode_proc_compilation invalid state %d", state);
  return routres_pop;
#undef l_argres
#undef l_proc
#undef l_outstr
#undef l_endreason
#undef n_procstatus
#undef SET_STATE
}

const struct momroutinedescr_st momrout_proc_compilation =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = (unsigned) pcoc__lastclosure,
  .rout_frame_nbval = (unsigned) pcov__lastval,
  .rout_frame_nbnum = (unsigned) pcon__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "proc_compilation",
  .rout_code = (const momrout_sig_t *) momcode_proc_compilation
};
