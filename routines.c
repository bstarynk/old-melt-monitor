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
  MOM_DEBUG (web, "momcode_web_form_exit state=%d webnum=%ld nowbuf=%s",
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
      MOM_DEBUG (web, "momcode_web_form_exit POST");
      MOM_DBG_VALUE (web, "web_form_exit jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      if (mom_item_webrequest_post_arg (webv, "do_savexit").ptr)
	{
	  MOM_DEBUG (web, "momcode_web_form_exit do_savexit");
	  mom_item_webrequest_add
	    (webv,
	     MOMWEB_SET_MIME, "text/html",
	     MOMWEB_LIT_STRING,
	     "<!doctype html><head><title>dump and exit Monimelt</title></head>\n"
	     "<body><h1>Monimelt dump and exit</h1>\n"
	     "<p>dump to default <tt>" MOM_DEFAULT_STATE_FILE
	     "</tt> reqnum#",
	     MOMWEB_DEC_LONG, (long) mom_item_webrequest_webnum (webv),
	     MOMWEB_LIT_STRING, " at <i>",
	     MOMWEB_HTML_STRING, nowbuf,
	     MOMWEB_LIT_STRING, "</i></p>\n" "</body></html>\n",
	     MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  usleep (25000);
	  MOM_DEBUG (web,
		     "momcode_web_form_exit do_savexit before request stop");
	  mom_request_stop ("momcode_web_form_exit do_savexit", NULL, NULL);
	  usleep (2000);
	  MOM_DEBUG (web, "momcode_web_form_exit do_savexit before fulldump");
	  mom_full_dump ("web save&exit dump", MOM_DEFAULT_STATE_FILE);;
	  MOM_DEBUG (web, "momcode_web_form_exit do_savexit after fulldump");
	}
      else if (mom_item_webrequest_post_arg (webv, "do_quit").ptr)
	{
	  MOM_DEBUG (web, "momcode_web_form_exit do_quit");
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
	  MOM_DEBUG (web,
		     "momcode_web_form_exit do_quit before request stop");
	  mom_request_stop ("momcode_web_form_exit do_quit", NULL, NULL);
	}
      else
	MOM_WARNING ("unexpected post query for webnum#%ld",
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
  .rout_code = (const momrout_sig_t *) momcode_web_form_exit,
  .rout_timestamp = __DATE__ "@" __TIME__
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
  MOM_DEBUG (web,
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
      MOM_DEBUG (web, "momcode_web_form_new_named POST");
      MOM_DBG_VALUE (web, "web_form_new_named jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      if (mom_item_webrequest_post_arg (webv, "do_create_new_named").ptr)
	{
	  mom_anyitem_t *newitm = NULL;
	  char *errmsg = NULL;
	  MOM_DEBUG (web, "momcode_web_form_new_named do_create_new_named");
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
		  (mom_anyitem_t *) mom_make_item_assoc (MOM_SPACE_ROOT);
	      else if (!strcmp (typec, "box"))
		newitm = (mom_anyitem_t *) mom_make_item_box (MOM_SPACE_ROOT);
	      else if (!strcmp (typec, "buffer"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_buffer (MOM_SPACE_ROOT);
	      else if (!strcmp (typec, "dictionnary"))
		newitm =
		  (mom_anyitem_t *)
		  mom_make_item_dictionnary (MOM_SPACE_ROOT);
	      else if (!strcmp (typec, "json_name"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_json_name (namec,
							     MOM_SPACE_ROOT);
	      else if (!strcmp (typec, "queue"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_queue (MOM_SPACE_ROOT);
	      else if (!strcmp (typec, "routine"))
		{
		  newitm =
		    (mom_anyitem_t *) mom_make_item_embryonic_routine (namec,
								       MOM_SPACE_ROOT);
		}
	      else if (!strcmp (typec, "vector"))
		newitm =
		  (mom_anyitem_t *) mom_make_item_vector (MOM_SPACE_ROOT, 15);
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

	  MOM_DEBUG (web,
		     "momcode_web_form_new_named do_create_new_named after");
	}
      else
	MOM_WARNING ("unexpected post query for webnum#%ld",
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
  .rout_code = (const momrout_sig_t *) momcode_web_form_new_named,
  .rout_timestamp = __DATE__ "@" __TIME__
};


////////////////////////////////////////////////////////////////

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
	  mom_item_webrequest_add (_L (web), MOMWEB_SET_MIME, "text/html",
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
				   MOMWEB_REPLY_CODE, HTTP_NOT_FOUND,
				   MOMWEB_END);
	  return routres_pop;
	}
      if (mom_item_webrequest_post_arg (_L (web), "do_addrout").ptr)
	{
	  momval_t oldset =
	    (momval_t) mom_item_get_attr ((mom_anyitem_t *)
					  mom_item__first_module,
					  (mom_anyitem_t *)
					  mom_item__routines);
	  MOM_DBG_VALUE (web, "old set of routines in first module=", oldset);
	  momval_t newset =
	    (momval_t) mom_make_set_til_nil (_L (routine), oldset, NULL);
	  MOM_DBG_VALUE (web, "new grown set of routines in first module=",
			 newset);
	  mom_item_put_attr ((mom_anyitem_t *) mom_item__first_module,
			     (mom_anyitem_t *) mom_item__routines, newset);
	  mom_item_webrequest_add (_L (web), MOMWEB_SET_MIME, "text/html",
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
				   MOMWEB_REPLY_CODE, HTTP_OK, MOMWEB_END);
	  return routres_pop;
	}
      else if (mom_item_webrequest_post_arg (_L (web), "do_removerout").ptr)
	{
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
	      mom_item_webrequest_add (_L (web), MOMWEB_SET_MIME, "text/html",
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
				       MOMWEB_REPLY_CODE, HTTP_OK,
				       MOMWEB_END);
	      return routres_pop;
	    }
	  else
	    {
	      MOM_WARNING ("web_form_handle_routine absent routine name %s",
			   mom_string_cstr (namestrv));
	      mom_item_webrequest_add (_L (web), MOMWEB_SET_MIME, "text/html",
				       MOMWEB_LIT_STRING,
				       "<!doctype html><head><title>Absent Routine in Monimelt</title></head>\n"
				       "<body><h1>Absent Routine Name</h1>\n",
				       MOMWEB_LIT_STRING, "<p>Name <tt>",
				       MOMWEB_HTML_STRING,
				       mom_string_cstr (namestrv),
				       MOMWEB_LIT_STRING,
				       "</tt> not found at <i>",
				       MOMWEB_HTML_STRING, nowbuf,
				       MOMWEB_LIT_STRING,
				       "</i>.</p></body></html>\n",
				       MOMWEB_REPLY_CODE, HTTP_NOT_FOUND,
				       MOMWEB_END);
	      return routres_pop;

	    }
	}
      else if (mom_item_webrequest_post_arg (_L (web), "do_editrout").ptr)
	{
#warning momcode_web_form_handle_routine incomplete should do_editrout
	}
      MOM_WARNING ("momcode_web_form_handle_routine incomplete");
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
  .rout_code = (const momrout_sig_t *) momcode_ajax_start,
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
    wfcs__laststate
  };
#define SET_STATE(St) do {						\
    MOM_DEBUG (web,							\
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
  MOM_DEBUG (web,
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
	MOM_DEBUG (web, "momcode_web_form_compile start webnum=%ld",
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
	    MOM_WARNING ("no routines in first_module");
	    l_routines = MOM_NULLV;
	    SET_STATE (begin_emission);
	  }
	MOM_FATAL
	  ("momcode_web_form_compile unimplemented routine form at start");
      }
      break;
      ////////////////
    case wfcs_compute_routines:	////================ compute routines
      {
	MOM_DEBUG (web,
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
	    MOM_WARNING ("got no routines in module");
	    l_routines = MOM_NULLV;
	  }
	SET_STATE (begin_emission);
      }
      break;
      ////////////////
    case wfcs_begin_emission:	////================ begin emissions
      {
	goodstate = true;
	l_dashboard = (momval_t) mom_make_item_assoc (MOM_SPACE_NONE);
	l_buffer = (momval_t) mom_make_item_buffer (MOM_SPACE_NONE);
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
	MOM_DEBUG (web,
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
	MOM_DEBUG (web,
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
	momval_t statev = MOM_NULLV, closurev = MOM_NULLV;
	momval_t valuesv = MOM_NULLV, numbersv = MOM_NULLV;
	momval_t doublesv = MOM_NULLV;
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
				" .rout_code = (const momrout_sig_t *) momcode_%s,\n"
				" .rout_timestamp = __DATE__ \"@\" __TIME__\n"
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
	MOM_DEBUG (web,
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
	MOM_DEBUG (web, "web_form_compile emit routine %s", cnam);
	goodstate = true;
	MOM_DBG_VALUE (web, "web_form_compile emit routine l_curout=",
		       l_curout);
	l_routemp = (momval_t) mom_make_item_assoc (MOM_SPACE_NONE);
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
	    MOM_WARNING ("no routine emitter for %s", cnam);
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
	MOM_DEBUG (web, "web_form_compile got emitter routine %s", cnam);
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
	MOM_DEBUG (web,
		   "web_form_compile run compiler buffer of %d bytes",
		   (int) mom_item_buffer_length (l_buffer));
	/// write the buffer
	rename (GENERATED_SOURCE_FILE_NAME, GENERATED_SOURCE_FILE_NAME "~");
	FILE *fout = fopen (GENERATED_SOURCE_FILE_NAME, "w");
	if (MOM_UNLIKELY (!fout))
	  MOM_FATAL ("failed to open file " GENERATED_SOURCE_FILE_NAME);
	unsigned blen = mom_item_buffer_length (l_buffer);
	if (mom_item_buffer_output_content_to_file (l_buffer, fout)
	    != (int) blen)
	  MOM_WARNING ("failed to output all %d bytes to "
		       GENERATED_SOURCE_FILE_NAME, (int) blen);
	fclose (fout), fout = NULL;
	MOM_INFORM ("wrote %d bytes of code into %s", (int) blen,
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
	MOM_DEBUG (web, "after compilation start");
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
    case wfcs__laststate:
      {
	MOM_FATAL ("momcode_web_form_compile unexpected last");
      }
    }
  if (!goodstate)
    MOM_FATAL ("momcode_web_form_compile invalid state %d", state);
  MOM_DEBUG (web, "momcode_web_form_compile ending state %d", state);
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
  .rout_code = (const momrout_sig_t *) momcode_web_form_compile,
  .rout_timestamp = __DATE__ "@" __TIME__
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
	    if (!mom_is_string (curstatetext))
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
	    if (!mom_is_string (curstatetext))
	      continue;
	    mom_item_buffer_printf (_L (buffer),
				    "\n\n ///// **************** state #%d %s **************** \n",
				    stix, namstatsuffix);
	    mom_item_buffer_printf (_L (buffer),
				    "momlabstate_%d: MOM_DEBUG (run, \"%s at state #%%d %s\", momp_state);\n",
				    stix, cnam, namstatsuffix);
	    mom_item_buffer_printf (_L (buffer), "%s\n;//// end state #%d\n",
				    mom_string_cstr (curstatetext), stix);
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
