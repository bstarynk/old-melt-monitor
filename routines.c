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
    else if (mom_string_same (todov, "mom_initial_system"))
      {
	MOM_WEBX_OUT
	  (_L (webx).pitem,
	   MOMOUT_LITERAL
	   ("<em>Monimelt</em> started at <i>"),
	   MOMOUT_DOUBLE_TIME ((const char *) "%c",
			       mom_clock_time (CLOCK_REALTIME)),
	   MOMOUT_LITERAL ("</i> process "),
	   MOMOUT_DEC_INT ((int) getpid ()),
	   MOMOUT_LITERAL ("<br/> build <tt>"),
	   MOMOUT_LITERAL (monimelt_timestamp),
	   MOMOUT_LITERAL ("</tt> <small>version <tt>"),
	   MOMOUT_LITERAL (monimelt_lastgitcommit),
	   MOMOUT_LITERAL ("</tt></small>"), MOMOUT_NEWLINE (), NULL);
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
  ajaxobjs_v_curcontent,
  ajaxobjs_v__lastval
};

enum ajax_objects_closure_en
{
  ajaxobjs_c_editvalueclos,
  ajaxobjs_c_editors,
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
    ajaxobjs_s_didattredit,
    ajaxobjs_s_didcontentedit,
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
	//
      case ajaxobjs_s_beginedit:
	goto ajaxobjs_lab_beginedit;
	//
      case ajaxobjs_s_didattredit:
	goto ajaxobjs_lab_didattredit;
	//
      case ajaxobjs_s_didcontentedit:
	goto ajaxobjs_lab_didcontentedit;
	//
      case ajaxobjs_s__laststate:;
      }
  MOM_FATAPRINTF ("ajax_objects invalid state #%d", momstate_);
  ////
ajaxobjs_lab_start:
  _L (webx) = _L (arg0res);
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom start webx="),
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
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom makenamed"));
	momval_t namev = mom_webx_post_arg (_L (webx).pitem, "name_mom");
	momval_t commentv =
	  mom_webx_post_arg (_L (webx).pitem, "comment_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom makenamed namev="),
		   MOMOUT_VALUE ((const momval_t) namev),
		   MOMOUT_LITERAL ("; commentv="),
		   MOMOUT_VALUE ((const momval_t) commentv));
	_L (editeditm) = (momval_t) mom_make_item ();
	mom_item_put_attribute (_L (editeditm).pitem,
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
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom editnamed"));
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
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom doeditnamed editeditm="),
		   MOMOUT_VALUE ((const momval_t) _L (editeditm)));
	{
	  mom_unlock_item (_L (webx).pitem);
	  _SET_STATE (beginedit);
	}
      }
    else if (mom_string_same (todov, "mom_doeditorclose"))
      {
	momval_t editoridv =
	  mom_webx_post_arg (_L (webx).pitem, "closedid_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom doeditorclose editoridv="),
		   MOMOUT_VALUE ((const momval_t) editoridv));
	assert (mom_is_string (editoridv));
	const char *editoridstr = mom_string_cstr (editoridv);
	assert (editoridstr != NULL
		&& strncmp (editoridstr, "momeditor",
			    strlen ("momeditor")) == 0);
	momitem_t *editoritm =
	  mom_get_item_of_identcstr (editoridstr + strlen ("momeditor"));
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom doeditorclose editoritm="),
		   MOMOUT_ITEM ((const momitem_t *) editoritm));
	assert (mom_is_item ((momval_t) editoritm));
	momitem_t *editeditm = NULL;
	{
	  mom_should_lock_item (editoritm);
	  editeditm =
	    mom_value_to_item (mom_item_get_attribute
			       (editoritm, mom_named__item));
	  mom_unlock_item (editoritm);
	}
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom doeditorclose editeditm="),
		   MOMOUT_ITEM ((const momitem_t *) editeditm));
	assert (mom_is_item ((momval_t) editeditm));
	assert (mom_is_item ((momval_t) _C (editors)));
	{
	  mom_should_lock_item (_C (editors).pitem);
	  mom_item_assoc_remove (_C (editors).pitem, editeditm);
	  mom_unlock_item (_C (editors).pitem);
	}
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL ("Closed editor for <tt>"),
		      MOMOUT_LITERALV (mom_string_cstr
				       ((momval_t)
					mom_item_get_name_or_idstr
					(editeditm))),
		      MOMOUT_LITERAL ("</tt>."
				      " <input type='submit' id='mom_cancel' class='mom_cancel_cl' value='cancel' onclick='mom_erase_maindiv()'/>"),
		      NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
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
    mom_item_start_vector (_L (editor).pitem);
    mom_item_vector_reserve (_L (editor).pitem, 16);
    mom_item_put_attribute (_L (editor).pitem, mom_named__item,
			    _L (editeditm));
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom beginedit editor="),
	       MOMOUT_VALUE ((const momval_t) _L (editor)),
	       MOMOUT_LITERAL ("; webx="),
	       MOMOUT_VALUE ((const momval_t) _L (webx)),
	       MOMOUT_LITERAL ("; editeditm="),
	       MOMOUT_VALUE ((const momval_t) _L (editeditm)),
	       MOMOUT_LITERAL ("; editors="),
	       MOMOUT_VALUE ((const momval_t) _C (editors)), NULL);
    {
      mom_should_lock_item (_C (editors).pitem);
      if (_C (editors).pitem->i_paylkind != mompayk_assoc)
	mom_item_start_assoc (_C (editors).pitem);
      mom_item_assoc_put (_C (editors).pitem, _L (editeditm).pitem,
			  _L (editor));
      mom_unlock_item (_C (editors).pitem);
    }
    {
      mom_lock_item (_L (editeditm).pitem);
      _L (setattrs) =
	(momval_t) mom_item_set_attributes (_L (editeditm).pitem);
      _N (nbattrs) = mom_set_cardinal (_L (setattrs));
      mom_unlock_item (_L (editeditm).pitem);
    }
    MOM_DEBUG (run,
	       MOMOUT_LITERAL ("ajax_objects_codmom beginedit editeditm="),
	       MOMOUT_VALUE ((const momval_t) _L (editeditm)),
	       MOMOUT_LITERAL ("; setattrs="),
	       MOMOUT_VALUE ((const momval_t) _L (setattrs)),
	       MOMOUT_LITERAL ("; nbattrs="),
	       MOMOUT_DEC_INT ((int) _N (nbattrs)),
	       MOMOUT_LITERAL ("; editor="),
	       MOMOUT_VALUE ((const momval_t) _L (editor)),
	       MOMOUT_LITERAL ("; editvalueclos="),
	       MOMOUT_VALUE ((const momval_t) _C (editvalueclos)));
    {
      assert (mom_is_item (_L (webx)));
      momval_t namidv =
	(momval_t) mom_item_get_name_or_idstr (_L (editeditm).pitem);
      momval_t idv = (momval_t) mom_item_get_idstr (_L (editeditm).pitem);
      bool anonymous = (namidv.ptr == idv.ptr);
      mom_lock_item (_L (webx).pitem);
      MOM_WEBX_OUT (_L (webx).pitem,
		    MOMOUT_LITERAL ("<div id='momeditor_"),
		    MOMOUT_LITERALV ((const char *)
				     mom_string_cstr ((momval_t)
						      mom_item_get_idstr (_L
									  (editor).pitem))),
		    MOMOUT_LITERAL ("' class='mom_editor_cl' title='"),
		    MOMOUT_LITERALV ((const char *)
				     mom_string_cstr ((namidv))),
		    MOMOUT_LITERAL ("'>"), MOMOUT_NEWLINE (),
		    MOMOUT_LITERAL
		    ("<p class='mom_edit_title_cl' data-momediteditemid='"),
		    MOMOUT_LITERALV ((const char *)
				     mom_string_cstr ((momval_t) idv)),
		    MOMOUT_LITERAL ("'>"));
      if (anonymous)
	{
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_LITERAL
			("anonymous item <tt class='mom_edititemid_cl'>"),
			MOMOUT_LITERALV ((const char *)
					 mom_string_cstr ((momval_t) idv)),
			MOMOUT_LITERAL ("</tt>"), MOMOUT_NEWLINE ());
	}
      else
	{
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_LITERAL
			("item <tt class='mom_edititemname_cl'>"),
			MOMOUT_LITERALV ((const char *)
					 mom_string_cstr ((momval_t) namidv)),
			MOMOUT_LITERAL
			("</tt> <small>of id:</small> <code class='mom_itemid_cl'>"),
			MOMOUT_LITERALV ((const char *)
					 mom_string_cstr ((momval_t) idv)),
			MOMOUT_LITERAL ("</code>"), MOMOUT_NEWLINE ());
	}
      MOM_WEBX_OUT (_L (webx).pitem,
		    MOMOUT_LITERAL ("<br/>"),
		    MOMOUT_LITERAL
		    ("<span class='mom_editdate_cl'>edited at "),
		    MOMOUT_DOUBLE_TIME ((const char *) "%c",
					mom_clock_time (CLOCK_REALTIME)),
		    MOMOUT_LITERAL ("</span></p>"), MOMOUT_NEWLINE ());
      mom_unlock_item (_L (webx).pitem);
    }
    MOM_DEBUG (run,
	       MOMOUT_LITERAL
	       ("ajax_objects_codmom before atix loop nbattrs="),
	       MOMOUT_DEC_INT ((int) _N (nbattrs)));
    for (_N (atix) = 0; _N (atix) < _N (nbattrs); _N (atix)++)
      {
	_L (curattritm) =
	  (momval_t) mom_set_nth_item (_L (setattrs), _N (atix));
	{
	  assert (mom_is_item (_L (editeditm)));
	  assert (mom_is_item (_L (curattritm)));
	  mom_lock_item (_L (editeditm).pitem);
	  _L (curvalattr) = mom_item_get_attribute (_L (editeditm).pitem,
						    _L (curattritm).pitem);
	  mom_unlock_item (_L (editeditm).pitem);
	}
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom atix="),
		   MOMOUT_DEC_INT ((int) _N (atix)),
		   MOMOUT_LITERAL ("; curattritm="),
		   MOMOUT_VALUE ((const momval_t) _L (curattritm)),
		   MOMOUT_LITERAL ("; curvalattr="),
		   MOMOUT_VALUE ((const momval_t) _L (curvalattr)),
		   MOMOUT_LITERAL ("; editvalueclos="),
		   MOMOUT_VALUE ((const momval_t) _C (editvalueclos)), NULL);
	assert (mom_is_node (_C (editvalueclos)));
	{
	  momval_t namidatv =
	    (momval_t) mom_item_get_name_or_idstr (_L (curattritm).pitem);
	  momval_t idatv =
	    (momval_t) mom_item_get_idstr (_L (curattritm).pitem);
	  assert (mom_is_item (_L (webx)));
	  momval_t jorigat =	////
	    (momval_t)
	    mom_make_json_object (MOMJSOB_ENTRY ((momval_t) mom_named__kind,
						 (momval_t) mom_named__attr),
				  MOMJSOB_ENTRY ((momval_t) mom_named__item,
						 (momval_t)
						 mom_item_get_idstr
						 (mom_value_to_item
						  (_L (editeditm)))),
				  MOMJSOB_ENTRY ((momval_t) mom_named__attr,
						 (momval_t) idatv),
				  NULL);
	  momval_t exprat =	///
	    (momval_t) mom_make_node_til_nil (mom_named__attr,
					      _L (editeditm),
					      _L (curattritm),
					      NULL);
	  MOM_DEBUG (run,
		     MOMOUT_LITERAL ("ajax_objects_codmom jorigat="),
		     MOMOUT_JSON_VALUE (jorigat),
		     MOMOUT_LITERAL ("; exprat="),
		     MOMOUT_JSON_VALUE (exprat));
	  mom_lock_item (_L (webx).pitem);
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL ("<li class='mom_attrentry_cl'>" "<span class='mom_attritem_cl' data-momitemid='"), MOMOUT_LITERALV ((const char *) mom_string_cstr (idatv)), MOMOUT_LITERAL ("'>"), MOMOUT_HTML (mom_string_cstr (namidatv)),	//
			MOMOUT_LITERAL ("</span> " "&#8594;"	/* U+2192 RIGHTWARDS ARROW → */
					" "), NULL);
	  mom_unlock_item (_L (webx).pitem);
	  mom_item_tasklet_push_frame	///
	    (momtasklet_, _C (editvalueclos),
	     MOMPFR_FIVE_VALUES (_L (editor),
				 _L (webx),
				 _L (curvalattr),
				 jorigat,
				 exprat), MOMPFR_INT ((intptr_t) 0), NULL);
	  _SET_STATE (didattredit);
	}
	////
      ajaxobjs_lab_didattredit:
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom didattredit atix="),
		   MOMOUT_DEC_INT ((int) _N (atix)));
	mom_lock_item (_L (webx).pitem);
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL ("</li>"), MOMOUT_NEWLINE (), NULL);
	mom_unlock_item (_L (webx).pitem);
	continue;
      }				// end for atix
    MOM_DEBUG (run,
	       MOMOUT_LITERAL ("ajax_objects_codmom endloop nbattrs="),
	       MOMOUT_DEC_INT ((int) _N (nbattrs)));
    {
      mom_lock_item (_L (editeditm).pitem);
      _L (curcontent) = _L (editeditm).pitem->i_content;
      mom_unlock_item (_L (editeditm).pitem);
    }				// end for atix
    MOM_DEBUG (run,
	       MOMOUT_LITERAL ("ajax_objects_codmom curcontent="),
	       MOMOUT_VALUE ((const momval_t) _L (curcontent)));
    mom_lock_item (_L (webx).pitem);
    MOM_WEBX_OUT (_L (webx).pitem,
		  MOMOUT_LITERAL ("</ul>"), MOMOUT_NEWLINE (), NULL);
    MOM_WEBX_OUT (_L (webx).pitem,
		  MOMOUT_LITERAL ("<p class='mom_content_cl'>" "&#8281; "
				  /* U+2059 FIVE DOT PUNCTUATION ⁙ */ ),
		  MOMOUT_NEWLINE (), NULL);
    mom_unlock_item (_L (webx).pitem);
    momval_t jorigcont = (momval_t)
      mom_make_json_object (MOMJSOB_ENTRY ((momval_t) mom_named__kind,
					   (momval_t) mom_named__content),
			    MOMJSOB_ENTRY ((momval_t) mom_named__item,
					   (momval_t)
					   mom_item_get_idstr
					   (mom_value_to_item
					    (_L (editeditm)))), NULL);
    momval_t exprcont = (momval_t)
      mom_make_node_til_nil (mom_named__content, _L (editeditm), NULL);
    MOM_DEBUG (run,
	       MOMOUT_LITERAL ("ajax_objects_codmom momtasklet_="),
	       MOMOUT_VALUE ((const momval_t) momtasklet_),
	       MOMOUT_LITERAL ("; jorigcont="),
	       MOMOUT_VALUE ((const momval_t) jorigcont),
	       MOMOUT_LITERAL ("; exprcont="),
	       MOMOUT_VALUE ((const momval_t) exprcont));
    mom_item_tasklet_push_frame	///
      (momtasklet_, _C (editvalueclos),
       MOMPFR_FIVE_VALUES (_L (editor),
			   _L (webx),
			   _L (curcontent),
			   jorigcont,
			   exprcont), MOMPFR_INT ((intptr_t) 0), NULL);
    _SET_STATE (didcontentedit);
  ajaxobjs_lab_didcontentedit:
    MOM_DEBUG (run,
	       MOMOUT_LITERAL
	       ("ajax_objects_codmom didcontentedit curcontent="),
	       MOMOUT_VALUE (_L (curcontent)));
    mom_lock_item (_L (webx).pitem);
    {
      const char *editoridstr =
	mom_string_cstr ((momval_t) mom_item_get_idstr (_L (editor).pitem));
      MOM_WEBX_OUT		//
	(_L (webx).pitem,
	 MOMOUT_LITERAL ("</div> <!-- end momeditor_"),
	 MOMOUT_LITERALV ((const char *) editoridstr),
	 MOMOUT_LITERAL (" -->"), MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL
	 ("<script type='text/javascript'>mom_add_editor_tab_id($('#momeditor_"),
	 MOMOUT_LITERALV ((const char *) editoridstr),
	 MOMOUT_LITERAL ("'),'"),
	 MOMOUT_LITERALV ((const char *) editoridstr),
	 MOMOUT_LITERAL ("');</script>"));
    }
    mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
    mom_unlock_item (_L (webx).pitem);
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
  .rout_magic = MOM_ROUTINE_MAGIC,.rout_minclosize =
    ajaxobjs_c__lastclosure,.rout_frame_nbval =
    ajaxobjs_v__lastval,.rout_frame_nbnum =
    ajaxobjs_n__lastnum,.rout_frame_nbdbl = 0,.rout_name =
    "ajax_objects",.rout_module = MONIMELT_CURRENT_MODULE,.rout_codefun =
    ajax_objects_codmom,.rout_timestamp = __DATE__ "@" __TIME__
};




////////////////////////////////////////////////////////////////
///// edit_value
enum edit_value_values_en
{
  edit_value_v_arg0res,		/* arg0 is the editor */
  edit_value_v_webx,
  edit_value_v_curval,
  edit_value_v_jorig,
  edit_value_v_expr,
  edit_value_v_spare,
  edit_value_v_editor,
  edit_value_v_connitm,
  edit_value_v_curson,
  edit_value_v__lastval
};

enum edit_value_closure_en
{
  edit_value_c_editors,
  edit_value_c__lastclosure
};

enum edit_value_numbers_en
{
  edit_value_n_depth,
  edit_value_n_numval,
  edit_value_n_nbsons,
  edit_value_n_sonix,
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
    edit_value_s_didsonedit,
    edit_value_s_impossible,
    edit_value_s__laststate
  };
#define _SET_STATE(St) do {					\
    MOM_DEBUGPRINTF (run,					\
		     "edit_value_codmom setstate " #St " = %d",	\
		     (int)edit_value_s_##St);			\
    return edit_value_s_##St; } while(0)
  if (momstate_ >= 0 && momstate_ < edit_value_s__laststate)
    switch ((enum edit_value_state_en) momstate_)
      {
      case edit_value_s_start:
	goto edit_value_lab_start;
      case edit_value_s_impossible:
	goto edit_value_lab_impossible;
      case edit_value_s_didsonedit:
	goto edit_value_lab_didsonedit;
      case edit_value_s__laststate:;
      }
  MOM_FATAPRINTF ("edit_value invalid state #%d", momstate_);
edit_value_lab_start:
  MOM_DEBUG (run, MOMOUT_LITERAL ("edit_value start arg0res="),
	     MOMOUT_VALUE ((const momval_t) _L (arg0res)),
	     MOMOUT_LITERAL ("; webx="),
	     MOMOUT_VALUE ((const momval_t) _L (webx)),
	     MOMOUT_LITERAL ("; jorig="),
	     MOMOUT_VALUE ((const momval_t) _L (jorig)),
	     MOMOUT_LITERAL ("; curval="),
	     MOMOUT_VALUE ((const momval_t) _L (curval)),
	     MOMOUT_LITERAL ("; expr="),
	     MOMOUT_VALUE ((const momval_t) _L (expr)));
  if (_L (arg0res).ptr == MOM_EMPTY)
    _SET_STATE (impossible);
  _L (editor) = _L (arg0res);
  if (mom_lock_item (_L (editor).pitem))
    {
      mom_item_vector_append1	///
	(_L (editor).pitem,
	 (momval_t) mom_make_node_til_nil (mom_named__val,
					   _L (curval), _L (expr), NULL));
      _N (numval) = mom_item_vector_count (_L (editor).pitem);
      mom_unlock_item (_L (editor).pitem);
    }
  MOM_DEBUG (run, MOMOUT_LITERAL ("edit_value numval="),
	     MOMOUT_DEC_INT ((int) _N (numval)));
  assert (_N (numval) > 0);
  if (mom_lock_item (_L (webx).pitem))
    {
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_LITERAL ("<span class='mom_value_cl' id='momedval_"), MOMOUT_LITERALV (mom_ident_cstr_of_item (mom_value_to_item (_L (editor)))),	//
		    MOMOUT_LITERAL ("_n"), MOMOUT_DEC_INT ((int) _N (numval)), MOMOUT_LITERAL ("' data-momeditor='"), MOMOUT_LITERALV (mom_ident_cstr_of_item (mom_value_to_item (_L (editor)))),	//
		    MOMOUT_LITERAL ("' data-momorig='"), MOMOUT_JSON_VALUE (_L (jorig)),	//
		    MOMOUT_LITERAL ("' data-momnumval='"), MOMOUT_DEC_INT ((int) _N (numval)),	//
		    MOMOUT_LITERAL ("' data-momtype='"), MOMOUT_LITERALV (mom_type_cstring (mom_type (_L (curval)))),	//
		    MOMOUT_LITERAL ("'>"),	// end span
		    NULL);
      switch ((enum momvaltype_en) mom_type (_L (curval)))
	{
	case momty_null:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL
			("<span class='mom_nullval_cl'>_</span>"), NULL);
	  break;
	case momty_int:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL ("&nbsp;<span class='mom_intval_cl'>"),
			MOMOUT_JSON_VALUE (_L (curval)),
			MOMOUT_LITERAL ("</span>"), NULL);
	  break;
	case momty_double:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL
			("&nbsp;<span class='mom_doubleval_cl'>"),
			MOMOUT_JSON_VALUE (_L (curval)),
			MOMOUT_LITERAL ("</span>"), NULL);
	  break;
	case momty_string:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL ("&#8220;"	/* U+201C LEFT DOUBLE QUOTATION MARK “ */
					"<span class='mom_stringval_cl'>"), MOMOUT_HTML ((const char *) mom_string_cstr (_L (curval))), MOMOUT_LITERAL ("</span>" "&#8221;"	/* U+201D RIGHT DOUBLE QUOTATION MARK ” */
			), NULL);
	  break;
	case momty_jsonarray:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL ("<span class='mom_jsonarrayval_cl'>"),
			MOMOUT_HTML ((const char *)
				     mom_string_cstr (MOM_OUTSTRING
						      (outf_jsonhalfindent,
						       MOMOUT_JSON_VALUE (_L
									  (curval))))),
			MOMOUT_LITERAL ("</span>"), NULL);
	  break;
	case momty_jsonobject:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_LITERAL
			("<span class='mom_jsonobjectval_cl'>"),
			MOMOUT_HTML ((const char *)
				     mom_string_cstr (MOM_OUTSTRING
						      (outf_jsonhalfindent,
						       MOMOUT_JSON_VALUE (_L
									  (curval))))),
			MOMOUT_LITERAL ("</span>"), NULL);
	  break;
	case momty_item:
	  {
	    momval_t nsidv =
	      (momval_t)
	      mom_item_get_name_or_idstr (mom_value_to_item (_L (curval)));
	    bool isanonym =
	      mom_looks_like_random_id_cstr (mom_string_cstr (nsidv), NULL);
	    if (isanonym)
	      MOM_WEBX_OUT (_L (webx).pitem,
			    //
			    MOMOUT_LITERAL
			    ("<span class='mom_anonymousitemval_cl' id='momitem"),
			    MOMOUT_LITERALV (mom_string_cstr (nsidv)),
			    MOMOUT_LITERAL ("'>"),
			    MOMOUT_LITERALV (mom_string_cstr (nsidv)),
			    MOMOUT_LITERAL ("</span>"), NULL);
	    else
	      MOM_WEBX_OUT (_L (webx).pitem,
			    //
			    MOMOUT_LITERAL
			    ("<span class='mom_nameditemval_cl' id='momitem'"),
			    MOMOUT_LITERALV (mom_string_cstr
					     ((momval_t)
					      mom_item_get_idstr
					      (mom_value_to_item
					       (_L (curval))))),
			    MOMOUT_LITERAL (">"),
			    MOMOUT_LITERALV (mom_string_cstr (nsidv)),
			    MOMOUT_LITERAL ("</span>"), NULL);
	    break;
	  }
	case momty_set:
	case momty_tuple:
	  {
	    unsigned slen = mom_seqitem_length (_L (curval));
	    bool isset = mom_is_set (_L (curval));
	    if (slen == 0)
	      {
		if (isset)	// the empty set
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL ("<span class='mom_emptysetval_cl'>" "&#8709;"	/* U+2205 EMPTY SET ∅ */
						"</span>"), NULL);
		else		// the empty tuple
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL
				("<span class='mom_emptytupleval_cl'>" "[]"
				 "</span>"), NULL);
	      }
	    else
	      {
		unsigned ix = 0;
		if (isset)
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL
				("<span class='mom_setval_cl'>{"), NULL);
		else
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL
				("<span class='mom_tupleval_cl'>["), NULL);
		for (ix = 0; ix < slen; ix++)
		  {
		    if (ix > 0)
		      MOM_WEBX_OUT (_L (webx).pitem,
				    //
				    MOMOUT_LITERAL (","), MOMOUT_SPACE (48),
				    NULL);
		    momitem_t *cursubitem =
		      mom_seqitem_nth_item (_L (curval), ix);
		    if (!cursubitem)
		      MOM_WEBX_OUT (_L (webx).pitem,
				    //
				    MOMOUT_LITERAL
				    ("<span class='mom_nullitem_cl'>_</span>"),
				    NULL);
		    else
		      {
			momval_t nsubidv =
			  (momval_t) mom_item_get_name_or_idstr (cursubitem);
			bool subisanonym =
			  mom_looks_like_random_id_cstr (mom_string_cstr
							 (nsubidv), NULL);
			if (subisanonym)
			  MOM_WEBX_OUT (_L (webx).pitem,
					//
					MOMOUT_LITERAL
					("<span class='mom_anonymousitemval_cl' id='momitem"),
					MOMOUT_LITERALV (mom_string_cstr
							 (nsubidv)),
					MOMOUT_LITERAL ("'>"),
					MOMOUT_LITERALV (mom_string_cstr
							 (nsubidv)),
					MOMOUT_LITERAL ("</span>"), NULL);
			else
			  MOM_WEBX_OUT (_L (webx).pitem,
					//
					MOMOUT_LITERAL
					("<span class='mom_nameditemval_cl' id='momitem"),
					MOMOUT_LITERALV (mom_string_cstr
							 ((momval_t)
							  mom_item_get_idstr
							  (cursubitem))),
					MOMOUT_LITERAL ("'>"),
					MOMOUT_LITERALV (mom_string_cstr
							 (nsubidv)),
					MOMOUT_LITERAL ("</span>"), NULL);
		      }
		  }
		if (isset)
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL ("}</span>"), NULL);
		else
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL ("]</span>"), NULL);
	      }
	    break;
	  }			// end case momty_set, momty_tuple
	case momty_node:
	  {
	    _L (connitm) = (momval_t) mom_node_conn (_L (curval));
	    _N (nbsons) = mom_node_arity (_L (curval));
	    MOM_DEBUG (run, MOMOUT_LITERAL ("edit_value node connitm="),
		       MOMOUT_VALUE (_L (connitm)),
		       MOMOUT_LITERAL ("; nbsons="),
		       MOMOUT_DEC_INT ((int) _N (nbsons)), NULL);
	    MOM_WEBX_OUT (_L (webx).pitem,
			  //
			  MOMOUT_LITERAL
			  ("<span class='mom_nodeval_cl'>*"), NULL);
	    {
	      momval_t connidv =
		(momval_t) mom_item_get_name_or_idstr (_L (connitm).pitem);
	      bool connisanonym =
		mom_looks_like_random_id_cstr (mom_string_cstr (connidv),
					       NULL);
	      if (connisanonym)
		MOM_WEBX_OUT (_L (webx).pitem,
			      //
			      MOMOUT_LITERAL
			      ("<span class='mom_anonymousitemval_cl' id='momitem'"),
			      MOMOUT_LITERALV (mom_string_cstr (connidv)),
			      MOMOUT_LITERAL (">"),
			      MOMOUT_LITERALV (mom_string_cstr (connidv)),
			      MOMOUT_LITERAL ("</span>("), NULL);
	      else
		MOM_WEBX_OUT (_L (webx).pitem,
			      //
			      MOMOUT_LITERAL
			      ("<span class='mom_nameditemval_cl' id='momitem'"),
			      MOMOUT_LITERALV (mom_string_cstr
					       ((momval_t)
						mom_item_get_idstr (_L
								    (connitm).pitem))),
			      MOMOUT_LITERAL (">"),
			      MOMOUT_LITERALV (mom_string_cstr (connidv)),
			      MOMOUT_LITERAL ("</span>("),
			      MOMOUT_INDENT_MORE (), NULL);
	    }
	    for (_N (sonix) = 0; _N (sonix) < _N (nbsons); _N (sonix)++)
	      {
		if (_N (sonix) > 0)
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_LITERAL (","), MOMOUT_SPACE (48),
				NULL);
		mom_unlock_item (_L (webx).pitem);
		_L (curson) = mom_node_nth (_L (curval), _N (sonix));
		momval_t jnodorig = (momval_t) mom_make_json_object	//
		  (MOMJSOB_ENTRY ((momval_t) mom_named__kind,
				  (momval_t) mom_named__node),
		   MOMJSOB_ENTRY ((momval_t) mom_named__node,
				  mom_make_integer (_N (numval))),
		   MOMJSOB_ENTRY ((momval_t) mom_named__sons,
				  mom_make_integer (_N (sonix))), NULL);
		momval_t nodexpr = (momval_t)
		  mom_make_node_til_nil (mom_named__node,
					 mom_make_integer (_N (numval)),
					 mom_make_integer (_N (sonix)), NULL);
		mom_item_tasklet_push_frame	//
		  (momtasklet_, (momval_t) momclosure_,
		   MOMPFR_FIVE_VALUES (_L (editor), _L (webx), _L (curson),
				       jnodorig,
				       nodexpr),
		   MOMPFR_INT (_N (depth) + 1), NULL);
		_SET_STATE (didsonedit);
	      edit_value_lab_didsonedit:
		if (!mom_lock_item (_L (webx).pitem))
		  break;
	      }
	    MOM_WEBX_OUT (_L (webx).pitem,
			  //
			  MOMOUT_INDENT_LESS (),
			  MOMOUT_LITERAL (")</span>"), NULL);
	  }
	  break;
	}
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_LITERAL ("</span>"), NULL);
      mom_unlock_item (_L (webx).pitem);
    }
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
  .rout_magic = MOM_ROUTINE_MAGIC,.rout_minclosize =
    edit_value_c__lastclosure,.rout_frame_nbval =
    edit_value_v__lastval,.rout_frame_nbnum =
    edit_value_n__lastnum,.rout_frame_nbdbl = 0,.rout_name =
    "edit_value",.rout_module = MONIMELT_CURRENT_MODULE,.rout_codefun =
    edit_value_codmom,.rout_timestamp = __DATE__ "@" __TIME__
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
  .rout_magic = MOM_ROUTINE_MAGIC,.rout_minclosize =
    ajaxcompnam_c__lastclosure,.rout_frame_nbval =
    ajaxcompnam_v__lastval,.rout_frame_nbnum =
    ajaxcompnam_n__lastnum,.rout_frame_nbdbl = 0,.rout_name =
    "ajax_complete_name",.rout_module =
    MONIMELT_CURRENT_MODULE,.rout_codefun =
    ajax_complete_name_codmom,.rout_timestamp = __DATE__ "@" __TIME__
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
  .rout_magic = MOM_ROUTINE_MAGIC,.rout_minclosize =
    noop_c__lastclosure,.rout_frame_nbval =
    noop_v__lastval,.rout_frame_nbnum = noop_n__lastnum,.rout_frame_nbdbl =
    0,.rout_name = "noop",.rout_module =
    MONIMELT_CURRENT_MODULE,.rout_codefun = noop_codmom,.rout_timestamp =
    __DATE__ "@" __TIME__
};
