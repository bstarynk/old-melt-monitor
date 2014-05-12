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

////////////////////////////////////////////////////////////////

#define JS_FROM_AT_2(Fil,Lin) "//from " # Fil "@" # Lin "\n"
#define JS_FROM_AT(Fil,Lin) JS_FROM_AT_2(Fil,Lin)
#define JS_FROM JS_FROM_AT(__FILE__,__LINE__)
#define C_FROM JS_FROM

#define HTML_FROM_AT_2(Fil,Lin) "<!-- from " # Fil "@" # Lin " -->\n"
#define HTML_FROM_AT(Fil,Lin) HTML_FROM_AT_2(Fil,Lin)
#define HTML_FROM HTML_FROM_AT(__FILE__,__LINE__)


int
momcode_ajax_exit (int state, momit_tasklet_t * tasklet,
		   momclosure_t * closure, momval_t * locvals,
		   intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MOM_DEBUG (web, "momcode_ajax_exit state=%d webnum=%ld nowbuf=%s",
	     state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "ajax_exit tasklet=", (const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "ajax_exit webv=", webv);
  MOM_DBG_VALUE (web, "ajax_exit closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "ajax_exit method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MOM_DEBUG (web, "momcode_ajax_exit POST");
      MOM_DBG_VALUE (web, "ajax_exit jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      momval_t idxdov = mom_item_webrequest_post_arg (webv, "id");
      MOM_DBG_VALUE (web, "ajax_exit idxdov=", idxdov);
      if (!strcmp (mom_string_cstr (idxdov), "exit_save_id"))
	{
	  MOM_DEBUG (run, "ajax_exit save and exiting webnum#%ld",
		     mom_item_webrequest_webnum (webv));
	  mom_item_webrequest_add (webv, MOMWEB_SET_MIME, "text/html",
				   MOMWEB_LIT_STRING,
				   "Dump to default <tt>"
				   MOM_DEFAULT_STATE_FILE "</tt> reqnum#",
				   MOMWEB_DEC_LONG,
				   (long) mom_item_webrequest_webnum (webv),
				   MOMWEB_LIT_STRING, " at <i>",
				   MOMWEB_HTML_STRING, nowbuf,
				   MOMWEB_LIT_STRING, "</i>.\n",
				   MOMWEB_LIT_STRING, HTML_FROM,
				   MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  usleep (25000);
	  MOM_DEBUG (web, "ajax_exit do_savexit before request stop");
	  mom_request_stop ("ajax_exit savexit", NULL, NULL);
	  usleep (2000);
	  MOM_DEBUG (web, "ajax_exit savexit before fulldump");
	  mom_full_dump ("ajax save&exit dump", MOM_DEFAULT_STATE_FILE);;
	  MOM_DEBUG (web, "ajax_exit savexit after fulldump");
	}
      else if (!strcmp (mom_string_cstr (idxdov), "exit_quit_id"))
	{
	  MOM_DEBUG (run, "ajax_exit exit and quitting webnum#%ld",
		     mom_item_webrequest_webnum (webv));
	  mom_item_webrequest_add (webv, MOMWEB_SET_MIME, "text/html",
				   MOMWEB_LIT_STRING,
				   "Quitting Moniweb without saving, reqnum#",
				   MOMWEB_DEC_LONG,
				   (long) mom_item_webrequest_webnum (webv),
				   MOMWEB_LIT_STRING, " at <i>",
				   MOMWEB_HTML_STRING, nowbuf,
				   MOMWEB_LIT_STRING, "</i>.\n",
				   MOMWEB_LIT_STRING, HTML_FROM,
				   MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  usleep (25000);
	  MOM_DEBUG (web, "ajax_exit do_quit before request stop");
	  mom_request_stop ("ajax_exit quit", NULL, NULL);
	  usleep (2000);
	  MOM_DEBUG (web, "ajax_exit quit after fulldump");
	}
      else
	MOM_WARNING ("ajax_exit strange idxdov=%s", mom_string_cstr (idxdov));
    }
  else
    MOM_WARNING ("ajax_exit strange request webnum#%ld",
		 mom_item_webrequest_webnum (webv));
  return routres_pop;
}

const struct momroutinedescr_st momrout_ajax_exit =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_exit",
  .rout_code = (const momrout_sig_t *) momcode_ajax_exit,
  .rout_timestamp = __DATE__ "@" __TIME__
};



////////////////////////////////////////////////////////////////

#define MOM_GARBAGE_COLLECTION_PERIOD 8
static pthread_mutex_t periodic_mtx = PTHREAD_MUTEX_INITIALIZER;
int
momcode_ajax_periodic (int state, momit_tasklet_t * tasklet,
		       momclosure_t * closure, momval_t * locvals,
		       intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  static long periodic_counter;
  long count = 0;
  {
    pthread_mutex_lock (&periodic_mtx);
    periodic_counter++;
    count = periodic_counter;
    pthread_mutex_unlock (&periodic_mtx);
  }
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MOM_DEBUG (web,
	     "momcode_ajax_periodic state=%d webnum=%ld nowbuf=%s count=%ld",
	     state, mom_item_webrequest_webnum (webv), nowbuf, count);
  MOM_DBG_ITEM (web, "ajax_periodic tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "ajax_periodic webv=", webv);
  MOM_DBG_VALUE (web, "ajax_periodic closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "ajax_periodic method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MOM_DEBUG (web, "momcode_ajax_periodic POST");
      MOM_DBG_VALUE (web, "ajax_periodic jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      GC_word gcheapsize = 0, gcfreebytes = 0, gcunmappedbytes =
	0, gcbytessincegc = 0, gctotalbytes = 0;
      GC_word gcnb =
	(GC_word) GC_call_with_alloc_lock ((GC_fn_type) GC_get_gc_no, NULL);
      GC_get_heap_usage_safe (&gcheapsize, &gcfreebytes, &gcunmappedbytes,
			      &gcbytessincegc, &gctotalbytes);
      mom_item_webrequest_add (webv, MOMWEB_SET_MIME, "text/html",
			       MOMWEB_LIT_STRING,
			       "<!-- ajax_periodic fragment -->\n",
			       MOMWEB_LIT_STRING, "elapsed real: ",
			       MOMWEB_FIX2DIG_DOUBLE,
			       mom_elapsed_real_time (), MOMWEB_LIT_STRING,
			       ", cpu:", MOMWEB_FIX2DIG_DOUBLE,
			       mom_clock_time (CLOCK_PROCESS_CPUTIME_ID),
			       MOMWEB_LIT_STRING, " sec., ", MOMWEB_DEC_INT64,
			       (int64_t) mom_agenda_work_counter (),
			       MOMWEB_LIT_STRING, " done tasklets, ",
			       MOMWEB_DEC_LONG, (long) mom_nb_items (),
			       MOMWEB_LIT_STRING, " items. <small>(GC: ",
			       MOMWEB_DEC_LONG, (long) gcheapsize / 1024,
			       MOMWEB_LIT_STRING, "kb heapsize, ",
			       MOMWEB_DEC_LONG, (long) gcfreebytes / 1024,
			       MOMWEB_LIT_STRING, "kb free, ",
			       MOMWEB_DEC_LONG, (long) gcunmappedbytes / 1024,
			       MOMWEB_LIT_STRING, "kb unmapped, ",
			       MOMWEB_DEC_LONG, (long) gcbytessincegc / 1024,
			       MOMWEB_LIT_STRING, "kb since gc, ",
			       MOMWEB_DEC_LONG, (long) gctotalbytes / 1024,
			       MOMWEB_LIT_STRING, "kb total, ",
			       MOMWEB_DEC_LONG, (long) gcnb,
			       MOMWEB_LIT_STRING, " nb, ", MOMWEB_HEX_LONG,
			       (long) GC_get_version (), MOMWEB_LIT_STRING,
			       " version)</small>", MOMWEB_LIT_STRING,
			       HTML_FROM, MOMWEB_REPLY_CODE, HTTP_OK,
			       MOMWEB_END);
      MOM_DBG_VALUE (web, "ajax_periodic replied webv=", webv);
    }
  return routres_pop;
}

const struct momroutinedescr_st momrout_ajax_periodic =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_periodic",
  .rout_code = (const momrout_sig_t *) momcode_ajax_periodic,
  .rout_timestamp = __DATE__ "@" __TIME__
};

////////////////////////////////////////////////////////////////

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
  MOM_DEBUG (web,
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
      MOM_DEBUG (web, "momcode_ajax_start GET myhostname=%s", myhostname);
      mom_item_webrequest_add
	(webv, MOMWEB_SET_MIME, "text/html",
	 MOMWEB_LIT_STRING,
	 "<!-- ajax_start fragment -->\n",
	 MOMWEB_LIT_STRING, "<b>Monimelt</b> at <tt>",
	 MOMWEB_HTML_STRING, nowbuf, MOMWEB_LIT_STRING,
	 "</tt> pid ", MOMWEB_DEC_LONG,
	 (long) getpid (), MOMWEB_LIT_STRING,
	 " on host <i>", MOMWEB_HTML_STRING, myhostname,
	 MOMWEB_LIT_STRING, "</i> <span class='note_cl'>built: <i>",
	 MOMWEB_HTML_STRING, monimelt_timestamp,
	 MOMWEB_LIT_STRING, "</i> commit <tt>",
	 MOMWEB_HTML_STRING, monimelt_lastgitcommit,
	 MOMWEB_LIT_STRING, "</tt></span>",
	 MOMWEB_LIT_STRING, HTML_FROM,
	 MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
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
  .rout_code = (const momrout_sig_t *) momcode_ajax_start,
  .rout_timestamp = __DATE__ "@" __TIME__
};



////////////////////////////////////////////////////////////////
int
momcode_ajax_named (int state, momit_tasklet_t * tasklet,
		    momclosure_t * closure, momval_t * locvals,
		    intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MOM_DEBUG (web,
	     "momcode_ajax_named state=%d webnum=%ld nowbuf=%s",
	     state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "ajax_named tasklet=", (const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "ajax_named webv=", webv);
  MOM_DBG_VALUE (web, "ajax_named closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "ajax_named method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MOM_DEBUG (web, "momcode_ajax_named POST");
      MOM_DBG_VALUE (web, "ajax_named jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      momval_t idw = mom_item_webrequest_post_arg (webv, "id");
      if (mom_same_string (idw, "named_create_id"))
	{
	  MOM_DEBUG (web, "momcode_ajax_named named_create_id");
	  mom_item_webrequest_add
	    (webv,
	     MOMWEB_SET_MIME, "application/javascript",
	     MOMWEB_LIT_STRING, "install_create_named_form('",
	     MOMWEB_JS_STRING, nowbuf,
	     MOMWEB_LIT_STRING, "');\n",
	     MOMWEB_LIT_STRING, JS_FROM,
	     MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	}
      else if (mom_same_string (idw, "named_forget_id"))
	{
	  MOM_DEBUG (web,
		     "momcode_ajax_named named_forget_id inserting a forget form");
	  mom_item_webrequest_add (webv, MOMWEB_SET_MIME,
				   "application/javascript",
				   MOMWEB_LIT_STRING,
				   "install_forget_named_form('",
				   MOMWEB_JS_STRING, nowbuf,
				   MOMWEB_LIT_STRING, "');\n",
				   MOMWEB_LIT_STRING, JS_FROM,
				   MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	}
      else if (mom_same_string (idw, "do_create_named"))
	{
	  mom_anyitem_t *newitm = NULL;
	  char *errmsg = NULL;
	  MOM_DEBUG (web, "ajax_named do_create_named webnum#%ld",
		     mom_item_webrequest_webnum (webv));
	  momval_t namev = mom_item_webrequest_post_arg (webv, "name");
	  momval_t commv = mom_item_webrequest_post_arg (webv, "comment");
	  momval_t typev = mom_item_webrequest_post_arg (webv, "type");
	  MOM_DBG_VALUE (web, "ajax_named do_create_named namev=", namev);
	  MOM_DBG_VALUE (web, "ajax_named do_create_named commv=", commv);
	  MOM_DBG_VALUE (web, "ajax_named do_create_named typev=", typev);
	  if (mom_item_of_name_string (namev) != NULL)
	    errmsg = "already existing named item";
	  const char *namec = mom_string_cstr (namev);
	  if (!namec)
	    errmsg = "invalid name";
	  bool goodname = !errmsg && isalpha (namec[0]);
	  for (const char *pc = namec; *pc && goodname; pc++)
	    if (pc[0] == '_')
	      goodname = pc[1] != '_';
	    else if (!isalnum (*pc))
	      goodname = false;
	  if (!goodname)
	    {
	      if (!errmsg)
		errmsg = "invalid name";
	    }
	  else if (mom_same_string (typev, "assoc"))
	    newitm = (mom_anyitem_t *) mom_make_item_assoc (MOM_SPACE_ROOT);
	  else if (mom_same_string (typev, "box"))
	    newitm = (mom_anyitem_t *) mom_make_item_box (MOM_SPACE_ROOT);
	  else if (mom_same_string (typev, "dictionnary"))
	    newitm =
	      (mom_anyitem_t *) mom_make_item_dictionnary (MOM_SPACE_ROOT);
	  else if (mom_same_string (typev, "json_name"))
	    newitm =
	      (mom_anyitem_t *) mom_make_item_json_name (namec,
							 MOM_SPACE_ROOT);
	  else if (mom_same_string (typev, "queue"))
	    newitm = (mom_anyitem_t *) mom_make_item_queue (MOM_SPACE_ROOT);
	  else if (mom_same_string (typev, "routine"))
	    newitm =
	      (mom_anyitem_t *) mom_make_item_embryonic_routine (namec,
								 MOM_SPACE_ROOT);
	  else if (mom_same_string (typev, "vector"))
	    newitm =
	      (mom_anyitem_t *) mom_make_item_vector (MOM_SPACE_ROOT, 15);
	  else
	    errmsg = "bad type";
	  if (!newitm && !errmsg)
	    errmsg = "failed to make item";
	  else
	    {
	      mom_register_new_name_string (namev.pstring, newitm);
	      const char *commc = mom_string_cstr (commv);
	      if (commc && commc[0])
		mom_item_put_attr (newitm,
				   (mom_anyitem_t *) mom_item__comment,
				   commv);
	    }
	  MOM_DEBUG (web, "ajax_named errmsg=%s", errmsg);
	  MOM_DBG_ITEM (web, "ajax_named newitm=", newitm);
	  if (errmsg)
	    {
	      mom_item_webrequest_add
		(webv,
		 MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING, "<b>Failed to create named</b> <tt>",
		 MOMWEB_HTML_STRING, mom_string_cstr (namev),
		 MOMWEB_LIT_STRING, "</tt>:\n <em>",
		 MOMWEB_HTML_STRING, errmsg,
		 MOMWEB_LIT_STRING, "</em>\n at <i>",
		 MOMWEB_HTML_STRING, nowbuf,
		 MOMWEB_LIT_STRING, "</i>.\n",
		 MOMWEB_LIT_STRING, HTML_FROM,
		 MOMWEB_REPLY_CODE, HTTP_FORBIDDEN, MOMWEB_END);
	      MOM_DEBUG (web, "ajax_named do_create_named forbidden %s",
			 errmsg);
	    }
	  else
	    {
	      char uidstr[UUID_PARSED_LEN];
	      memset (uidstr, 0, sizeof (uidstr));
	      mom_item_webrequest_add
		(webv,
		 MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING, "<b>Create named</b> <tt>",
		 MOMWEB_HTML_STRING, mom_string_cstr (namev),
		 MOMWEB_LIT_STRING, "</tt> <small>of uuid <tt>",
		 MOMWEB_LIT_STRING, mom_unparse_item_uuid (newitm, uidstr),
		 MOMWEB_LIT_STRING, "</tt></small> at <i>",
		 MOMWEB_HTML_STRING, nowbuf,
		 MOMWEB_LIT_STRING, "</i>.\n",
		 MOMWEB_LIT_STRING, HTML_FROM,
		 MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	      MOM_DEBUG (web, "ajax_named do_create_named ok uidstr=%s",
			 uidstr);
	    }
	  return routres_pop;
	}
      else if (mom_same_string (idw, "do_forget_named"))
	{
	  momval_t namev = mom_item_webrequest_post_arg (webv, "name");
	  char uidstr[UUID_PARSED_LEN];
	  memset (uidstr, 0, sizeof (uidstr));
	  MOM_DBG_VALUE (web, "ajax_named do_forget_named namev=", namev);
	  mom_anyitem_t *oldnameditm = mom_item_of_name_string (namev);
	  MOM_DBG_ITEM (web, "ajax_named oldnameditm=", oldnameditm);
	  if (oldnameditm)
	    {
	      // got oldnameditm
	      mom_forget_item (oldnameditm);
	      mom_item_webrequest_add
		(webv,
		 MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING, "<b>Forgotten named</b> <tt>",
		 MOMWEB_HTML_STRING, mom_string_cstr (namev),
		 MOMWEB_LIT_STRING, "</tt> <small>of uuid <tt>",
		 MOMWEB_LIT_STRING, mom_unparse_item_uuid (oldnameditm,
							   uidstr),
		 MOMWEB_LIT_STRING, "</tt></small> at <i>",
		 MOMWEB_HTML_STRING, nowbuf, MOMWEB_LIT_STRING, "</i>.\n",
		 MOMWEB_LIT_STRING, HTML_FROM, MOMWEB_REPLY_CODE, HTTP_OK,
		 MOMWEB_END);
	      MOM_DEBUG (web, "ajax_named do_forget_named ok uidstr=%s",
			 uidstr);
	      return routres_pop;
	    }
	  else
	    {
	      // no oldnameditm
	      mom_item_webrequest_add
		(webv,
		 MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING,
		 "<b>Failed to forget invalid named</b> <tt>",
		 MOMWEB_HTML_STRING, mom_string_cstr (namev),
		 MOMWEB_LIT_STRING, "</tt> at <i>", MOMWEB_HTML_STRING,
		 nowbuf, MOMWEB_LIT_STRING, "</i>.\n", MOMWEB_LIT_STRING,
		 HTML_FROM, MOMWEB_REPLY_CODE, HTTP_FORBIDDEN, MOMWEB_END);
	      MOM_DEBUG (web, "ajax_named do_forget_named failed nowbuf=%s",
			 nowbuf);
	      return routres_pop;
	    }
	}
      MOM_WARNING ("momcode_ajax_named incomplete");
    }
  else
    MOM_WARNING ("ajax_named strange request webnum#%ld",
		 mom_item_webrequest_webnum (webv));
  return routres_pop;
}

const struct momroutinedescr_st momrout_ajax_named =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_named",
  .rout_code = (const momrout_sig_t *) momcode_ajax_named,
  .rout_timestamp = __DATE__ "@" __TIME__
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
  MOM_DEBUG (web,
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
      momval_t queryv = mom_item_webrequest_jsob_query (webv);
      momval_t qtermv = mom_item_webrequest_query_arg (webv, "term");
      MOM_DBG_VALUE (web, "ajax_complete_routine_name qtermv=", qtermv);
      MOM_DBG_VALUE (web, "ajax_complete_routine_name queryv=", queryv);
      const char *qtermstr = mom_string_cstr (qtermv);
      assert (qtermstr != NULL);
      momval_t nodev =
	mom_node_sorted_names_prefixed ((const mom_anyitem_t *)
					mom_item__dictionnary, qtermstr);
      unsigned nbnames = mom_node_arity (nodev);
      momval_t jres = MOM_NULLV;
      if (nbnames > 0)
	{
	  momval_t *goodnames = GC_MALLOC (nbnames * sizeof (momval_t));
	  if (!goodnames)
	    MOM_FATAL ("failed to allocate %d names", nbnames);
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
  .rout_code = (const momrout_sig_t *) momcode_ajax_complete_routine_name,
  .rout_timestamp = __DATE__ "@" __TIME__
};

////////////////////////////////////////////////////////////////



int
momcode_ajax_complete_name (int state, momit_tasklet_t * tasklet,
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
  MOM_DEBUG (web,
	     "momcode_ajax_complete_name state=%d webnum=%ld nowbuf=%s",
	     state, mom_item_webrequest_webnum (webv), nowbuf);
  MOM_DBG_ITEM (web, "ajax_complete_name tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "ajax_complete_name webv=", webv);
  MOM_DBG_VALUE (web, "ajax_complete_name closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "ajax_complete_name method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      momval_t postv = mom_item_webrequest_jsob_query (webv);
      momval_t termv = mom_item_webrequest_post_arg (webv, "term");
      MOM_DBG_VALUE (web, "ajax_complete_name postv=", postv);
      MOM_DBG_VALUE (web, "ajax_complete_name termv=", termv);
      const char *termstr = mom_string_cstr (termv);
      momval_t nodev =
	mom_node_sorted_names_prefixed ((const mom_anyitem_t *)
					mom_item__dictionnary, termstr);
      MOM_DBG_VALUE (web, "ajax_complete_name nodev=", nodev);
      unsigned nbnames = mom_node_arity (nodev);
      momval_t jres = MOM_NULLV;
      if (nbnames > 0)
	{
	  momval_t *goodnames = GC_MALLOC (nbnames * sizeof (momval_t));
	  if (!goodnames)
	    MOM_FATAL ("failed to allocate %d names", nbnames);
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
	      if (!curitm)
		continue;
	      goodnames[goodcount++] = curname;
	    }
	  jres =
	    (momval_t) ((goodcount > 0)
			? mom_make_json_array_count (goodcount, goodnames)
			: NULL);
	}
      MOM_DBG_VALUE (web, "ajax_complete_name jres=", jres);
      mom_item_webrequest_add
	(webv,
	 MOMWEB_SET_MIME, "application/json",
	 MOMWEB_JSON_VALUE, jres, MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
      MOM_DBG_VALUE (web, "ajax_complete_name replied webv=", webv);
    }
  return routres_pop;
}

const struct momroutinedescr_st momrout_ajax_complete_name =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_complete_name",
  .rout_code = (const momrout_sig_t *) momcode_ajax_complete_name,
  .rout_timestamp = __DATE__ "@" __TIME__
};


////////////////////////////////////////////////////////////////
static inline const char *
c_name_suffix (const mom_anyitem_t * itm)
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
	MOM_FATAL ("failed to make c_name_suffix");
    }
  return cn;
}

#define GENERATED_BASE_NAME MOM_SHARED_MODULE_PREFIX "first"
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
      MOM_INFORM ("backup of old shared object to %s", pathbuf);
    }
  pthread_mutex_unlock (&backup_mtx);
}

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
    MOM_DEBUG (run,							\
		    "momcode_proc_compilation setstate " #St " = %d",	\
		    (int)pcos_##St);					\
    return pcos_##St; } while(0)
  MOM_DEBUG (run, "begin momcode_proc_compilation state=%d", state);
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
	MOM_DEBUG (run,
		   "momcode_proc_compilation start n_procstatus=%ld",
		   (long) n_procstatus);
	if (l_endreason.panyitem == (mom_anyitem_t *) mom_item__exited
	    && n_procstatus == 0)
	  {
	    MOM_DEBUG (run,
		       "momcode_proc_compilation make process succeeded");
	    SET_STATE (loadnewmodule);
	  }
	else if (l_endreason.panyitem == (mom_anyitem_t *) mom_item__exited
		 && n_procstatus != 0)
	  {
	    MOM_WARNING
	      ("momcode_proc_compilation make process failed, exit code=%ld",
	       (long) n_procstatus);
	    return routres_pop;
	  }
	else if (l_endreason.panyitem ==
		 (mom_anyitem_t *) mom_item__terminated)
	  {
	    MOM_WARNING
	      ("momcode_proc_compilation make process terminated with signal#%d = %s",
	       (int) n_procstatus, strsignal (n_procstatus));
	    return routres_pop;
	  }
	else
	  MOM_FATAL ("momcode_proc_compilation bad endreason");
      }
      break;
      ////////////////
    case pcos_loadnewmodule:	////================ load new module
      {
	goodstate = true;
	MOM_DEBUG (run, "momcode_proc_compilation loadnewmodule");
	mom_request_stop ("proc_compilation loadnewmodule",
			  (mom_post_runner_sig_t *) mom_load_code_post_runner,
			  GENERATED_BASE_NAME);
	MOM_DEBUG (run,
		   "momcode_proc_compilation stop to load "
		   GENERATED_BASE_NAME);
	return routres_pop;
      }
      break;
      ////////////////
    case pcos__laststate:
      {
	MOM_FATAL ("momcode_proc_compilation unexpected last");
      }
    }
  if (!goodstate)
    MOM_FATAL ("momcode_proc_compilation invalid state %d", state);
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
  .rout_code = (const momrout_sig_t *) momcode_proc_compilation,
  .rout_timestamp = __DATE__ "@" __TIME__
};






////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
enum web_ajax_routine_values_en
{
  ajrv_arg0res,
  ajrv_web,
  ajrv_module,
  ajrv_routines,
  ajrv_dashboard,
  ajrv_buffer,
  ajrv_curout,
  ajrv_curprep,
  ajrv_routdata,
  ajrv_routemp,
  ajrv_curemit,
  ajrv_compilproc,
  ajrv__lastvalue
};

enum web_ajax_routine_closure_en
{
  ajrc_aftercompilation,
  ajrc__lastclosed
};

enum web_ajax_routine_numbers_en
{
  ajrn_ix,
  ajrn__lastnumber
};

int
momcode_ajax_routine (int state, momit_tasklet_t * tasklet,
		      momclosure_t * closure, momval_t * locvals,
		      intptr_t * locnums, double *locdbls)
{
  enum ajax_routine_state_en
  {
    ajrs_start,
    ajrs_compile_compute_routines,
    ajrs_compile_got_routines,
    ajrs_compile_begin_emission,
    ajrs_compile_preparation_loop,
    ajrs_compile_prepare_routine,
    ajrs_compile_emission_loop,
    ajrs_compile_got_preparation,
    ajrs_compile_declare_routine,
    ajrs_compile_got_emitter,
    ajrs_compile_emit_routine,
    ajrs_compile_run_compiler,
    ajrs__laststate
  };
#define SET_STATE(St) do {					\
    MOM_DEBUG (run,						\
	       "momcode_ajax_routine setstate " #St " = %d",	\
	       (int)ajrs_##St);					\
    return ajrs_##St; } while(0)
#define _L(N) locvals[ajrv_##N]
#define _C(N) closure->sontab[ajrc_##N]
#define _N(N) locnums[ajrn_##N]
  MOM_DEBUG (run, "momcode_ajax_routine start state=%d", state);
  bool goodstate = false;
  switch ((enum ajax_routine_state_en) state)
    {
      ////////////////
    case ajrs_start:
      goodstate = true;
      {
	_L (web) = _L (arg0res);
	MOM_DBG_VALUE (run, "ajax_routine web=", _L (web));
	if (mom_item_webrequest_method (_L (web)).ptr ==
	    ((momval_t) mom_item__POST).ptr)
	  {
	    MOM_DEBUG (web, "momcode_ajax_routine POST");
	    MOM_DBG_VALUE (web, "ajax_routine jsobpost=",
			   mom_item_webrequest_jsob_post (_L (web)));
	    momval_t idw = mom_item_webrequest_post_arg (_L (web), "id");
	    if (mom_same_string (idw, "routine_compile_id"))
	      {
		MOM_DEBUG (web, "ajax_routine routine_compile_id");
		_L (module) = (momval_t) mom_item__first_module;
		_L (routines) =
		  (momval_t)
		  mom_item_get_attr (mom_value_as_item (_L (module)),
				     (mom_anyitem_t *) mom_item__routines);
		MOM_DBG_VALUE (web, "ajax_routine compile routines=",
			       _L (routines));
		if (mom_type (_L (routines)) == momty_closure)
		  SET_STATE (compile_compute_routines);
		else if (mom_type (_L (routines)) == momty_set)
		  SET_STATE (compile_begin_emission);
		else
		  {
		    MOM_WARNING ("no routines in first_module");
		    _L (routines) = MOM_NULLV;
		    SET_STATE (compile_begin_emission);
		  }
	      }
	    else if (mom_same_string (idw, "routine_add_id"))
	      {
		MOM_DEBUG (web, "ajax_routine routine_add_id");
		MOM_WARNING ("unimplemented ajax_routine routine_add_id");
	      }
	    else if (mom_same_string (idw, "routine_remove_id"))
	      {
		MOM_DEBUG (web, "ajax_routine routine_remove_id");
		MOM_WARNING ("unimplemented ajax_routine routine_remove_id");
	      }
	    else if (mom_same_string (idw, "routine_edit_id"))
	      {
		MOM_DEBUG (web, "ajax_routine routine_edit_id");
		MOM_WARNING ("unimplemented ajax_routine routine_edit_id");
	      }
	  }
      }
      break;
      ////////////////
    case ajrs_compile_compute_routines:	////================ compile compute routines
      goodstate = true;
      {
	MOM_DBG_VALUE (web,
		       "ajax_routine compile compute_routines clo routines=",
		       _L (routines));
	mom_tasklet_push_frame ((momval_t) tasklet, (momval_t) _L (routines),
				MOMPFR_VALUE, _L (module), MOMPFR_END);
	SET_STATE (compile_got_routines);
      }
      break;
      ////////////////
    case ajrs_compile_got_routines:	////================ compile got_routines
      goodstate = true;
      {
	_L (routines) = _L (arg0res);
	MOM_DBG_VALUE (web, "ajax_routine compile got_routine routines=",
		       _L (routines));
	if (mom_type (_L (routines)) != momty_set)
	  {
	    MOM_WARNING ("ajax_routine compile got bad routines");
	    _L (routines) = MOM_NULLV;
	  }
	SET_STATE (compile_begin_emission);
      }
      break;
      ////////////////
    case ajrs_compile_begin_emission:	////================ compile begin_emission
      goodstate = true;
      {
	_L (dashboard) = (momval_t) mom_make_item_assoc (MOM_SPACE_NONE);
	_L (buffer) = (momval_t) mom_make_item_buffer (MOM_SPACE_NONE);
	MOM_DBG_VALUE (web, "ajax_routine compile begin_emission dashboard",
		       _L (dashboard));
	MOM_DBG_VALUE (web, "ajax_routine compile begin_emission buffer",
		       _L (buffer));
	mom_item_buffer_puts (_L (buffer),
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
	mom_item_buffer_printf (_L (buffer), "// generated on %s\n\n",
				nowdate);
	mom_item_buffer_printf
	  (_L (buffer),
	   "/**   Copyright (C) %s Free Software Foundation, Inc.\n"
	   " MONIMELT is a monitor for MELT - see http://gcc-melt.org/\n"
	   " This generated file is part of GCC.\n" "\n"
	   " GCC is free software; you can redistribute it and/or modify\n"
	   " it under the terms of the GNU General Public License as published by\n"
	   " the Free Software Foundation; either version 3, or (at your option)\n"
	   " any later version.\n" "\n"
	   " GCC is distributed in the hope that it will be useful,\n"
	   " but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	   " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	   " GNU General Public License for more details.\n"
	   " You should have received a copy of the GNU General Public License\n"
	   " along with GCC; see the file COPYING3.   If not see\n"
	   " <http://www.gnu.org/licenses/>.\n" "**/\n\n"
	   "#" "include \"monimelt.h\"\n\n", nowyear);
	_N (ix) = 0;
	SET_STATE (compile_preparation_loop);
      }
      break;
    case ajrs_compile_preparation_loop:	////================ compile preparation_loop
      goodstate = true;
      {
	MOM_DEBUG (web, "ajax_routine compile preparation_loop ix=%ld",
		   (long) _N (ix));
	if (_N (ix) > (long) mom_set_cardinal (_L (routines)))
	  {
	    mom_item_buffer_printf
	      (_L (buffer), "\n\n"
	       "// prepared %ld routines\n",
	       (long) mom_set_cardinal (_L (routines)));
	    _N (ix) = 0;
	    SET_STATE (compile_emission_loop);
	  }
	else
	  {
	    _L (curout) =
	      (momval_t) mom_set_nth_item (_L (routines), _N (ix));
	    MOM_DBG_VALUE (web,
			   "ajax_routine compile preparation_loop curout=",
			   _L (curout));
	    _N (ix)++;
	    if (mom_value_as_item (_L (curout)) != NULL)
	      SET_STATE (compile_prepare_routine);
	    else
	      SET_STATE (compile_preparation_loop);
	  }
      }
      break;
    case ajrs_compile_prepare_routine:	////================ compile prepare_routine
      goodstate = true;
      {
	MOM_DEBUG (web, "ajax_routine compile prepare_routine ix=%ld",
		   (long) _N (ix));
	// get the preparator in the routine, or else in the module
	_L (curprep) =
	  (momval_t) mom_item_get_attr (mom_value_as_item (_L (curout)),
					(mom_anyitem_t *)
					mom_item__routine_preparator);
	if (mom_type (_L (curprep)) != momty_closure)
	  _L (curprep) =
	    (momval_t) mom_item_get_attr (mom_value_as_item (_L (module)),
					  (mom_anyitem_t *)
					  mom_item__routine_preparator);
	if (mom_type (_L (curprep)) == momty_closure)
	  {
	    MOM_DBG_VALUE (web,
			   "ajax_routine compile prepare_routine curprep=",
			   _L (curprep));
	    mom_tasklet_push_frame ((momval_t) tasklet,
				    (momval_t) _L (curprep),
				    MOMPFR_FOUR_VALUES, _L (curout),
				    _L (dashboard), _L (buffer), _L (module),
				    MOMPFR_END);
	    SET_STATE (compile_got_preparation);
	  }
	else
	  {
	    SET_STATE (compile_declare_routine);
	  }
      }
      break;
      ////////////////****************
    case ajrs_compile_got_preparation:	////================ compile got_preparation
      goodstate = true;
      {
	MOM_DBG_VALUE (web, "ajax_routine compile got_preparation arg0res=",
		       _L (arg0res));
	if (_L (arg0res).ptr)
	  SET_STATE (compile_declare_routine);
	else
	  SET_STATE (compile_preparation_loop);
      }
      break;
      ////////////////****************
    case ajrs_compile_declare_routine:	////================ compile declare_routine
      goodstate = true;
      {
	// should emit the declaration of the routine
	const char *cnam = c_name_suffix (mom_value_as_item (_L (curout)));
	assert (cnam != NULL);
	_L (routdata) = mom_item_assoc_get1 (_L (dashboard), _L (curout));
	if (!_L (routdata).ptr)
	  _L (routdata) = _L (curout);
	mom_item_buffer_printf
	  (_L (buffer), "\n"
	   "// declaration of code for %s\n"
	   "int momcode_%s (int, momit_tasklet_t *, momclosure_t *,\n"
	   "       momval_t *,intptr_t *, double *);\n" C_FROM, cnam, cnam);
	momval_t statev = MOM_NULLV, closurev = MOM_NULLV;
	momval_t valuesv = MOM_NULLV, numbersv = MOM_NULLV;
	momval_t doublesv = MOM_NULLV;
	mom_item_get_several_attrs (mom_value_as_item (_L (routdata)),
				    mom_item__state, &statev,
				    mom_item__closure, &closurev,
				    mom_item__values, &valuesv,
				    mom_item__numbers, &numbersv,
				    mom_item__doubles, &doublesv, NULL);
	mom_item_buffer_printf
	  (_L (buffer),
	   "\n" "// routine descriptor for %s\n"
	   "const struct momroutinedescr_st momrout_%s = { .rout_magic = ROUTINE_MAGIC, \n"
	   " .rout_minclosize = %d,\n"
	   " .rout_frame_nbval = %d,\n"
	   " .rout_frame_nbnum = %d,\n"
	   " .rout_frame_nbdbl = %d,\n"
	   " .rout_name = \"%s\",\n"
	   " .rout_code = (const momrout_sig_t *) momcode_%s,\n"
	   " .rout_timestamp = __DATE__ \"@\" __TIME__\n"
	   C_FROM "};\n", cnam, cnam,
	   mom_seqitem_length (closurev),
	   mom_seqitem_length (valuesv),
	   mom_seqitem_length (numbersv),
	   mom_seqitem_length (doublesv), cnam, cnam);
	SET_STATE (compile_preparation_loop);
      }
      break;
      ////////////////****************
    case ajrs_compile_emission_loop:	////================ compile emission_loop
      goodstate = true;
      {
	MOM_DEBUG (web, "ajax_routine compile emission_loop ix=%ld",
		   (long) _N (ix));
	if (_N (ix) > (long) mom_set_cardinal (_L (routines)))
	  {
	    mom_item_buffer_printf
	      (_L (buffer), "\n\n"
	       "// emitted %ld routines\n" C_FROM,
	       (long) mom_set_cardinal (_L (routines)));
	    _N (ix) = 0;
	    SET_STATE (compile_run_compiler);
	  }
	_L (curout) = (momval_t) mom_set_nth_item (_L (routines), _N (ix));
	MOM_DBG_VALUE (web, "ajax_routine compile emission_loop curout=",
		       _L (curout));
	_N (ix)++;
	if (mom_value_as_item (_L (curout)) != NULL)
	  SET_STATE (compile_emit_routine);
	else
	  SET_STATE (compile_emission_loop);
      }
      break;
      ////////////////****************
    case ajrs_compile_emit_routine:	////================ compile emit_routine
      goodstate = true;
      {
	const char *cnam = c_name_suffix (mom_value_as_item (_L (curout)));
	MOM_DEBUG (web, "ajax_routine compile emit routine cnam %s", cnam);
	MOM_DBG_VALUE (web, "ajax_routine compile emit routine curout",
		       _L (curout));
	_L (routemp) = (momval_t) mom_make_item_assoc (MOM_SPACE_NONE);
	_L (curemit) =
	  (momval_t) mom_item_get_attr (mom_value_as_item (_L (curout)),
					(mom_anyitem_t *)
					mom_item__routine_emitter);
	if (mom_type (_L (curemit)) != momty_closure)
	  _L (curemit) =
	    (momval_t) mom_item_get_attr (mom_value_as_item (_L (module)),
					  (mom_anyitem_t *)
					  mom_item__routine_emitter);
	MOM_DBG_VALUE (web, "ajax_routine compile emit routine curemit=",
		       _L (curemit));
	if (mom_type (_L (curemit)) == momty_closure)
	  {
	    mom_tasklet_push_frame
	      ((momval_t) tasklet, (momval_t) _L (curemit),
	       MOMPFR_FOUR_VALUES, _L (curout), _L (routdata), _L (routemp),
	       _L (dashboard), MOMPFR_TWO_VALUES, _L (buffer), _L (module),
	       MOMPFR_END);
	    SET_STATE (compile_got_emitter);
	  }
	else
	  {
	    MOM_WARNING ("no routine emitter for %s", cnam);
	    return routres_pop;
	  }
      }
      break;
      ////////////////****************
    case ajrs_compile_got_emitter:	////================ compile got_emitter
      goodstate = true;
      {
	// essentially a no-op
	goodstate = true;
	const char *cnam = c_name_suffix (mom_value_as_item (_L (curout)));
	MOM_DEBUG (web, "web_form_compile got emitter routine %s", cnam);
	SET_STATE (compile_emission_loop);
      }
      break;
      ////////////////****************
    case ajrs_compile_run_compiler:	////================ compile run_compiler
      goodstate = true;
      {
	mom_item_buffer_printf (_L (buffer),
				"\n\n///// end of %d routines \n\n"
				"/*** eof " GENERATED_SOURCE_FILE_NAME
				" ****/\n" C_FROM, (int) _N (ix));
	MOM_DEBUG (web,
		   "ajax_routine compiler run compiler buffer of %d bytes",
		   (int) mom_item_buffer_length (_L (buffer)));
	/// write the buffer
	rename (GENERATED_SOURCE_FILE_NAME, GENERATED_SOURCE_FILE_NAME "~");
	FILE *fout = fopen (GENERATED_SOURCE_FILE_NAME, "w");
	if (MOM_UNLIKELY (!fout))
	  MOM_FATAL ("failed to open file " GENERATED_SOURCE_FILE_NAME);
	unsigned blen = mom_item_buffer_length (_L (buffer));
	if (mom_item_buffer_output_content_to_file (_L (buffer), fout)
	    != (int) blen)
	  MOM_WARNING ("failed to output all %d bytes to "
		       GENERATED_SOURCE_FILE_NAME, (int) blen);
	MOM_INFORM ("wrote %d bytes of code into %s", (int) blen,
		    GENERATED_SOURCE_FILE_NAME);
	backup_old_shared_object ();
	fclose (fout), fout = NULL;
	/// create and start the compilation process
	_L (compilproc) = (momval_t)
	  mom_make_item_process_argvals ((momval_t) mom_make_string ("make"),
					 (momval_t)
					 mom_make_string
					 (GENERATED_SHAROB_FILE_NAME), NULL);
	MOM_DBG_VALUE (web, "ajax_routine compile compilproc=",
		       _L (compilproc));
	MOM_DBG_VALUE (web, "ajax_routine compile aftercompilation=",
		       _C (aftercompilation));
	mom_item_process_start (_L (compilproc), _C (aftercompilation));
	MOM_DEBUG (web, "ajax_routine compile after compilation start");
	char timbuf[64] = { };
	memset (timbuf, 0, sizeof (timbuf));
	struct tm curtm = { };
	time_t now = 0;
	time (&now);
	strftime (timbuf, sizeof (timbuf), "%c", localtime_r (&now, &curtm));
	/// answer the Ajax web request
	mom_item_webrequest_add
	  (_L (web),
	   MOMWEB_SET_MIME, "text/html",
	   MOMWEB_LIT_STRING,
	   "<b>Monimelt generates &amp; compile code</b> in <tt>"
	   GENERATED_SOURCE_FILE_NAME "</tt> with ", MOMWEB_DEC_LONG,
	   (long) _N (ix), MOMWEB_LIT_STRING, " routines and ",
	   MOMWEB_DEC_LONG, (long) blen, MOMWEB_LIT_STRING,
	   " bytes <small>at ", MOMWEB_LIT_STRING, timbuf, MOMWEB_LIT_STRING,
	   "</small>.</p>\n" HTML_FROM, MOMWEB_REPLY_CODE, HTTP_OK,
	   MOMWEB_END);
	return routres_pop;
      }
      break;
      ////////////////*************
      ////////////////
    case ajrs__laststate:
      MOM_FATAL ("momcode_ajax_routine impossible last state %d", state);
    }
  if (!goodstate)
    MOM_FATAL ("momcode_ajax_routine invalid state %d", state);
  MOM_DEBUG (web, "momcode_ajax_routine final popping");
  return routres_pop;
#undef SET_STATE
#undef _L
#undef _C
#undef _N
}


const struct momroutinedescr_st momrout_ajax_routine =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = (unsigned) ajrc__lastclosed,
  .rout_frame_nbval = (unsigned) ajrv__lastvalue,
  .rout_frame_nbnum = (unsigned) ajrn__lastnumber,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_routine",
  .rout_code = (const momrout_sig_t *) momcode_ajax_routine,
  .rout_timestamp = __DATE__ "@" __TIME__
};

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////

#warning web_form_handle_routine is removed

/*****************/
#if 0
enum formhandlevalues_en
{
  wfhv_web,
  wfhv_routine,
  wfhv__lastval
};
int
momcode_web_form_handle_routine (int state, momit_tasklet_t * tasklet,
				 momclosure_t * closure, momval_t * locvals,
				 intptr_t * locnums, double *locdbls)
{
#define _L(N) locvals[wfhv_##N]
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MOM_DEBUG (web,
	     "momcode_web_form_handle_routine state=%d webnum=%ld nowbuf=%s",
	     state, mom_item_webrequest_webnum (_L (web)), nowbuf);
  MOM_DBG_ITEM (web, "web_form_handle_routine tasklet=",
		(const mom_anyitem_t *) tasklet);
  MOM_DBG_VALUE (web, "web_form_handle_routine web=", _L (web));
  MOM_DBG_VALUE (web, "web_form_handle_routine closure=",
		 (momval_t) (const momclosure_t *) closure);
  MOM_DBG_VALUE (web, "web_form_handle_routine method=",
		 (momval_t) mom_item_webrequest_method (_L (web)));
  if (mom_item_webrequest_method (_L (web)).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MOM_DEBUG (web, "web_form_handle_routine POST");
      MOM_DBG_VALUE (web, "web_form_handle_routine jsobpost=",
		     mom_item_webrequest_jsob_post (_L (web)));
      momval_t namestrv =
	mom_item_webrequest_post_arg (_L (web), "routine_name");
      _L (routine) = (momval_t) mom_item_of_name_string (namestrv);
      MOM_DBG_VALUE (web, "web_form_handle_routine routine=", _L (routine));
      if (mom_type (_L (routine)) != momty_routineitem)
	{
	  MOM_WARNING ("web_form_handle_routine bad routine name %s",
		       mom_string_cstr (namestrv));
	  mom_item_webrequest_add
	    (_L (web), MOMWEB_SET_MIME, "text/html",
	     MOMWEB_LIT_STRING,
	     "<!doctype html><head><title>Bad Routine Name in Monimelt</title></head>\n"
	     "<body><h1>Bad Routine Name</h1>\n",
	     MOMWEB_LIT_STRING, "<p>Name <tt>",
	     MOMWEB_HTML_STRING,
	     mom_string_cstr (namestrv),
	     MOMWEB_LIT_STRING,
	     "</tt> not found at <i>",
	     MOMWEB_HTML_STRING, nowbuf,
	     MOMWEB_LIT_STRING,
	     "</i>.</p></body></html>\n",
	     MOMWEB_LIT_STRING, HTML_FROM,
	     MOMWEB_REPLY_CODE, HTTP_NOT_FOUND, MOMWEB_END);
	  return routres_pop;
	}
      if (mom_item_webrequest_post_arg (_L (web), "do_addrout").ptr)
	{
	  MOM_DEBUG (web, "web_form_handle_routine adding routine");
	  momval_t oldset =
	    (momval_t) mom_item_get_attr ((mom_anyitem_t *)
					  mom_item__first_module,
					  (mom_anyitem_t *)
					  mom_item__routines);
	  MOM_DBG_VALUE (web,
			 "web_form_handle_routine old set of routines in first module=",
			 oldset);
	  momval_t newset =
	    (momval_t) mom_make_set_til_nil (_L (routine), oldset, NULL);
	  MOM_DBG_VALUE (web,
			 "web_form_handle_routine new grown set of routines in first module=",
			 newset);
	  mom_item_put_attr ((mom_anyitem_t *) mom_item__first_module,
			     (mom_anyitem_t *) mom_item__routines, newset);
	  mom_item_webrequest_add
	    (_L (web), MOMWEB_SET_MIME, "text/html",
	     MOMWEB_LIT_STRING,
	     "<!doctype html><head><title>Grown first_module"
	     " in Monimelt</title></head>\n"
	     "<body><h1>Update grown <tt>first_module</tt></h1>\n",
	     MOMWEB_LIT_STRING, "<p>Routine named <tt>",
	     MOMWEB_HTML_STRING,
	     mom_string_cstr (namestrv),
	     MOMWEB_LIT_STRING,
	     "</tt> added, now having ",
	     MOMWEB_DEC_LONG,
	     (long) mom_set_cardinal (newset),
	     MOMWEB_LIT_STRING, " routines, at <i>",
	     MOMWEB_HTML_STRING, nowbuf,
	     MOMWEB_LIT_STRING,
	     "</i>.</p></body></html>\n",
	     MOMWEB_LIT_STRING, HTML_FROM,
	     MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  return routres_pop;
	}
      else if (mom_item_webrequest_post_arg (_L (web), "do_removerout").ptr)
	{
	  MOM_DEBUG (web, "web_form_handle_routine removing routine");
	  momval_t oldset =
	    (momval_t) mom_item_get_attr ((mom_anyitem_t *)
					  mom_item__first_module,
					  (mom_anyitem_t *)
					  mom_item__routines);
	  bool waspresent = mom_set_contains (oldset, _L (routine).panyitem);
	  MOM_DBG_VALUE (web, "old set of routines in first module=", oldset);
	  if (waspresent)
	    {
	      momval_t newset = mom_make_set_without (oldset, _L (routine));
	      MOM_DBG_VALUE (web,
			     "new smaller set of routines in first module=",
			     newset);
	      mom_item_put_attr ((mom_anyitem_t *) mom_item__first_module,
				 (mom_anyitem_t *) mom_item__routines,
				 newset);
	      mom_item_webrequest_add
		(_L (web), MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING,
		 "<!doctype html><head><title>Shrinken first_module"
		 " in Monimelt</title></head>\n"
		 "<body><h1>Update shrinken <tt>first_module</tt></h1>\n",
		 MOMWEB_LIT_STRING,
		 "<p>Routine named <tt>",
		 MOMWEB_HTML_STRING,
		 mom_string_cstr (namestrv),
		 MOMWEB_LIT_STRING,
		 "</tt> removed, now having ",
		 MOMWEB_DEC_LONG,
		 (long) mom_set_cardinal (newset),
		 MOMWEB_LIT_STRING, " routines, at <i>",
		 MOMWEB_HTML_STRING, nowbuf,
		 MOMWEB_LIT_STRING,
		 "</i>.</p></body></html>\n",
		 MOMWEB_LIT_STRING, HTML_FROM,
		 MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	      return routres_pop;
	    }
	  else
	    {
	      MOM_WARNING ("web_form_handle_routine missing routine name %s",
			   mom_string_cstr (namestrv));
	      mom_item_webrequest_add
		(_L (web), MOMWEB_SET_MIME, "text/html",
		 MOMWEB_LIT_STRING,
		 "<!doctype html><head><title>Missing Routine in Monimelt</title></head>\n"
		 "<body><h1>Missing Routine Name</h1>\n",
		 MOMWEB_LIT_STRING, "<p>Name <tt>",
		 MOMWEB_HTML_STRING,
		 mom_string_cstr (namestrv),
		 MOMWEB_LIT_STRING,
		 "</tt> not found in <tt>first_module</tt> at <i>",
		 MOMWEB_HTML_STRING, nowbuf,
		 MOMWEB_LIT_STRING,
		 "</i>.</p></body></html>\n",
		 MOMWEB_LIT_STRING, HTML_FROM,
		 MOMWEB_REPLY_CODE, HTTP_NOT_FOUND, MOMWEB_END);
	      return routres_pop;
	    }
	}
      else if (mom_item_webrequest_post_arg (_L (web), "do_editrout").ptr)
	{
	  MOM_DEBUG (web, "web_form_handle_routine editrout");
	  momval_t statev = MOM_NULLV, closurev = MOM_NULLV;
	  momval_t valuesv = MOM_NULLV, numbersv = MOM_NULLV;
	  momval_t doublesv = MOM_NULLV;
	  mom_item_get_several_attrs (mom_value_as_item (_L (routine)),
				      mom_item__state, &statev,
				      mom_item__closure, &closurev,
				      mom_item__values, &valuesv,
				      mom_item__numbers, &numbersv,
				      mom_item__doubles, &doublesv, NULL);
	  MOM_DBG_VALUE (web, "web_form_handle_routine got statev=", statev);
	  MOM_DBG_VALUE (web, "web_form_handle_routine got closurev=",
			 closurev);
	  MOM_DBG_VALUE (web, "web_form_handle_routine got valuesv=",
			 valuesv);
	  MOM_DBG_VALUE (web, "web_form_handle_routine got numbersv=",
			 numbersv);
	  MOM_DBG_VALUE (web, "web_form_handle_routine got doublesv=",
			 doublesv);
	  if (!statev.ptr)
	    {
	      statev =
		(momval_t) mom_make_node_til_nil ((mom_anyitem_t *)
						  mom_item__state, NULL);
	      mom_item_put_attr (mom_value_as_item (_L (routine)),
				 (mom_anyitem_t *) mom_item__state, statev);
	      MOM_DBG_VALUE (web, "web_form_handle_routine new statev=",
			     statev);
	    }
	  if (!closurev.ptr)
	    {
	      closurev = (momval_t) mom_make_tuple_sized (0, NULL);
	      mom_item_put_attr (mom_value_as_item (_L (routine)),
				 (mom_anyitem_t *) mom_item__closure,
				 closurev);
	      MOM_DBG_VALUE (web, "web_form_handle_routine new closurev=",
			     statev);
	    }
	  if (!valuesv.ptr)
	    {
	      valuesv = (momval_t) mom_make_tuple_sized (0, NULL);
	      mom_item_put_attr (mom_value_as_item (_L (routine)),
				 (mom_anyitem_t *) mom_item__values, valuesv);
	      MOM_DBG_VALUE (web, "web_form_handle_routine new valuesv=",
			     valuesv);
	    }
	  if (!numbersv.ptr)
	    {
	      numbersv = (momval_t) mom_make_tuple_sized (0, NULL);
	      mom_item_put_attr (mom_value_as_item (_L (routine)),
				 (mom_anyitem_t *) mom_item__numbers,
				 numbersv);
	      MOM_DBG_VALUE (web, "web_form_handle_routine new numbersv=",
			     numbersv);
	    }
	  if (!doublesv.ptr)
	    {
	      doublesv = (momval_t) mom_make_tuple_sized (0, NULL);
	      mom_item_put_attr (mom_value_as_item (_L (routine)),
				 (mom_anyitem_t *) mom_item__doubles,
				 doublesv);
	      MOM_DBG_VALUE (web, "web_form_handle_routine new doublesv=",
			     doublesv);
	    }
#warning momcode_web_form_handle_routine incomplete should do_editrout
	  MOM_DEBUG (web, "begin routine addition");
	  // we should edit the DOM to add the appropriate elements in it.
	  mom_item_webrequest_add
	    (_L (web), MOMWEB_SET_MIME,
	     "application/javascript",
	     MOMWEB_LIT_STRING, "put_edited_routine('",
	     MOMWEB_HTML_STRING,
	     mom_string_cstr (namestrv),
	     MOMWEB_LIT_STRING, "');\n",
	     MOMWEB_LIT_STRING, JS_FROM, MOMWEB_END);
	  MOM_WARNING ("momcode_web_form_handle_routine incomplete");
	  mom_item_webrequest_add
	    (_L (web), MOMWEB_LIT_STRING,
	     "// end of routine edition\n",
	     MOMWEB_LIT_STRING, JS_FROM,
	     MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  MOM_DEBUG (web, "end of routine addition");
	}
    }
  return routres_pop;
#undef _L
}

const struct momroutinedescr_st momrout_web_form_handle_routine =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = wfhv__lastval,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "web_form_handle_routine",
  .rout_code = (const momrout_sig_t *) momcode_web_form_handle_routine,
  .rout_timestamp = __DATE__ "@" __TIME__
};

#endif /*0 */


/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

enum cold_routine_emit_values_en
{
  crev_arg0res,
  crev_routdata,
  crev_routemp,
  crev_dashboard,
  crev_buffer,
  crev_module,
  crev_savecurout,
  crev_xstate,
  crev_xclosure,
  crev_xvalues,
  crev_xnumbers,
  crev_xdoubles,
  crev__lastval
};

enum cold_routine_emit_closure_en
{
  crec__lastclosure
};

enum cold_routine_emit_numbers_en
{
  cren_nbstates,
  cren_nbclosed,
  cren_nbvalues,
  cren_nbnumbers,
  cren_nbdoubles,
  cren__lastnum
};


int
momcode_cold_routine_emit (int state, momit_tasklet_t * tasklet,
			   momclosure_t * closure, momval_t * locvals,
			   intptr_t * locnums, double *locdbls)
{
  enum cold_routine_emit_state_en
  {
    cres_start,
    cres_epilog,
    cres__laststate
  };
#define SET_STATE(St) do {						\
    MOM_DEBUG (run,							\
	       "momcode_cold_routine_emit setstate " #St " = %d",	\
	       (int)cres_##St);						\
    return cres_##St; } while(0)
#define _L(N) locvals[crev_##N]
#define _C(N) closure->sontab[crec_##N]
#define _N(N) locnums[cren_##N]
  bool goodstate = false;
  MOM_DEBUG (run, "cold_routine_emit start state=%d", state);
  switch ((enum cold_routine_emit_state_en) state)
    {
      ////////////////
    case cres_start:		////================ start
      {
	goodstate = true;
	_L (savecurout) = _L (arg0res);
	MOM_DBG_VALUE (run, "cold_routine_emit savecurout=", _L (savecurout));
	const char *cnam =
	  c_name_suffix (mom_value_as_item (_L (savecurout)));
	MOM_DBG_VALUE (run, "cold_routine_emit routdata=", _L (routdata));
	MOM_DEBUG (run, "cold_routine_emit cnam=%s", cnam);
	mom_item_get_several_attrs (mom_value_as_item (_L (routdata)),
				    mom_item__state, &_L (xstate),
				    mom_item__closure, &_L (xclosure),
				    mom_item__values, &_L (xvalues),
				    mom_item__numbers, &_L (xnumbers),
				    mom_item__doubles, &_L (xdoubles), NULL);
	if (_L (xstate).ptr == NULL)
	  _L (xstate) =
	    mom_item_get_attr (mom_value_as_item (_L (savecurout)),
			       (mom_anyitem_t *) mom_item__state);
	if (_L (xclosure).ptr == NULL)
	  _L (xclosure) =
	    mom_item_get_attr (mom_value_as_item (_L (savecurout)),
			       (mom_anyitem_t *) mom_item__closure);
	if (_L (xvalues).ptr == NULL)
	  _L (xvalues) =
	    mom_item_get_attr (mom_value_as_item (_L (savecurout)),
			       (mom_anyitem_t *) mom_item__values);
	if (_L (xnumbers).ptr == NULL)
	  _L (xnumbers) =
	    mom_item_get_attr (mom_value_as_item (_L (savecurout)),
			       (mom_anyitem_t *) mom_item__numbers);
	if (_L (xdoubles).ptr == NULL)
	  _L (xdoubles) =
	    mom_item_get_attr (mom_value_as_item (_L (savecurout)),
			       (mom_anyitem_t *) mom_item__doubles);
	MOM_DBG_VALUE (run, "cold_routine_emit xstate=", _L (xstate));
	MOM_DBG_VALUE (run, "cold_routine_emit xclosure=", _L (xclosure));
	MOM_DBG_VALUE (run, "cold_routine_emit xvalues=", _L (xvalues));
	MOM_DBG_VALUE (run, "cold_routine_emit xnumbers=", _L (xnumbers));
	MOM_DBG_VALUE (run, "cold_routine_emit xdoubles=", _L (xdoubles));
	mom_item_buffer_printf
	  (_L (buffer), "\n\n\n"
	   "//// *** implementation of code for %s\n"
	   "int momcode_%s (int momp_state, momit_tasklet_t *momp_tasklet, momclosure_t *momp_closure,\n"
	   "       momval_t *momp_locvals, intptr_t *momp_locnums, double *momp_locdoubles) {\n",
	   cnam, cnam);
	if (mom_node_conn (_L (xstate)) != (mom_anyitem_t *) mom_item__state)
	  {
	    mom_item_buffer_printf (_L (buffer),
				    "#error strange state in %s\n", cnam);
	    MOM_WARNING ("cold_routine_emit: strange state in %s", cnam);
	  }
	_N (nbstates) = mom_node_arity (_L (xstate));
	MOM_DEBUG (run, "cold_routine_emit nbstates=%d", (int) _N (nbstates));
	/////
	/// check the states
	for (unsigned stix = 0; stix < (unsigned) _N (nbstates); stix++)
	  {
	    momval_t curstatev = mom_node_nth (_L (xstate), stix);
	    MOM_DEBUG (run, "cold_routine_emit stix=%d", (int) stix);
	    MOM_DBG_VALUE (run, "cold_routine_emit curstatev=", curstatev);
	    if (mom_node_conn (curstatev) !=
		(mom_anyitem_t *) mom_item__cold_state
		|| mom_node_arity (curstatev) < 2)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error strange cold state #%d in %s\n",
					(int) stix, cnam);
		MOM_WARNING
		  ("cold_routine_emit: strange cold state #%d in %s",
		   (int) stix, cnam);
	      }
	    mom_anyitem_t *curstateitm =
	      mom_value_as_item (mom_node_nth (curstatev, 0));
	    MOM_DBG_ITEM (run, "cold_routine_emit curstateitm=", curstateitm);
	    if (!curstateitm)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error bad current state item #%d in %s\n",
					(int) stix, cnam);
		MOM_WARNING
		  ("cold_routine_emit: bad current state item #%d in %s\n",
		   (int) stix, cnam);
	      }
	    momval_t curstatetext = mom_node_nth (curstatev, 1);
	    if (!mom_is_string (curstatetext)
		&& mom_type (curstatetext) != momty_bufferitem)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error bad current state string #%d in %s\n",
					(int) stix, cnam);
		MOM_WARNING
		  ("cold_routine_emit: bad current state item #%d in %s\n",
		   (int) stix, cnam);
	      }
	  }
	/////
	/// emit the state enumeration and #define-s
	mom_item_buffer_printf (_L (buffer),
				"// %d states enumeration in %s\n",
				(int) _N (nbstates), cnam);
	mom_item_buffer_printf (_L (buffer), "enum momstates_%s_en {\n",
				cnam);
	for (unsigned stix = 0; stix < (unsigned) _N (nbstates); stix++)
	  {
	    momval_t curstatev = mom_node_nth (_L (xstate), stix);
	    if (!curstatev.ptr)
	      continue;
	    mom_anyitem_t *curstateitm =
	      mom_value_as_item (mom_node_nth (curstatev, 0));
	    if (!curstateitm)
	      continue;
	    const char *namstatsuffix = c_name_suffix (curstateitm);
	    if (!namstatsuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer), " momstate%s_num%d_%s = %d,",
				    cnam, stix, namstatsuffix, stix);
	    mom_item_buffer_printf (_L (buffer),
				    "#define momsta_%s momstate%s_num%d_%s\n",
				    namstatsuffix, cnam, stix, namstatsuffix);
	  };
	mom_item_buffer_printf (_L (buffer),
				" momstate%s__laststate }; // end enum momstates_%s_en\n",
				cnam, cnam);
	mom_item_buffer_printf (_L (buffer),
				"#define GO(St) do { MOM_DEBUG (run, \\\n"
				"    \"%s go state \" #St \" = %%d\", (int)  momsta_##St); \\\n"
				"  return momsta_##St; } while(0)\n"
				"#define POP() do  { MOM_DEBUG (run, \\\n"
				"    \"%s pop state \"); return routres_pop; } while(0)\n"
				"#define STEADY() do  { MOM_DEBUG (run, \\\n"
				"    \"%s steady state \"); return routres_steady; } while(0)\n",
				cnam, cnam, cnam);
	//////
	// check and #define the local values
	if (_L (xvalues).ptr && !mom_is_seqitem (_L (xvalues)))
	  {
	    mom_item_buffer_printf (_L (buffer), "#error bad locals in %s\n",
				    cnam);
	    MOM_WARNING ("cold_routine_emit: bad local values in %s", cnam);
	  }
	_N (nbvalues) = mom_seqitem_length (_L (xvalues));
	MOM_DEBUG (run, "cold_routine_emit: nbvalues=%d",
		   (int) _N (nbvalues));
	for (unsigned lix = 0; lix < (unsigned) _N (nbvalues); lix++)
	  {
	    const mom_anyitem_t *curvalitm =
	      mom_seqitem_nth_item (_L (xvalues), lix);
	    MOM_DBG_ITEM (run, "cold_routine_emit: curvalitm=", curvalitm);
	    if (!curvalitm)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error bad local #%d in %s\n",
					(int) lix, cnam);
		MOM_WARNING ("cold_routine_emit: bad local value #%d in %s",
			     lix, cnam);
		continue;
	      };
	    const char *curvalsuffix = c_name_suffix (curvalitm);
	    mom_item_buffer_printf (_L (buffer),
				    "#define momloc_%s momp_locvals[%d]\n",
				    curvalsuffix, lix);
	    if (isalpha (curvalsuffix[0]))
	      mom_item_buffer_printf (_L (buffer), "#define %s  momloc_%s\n",
				      curvalsuffix, curvalsuffix);
	  }
	//////
	// check and #define the closed values
	if (_L (xclosure).ptr && !mom_is_seqitem (_L (xclosure)))
	  {
	    mom_item_buffer_printf (_L (buffer),
				    "#error bad closed values in %s\n", cnam);
	    MOM_WARNING ("cold_routine_emit: bad closed values in %s", cnam);
	  }
	_N (nbclosed) = mom_seqitem_length (_L (xclosure));
	MOM_DEBUG (run, "cold_routine_emit: nbclosed=%d",
		   (int) _N (nbclosed));
	for (unsigned cix = 0; cix < (unsigned) _N (nbclosed); cix++)
	  {
	    const mom_anyitem_t *curcloitm =
	      mom_seqitem_nth_item (_L (xclosure), cix);
	    MOM_DBG_ITEM (run, "cold_routine_emit: curcloitm=", curcloitm);
	    if (!curcloitm)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error bad closed value #%d in %s\n",
					(int) cix, cnam);
		MOM_WARNING ("cold_routine_emit: bad closed value #%d in %s",
			     cix, cnam);
		continue;
	      };
	    const char *curclosuffix = c_name_suffix (curcloitm);
	    if (!curclosuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer),
				    "#define momclo_%s momp_closure->sontab[%d]\n",
				    curclosuffix, cix);
	    if (curclosuffix && isalpha (curclosuffix[0]))
	      mom_item_buffer_printf (_L (buffer), "#define %s momclo_%s\n",
				      curclosuffix, curclosuffix);
	  }
	//////
	// check and #define the numbers
	if (_L (xnumbers).ptr && !mom_is_seqitem (_L (xnumbers)))
	  {
	    mom_item_buffer_printf (_L (buffer),
				    "#error bad local numbers in %s\n", cnam);
	    MOM_WARNING ("cold_routine_emit: bad local numbers in %s", cnam);
	  }
	_N (nbnumbers) = mom_seqitem_length (_L (xnumbers));
	MOM_DEBUG (run, "cold_routine_emit: nbnumbers=%d",
		   (int) _N (nbnumbers));
	for (unsigned nix = 0; nix < (unsigned) _N (nbnumbers); nix++)
	  {
	    const mom_anyitem_t *curnumitm =
	      mom_seqitem_nth_item (_L (xnumbers), nix);
	    MOM_DBG_ITEM (run, "cold_routine_emit: curnumitm=", curnumitm);
	    if (!curnumitm)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error bad local number #%d in %s\n",
					(int) nix, cnam);
		MOM_WARNING ("cold_routine_emit: bad local number #%d in %s",
			     nix, cnam);
		continue;
	      };
	    const char *curnumsuffix = c_name_suffix (curnumitm);
	    if (!curnumsuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer),
				    "#define momnum_%s momp_locnums[%d]\n",
				    curnumsuffix, (int) nix);
	    if (isalpha (curnumsuffix))
	      mom_item_buffer_printf (_L (buffer), "#define %s  momnum_%s\n",
				      curnumsuffix, curnumsuffix);
	  }
	//////
	// check and #define the doubles
	if (_L (xdoubles).ptr && !mom_is_seqitem (_L (xdoubles)))
	  {
	    mom_item_buffer_printf (_L (buffer), "#error bad doubles in %s\n",
				    cnam);
	    MOM_WARNING ("cold_routine_emit: bad local doubles in %s", cnam);
	  }
	_N (nbdoubles) = mom_seqitem_length (_L (xdoubles));
	MOM_DEBUG (run, "cold_routine_emit: nbdoubles=%d",
		   (int) _N (nbdoubles));
	for (unsigned dix = 0; dix < (unsigned) _N (nbdoubles); dix++)
	  {
	    const mom_anyitem_t *curdblitm =
	      mom_seqitem_nth_item (_L (xdoubles), dix);
	    MOM_DBG_ITEM (run, "cold_routine_emit: curdblitm=", curdblitm);
	    if (!curdblitm)
	      {
		mom_item_buffer_printf (_L (buffer),
					"#error bad local double #%d in %s\n",
					(int) dix, cnam);
		MOM_WARNING ("cold_routine_emit: bad local double #%d in %s",
			     dix, cnam);
		continue;
	      };
	    const char *curdblsuffix = c_name_suffix (curdblitm);
	    if (!curdblsuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer),
				    "#define momdbl_%s momp_locdoubles[%d]\n",
				    curdblsuffix, (int) dix);
	    if (isalpha (curdblsuffix))
	      mom_item_buffer_printf (_L (buffer), "#define %s  momdbl_%s\n",
				      curdblsuffix, curdblsuffix);
	  }
	/////
	// emit an assertion on the closure arity
	mom_item_buffer_printf (_L (buffer),
				"assert(momp_closure && mom_closure_arity(momp_closure) >= %d);\n",
				(int) _N (nbclosed));
	// emit the switch goto
	mom_item_buffer_printf (_L (buffer),
				" MOM_DEBUG(run, \"%s starting at state #%%d\", momp_state);\n",
				cnam);
	mom_item_buffer_printf (_L (buffer), " switch(momp_state) {\n");
	for (unsigned stix = 0; stix < (unsigned) _N (nbstates); stix++)
	  {
	    mom_item_buffer_printf (_L (buffer),
				    "  case %d: goto momlabstate_%d;\n", stix,
				    stix);
	  };
	mom_item_buffer_printf (_L (buffer),
				"  default: MOM_FATAL(\"%s: invalid state #%%d\", momp_state); } // end state switch\n",
				cnam);
	//////////////// ********
	// emit each state with its label;
	for (unsigned stix = 0; stix < (unsigned) _N (nbstates); stix++)
	  {
	    momval_t curstatev = mom_node_nth (_L (xstate), stix);
	    if (!curstatev.ptr)
	      continue;
	    mom_anyitem_t *curstateitm =
	      mom_value_as_item (mom_node_nth (curstatev, 0));
	    if (!curstateitm)
	      continue;
	    const char *namstatsuffix = c_name_suffix (curstateitm);
	    if (!namstatsuffix)
	      continue;
	    momval_t curstatetext = mom_node_nth (curstatev, 1);
	    unsigned typcurstatetext = mom_type (curstatetext);
	    if (typcurstatetext != momty_string
		&& typcurstatetext != momty_bufferitem)
	      continue;
	    mom_item_buffer_printf (_L (buffer),
				    "\n\n ///// **************** state #%d %s **************** \n",
				    stix, namstatsuffix);
	    mom_item_buffer_printf (_L (buffer),
				    "momlabstate_%d: MOM_DEBUG (run, \"%s at state #%%d %s\", momp_state);\n",
				    stix, cnam, namstatsuffix);
	    if (typcurstatetext == momty_string)
	      mom_item_buffer_printf (_L (buffer),
				      "// textual state\n"
				      "%s\n;//// end textual state #%d\n",
				      mom_string_cstr (curstatetext), stix);
	    else if (typcurstatetext == momty_bufferitem)
	      {
		unsigned buflen = 0;
		const char *bufstr =
		  mom_item_buffer_cstr (curstatetext, &buflen);
		mom_item_buffer_printf (_L (buffer),
					"// buffered state of %d bytes\n"
					"%s\n;//// end buffered state #%d\n",
					buflen, bufstr, stix);
		GC_FREE ((char *) bufstr);
	      }
	    else
	      MOM_FATAL ("impossible curstatetext");
	  }

      }
      break;
      ////////////////
    case cres_epilog:
      {
	goodstate = true;
	const char *cnam =
	  c_name_suffix (mom_value_as_item (_L (savecurout)));
	mom_item_buffer_printf (_L (buffer),
				"\n" "} //// end of emitted code for %s\n\n",
				cnam);
	// emit the state #undef-s
	for (unsigned stix = 0; stix < (unsigned) _N (nbstates); stix++)
	  {
	    momval_t curstatev = mom_node_nth (_L (xstate), stix);
	    mom_anyitem_t *curstateitm =
	      mom_value_as_item (mom_node_nth (curstatev, 0));
	    const char *namstatsuffix = c_name_suffix (curstateitm);
	    mom_item_buffer_printf (_L (buffer), "#undef momsta_%s\n",
				    namstatsuffix);
	  }
	// emit #undef GO POP STEADY
	mom_item_buffer_printf (_L (buffer), "#undef GO\n");
	mom_item_buffer_printf (_L (buffer), "#undef POP\n");
	mom_item_buffer_printf (_L (buffer), "#undef STEADY\n");
	// emit the local values #undef
	for (unsigned lix = 0; lix < (unsigned) _N (nbvalues); lix++)
	  {
	    const mom_anyitem_t *curvalitm =
	      mom_seqitem_nth_item (_L (xvalues), lix);
	    if (!curvalitm)
	      continue;
	    const char *curvalsuffix = c_name_suffix (curvalitm);
	    mom_item_buffer_printf (_L (buffer), "#undef momloc_%s\n",
				    curvalsuffix);
	    if (isalpha (curvalsuffix[0]))
	      mom_item_buffer_printf (_L (buffer), "#undef %s\n",
				      curvalsuffix);
	  }
	// emit the closed values #undef
	for (unsigned cix = 0; cix < (unsigned) _N (nbclosed); cix++)
	  {
	    const mom_anyitem_t *curcloitm =
	      mom_seqitem_nth_item (_L (xclosure), cix);
	    if (!curcloitm)
	      continue;
	    const char *curclosuffix = c_name_suffix (curcloitm);
	    if (!curclosuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer), "#undef momclo_%s\n",
				    curclosuffix);
	    if (isalpha (curclosuffix[0]))
	      mom_item_buffer_printf (_L (buffer), "#undef %s\n",
				      curclosuffix);
	  }
	// emit the local numbers #undef
	for (unsigned nix = 0; nix < (unsigned) _N (nbnumbers); nix++)
	  {
	    const mom_anyitem_t *curnumitm =
	      mom_seqitem_nth_item (_L (xnumbers), nix);
	    if (!curnumitm)
	      continue;
	    const char *curnumsuffix = c_name_suffix (curnumitm);
	    if (!curnumsuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer), "#undef momnum_%s\n",
				    curnumsuffix);
	    if (isalpha (curnumsuffix))
	      mom_item_buffer_printf (_L (buffer), "#undef %s\n",
				      curnumsuffix);
	  }
	// emit the local doubles #undef
	for (unsigned dix = 0; dix < (unsigned) _N (nbdoubles); dix++)
	  {
	    const mom_anyitem_t *curdblitm =
	      mom_seqitem_nth_item (_L (xdoubles), dix);
	    if (!curdblitm)
	      continue;
	    const char *curdblsuffix = c_name_suffix (curdblitm);
	    if (!curdblsuffix)
	      continue;
	    mom_item_buffer_printf (_L (buffer), "#undef momdbl_%s\n",
				    curdblsuffix);
	    if (isalpha (curdblsuffix))
	      mom_item_buffer_printf (_L (buffer), "#undef %s\n",
				      curdblsuffix);
	  }
	SET_STATE (epilog);
      }
      break;
      /////////
    case cres__laststate:
      {
	MOM_FATAL ("bad last state in momcode_cold_routine_emit");
      }
      break;
    }
  if (!goodstate)
    MOM_FATAL ("bad state %d in momcode_cold_routine_emit", state);
  return routres_pop;

#undef _L
#undef _C
#undef _N
}


const struct momroutinedescr_st momrout_cold_routine_emit =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = (unsigned) crec__lastclosure,
  .rout_frame_nbval = (unsigned) crev__lastval,
  .rout_frame_nbnum = (unsigned) cren__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "cold_routine_emit",
  .rout_code = (const momrout_sig_t *) momcode_cold_routine_emit,
  .rout_timestamp = __DATE__ "@" __TIME__
};
