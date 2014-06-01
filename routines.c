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
///// ajax_system
enum ajax_system_values_en
{
  ajaxsyst_v_arg0res,
  ajaxsyst_v_method,
  ajaxsyst_v_namid,
  ajaxsyst_v_restpath,
  ajaxsyst_v__spare,
  ajaxsyst_v_webx,
  ajaxsyst_v__lastval
};

enum ajax_system_closure_en
{
  ajaxsyst_c__lastclosure
};

enum ajax_system_numbers_en
{
  ajaxsyst_n__lastnum
};


static void
todo_dump_at_exit_mom (void *data)
{
  char *dpath = data;
  assert (dpath && dpath[0]);
  MOM_DEBUGPRINTF (run, "todo_dump_at_exit_mom should dump dpath=%s", dpath);
  mom_full_dump ("todo dump at exit", dpath, NULL);
  mom_stop_event_loop ();
  MOM_INFORMPRINTF ("dumped before exiting into directory %s", dpath);
}

static int
ajax_system_codmom (int momstate_, momitem_t *momtasklet_,
		    const momnode_t *momclosure_, momval_t *momlocvals_,
		    intptr_t * momlocnums_, double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[ajaxsyst_v_##Nam])
#define _C(Nam) (momclosure_->sontab[ajaxsyst_c_##Nam])
#define _N(Nam) (momlocnums_[ajaxsyst_n_##Nam])
  enum ajax_system_state_en
  {
    ajaxsyst_s_start,
    ajaxsyst_s__laststate
  };
#define _SET_STATE(St) do {						\
    MOM_DEBUGPRINTF (run,						\
		     "ajax_system_codmom setstate " #St " = %d",	\
	       (int)ajaxsystm_s_##St);					\
    return ajaxsystm_s_##St; } while(0)
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_system_codmom tasklet:"),
	     MOMOUT_ITEM ((const momitem_t *) momtasklet_),
	     MOMOUT_LITERAL (" state#"), MOMOUT_DEC_INT ((int) momstate_));
  if (momstate_ >= 0 && momstate_ < ajaxsyst_s__laststate)
    switch ((enum ajax_system_state_en) momstate_)
      {
      case ajaxsyst_s_start:
	goto ajaxsyst_lab_start;
      case ajaxsyst_s__laststate:;
      }
  MOM_FATAPRINTF ("ajax_system invalid state #%d", momstate_);
  ////
ajaxsyst_lab_start:
  _L (webx) = _L (arg0res);
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_system_codmom webx="),
	     MOMOUT_VALUE ((const momval_t) _L (webx)));
  assert (mom_is_item (_L (webx)));
  {
    mom_lock_item (_L (webx).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_system_codmom postjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_post (_L (webx).pitem)));
    momval_t todov = mom_webx_post_arg (_L (webx).pitem, "todo_mom");
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_system_codmom todov="),
	       MOMOUT_VALUE ((const momval_t) todov));
    if (mom_string_same (todov, "mom_menuitem_save_exit"))
      {
	mom_stop_work_with_todo (todo_dump_at_exit_mom, (char *) ".");
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("<em>Monimelt</em> <b>save then exit</b> at </i>"),
		      MOMOUT_DOUBLE_TIME ((const char *) "%c",
					  mom_clock_time (CLOCK_REALTIME)),
		      MOMOUT_LITERAL ("</i>"), MOMOUT_SPACE (32));
	mom_webx_reply (_L (webx).ptr, "text/html", HTTP_OK);
	usleep (1000);
	goto end;
      }
    else if (mom_string_same (todov, "mom_menuitem_quit"))
      {
	mom_stop_work_with_todo (NULL, NULL);
	MOM_WEBX_OUT
	  (_L (webx).pitem,
	   MOMOUT_LITERAL
	   ("<em>Monimelt</em> <b>quit without saving</b> at </i>"),
	   MOMOUT_DOUBLE_TIME ((const char *) "%c",
			       mom_clock_time (CLOCK_REALTIME)),
	   MOMOUT_LITERAL ("</i>"), MOMOUT_SPACE (32), NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
	usleep (1000);
	goto end;
      }
    else
      MOM_FATAL (MOMOUT_LITERAL ("ajax_system unexpected todov:"),
		 MOMOUT_VALUE ((const momval_t) todov));
  end:
    mom_unlock_item (_L (webx).pitem);
  }
  ;
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_ajax_system = {
  .rout_magic = MOM_ROUTINE_MAGIC,
  .rout_minclosize = ajaxsyst_c__lastclosure,
  .rout_frame_nbval = ajaxsyst_v__lastval,
  .rout_frame_nbnum = ajaxsyst_n__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_system",
  .rout_module = MONIMELT_CURRENT_MODULE,
  .rout_codefun = ajax_system_codmom,
  .rout_timestamp = __DATE__ "@" __TIME__
};



////////////////////////////////////////////////////////////////
///// ajax_objects
enum ajax_objects_values_en
{
  ajaxobjs_v_arg0res,
  ajaxobjs_v_method,
  ajaxobjs_v_namid,
  ajaxobjs_v_restpath,
  ajaxobjs_v__spare,
  ajaxobjs_v_webx,
  ajaxobjs_v__lastval
};

enum ajax_objects_closure_en
{
  ajaxobjs_c__lastclosure
};

enum ajax_objects_numbers_en
{
  ajaxobjs_n__lastnum
};


static int
ajax_objects_codmom (int momstate_, momitem_t *momtasklet_,
		     const momnode_t *momclosure_, momval_t *momlocvals_,
		     intptr_t * momlocnums_, double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[ajaxobjs_v_##Nam])
#define _C(Nam) (momclosure_->sontab[ajaxobjs_c_##Nam])
#define _N(Nam) (momlocnums_[ajaxobjs_n_##Nam])
  enum ajax_objects_state_en
  {
    ajaxobjs_s_start,
    ajaxobjs_s__laststate
  };
#define _SET_STATE(St) do {						\
    MOM_DEBUGPRINTF (run,						\
		     "ajax_objects_codmom setstate " #St " = %d",	\
	       (int)ajaxobjs_s_##St);					\
    return ajaxobjs_s_##St; } while(0)
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom tasklet:"),
	     MOMOUT_ITEM ((const momitem_t *) momtasklet_),
	     MOMOUT_LITERAL (" state#"), MOMOUT_DEC_INT ((int) momstate_));
  if (momstate_ >= 0 && momstate_ < ajaxobjs_s__laststate)
    switch ((enum ajax_objects_state_en) momstate_)
      {
      case ajaxobjs_s_start:
	goto ajaxobjs_lab_start;
      case ajaxobjs_s__laststate:;
      }
  MOM_FATAPRINTF ("ajax_objects invalid state #%d", momstate_);
  ////
ajaxobjs_lab_start:
  _L (webx) = _L (arg0res);
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom webx="),
	     MOMOUT_VALUE ((const momval_t) _L (webx)));
  assert (mom_is_item (_L (webx)));
  {
    mom_lock_item (_L (webx).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom postjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_post (_L (webx).pitem)));
    momval_t todov = mom_webx_post_arg (_L (webx).pitem, "todo_mom");
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom todov="),
	       MOMOUT_VALUE ((const momval_t) todov));
    if (mom_string_same (todov, "mom_menuitem_named"))
      {
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("Edit an existing <b>named</b> item <small>at </i>"),
		      MOMOUT_DOUBLE_TIME ((const char *) "%c",
					  mom_clock_time (CLOCK_REALTIME)),
		      MOMOUT_LITERAL ("</i></small><br/>"), MOMOUT_SPACE (32),
		      MOMOUT_LITERAL
		      ("<label for='mom_name_input'>Name:</label>"
		       " <input id='mom_name_input' class='easyui-combobox' name='mom_name'/>"
		       " <input type='submit' id='mom_cancel' class='mom_cancel_cl' value='cancel' onclick='mom_erase_maindiv()'/>"),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL
		      ("<script type='text/javascript'>mom_set_name_entry($('#mom_name_input'));"),
		      MOMOUT_SPACE (32), MOMOUT_LITERAL ("</script>"),
		      MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
	goto end;
      }
    if (mom_string_same (todov, "mom_menuitem_new"))
      {
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("Edit a <b>new</b> item <small>at </i>"),
		      MOMOUT_DOUBLE_TIME ((const char *) "%c",
					  mom_clock_time (CLOCK_REALTIME)),
		      MOMOUT_LITERAL ("</i></small><br/>"), MOMOUT_SPACE (32),
		      MOMOUT_LITERAL
		      ("<label for='mom_name_new'>Name:</label>"
		       " <input id='mom_name_new' class='mom_newname_cl' name='mom_new_name' pattern='[A-Za-z_][A-Za-z0-9_]*' /> "
		       "<label for='mom_comment'>Comment:</label>"
		       " <input id='mom_comment' class='mom_newcomment_cl' name='mom_new_comment'/>"
		       "<br/>"
		       " <input type='submit' id='mom_make_named' class='mom_make_named_cl' value='make' onclick='mom_make_named()'/>"
		       " <input type='submit' id='mom_cancel' class='mom_cancel_cl' value='cancel' onclick='mom_erase_maindiv()'/>"),
		      MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
	goto end;
      }
    else
      MOM_FATAL (MOMOUT_LITERAL ("ajax_objects unexpected todov:"),
		 MOMOUT_VALUE ((const momval_t) todov));
  end:
    mom_unlock_item (_L (webx).pitem);
  }
  ;
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_ajax_objects = {
  .rout_magic = MOM_ROUTINE_MAGIC,
  .rout_minclosize = ajaxobjs_c__lastclosure,
  .rout_frame_nbval = ajaxobjs_v__lastval,
  .rout_frame_nbnum = ajaxobjs_n__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_objects",
  .rout_module = MONIMELT_CURRENT_MODULE,
  .rout_codefun = ajax_objects_codmom,
  .rout_timestamp = __DATE__ "@" __TIME__
};



////////////////////////////////////////////////////////////////
///// ajax_complete_name
enum ajax_complete_name_values_en
{
  ajaxcompnam_v_arg0res,
  ajaxcompnam_v_method,
  ajaxcompnam_v_namid,
  ajaxcompnam_v_restpath,
  ajaxcompnam_v__spare,
  ajaxcompnam_v_webx,
  ajaxcompnam_v__lastval
};

enum ajax_complete_name_closure_en
{
  ajaxcompnam_c__lastclosure
};

enum ajax_complete_name_numbers_en
{
  ajaxcompnam_n__lastnum
};


static int
ajax_complete_name_codmom (int momstate_, momitem_t *momtasklet_,
			   const momnode_t *momclosure_,
			   momval_t *momlocvals_, intptr_t * momlocnums_,
			   double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[ajaxcompnam_v_##Nam])
#define _C(Nam) (momclosure_->sontab[ajaxcompnam_c_##Nam])
#define _N(Nam) (momlocnums_[ajaxcompnam_n_##Nam])
  enum ajax_complete_name_state_en
  {
    ajaxcompnam_s_start,
    ajaxcompnam_s__laststate
  };
#define _SET_STATE(St) do {						\
    MOM_DEBUGPRINTF (run,						\
		     "ajax_complete_name_codmom setstate " #St " = %d",	\
	       (int)ajaxcompnam_s_##St);				\
    return ajaxcompnam_s_##St; } while(0)
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_complete_name_codmom tasklet:"),
	     MOMOUT_ITEM ((const momitem_t *) momtasklet_),
	     MOMOUT_LITERAL (" state#"), MOMOUT_DEC_INT ((int) momstate_));
  if (momstate_ >= 0 && momstate_ < ajaxcompnam_s__laststate)
    switch ((enum ajax_complete_name_state_en) momstate_)
      {
      case ajaxcompnam_s_start:
	goto ajaxcompnam_lab_start;
      case ajaxcompnam_s__laststate:;
      }
  MOM_FATAPRINTF ("ajax_complete_name invalid state #%d", momstate_);
  ////
ajaxcompnam_lab_start:
  _L (webx) = _L (arg0res);
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_complete_name_codmom webx="),
	     MOMOUT_VALUE ((const momval_t) _L (webx)));
  assert (mom_is_item (_L (webx)));
  {
    mom_lock_item (_L (webx).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_complete_name_codmom queryjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_query (_L (webx).pitem)),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("ajax_complete_name_codmom postjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_post (_L (webx).pitem)),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("ajax_complete_name_codmom method="),
	       MOMOUT_VALUE ((const momval_t) (_L (method))), NULL);
    if (_L (method).pitem == mom_named__POST
	&& !mom_webx_jsob_post (_L (webx).pitem).ptr)
      {
	momval_t tupnamed = MOM_NULLV;
	momval_t jsarrnames = MOM_NULLV;
	tupnamed =
	  (momval_t) mom_alpha_ordered_tuple_of_named_items (&jsarrnames);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_complete_name_codmom tupnamed="),
		   MOMOUT_VALUE ((const momval_t) tupnamed),
		   MOMOUT_SPACE (24), MOMOUT_LITERAL ("jsarrnames="),
		   MOMOUT_VALUE ((const momval_t) jsarrnames));
	unsigned nbnames = mom_tuple_length (tupnamed);
	momval_t *jarr =
	  MOM_GC_ALLOC ("json named array", nbnames * sizeof (momval_t));
	for (unsigned nix = 0; nix < nbnames; nix++)
	  {
	    const momitem_t *curnamitm = mom_tuple_nth_item (tupnamed, nix);
	    momval_t curnamstr = mom_json_array_nth (jsarrnames, nix);
	    jarr[nix] = (momval_t)	////
	      mom_make_json_object (MOMJSOB_STRING
				    ((const char *) "name",
				     (momval_t) curnamstr),
				    MOMJSOB_STRING ((const char *) "id",
						    (momval_t)
						    mom_item_get_idstr ((momitem_t *) curnamitm)), NULL);
	  }
	momval_t jres = (momval_t) mom_make_json_array_count (nbnames, jarr);
	MOM_GC_FREE (jarr);
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_complete_name_codmom jres="),
		   MOMOUT_VALUE ((const momval_t) jres));
	MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JSON_VALUE (jres));
	mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	goto end;
      }
    MOM_FATAL (MOMOUT_LITERAL ("ajax_complete_name incomplete webx="),
	       MOMOUT_VALUE ((const momval_t) (_L (webx))), NULL);
    goto end;
  end:
    mom_unlock_item (_L (webx).pitem);
  }
  ;
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_ajax_complete_name = {
  .rout_magic = MOM_ROUTINE_MAGIC,
  .rout_minclosize = ajaxcompnam_c__lastclosure,
  .rout_frame_nbval = ajaxcompnam_v__lastval,
  .rout_frame_nbnum = ajaxcompnam_n__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "ajax_complete_name",
  .rout_module = MONIMELT_CURRENT_MODULE,
  .rout_codefun = ajax_complete_name_codmom,
  .rout_timestamp = __DATE__ "@" __TIME__
};
