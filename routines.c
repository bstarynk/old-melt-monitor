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
		    intptr_t * momlocnums_, double *momlocdbls_
		    __attribute__ ((unused)))
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
  ajaxobjs_v_editeditm,
  ajaxobjs_v_editor,
  ajaxobjs_v_setattrs,
  ajaxobjs_v_curattritm,
  ajaxobjs_v_curvalattr,
  ajaxobjs_v__lastval
};

enum ajax_objects_closure_en
{
  ajaxobjs_c_editvalueclos,
  ajaxobjs_c__lastclosure
};

enum ajax_objects_numbers_en
{
  ajaxobjs_n_nbattrs,
  ajaxobjs_n_atix,
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
    ajaxobjs_s_beginedit,
    ajaxobjs_s__laststate
  };
#define _SET_STATE(St) do {						\
    MOM_DEBUGPRINTF (run,						\
		     "ajax_objects_codmom setstate " #St " = %d",	\
		     (int)ajaxobjs_s_##St);				\
    return ajaxobjs_s_##St; } while(0)
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom tasklet:"),
	     MOMOUT_ITEM ((const momitem_t *) momtasklet_),
	     MOMOUT_LITERAL (" state#"), MOMOUT_DEC_INT ((int) momstate_));
  if (momstate_ >= 0 && momstate_ < ajaxobjs_s__laststate)
    switch ((enum ajax_objects_state_en) momstate_)
      {
      case ajaxobjs_s_start:
	goto ajaxobjs_lab_start;
      case ajaxobjs_s_beginedit:
	goto ajaxobjs_lab_beginedit;
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
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom named"));
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
    else if (mom_string_same (todov, "mom_menuitem_new"))
      {
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom new"));
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
    else if (mom_string_same (todov, "mom_domakenamed"))
      {
	momval_t namev = mom_webx_post_arg (_L (webx).pitem, "name_mom");
	momval_t commentv =
	  mom_webx_post_arg (_L (webx).pitem, "comment_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom makenamed namev="),
		   MOMOUT_VALUE ((const momval_t) namev),
		   MOMOUT_LITERAL ("; commentv="),
		   MOMOUT_VALUE ((const momval_t) commentv));
	_L (editeditm) = (momval_t) mom_make_item ();
	_L (editeditm).pitem->i_attrs =
	  mom_put_attribute (_L (editeditm).pitem->i_attrs,
			     mom_named__comment, commentv);
	mom_register_item_named (_L (editeditm).pitem, mom_to_string (namev));
	mom_item_set_space (_L (editeditm).pitem, momspa_root);
	{
	  mom_unlock_item (_L (webx).pitem);
	  _SET_STATE (beginedit);
	}
      }
    else if (mom_string_same (todov, "mom_doeditnamed"))
      {
	momval_t namev = mom_webx_post_arg (_L (webx).pitem, "name_mom");
	momval_t idv = mom_webx_post_arg (_L (webx).pitem, "id_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom doeditnamed namev="),
		   MOMOUT_VALUE ((const momval_t) namev),
		   MOMOUT_LITERAL ("; idv="),
		   MOMOUT_VALUE ((const momval_t) idv));
	_L (editeditm) =
	  (momval_t) mom_get_item_of_name_or_ident_string (namev);
	if (!_L (editeditm).ptr)
	  _L (editeditm) =
	    (momval_t) mom_get_item_of_name_or_ident_string (idv);
	{
	  mom_unlock_item (_L (webx).pitem);
	  _SET_STATE (beginedit);
	}
      }
    else
      MOM_FATAL (MOMOUT_LITERAL ("ajax_objects unexpected todov:"),
		 MOMOUT_VALUE ((const momval_t) todov));
  end:
    mom_unlock_item (_L (webx).pitem);
    return momroutres_pop;
  }
  ;
  ////
ajaxobjs_lab_beginedit:
  {
    _L (editor) = (momval_t) mom_make_item ();
    {
      mom_lock_item (_L (editeditm).pitem);
      _L (setattrs) =
	(momval_t) mom_set_attributes (_L (editeditm).pitem->i_attrs);
      _N (nbattrs) = mom_set_cardinal (_L (setattrs));
      mom_unlock_item (_L (editeditm).pitem);
    }
    MOM_DEBUG (run,
	       MOMOUT_LITERAL ("ajax_objects_codmom beginedit editeditm="),
	       MOMOUT_VALUE ((const momval_t) _L (editeditm)),
	       MOMOUT_LITERAL ("; setattrs="),
	       MOMOUT_VALUE ((const momval_t) _L (setattrs)),
	       MOMOUT_LITERAL ("; editor="),
	       MOMOUT_VALUE ((const momval_t) _L (editor)));
    {
      assert (mom_is_item (_L (webx)));
      momval_t namidv =
	(momval_t) mom_item_get_name_or_idstr (_L (editeditm).pitem);
      mom_lock_item (_L (webx).pitem);
      MOM_WEBX_OUT (_L (webx).pitem,
		    MOMOUT_LITERAL
		    ("<p class='mom_edit_title_cl'>Item <tt>"),
		    MOMOUT_HTML (mom_string_cstr (namidv)),
		    MOMOUT_LITERAL ("</tt></span> <small>at "),
		    MOMOUT_DOUBLE_TIME ((const char *) "%c",
					mom_clock_time (CLOCK_REALTIME)),
		    MOMOUT_LITERAL ("</small></p>"),
		    MOMOUT_NEWLINE (),
		    MOMOUT_LITERAL ("<ul class='mom_attrlist_cl'>"), NULL);
      mom_unlock_item (_L (webx).pitem);
    }
    for (_N (atix) = 0; _N (atix) < _N (nbattrs); _N (atix)++)
      {
	_L (curattritm) =
	  (momval_t) mom_set_nth_item (_L (setattrs), _N (atix));
	{
	  assert (mom_is_item (_L (editeditm)));
	  assert (mom_is_item (_L (curattritm)));
	  mom_lock_item (_L (editeditm).pitem);
	  _L (curvalattr) =
	    (momval_t) mom_get_attribute (_L (editeditm).pitem->i_attrs,
					  _L (curattritm).pitem);
	  mom_unlock_item (_L (editeditm).pitem);
	}
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom atix="),
		   MOMOUT_DEC_INT ((int) _N (atix)),
		   MOMOUT_LITERAL ("; curattritm="),
		   MOMOUT_VALUE ((const momval_t) _L (curattritm)),
		   MOMOUT_LITERAL ("; curvalattr="),
		   MOMOUT_VALUE ((const momval_t) _L (curvalattr)), NULL);
	{
	  momval_t namidatv =
	    (momval_t) mom_item_get_name_or_idstr (_L (curattritm).pitem);
	  assert (mom_is_item (_L (webx)));
	  mom_lock_item (_L (webx).pitem);
	  MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL ("<li class='mom_attrentry_cl'><span class='mom_attritem_cl'>"), MOMOUT_HTML (mom_string_cstr (namidatv)), MOMOUT_LITERAL ("</span> " "&#8594;"	/* U+2192 RIGHTWARDS ARROW → */
																						   " <span class='mom_value_cl'>"),
			NULL);
	  mom_unlock_item (_L (webx).pitem);
	}
	MOM_FATAL (MOMOUT_LITERAL ("ajax_objects unimplemented beginedit"));
      }
  }
  ;
  ////
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



////////////////////////////////////////////////////////////////
///// edit_value
enum edit_value_values_en
{
  edit_value_v_arg0res,
  edit_value_v__lastval
};

enum edit_value_closure_en
{
  edit_value_c__lastclosure
};

enum edit_value_numbers_en
{
  edit_value_n__lastnum
};


static int
edit_value_codmom (int momstate_, momitem_t *momtasklet_,
		   const momnode_t *momclosure_,
		   momval_t *momlocvals_, intptr_t * momlocnums_,
		   double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[edit_value_v_##Nam])
#define _C(Nam) (momclosure_->sontab[edit_value_c_##Nam])
#define _N(Nam) (momlocnums_[edit_value_n_##Nam])
  enum edit_value_state_en
  {
    edit_value_s_start,
    edit_value_s_impossible,
    edit_value_s__laststate
  };
#define _SET_STATE(St) do {					\
    MOM_DEBUGPRINTF (run,					\
		     "edit_value_codmom setstate " #St " = %d",	\
		     (int)edit_value_s_##St);				\
    return edit_value_s_##St; } while(0)
  if (momstate_ >= 0 && momstate_ < edit_value_s__laststate)
    switch ((enum edit_value_state_en) momstate_)
      {
      case edit_value_s_start:
	goto edit_value_lab_start;
      case edit_value_s_impossible:
	goto edit_value_lab_impossible;
      case edit_value_s__laststate:;
      }
  MOM_FATAPRINTF ("edit_value invalid state #%d", momstate_);
edit_value_lab_start:
  MOM_DEBUG (run, MOMOUT_LITERAL ("edit_value start arg0res="),
	     MOMOUT_VALUE ((const momval_t) _L (arg0res)));
  if (_L (arg0res).ptr == MOM_EMPTY)
    _SET_STATE (impossible);
  return momroutres_pop;
edit_value_lab_impossible:
  MOM_FATAPRINTF ("edit_value impossible state reached!");
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_edit_value = {
  .rout_magic = MOM_ROUTINE_MAGIC,
  .rout_minclosize = edit_value_c__lastclosure,
  .rout_frame_nbval = edit_value_v__lastval,
  .rout_frame_nbnum = edit_value_n__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "edit_value",
  .rout_module = MONIMELT_CURRENT_MODULE,
  .rout_codefun = edit_value_codmom,
  .rout_timestamp = __DATE__ "@" __TIME__
};





////////////////////////////////////////////////////////////////
///// noop
enum noop_values_en
{
  noop_v_arg0res,
  noop_v__lastval
};

enum noop_closure_en
{
  noop_c__lastclosure
};

enum noop_numbers_en
{
  noop_n__lastnum
};


static int
noop_codmom (int momstate_, momitem_t *momtasklet_,
	     const momnode_t *momclosure_,
	     momval_t *momlocvals_, intptr_t * momlocnums_,
	     double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[noop_v_##Nam])
#define _C(Nam) (momclosure_->sontab[noop_c_##Nam])
#define _N(Nam) (momlocnums_[noop_n_##Nam])
  enum noop_state_en
  {
    noop_s_start,
    noop_s_impossible,
    noop_s__laststate
  };
#define _SET_STATE(St) do {					\
    MOM_DEBUGPRINTF (run,					\
		     "noop_codmom setstate " #St " = %d",	\
		     (int)noop_s_##St);				\
    return noop_s_##St; } while(0)
  if (momstate_ >= 0 && momstate_ < noop_s__laststate)
    switch ((enum noop_state_en) momstate_)
      {
      case noop_s_start:
	goto noop_lab_start;
      case noop_s_impossible:
	goto noop_lab_impossible;
      case noop_s__laststate:;
      }
  MOM_FATAPRINTF ("noop invalid state #%d", momstate_);
noop_lab_start:
  MOM_DEBUG (run, MOMOUT_LITERAL ("noop start arg0res="),
	     MOMOUT_VALUE ((const momval_t) _L (arg0res)));
  if (_L (arg0res).ptr == MOM_EMPTY)
    _SET_STATE (impossible);
  return momroutres_pop;
noop_lab_impossible:
  MOM_FATAPRINTF ("noop impossible state reached!");
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_noop = {
  .rout_magic = MOM_ROUTINE_MAGIC,
  .rout_minclosize = noop_c__lastclosure,
  .rout_frame_nbval = noop_v__lastval,
  .rout_frame_nbnum = noop_n__lastnum,
  .rout_frame_nbdbl = 0,
  .rout_name = "noop",
  .rout_module = MONIMELT_CURRENT_MODULE,
  .rout_codefun = noop_codmom,
  .rout_timestamp = __DATE__ "@" __TIME__
};
