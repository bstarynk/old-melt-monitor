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
    if (mom_string_same (todov, "mom_menuitem_sys_close"))
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
    else if (mom_string_same (todov, "mom_menuitem_sys_quit"))
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
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom webx postjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_post (_L (webx).pitem)),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" queryjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_query (_L (webx).pitem)),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" fullpath="),
	       MOMOUT_VALUE ((const momval_t) (const momval_t)
			     mom_webx_fullpath ((_L (webx).pitem))),
	       MOMOUT_LITERAL ("; method="),
	       MOMOUT_VALUE ((const momval_t) (const momval_t)
			     mom_webx_method ((_L (webx).pitem))));
    momval_t todov = mom_webx_post_arg (_L (webx).pitem, "todo_mom");
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom todov="),
	       MOMOUT_VALUE ((const momval_t) todov));
    if (mom_string_same (todov, "mom_menuitem_obj_named"))
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
		       " <input id='mom_name_input' class='mom_nameinput_cl' name='mom_name' onChange='mom_name_input_changed(this)'/>"
		       " <input type='submit' id='mom_cancel' class='mom_cancel_cl' value='cancel' onclick='mom_erase_maindiv()'/>"),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL
		      ("<script>mom_set_name_entry($('#mom_name_input'));"),
		      MOMOUT_SPACE (32), MOMOUT_LITERAL ("</script>"),
		      MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
	goto end;
      }
    else if (mom_string_same (todov, "mom_menuitem_obj_new"))
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
	if (_L (editeditm).ptr)
	  {
	    mom_unlock_item (_L (webx).pitem);
	    _SET_STATE (beginedit);
	  }
	else
	  {
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL ("Unknown item <tt>"),
			  MOMOUT_HTML (mom_string_cstr (namev)),
			  MOMOUT_LITERAL ("</tt>"));
	    mom_webx_reply (_L (webx).pitem, "text/html", HTTP_NOT_FOUND);
	    MOM_WARNPRINTF ("unknown item to edit named %s",
			    mom_string_cstr (namev));
	    goto end;
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
      /*** see comment in mom-scripts.js regarding mom_install_editor
	   javascript function; we should send something like:

   { "momeditorj_id": "_0u15i1z87ei_jf2wwmpim72",       // editor id
     "momeditorj_tabtitle": "<span...",      // the HTML for the tab title
     "momeditorj_tabcontent": "<div..."      // the HTML for the tab content
   }

      ****/
      mom_lock_item (_L (webx).pitem);

      {
	// output the editor id JSON field:
	const char *editoridstr =
	  mom_string_cstr ((momval_t) mom_item_get_idstr (_L (editor).pitem));
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL ("{ \"momeditorj_id\": \""),
		      MOMOUT_JS_STRING (editoridstr),
		      MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());


	// output the tab title JSON field
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL ("  \"momeditorj_tabtitle\": \""));
	if (anonymous)
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_JS_LITERAL
			("<span class='mom_editabanon_cl' id='momeditab"),
			MOMOUT_LITERALV (editoridstr),
			MOMOUT_JS_LITERAL ("'>"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((idv))),
			MOMOUT_JS_LITERAL ("</span>"));
	else
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_JS_LITERAL
			("<span class='mom_editabnamed_cl' id='momeditab"),
			MOMOUT_LITERALV (editoridstr),
			MOMOUT_JS_LITERAL ("'>"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((namidv))),
			MOMOUT_JS_LITERAL ("</span>"));
	MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL ("\","),
		      MOMOUT_NEWLINE ());

	// begin outputting the tab content JS field
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL ("  \"momeditorj_tabcontent\": \""),
		      MOMOUT_JS_LITERAL ("<div id='momeditor"),
		      MOMOUT_JS_LITERALV ((const char *) editoridstr),
		      MOMOUT_JS_LITERAL ("' class='mom_editor_cl'>"),
		      MOMOUT_JS_RAW_NEWLINE (),
		      MOMOUT_JS_LITERAL ("<p class='mom_edit_title_cl'>"));
      }
      if (anonymous)
	{
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_JS_LITERAL
			("anonymous item <tt class='mom_edititemid_cl'"),
			MOMOUT_JS_LITERAL
			(" data-momitemid='"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((momval_t) idv)),
			MOMOUT_JS_LITERAL
			("'>"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((momval_t) idv)),
			MOMOUT_JS_LITERAL ("</tt>"),
			MOMOUT_JS_RAW_NEWLINE ());
	}
      else
	{
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_JS_LITERAL
			("item <tt class='mom_edititemname_cl' data-momitemid='"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((momval_t) idv)),
			MOMOUT_JS_LITERAL
			("'>"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((momval_t)
							     namidv)),
			MOMOUT_JS_LITERAL
			("</tt><br/><small>of id: </small><code class='mom_itemid_cl'>"),
			MOMOUT_JS_LITERALV ((const char *)
					    mom_string_cstr ((momval_t) idv)),
			MOMOUT_JS_LITERAL ("</code>"),
			MOMOUT_JS_RAW_NEWLINE ());
	}
      MOM_WEBX_OUT (_L (webx).pitem,
		    MOMOUT_JS_LITERAL ("<br/>"),
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_editdate_cl'>edited at "),
		    MOMOUT_DOUBLE_TIME ((const char *) "%c",
					mom_clock_time (CLOCK_REALTIME)),
		    MOMOUT_JS_LITERAL ("</span></p>"));
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL ("<div class='mom_attributes_cl'>"),
		    MOMOUT_JS_LITERAL ("<p class='mom_attrtitle_cl'>"),
		    MOMOUT_DEC_INT ((int) _N (nbattrs)),
		    MOMOUT_JS_LITERAL (" attributes:"),
		    MOMOUT_JS_LITERAL ("</p>"), MOMOUT_JS_RAW_NEWLINE ());
      //
      MOM_WEBX_OUT (_L (webx).pitem,
		    MOMOUT_JS_LITERAL ("<ul class='mom_attrlist_cl'>"),
		    MOMOUT_JS_RAW_NEWLINE ());
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
			MOMOUT_JS_LITERAL ("<li class='mom_attrentry_cl'>" "<span class='mom_attritem_cl' data-momitemid='"), MOMOUT_JS_LITERALV ((const char *) mom_string_cstr (idatv)), MOMOUT_JS_LITERAL ("'>"), MOMOUT_HTML (mom_string_cstr (namidatv)),	//
			MOMOUT_JS_LITERAL ("</span> " "&#8594;"	/* U+2192 RIGHTWARDS ARROW → */
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
		      MOMOUT_JS_LITERAL ("</li>"), MOMOUT_JS_RAW_NEWLINE (),
		      NULL);
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
		  MOMOUT_JS_LITERAL ("</ul>"), MOMOUT_JS_RAW_NEWLINE (),
		  MOMOUT_JS_LITERAL ("</div>"), MOMOUT_JS_RAW_NEWLINE (),
		  NULL);
    MOM_WEBX_OUT (_L (webx).pitem,
		  MOMOUT_JS_LITERAL ("<p class='mom_content_cl'>" "&#8281; "
				     /* U+2059 FIVE DOT PUNCTUATION ⁙ */ ),
		  MOMOUT_JS_RAW_NEWLINE (), NULL);
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
	 MOMOUT_JS_LITERAL ("</div>"), MOMOUT_JS_RAW_NEWLINE (),
	 MOMOUT_LITERAL ("\" "),
	 MOMOUT_NEWLINE (), MOMOUT_LITERAL ("}") /* to end the JSON */ ,
	 MOMOUT_NEWLINE ());
    }
    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
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
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = ajaxobjs_c__lastclosure,	//
  .rout_frame_nbval = ajaxobjs_v__lastval,	//
  .rout_frame_nbnum = ajaxobjs_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "ajax_objects",	//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = ajax_objects_codmom,	//
  .rout_timestamp = __DATE__ "@" __TIME__
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
      _N (numval) = mom_item_vector_count (_L (editor).pitem);
      mom_item_vector_append1	///
	(_L (editor).pitem,
	 (momval_t) mom_make_node_sized (mom_named__val, 2,
					 _L (curval), _L (expr), NULL));
      mom_unlock_item (_L (editor).pitem);
    }
  else
    _N (numval) = -1;
  MOM_DEBUG (run, MOMOUT_LITERAL ("edit_value numval="),
	     MOMOUT_DEC_INT ((int) _N (numval)));
  assert (_N (numval) >= 0);
  if (mom_lock_item (_L (webx).pitem))
    {
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL ("<span class='mom_value_cl' id='momedval"), MOMOUT_LITERALV (mom_ident_cstr_of_item (mom_value_to_item (_L (editor)))),	//
		    MOMOUT_LITERAL ("_N"), MOMOUT_DEC_INT ((int) _N (numval)),
		    // no need for a data-momeditor, it can be found from the id!
		    MOMOUT_JS_LITERAL ("' data-momorig='"));
      {
	struct momout_st outb = { 0 };
	mom_initialize_buffer_output (&outb, 0);
	MOM_OUT (&outb, MOMOUT_JSON_VALUE (_L (jorig)), MOMOUT_FLUSH ());
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_JS_STRING ((const char *) outb.mout_data),
		      //
		      MOMOUT_JS_LITERAL ("' data-momnumval='"), MOMOUT_DEC_INT ((int) _N (numval)),	//
		      MOMOUT_JS_LITERAL ("' data-momtype='"), MOMOUT_LITERALV (mom_type_cstring (mom_type (_L (curval)))),	//
		      MOMOUT_JS_LITERAL ("'>"),	// end span
		      NULL);
	mom_finalize_buffer_output (&outb);
      }
      switch ((enum momvaltype_en) mom_type (_L (curval)))
	{
	case momty_null:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_JS_LITERAL
			("<span class='mom_nullval_cl'>_</span>"), NULL);
	  break;
	case momty_int:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_JS_LITERAL
			("&nbsp;<span class='mom_intval_cl'>"),
			MOMOUT_JSON_VALUE (_L (curval)),
			MOMOUT_JS_LITERAL ("</span>"), NULL);
	  break;
	case momty_double:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_JS_LITERAL
			("&nbsp;<span class='mom_doubleval_cl'>"),
			MOMOUT_JSON_VALUE (_L (curval)),
			MOMOUT_JS_LITERAL ("</span>"), NULL);
	  break;
	case momty_string:
	  MOM_WEBX_OUT (_L (webx).pitem,
			//
			MOMOUT_JS_LITERAL ("&#8220;"	/* U+201C LEFT DOUBLE QUOTATION MARK “ */
					   "<span class='mom_stringval_cl'>"), MOMOUT_JS_HTML ((const char *) mom_string_cstr (_L (curval))), MOMOUT_JS_LITERAL ("</span>" "&#8221;"	/* U+201D RIGHT DOUBLE QUOTATION MARK ” */
			), NULL);
	  break;
	case momty_jsonarray:
	  {
	    struct momout_st outb = { 0 };
	    mom_initialize_buffer_output (&outb, outf_jsonhalfindent);
	    MOM_OUT (&outb, MOMOUT_JSON_VALUE (_L (curval)), MOMOUT_FLUSH ());
	    MOM_WEBX_OUT (_L (webx).pitem,
			  //
			  MOMOUT_JS_LITERAL
			  ("<span class='mom_jsonarrayval_cl'>"),
			  MOMOUT_JS_HTML ((const char *) outb.mout_data),
			  MOMOUT_JS_LITERAL ("</span>"), NULL);
	    mom_finalize_buffer_output (&outb);
	  }
	  break;
	case momty_jsonobject:
	  {
	    struct momout_st outb = { 0 };
	    mom_initialize_buffer_output (&outb, outf_jsonhalfindent);
	    MOM_OUT (&outb, MOMOUT_JSON_VALUE (_L (curval)), MOMOUT_FLUSH ());
	    MOM_WEBX_OUT (_L (webx).pitem,
			  //
			  MOMOUT_JS_LITERAL
			  ("<span class='mom_jsonobjectval_cl'>"),
			  MOMOUT_JS_HTML ((const char *) outb.mout_data),
			  MOMOUT_JS_LITERAL ("</span>"), NULL);
	    mom_finalize_buffer_output (&outb);
	  }
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
			    MOMOUT_JS_LITERAL
			    ("<span class='mom_anonymousitemval_cl' id='momitem"),
			    MOMOUT_JS_LITERALV (mom_string_cstr (nsidv)),
			    MOMOUT_JS_LITERAL ("'>"),
			    MOMOUT_JS_LITERALV (mom_string_cstr (nsidv)),
			    MOMOUT_JS_LITERAL ("</span>"), NULL);
	    else
	      MOM_WEBX_OUT (_L (webx).pitem,
			    //
			    MOMOUT_JS_LITERAL
			    ("<span class='mom_nameditemval_cl' id='momitem"),
			    MOMOUT_JS_LITERALV (mom_string_cstr
						((momval_t)
						 mom_item_get_idstr
						 (mom_value_to_item
						  (_L (curval))))),
			    MOMOUT_JS_LITERAL ("'>"),
			    MOMOUT_JS_LITERALV (mom_string_cstr (nsidv)),
			    MOMOUT_JS_LITERAL ("</span>"), NULL);
	  }
	  break;
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
				MOMOUT_JS_LITERAL ("<span class='mom_emptysetval_cl'>" "&#8709;"	/* U+2205 EMPTY SET ∅ */
						   "</span>"), NULL);
		else		// the empty tuple
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_JS_LITERAL
				("<span class='mom_emptytupleval_cl'>" "[]"
				 "</span>"), NULL);
	      }
	    else
	      {
		unsigned ix = 0;
		if (isset)
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_JS_LITERAL
				("<span class='mom_setval_cl'>{"), NULL);
		else
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_JS_LITERAL
				("<span class='mom_tupleval_cl'>["), NULL);
		for (ix = 0; ix < slen; ix++)
		  {
		    if (ix > 0)
		      MOM_WEBX_OUT (_L (webx).pitem,
				    //
				    MOMOUT_JS_LITERAL (", "), NULL);
		    momitem_t *cursubitem =
		      mom_seqitem_nth_item (_L (curval), ix);
		    if (!cursubitem)
		      MOM_WEBX_OUT (_L (webx).pitem,
				    //
				    MOMOUT_JS_LITERAL
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
					MOMOUT_JS_LITERAL
					("<span class='mom_anonymousitemval_cl' id='momitem"),
					MOMOUT_JS_LITERALV (mom_string_cstr
							    (nsubidv)),
					MOMOUT_JS_LITERAL ("'>"),
					MOMOUT_JS_LITERALV (mom_string_cstr
							    (nsubidv)),
					MOMOUT_JS_LITERAL ("</span>"), NULL);
			else
			  MOM_WEBX_OUT (_L (webx).pitem,
					//
					MOMOUT_JS_LITERAL
					("<span class='mom_nameditemval_cl' id='momitem"),
					MOMOUT_JS_LITERALV (mom_string_cstr
							    ((momval_t)
							     mom_item_get_idstr
							     (cursubitem))),
					MOMOUT_JS_LITERAL ("'>"),
					MOMOUT_JS_LITERALV (mom_string_cstr
							    (nsubidv)),
					MOMOUT_JS_LITERAL ("</span>"), NULL);
		      }
		  }
		if (isset)
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_JS_LITERAL ("}</span>"), NULL);
		else
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_JS_LITERAL ("]</span>"), NULL);
	      }
	  }			// end case momty_set, momty_tuple
	  break;
	case momty_node:
	  {
	    _L (connitm) = (momval_t) mom_node_conn (_L (curval));
	    _N (nbsons) = mom_node_arity (_L (curval));
	    MOM_DEBUG (run, MOMOUT_JS_LITERAL ("edit_value node connitm="),
		       MOMOUT_VALUE (_L (connitm)),
		       MOMOUT_JS_LITERAL ("; nbsons="),
		       MOMOUT_DEC_INT ((int) _N (nbsons)), NULL);
	    MOM_WEBX_OUT (_L (webx).pitem,
			  //
			  MOMOUT_JS_LITERAL
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
			      MOMOUT_JS_LITERAL
			      ("<span class='mom_anonymousitemval_cl' id='momitem"),
			      MOMOUT_JS_LITERALV (mom_string_cstr (connidv)),
			      MOMOUT_JS_LITERAL ("'>"),
			      MOMOUT_JS_LITERALV (mom_string_cstr (connidv)),
			      MOMOUT_JS_LITERAL ("</span>("), NULL);
	      else
		MOM_WEBX_OUT (_L (webx).pitem,
			      //
			      MOMOUT_JS_LITERAL
			      ("<span class='mom_nameditemval_cl' id='momitem"),
			      MOMOUT_JS_LITERALV (mom_string_cstr
						  ((momval_t)
						   mom_item_get_idstr (_L
								       (connitm).pitem))),
			      MOMOUT_JS_LITERAL ("'>"),
			      MOMOUT_JS_LITERALV (mom_string_cstr (connidv)),
			      MOMOUT_JS_LITERAL ("</span>("),
			      MOMOUT_INDENT_MORE (), NULL);
	    }
	    for (_N (sonix) = 0; _N (sonix) < _N (nbsons); _N (sonix)++)
	      {
		if (_N (sonix) > 0)
		  MOM_WEBX_OUT (_L (webx).pitem,
				//
				MOMOUT_JS_LITERAL (", "), NULL);
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
			  MOMOUT_JS_LITERAL (")</span>"), NULL);
	  }			/* end case momty_node */
	  break;
	}
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL ("</span>"), NULL);
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
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = edit_value_c__lastclosure,	//
  .rout_frame_nbval = edit_value_v__lastval,	//
  .rout_frame_nbnum = edit_value_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "edit_value",	//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = edit_value_codmom,	//
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
	MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JSON_VALUE (jsarrnames));
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
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = ajaxcompnam_c__lastclosure,	//
  .rout_frame_nbval = ajaxcompnam_v__lastval,	//
  .rout_frame_nbnum = ajaxcompnam_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "ajax_complete_name",	//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = ajax_complete_name_codmom,	//
  .rout_timestamp = __DATE__ "@" __TIME__
};


////////////////////////////////////////////////////////////////
///// ajax_edit
enum ajax_edit_values_en
{
  ajaxedit_v_arg0res,
  ajaxedit_v_method,
  ajaxedit_v_namid,
  ajaxedit_v_restpath,
  ajaxedit_v__spare,
  ajaxedit_v_webx,
  ajaxedit_v_editor,
  ajaxedit_v_edinode,
  ajaxedit_v_curval,
  ajaxedit_v_origin,
  ajaxedit_v__lastval
};

enum ajax_edit_closure_en
{
  ajaxedit_c_edit_value,
  ajaxedit_c_editors,
  ajaxedit_c__lastclosure
};

enum ajax_edit_numbers_en
{
  ajaxedit_n__lastnum
};


static int
ajax_edit_codmom (int momstate_, momitem_t *momtasklet_,
		  const momnode_t *momclosure_,
		  momval_t *momlocvals_, intptr_t * momlocnums_,
		  double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[ajaxedit_v_##Nam])
#define _C(Nam) (momclosure_->sontab[ajaxedit_c_##Nam])
#define _N(Nam) (momlocnums_[ajaxedit_n_##Nam])
  enum ajax_edit_state_en
  {
    ajaxedit_s_start,
    ajaxedit_s_dideditcopy,
    ajaxedit_s__laststate
  };
#define _SET_STATE(St) do {					\
    MOM_DEBUGPRINTF (run,					\
		     "ajax_edit_codmom setstate " #St " = %d",	\
		     (int)ajaxedit_s_##St);			\
    return ajaxedit_s_##St; } while(0)
  //
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom tasklet:"),
	     MOMOUT_ITEM ((const momitem_t *) momtasklet_),
	     MOMOUT_LITERAL (" state#"), MOMOUT_DEC_INT ((int) momstate_));
  if (momstate_ >= 0 && momstate_ < ajaxedit_s__laststate)
    switch ((enum ajax_edit_state_en) momstate_)
      {
      case ajaxedit_s_start:
	goto ajaxedit_lab_start;
      case ajaxedit_s_dideditcopy:
	goto ajaxedit_lab_dideditcopy;
      case ajaxedit_s__laststate:;
      }
  MOM_FATAPRINTF ("ajax_edit invalid state #%d", momstate_);
  ////
ajaxedit_lab_start:
  _L (webx) = _L (arg0res);
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom webx="),
	     MOMOUT_VALUE ((const momval_t) _L (webx)));
  assert (mom_is_item (_L (webx)));
  {
    mom_lock_item (_L (webx).pitem);
    momval_t todov = mom_webx_post_arg (_L (webx).pitem, "todo_mom");
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom queryjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_query (_L (webx).pitem)),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" postjsob="),
	       MOMOUT_VALUE ((const momval_t)
			     mom_webx_jsob_post (_L (webx).pitem)),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" method="),
	       MOMOUT_VALUE ((const momval_t) (_L (method))),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" todo="),
	       MOMOUT_VALUE ((const momval_t) todov), NULL);
    if (mom_string_same (todov, "mom_menuitem_editval_copy"))
      {
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_edit_codmom editval_copy idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	char editidbuf[MOM_IDSTRING_LEN + 8];
	memset (editidbuf, 0, sizeof (editidbuf));
	int numval = -1;
	const char *end = NULL;
	if (idvalstr && !strncmp (idvalstr, "momedval", strlen ("momedval"))
	    && mom_looks_like_random_id_cstr (idvalstr + strlen ("momedval"),
					      &end) && end
	    && sscanf (end, "_N%d", &numval) > 0 && numval >= 0)
	  {
	    strncpy (editidbuf, idvalstr + strlen ("momedval"),
		     MOM_IDSTRING_LEN);
	    _L (editor) = (momval_t) (mom_get_item_of_identcstr (editidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_copy editidbuf="),
		       MOMOUT_LITERALV ((const char *) editidbuf),
		       MOMOUT_LITERAL ("; editor="),
		       MOMOUT_VALUE ((const momval_t) _L (editor)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end),
		       MOMOUT_LITERAL ("; numval="),
		       MOMOUT_DEC_INT ((int) numval));
	    {
	      mom_should_lock_item (_L (editor).pitem);
	      _L (edinode) = mom_item_vector_nth (_L (editor).pitem, numval);
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_copy edinode="),
		       MOMOUT_VALUE (_L (edinode)));
	  }
	else
	  MOM_FATAPRINTF ("ajax_edit bad idvalstr=%s end=%s numval=%d",
			  idvalstr, end, numval);
	/// here we got the correct edinode. It should be a binary
	/// node of connective val whose first son is the copied
	/// value, and whose second son describes how to get it.
	assert (mom_node_conn (_L (edinode)) == mom_named__val);
	_L (curval) = mom_node_nth (_L (edinode), 0);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom editval_copy curval="),
		   MOMOUT_VALUE (_L (curval)));
	/**** we send a JSON like
	      { "momedit_do": "momedit_copytobuffer",
	      "momedit_editors_id": id of 'editors' item,
	      "momedit_content": "<span..." // HTML for the buffer content
              }
	****/
	const char *editorsidstr =
	  mom_string_cstr ((momval_t)
			   mom_item_get_idstr (_C (editors).pitem));
	assert (editorsidstr != NULL);
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("{ \"momedit_do\": \"momedit_copytobuffer\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL ("  \"momedit_editors_id\": \""),
		      MOMOUT_JS_STRING (editorsidstr), MOMOUT_LITERAL ("\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL ("  \"momedit_content\": \""), NULL);
	mom_item_tasklet_push_frame	//
	  (momtasklet_, (momval_t) _C (edit_value),
	   MOMPFR_FIVE_VALUES (_C (editors), _L (webx), _L (curval),
			       /*jorig: */ (momval_t) mom_named__buffer,
			       /*nodexpr: */ (momval_t) mom_named__buffer),
	   MOMPFR_INT ((intptr_t) 0), NULL);
	mom_unlock_item (_L (webx).pitem);
	_SET_STATE (dideditcopy);
      }				// end if todov is mom_menuitem_editval_copy
    //
    else if (mom_string_same (todov, "mom_menuitem_edititem_copy"))
      {
	momval_t iditemv = mom_webx_post_arg (_L (webx).pitem, "iditem_mom");
	_L (curval) = (momval_t) mom_make_item_of_ident (iditemv.pstring);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_edit_codmom edititem_copy curval="),
		   MOMOUT_VALUE (_L (curval)), NULL);
	/**** we send a JSON like
	      { "momedit_do": "momedit_copytobuffer",
	      "momedit_editors_id": id of 'editors' item,
	      "momedit_content": "<span..." // HTML for the buffer content
	      }
	****/
	const char *editorsidstr =
	  mom_string_cstr ((momval_t)
			   mom_item_get_idstr (_C (editors).pitem));
	assert (editorsidstr != NULL);
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("{ \"momedit_do\": \"momedit_copytobuffer\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL ("  \"momedit_editors_id\": \""),
		      MOMOUT_JS_STRING (editorsidstr), MOMOUT_LITERAL ("\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL ("  \"momedit_content\": \""), NULL);
	mom_item_tasklet_push_frame	//
	  (momtasklet_, (momval_t) _C (edit_value),
	   MOMPFR_FIVE_VALUES (_C (editors), _L (webx), _L (curval),
			       /*jorig: */ (momval_t) mom_named__buffer,
			       /*nodexpr: */ (momval_t) mom_named__buffer),
	   MOMPFR_INT ((intptr_t) 0), NULL);
	mom_unlock_item (_L (webx).pitem);
	_SET_STATE (dideditcopy);
      }				//// end if todo is mom_menuitem_edititem_copy

    //
    else if (mom_string_same (todov, "mom_prepare_editval_menu"))
      {
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom prepareditvalmenu idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	char editidbuf[MOM_IDSTRING_LEN + 8];
	memset (editidbuf, 0, sizeof (editidbuf));
	int numval = -1;
	const char *end = NULL;
	if (idvalstr && !strncmp (idvalstr, "momedval", strlen ("momedval"))
	    && mom_looks_like_random_id_cstr (idvalstr + strlen ("momedval"),
					      &end) && end
	    && sscanf (end, "_N%d", &numval) > 0 && numval >= 0)
	  {
	    strncpy (editidbuf, idvalstr + strlen ("momedval"),
		     MOM_IDSTRING_LEN);
	    _L (editor) = (momval_t) (mom_get_item_of_identcstr (editidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom  prepareditvalmenu editidbuf="),
		       MOMOUT_LITERALV ((const char *) editidbuf),
		       MOMOUT_LITERAL ("; editor="),
		       MOMOUT_VALUE ((const momval_t) _L (editor)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end),
		       MOMOUT_LITERAL ("; numval="),
		       MOMOUT_DEC_INT ((int) numval));
	    {
	      mom_should_lock_item (_L (editor).pitem);
	      _L (edinode) = mom_item_vector_nth (_L (editor).pitem, numval);
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom prepareditvalmenu edinode="),
		       MOMOUT_VALUE (_L (edinode)));
	  }
	else
	  MOM_FATAPRINTF ("ajax_edit bad idvalstr=%s end=%s numval=%d",
			  idvalstr, end, numval);
	/// here we got the correct edinode. It should be a binary
	/// node of connective val whose first son is the edited
	/// value, and whose second son describes how to get it.
	assert (mom_node_conn (_L (edinode)) == mom_named__val);
	_L (curval) = mom_node_nth (_L (edinode), 0);
	_L (origin) = mom_node_nth (_L (edinode), 1);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom prepareditvalmenu  curval="),
		   MOMOUT_VALUE (_L (curval)), MOMOUT_LITERAL (";"),
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom prepareditvalmenu origin"),
		   MOMOUT_VALUE (_L (origin)), NULL);
	if (mom_node_conn (_L (origin)) == mom_named__attr)
	  {			// node: *attr(<item>,<attr>)
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom prepareditvalmenu attr origin"));
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_add_to_editval_menu\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_menuval\": ["),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL
			  (" \"<li id='mom_menuitem_editval_removeattr'><a href='#'>Remove attribute</a></li>\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL
			  (" \"<li id='mom_menuitem_editval_replaceattr'><a href='#'>Replace attribute</a></li>\" ]"),
			  MOMOUT_NEWLINE (), MOMOUT_LITERAL ("}"),
			  MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
	else if (mom_node_conn (_L (origin)) == mom_named__node)
	  {			/* *node(<parentix>,<sonrank>) */
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom prepareditvalmenu node origin"));
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_add_to_editval_menu\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_menuval\": ["),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL
			  (" \"<li id='mom_menuitem_editval_removeson'><a href='#'>Remove son</a></li>\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL
			  (" \"<li id='mom_menuitem_editval_pasteson'><a href='#'>Paste as son</a></li>\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL
			  (" \"<li id='mom_menuitem_editval_replaceson'><a href='#'>Replace son</a></li>\" ]"),
			  MOMOUT_NEWLINE (), MOMOUT_LITERAL ("}"),
			  MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
      }				//// end if todo is mom_prepare_editval_menu
    else if (mom_string_same (todov, "mom_menuitem_editval_replaceson"))
      {
	/**** we send a JSON like
	      { "momedit_do": "momedit_replaceinput",
	        "momedit_oldid": <id-of-old-element>,
		"momedit_newid": <id-of-new-input>
	      }
	****/
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom editval_replace_son idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	char editidbuf[MOM_IDSTRING_LEN + 8];
	memset (editidbuf, 0, sizeof (editidbuf));
	int numval = -1;
	int newnum = -1;
	const char *end = NULL;
	if (idvalstr && !strncmp (idvalstr, "momedval", strlen ("momedval"))
	    && mom_looks_like_random_id_cstr (idvalstr + strlen ("momedval"),
					      &end) && end
	    && sscanf (end, "_N%d", &numval) > 0 && numval >= 0)
	  {
	    strncpy (editidbuf, idvalstr + strlen ("momedval"),
		     MOM_IDSTRING_LEN);
	    _L (editor) = (momval_t) (mom_get_item_of_identcstr (editidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_replaceson editidbuf="),
		       MOMOUT_LITERALV ((const char *) editidbuf),
		       MOMOUT_LITERAL ("; editor="),
		       MOMOUT_VALUE ((const momval_t) _L (editor)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end),
		       MOMOUT_LITERAL ("; numval="),
		       MOMOUT_DEC_INT ((int) numval));
	    {
	      mom_should_lock_item (_L (editor).pitem);
	      _L (edinode) = mom_item_vector_nth (_L (editor).pitem, numval);
	      newnum = mom_item_vector_count (_L (editor).pitem);
	      mom_item_vector_append1 (_L (editor).pitem, MOM_NULLV);
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_replaceson edinode="),
		       MOMOUT_VALUE (_L (edinode)),
		       MOMOUT_LITERAL (" newnum#"), MOMOUT_DEC_INT (newnum));
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_replaceinput\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL ("  \"momedit_oldid\": \""),
			  MOMOUT_LITERALV ((const char *) idvalstr),
			  MOMOUT_LITERAL ("\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL ("  \"momedit_newid\": \"momedval"),
			  MOMOUT_LITERALV ((const char *)
					   mom_string_cstr ((momval_t)
							    mom_item_get_idstr
							    (_L
							     (editor).pitem))),
			  MOMOUT_LITERAL ("_N"),
			  MOMOUT_DEC_INT ((int) newnum),
			  MOMOUT_LITERAL ("\" }"), MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
      }				/* end if todo mom_menuitem_editval_replace_son */
    else if (mom_string_same (todov, "mom_menuitem_editval_replaceattr"))
      {
	/**** we send a JSON like
	      { "momedit_do": "momedit_replaceinput",
	        "momedit_oldid": <id-of-old-element>,
		"momedit_newid": <id-of-new-input>
	      }
	****/
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom editval_replaceattr idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	char editidbuf[MOM_IDSTRING_LEN + 8];
	memset (editidbuf, 0, sizeof (editidbuf));
	int numval = -1;
	int newnum = -1;
	const char *end = NULL;
	if (idvalstr && !strncmp (idvalstr, "momedval", strlen ("momedval"))
	    && mom_looks_like_random_id_cstr (idvalstr + strlen ("momedval"),
					      &end) && end
	    && sscanf (end, "_N%d", &numval) > 0 && numval >= 0)
	  {
	    strncpy (editidbuf, idvalstr + strlen ("momedval"),
		     MOM_IDSTRING_LEN);
	    _L (editor) = (momval_t) (mom_get_item_of_identcstr (editidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_replaceattr editidbuf="),
		       MOMOUT_LITERALV ((const char *) editidbuf),
		       MOMOUT_LITERAL ("; editor="),
		       MOMOUT_VALUE ((const momval_t) _L (editor)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end),
		       MOMOUT_LITERAL ("; numval="),
		       MOMOUT_DEC_INT ((int) numval));
	    {
	      mom_should_lock_item (_L (editor).pitem);
	      _L (edinode) = mom_item_vector_nth (_L (editor).pitem, numval);
	      newnum = mom_item_vector_count (_L (editor).pitem);
	      mom_item_vector_append1 (_L (editor).pitem, MOM_NULLV);
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_replaceattr edinode="),
		       MOMOUT_VALUE (_L (edinode)),
		       MOMOUT_LITERAL (" newnum#"), MOMOUT_DEC_INT (newnum));
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_replaceinput\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL ("  \"momedit_oldid\": \""),
			  MOMOUT_LITERALV ((const char *) idvalstr),
			  MOMOUT_LITERAL ("\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL ("  \"momedit_newid\": \"momedval"),
			  MOMOUT_LITERALV ((const char *)
					   mom_string_cstr ((momval_t)
							    mom_item_get_idstr
							    (_L
							     (editor).pitem))),
			  MOMOUT_LITERAL ("_N"),
			  MOMOUT_DEC_INT ((int) newnum),
			  MOMOUT_LITERAL ("\" }"), MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
      }				/* end if todo mom_menuitem_editval_replaceattr */
    else if (mom_string_same (todov, "mom_newinput"))
      {
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	momval_t inputv = mom_webx_post_arg (_L (webx).pitem, "input_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom editval_newinput idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv),
		   MOMOUT_LITERAL (" input="),
		   MOMOUT_VALUE ((const momval_t) inputv));
	const char *idvalstr = mom_string_cstr (idvalv);
	const char *inputstr = mom_string_cstr (inputv);
	char editidbuf[MOM_IDSTRING_LEN + 8];
	memset (editidbuf, 0, sizeof (editidbuf));
	int numval = -1;
	int newnum = -1;
	const char *end = NULL;
	if (idvalstr && !strncmp (idvalstr, "momedval", strlen ("momedval"))
	    && mom_looks_like_random_id_cstr (idvalstr + strlen ("momedval"),
					      &end) && end
	    && sscanf (end, "_N%d", &numval) > 0 && numval >= 0)
	  {
	    strncpy (editidbuf, idvalstr + strlen ("momedval"),
		     MOM_IDSTRING_LEN);
	    _L (editor) = (momval_t) (mom_get_item_of_identcstr (editidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_newinput editidbuf="),
		       MOMOUT_LITERALV ((const char *) editidbuf),
		       MOMOUT_LITERAL ("; editor="),
		       MOMOUT_VALUE ((const momval_t) _L (editor)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end),
		       MOMOUT_LITERAL ("; numval="),
		       MOMOUT_DEC_INT ((int) numval));
	  }
#warning ajax_edit todo mom_newinput should parse inputstr
      }				/* end if todo mom_newinput */
    MOM_FATAPRINTF ("ajax_edit incomplete");
#warning ajax_edit incomplete
    goto end;
  end:
    mom_unlock_item (_L (webx).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom return pop"));
    return momroutres_pop;
  }
  ;
  ////
ajaxedit_lab_dideditcopy:
  {
    mom_should_lock_item (_L (webx).pitem);
    MOM_DEBUG (run,
	       MOMOUT_LITERAL ("ajax_edit_codmom dideditcopy curval="),
	       MOMOUT_VALUE (_L (curval)),
	       MOMOUT_LITERAL (" webx="), MOMOUT_VALUE (_L (webx)), NULL);
    MOM_WEBX_OUT (_L (webx).pitem,
		  MOMOUT_LITERAL ("\" }"), MOMOUT_NEWLINE ());
    {
      mom_should_lock_item (_C (editors).pitem);
      mom_item_put_attribute (_C (editors).pitem, mom_named__buffer,
			      _L (curval));
      mom_unlock_item (_C (editors).pitem);
    }
    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
    mom_unlock_item (_L (webx).pitem);
    return momroutres_pop;
  }
  ;
  ////
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_ajax_edit = {
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = ajaxedit_c__lastclosure,	//
  .rout_frame_nbval = ajaxedit_v__lastval,	//
  .rout_frame_nbnum = ajaxedit_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "ajax_edit",	//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = ajax_edit_codmom,	//
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
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = noop_c__lastclosure,	//
  .rout_frame_nbval = noop_v__lastval,	//
  .rout_frame_nbnum = noop_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "noop",		//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = noop_codmom,	//
  .rout_timestamp = __DATE__ "@" __TIME__
};
