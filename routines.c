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
enum ajax_system_valindex_en
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

/*****
  About editor and editing. [to be implemented, not corresponding to old code]

  The general idea is that an edited item is not updated immediately;
  the user is making some editing, the web page is changing, but the
  edited item is changing only at the end of the editing.

  When the user starts the edition of some item, an editor item is
  created, and an editor tab appears in the browser. Then both editor
  item and editor tab are filled.

  The editor item is actually a vector. It knows the edited item thru
  the attribute `item'; each displayed value corresponds to a
  component of that vector, called a display item. When the user is
  changing something, the display item is updated accordingly. At the
  end of the edition the displays are played, to actually change the
  edited item. Until that, the edition can be cancelled.

  Display for an attribute:

     display: attr(<item>,<attr>,<display:value-of-attr>)

  Display for a node:

     display: node(<item:connective>,<tuple:sons-displays>)
     origin: <origin-display>

  Display for an item:

     display: item
     item:  <item>

  Display for null:
     display: empty
  Display for a string:
     display: string
     string: <string-value>

  Display for some user input
     display: input

 *****/



////////////////////////////////////////////////////////////////
///// utility function for display_value routine
static void
display_item_occ_mom (momitem_t *webx, momitem_t *itm)
{
  assert (webx && webx->i_typnum == momty_item
	  && webx->i_paylkind == mompayk_webexchange);
  if (!itm)
    {
      MOM_WEBX_OUT (webx, MOMOUT_JS_LITERAL ("<span class='mom_nil_itemocc_cl'>"	///
					     "&#9671;"	/* U+25C7 WHITE DIAMOND ◇ */
					     "</span>"));
      return;
    }
  assert (itm && itm->i_typnum == momty_item);
  const momstring_t *namestrv = mom_item_get_name (itm);
  const momstring_t *idstrv = mom_item_get_idstr (itm);
  if (namestrv)
    {
      MOM_WEBX_OUT (webx,
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_named_itemocc_cl' data-momitemid='"),
		    MOMOUT_LITERALV (mom_string_cstr ((momval_t) idstrv)),
		    MOMOUT_JS_LITERAL ("'>"),
		    MOMOUT_JS_HTML (mom_string_cstr ((momval_t) namestrv)),
		    MOMOUT_JS_LITERAL ("</span>"));
    }
  else
    {
      MOM_WEBX_OUT (webx,
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_anonymous_itemocc_cl' data-momitemid='"),
		    MOMOUT_LITERALV (mom_string_cstr ((momval_t) idstrv)),
		    MOMOUT_JS_LITERAL ("'>"),
		    MOMOUT_LITERALV (mom_string_cstr ((momval_t) idstrv)),
		    MOMOUT_JS_LITERAL ("</span>"));
    }
}

///// display_value
enum display_value_valindex_en
{
  display_value_v_editor,
  display_value_v_webx,
  display_value_v_curval,
  display_value_v_orig,
  display_value_v_olddisplay,
  display_value_v_spare,
  display_value_v_backup,
  display_value_v_newdisplay,
  display_value_v_conn,
  display_value_v_curson,
  display_value_v_vectsubdisp,
  display_value_v_subdisplay,
  display_value_v__lastval
};

enum display_value_closure_en
{
  display_value_c__lastclosure
};

enum display_value_numbers_en
{
  display_value_n_depth,
  display_value_n_rank,
  display_value_n_nbsons,
  display_value_n_sonix,
  display_value_n__lastnum
};


static int
display_value_codmom (int momstate_, momitem_t *momtasklet_,
		      const momnode_t *momclosure_,
		      momval_t *momlocvals_, intptr_t * momlocnums_,
		      double *momlocdbls_)
{

#define _L(Nam) (momlocvals_[display_value_v_##Nam])
#define _C(Nam) (momclosure_->sontab[display_value_c_##Nam])
#define _N(Nam) (momlocnums_[display_value_n_##Nam])
  enum display_value_state_en
  {
    display_value_s_start,
    display_value_s_impossible,
    display_value_s_didson,
    display_value_s__laststate
  };
  unsigned taskdepth = mom_item_tasklet_depth (momtasklet_);
  // after a push, rebase the arrays
#define DISPLAY_VALUE_REBASE() do  {				\
    if (momlocvals_)						\
      momlocvals_						\
	=  mom_item_tasklet_frame_values_pointer(momtasklet_,	\
						 taskdepth-1);	\
    if (momlocnums_)						\
      momlocnums_						\
	=  mom_item_tasklet_frame_ints_pointer(momtasklet_,	\
					       taskdepth-1);	\
    if (momlocdbls_)						\
      momlocdbls_						\
	= mom_item_tasklet_frame_doubles_pointer(momtasklet_,	\
						 taskdepth-1);	\
  } while(0)
#define DISPLAY_VALUE_UNLOCK() do {		\
    mom_unlock_item(_L(editor).pitem);		\
    if (_L(vectsubdisp).pitem)			\
      mom_unlock_item(_L(vectsubdisp).pitem);	\
    mom_unlock_item(_L(webx).pitem); } while(0)
  //
#define _SET_STATE(St) do {						\
    MOM_DEBUGPRINTF (run,						\
		     "display_value_codmom setstate " #St " = %d",	\
		     (int)display_value_s_##St);			\
    return display_value_s_##St; } while(0)
#define DISPLAY_VALUE_SET_STATE(St) \
  do { DISPLAY_VALUE_UNLOCK(); _SET_STATE(St); } while(0)
  //
#define DISPLAY_VALUE_POP_RETURN()				\
  do { DISPLAY_VALUE_UNLOCK(); 					\
    MOM_DEBUGPRINTF (run, "display_value_codmom popreturn");	\
    return momroutres_pop; } while(0)
  // lock the webx & the editor
  {
    mom_should_lock_item (_L (webx).pitem);
    mom_should_lock_item (_L (editor).pitem);
  }
  //
  MOM_DEBUG (run, MOMOUT_LITERAL ("display_value_codmom start state#"),
	     MOMOUT_DEC_INT ((int) momstate_),
	     MOMOUT_LITERAL ("; curval="),
	     MOMOUT_VALUE ((const momval_t) _L (curval)),
	     MOMOUT_LITERAL ("; depth="), MOMOUT_DEC_INT ((int) _N (depth)),
	     MOMOUT_LITERAL ("; taskdepth="),
	     MOMOUT_DEC_INT ((int) taskdepth), NULL);
  if (momstate_ >= 0 && momstate_ < display_value_s__laststate)
    switch ((enum display_value_state_en) momstate_)
      {
      case display_value_s_start:
	goto display_value_lab_start;
      case display_value_s_impossible:
	goto display_value_lab_impossible;
      case display_value_s_didson:
	goto display_value_lab_didson;
      case display_value_s__laststate:;
      }
  MOM_FATAPRINTF ("display_value invalid state #%d", momstate_);
display_value_lab_start:
  MOM_DEBUG (run, MOMOUT_LITERAL ("display_value start editor="),
	     MOMOUT_VALUE ((const momval_t) _L (editor)),
	     MOMOUT_SPACE (48),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				     mom_value_to_item (_L (editor))),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" olddisplay="),
	     MOMOUT_VALUE ((const momval_t) _L (olddisplay)),
	     MOMOUT_SPACE (48),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				     mom_value_to_item (_L (olddisplay))),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" orig="),
	     MOMOUT_VALUE ((const momval_t) _L (orig)), MOMOUT_SPACE (48),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				     mom_value_to_item (_L (orig))),
	     MOMOUT_NEWLINE (), NULL);
  //
  if (MOM_UNLIKELY (_L (editor).ptr == MOM_EMPTY))	// this cannot happen!
    _SET_STATE (impossible);
  //
  _N (rank) = mom_item_vector_count (_L (editor).pitem);
  _L (newdisplay) = MOM_NULLV;
  if (_L (olddisplay).pitem)
    {
      bool goodolddisplay = true;
      mom_should_lock_item (_L (olddisplay).pitem);
      if (mom_item_get_attribute
	  (_L (olddisplay).pitem, mom_named__editor).ptr != _L (editor).ptr)
	goodolddisplay = false;
      if (mom_item_get_attribute
	  (_L (olddisplay).pitem, mom_named__origin).ptr != _L (orig).ptr)
	goodolddisplay = false;
      MOM_DEBUG (run,
		 MOMOUT_LITERALV ((const char *) (goodolddisplay ?
						  "display_value start goodolddisplay true"
						  :
						  "display_value start goodolddisplay false")));
      if (goodolddisplay)
	{
	  _L (newdisplay) = _L (olddisplay);
	  _N (rank) =
	    mom_integer_val ((mom_item_get_attribute
			      (_L (newdisplay).pitem, mom_named__rank)));
	  mom_item_put_attribute (_L (olddisplay).pitem, mom_named__val,
				  _L (curval));
	  MOM_DEBUG (run,
		     MOMOUT_LITERAL
		     ("display_value start newdisplay=olddisplay="),
		     MOMOUT_VALUE (_L (olddisplay)), MOMOUT_LITERAL (" !:"),
		     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					     mom_value_to_item (_L
								(olddisplay))),
		     NULL);
	}
      mom_unlock_item (_L (olddisplay).pitem);
    }
  if (!_L (newdisplay).pitem)
    {
      _L (newdisplay) = (momval_t) mom_make_item ();
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__editor,
			      _L (editor));
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__rank,
			      mom_make_integer (_N (rank)));
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__origin,
			      _L (orig));
      mom_item_vector_append1 (_L (editor).pitem, _L (newdisplay));
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__val,
			      _L (curval));
      MOM_DEBUG (run, MOMOUT_LITERAL ("display_value start made newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)),
		 MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
    }
  switch ((enum momvaltype_en) mom_type (_L (curval)))
    {
    case momty_null:		///// nil
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__empty);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL ("<span class='mom_null_value_cl mom_value_cl' id='momdisplay"), MOMOUT_LITERALV (mom_ident_cstr_of_item (_L (newdisplay).pitem)), MOMOUT_JS_LITERAL ("'>" "&#9109;"	/* U+2395 APL FUNCTIONAL SYMBOL QUAD ⎕ */
																							    "</span>"));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("display_value for null updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_int:		///// integer value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__integer);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_integer_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>"),
		    MOMOUT_FMT_LONG_LONG ((const char *) "%lld",
					  ((long long)
					   mom_integer_val (_L (curval)))),
		    MOMOUT_JS_LITERAL ("</span>"));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL ("display_value for int updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_double:		///// double value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__double);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_double_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>"),
		    MOMOUT_FMT_DOUBLE ((const char *) "%.8g",
				       ((double)
					mom_double_val (_L (curval)))),
		    MOMOUT_JS_LITERAL ("</span>"));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("display_value for double updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_string:		///// string value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__string);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_string_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>"
				       "&#8220;<span class='mom_string_content_cl'>"
				       /* U+201C LEFT DOUBLE QUOTATION MARK “ */
		    ),
		    MOMOUT_JS_HTML (mom_string_cstr (_L (curval))),
		    MOMOUT_JS_LITERAL ("</span>" "&#8221;"
				       /* U+201D RIGHT DOUBLE QUOTATION MARK ” */
				       "</span>"));
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("display_value for string updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_jsonarray:	/// jsonarray value, displayed as a whole
      {
	struct momout_st outb = { 0 };
	mom_initialize_buffer_output (&outb, outf_jsonhalfindent);
	MOM_OUT (&outb, MOMOUT_JSON_VALUE (_L (curval)), MOMOUT_FLUSH ());
	MOM_WEBX_OUT (_L (webx).pitem,
		      //
		      MOMOUT_JS_LITERAL
		      ("<span class='mom_jsonarray_value_cl mom_value_cl' id='momdisplay"),
		      MOMOUT_LITERALV (mom_ident_cstr_of_item
				       (_L (newdisplay).pitem)),
		      MOMOUT_JS_LITERAL ("'>"),
		      MOMOUT_JS_HTML ((const char *) outb.mout_data),
		      MOMOUT_JS_LITERAL ("</span>"));
	mom_finalize_buffer_output (&outb);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("display_value for jsonarray updated newdisplay="),
		   MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		   MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					   mom_value_to_item (_L
							      (newdisplay))),
		   NULL);
	mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
	DISPLAY_VALUE_POP_RETURN ();
	break;
      }
      /*****************/
    case momty_jsonobject:	/// jsonobject value, displayed as a whole
      {
	struct momout_st outb = { 0 };
	mom_initialize_buffer_output (&outb, outf_jsonhalfindent);
	MOM_OUT (&outb, MOMOUT_JSON_VALUE (_L (curval)), MOMOUT_FLUSH ());
	MOM_WEBX_OUT (_L (webx).pitem,
		      //
		      MOMOUT_JS_LITERAL
		      ("<span class='mom_jsonobject_value_cl mom_value_cl' id='momdisplay"),
		      MOMOUT_LITERALV (mom_ident_cstr_of_item
				       (_L (newdisplay).pitem)),
		      MOMOUT_JS_LITERAL ("'>"),
		      MOMOUT_JS_HTML ((const char *) outb.mout_data),
		      MOMOUT_JS_LITERAL ("</span>"));
	mom_finalize_buffer_output (&outb);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("display_value for jsonobject updated newdisplay="),
		   MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		   MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					   mom_value_to_item (_L
							      (newdisplay))),
		   NULL);
	mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
	DISPLAY_VALUE_POP_RETURN ();
	break;
      }
      /*****************/
    case momty_item:		///// item value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__item);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_item_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>"));
      display_item_occ_mom (_L (webx).pitem, _L (curval).pitem);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL ("</span>"));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("display_value for item updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_set:		///// set value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__set);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_set_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>{ "));
      {
	unsigned card = mom_set_cardinal (_L (curval));
	for (unsigned ix = 0; ix < card; ix++)
	  {
	    if (ix > 0)
	      MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL (", "));
	    display_item_occ_mom (_L (webx).pitem,
				  mom_set_nth_item (_L (curval), ix));
	  }
      }
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL (" }</span>"));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL ("display_value for set updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_tuple:		///// tuple value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__tuple);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_tuple_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>[ "));
      {
	unsigned card = mom_tuple_length (_L (curval));
	for (unsigned ix = 0; ix < card; ix++)
	  {
	    if (ix > 0)
	      MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL (", "));
	    display_item_occ_mom (_L (webx).pitem,
				  mom_tuple_nth_item (_L (curval), ix));
	  }
      }
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL (" ]</span>"));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("display_value for tuple updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_POP_RETURN ();
      break;
      /*****************/
    case momty_node:		///// node value
      mom_item_put_attribute (_L (newdisplay).pitem, mom_named__display,
			      (momval_t) mom_named__node);
      _L (conn) = (momval_t) mom_node_conn (_L (curval));
      _N (nbsons) = mom_node_arity (_L (curval));
      _L (vectsubdisp) = (momval_t) mom_make_item ();
      mom_should_lock_item (_L (vectsubdisp).pitem);
      mom_item_start_vector (_L (vectsubdisp).pitem);
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL
		    ("<span class='mom_node_value_cl mom_value_cl' id='momdisplay"),
		    MOMOUT_LITERALV (mom_ident_cstr_of_item
				     (_L (newdisplay).pitem)),
		    MOMOUT_JS_LITERAL ("'>*"));
      display_item_occ_mom (_L (webx).pitem, _L (conn).pitem);
      _N (nbsons) = mom_node_arity (_L (curval));
      MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL (" ("));
      for (_N (sonix) = 0; _N (sonix) < _N (nbsons); _N (sonix)++)
	{
	  if (_N (sonix) > 0)
	    MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL (", "));
	  _L (curson) = mom_node_nth (_L (curval), _N (sonix));
	  mom_item_tasklet_push_frame	//
	    (momtasklet_, (momval_t) momclosure_,
	     MOMPFR_FIVE_VALUES (_L (editor),
				 _L (webx), _L (curson), _L (orig),
				 MOM_NULLV),
	     MOMPFR_INT ((intptr_t) (_N (depth) + 1)), NULL);
	  mom_item_tasklet_clear_res (momtasklet_);
	  DISPLAY_VALUE_REBASE ();
	  DISPLAY_VALUE_SET_STATE (didson);
	display_value_lab_didson:
	  _L (subdisplay) = mom_item_tasklet_res1 (momtasklet_);
	  mom_item_tasklet_clear_res (momtasklet_);
	  mom_item_vector_append1 (_L (vectsubdisp).pitem, _L (subdisplay));
	}
      MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_LITERAL (")"));
      MOM_WEBX_OUT (_L (webx).pitem,
		    //
		    MOMOUT_JS_LITERAL ("</span>"));
      /// display: node(<item:connective>,<tuple:sons-displays>)
      mom_item_put_attribute	//
	(_L (newdisplay).pitem, mom_named__display,
	 (momval_t)
	 mom_make_node_sized (mom_named__node, 2,
			      _L (conn),
			      (momval_t) mom_make_tuple_from_array
			      ((unsigned) _N (nbsons),
			       (const momitem_t **)
			       mom_item_vector_ptr_nth (_L
							(vectsubdisp).pitem,
							0))));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("display_value for node updated newdisplay="),
		 MOMOUT_VALUE (_L (newdisplay)), MOMOUT_LITERAL (" !:"),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (newdisplay))),
		 NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (newdisplay));
      DISPLAY_VALUE_REBASE ();
      DISPLAY_VALUE_POP_RETURN ();
      break;
    }
  //
display_value_lab_impossible:
  MOM_FATAPRINTF ("display_value impossible state reached!");
  return momroutres_pop;
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
#undef DISPLAY_VALUE_UNLOCK
#undef DISPLAY_VALUE_REBASE
#undef DISPLAY_VALUE_POP_RETURN
#undef DISPLAY_VALUE_SET_STATE
}				/* end routine display_value_codmom */

const struct momroutinedescr_st momrout_display_value = {
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = display_value_c__lastclosure,	//
  .rout_frame_nbval = display_value_v__lastval,	//
  .rout_frame_nbnum = display_value_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "display_value",	//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = display_value_codmom,	//
  .rout_timestamp = __DATE__ "@" __TIME__
};





////////////////////////////////////////////////////////////////
///// ajax_edit
enum ajax_edit_valindex_en
{
  ajaxedit_v_arg0res,
  ajaxedit_v_method,
  ajaxedit_v_namid,
  ajaxedit_v_restpath,
  ajaxedit_v__spare,
  ajaxedit_v_webx,
  ajaxedit_v_editor,
  ajaxedit_v_display,
  ajaxedit_v_edinode,
  ajaxedit_v_dispnode,
  ajaxedit_v_curval,
  ajaxedit_v_origin,
  ajaxedit_v_curattr,
  ajaxedit_v_curitem,
  ajaxedit_v_updated,
  ajaxedit_v_dispconn,
  ajaxedit_v_subdisplay,
  ajaxedit_v_subval,
  ajaxedit_v__lastval
};

enum ajax_edit_closure_en
{
  ajaxedit_c_editors,
  ajaxedit_c_edit_value,
  ajaxedit_c_display_value,
  ajaxedit_c_update_display_value,
  ajaxedit_c__lastclosure
};

enum ajax_edit_numbers_en
{
  ajaxedit_n_rank,
  ajaxedit_n_num,
  ajaxedit_n_good_input,
  ajaxedit_n_size,
  ajaxedit_n_ix,
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
  char *endp = NULL;
  int arity = 0;
  char nambuf[72];
  memset (nambuf, 0, sizeof (nambuf));
  enum ajax_edit_state_en
  {
    ajaxedit_s_start,
    ajaxedit_s_dideditcopy,
    ajaxedit_s_didputvalue,
    ajaxedit_s_didupdatedisplayvalue,
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
      case ajaxedit_s_didputvalue:
	goto ajaxedit_lab_didputvalue;
      case ajaxedit_s_didupdatedisplayvalue:
	goto ajaxedit_lab_didupdatedisplayvalue;
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
    /***** todo= mom_menuitem_editval_copy ****/
    if (mom_string_same (todov, "mom_menuitem_editval_copy"))
      {
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_edit_codmom editval_copy idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	_L (display) = MOM_NULLV;
	MOM_DEBUGPRINTF (run, "ajax_edit_codmom editval_copy idvalstr=%s",
			 idvalstr);
	if (idvalstr
	    && !strncmp (idvalstr, "momdisplay", strlen ("momdisplay"))
	    && (_L (display) =
		(momval_t) (mom_get_item_of_identcstr
			    (idvalstr + strlen ("momdisplay")))).ptr)
	  {
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_copy display="),
		       MOMOUT_VALUE ((const momval_t) _L (display)), NULL);
	    {
	      mom_should_lock_item (_L (display).pitem);
	      _L (edinode) =
		mom_item_get_attribute (_L (display).pitem,
					mom_named__display);
	      _L (curval) =
		mom_item_get_attribute (_L (display).pitem, mom_named__val);
	      mom_unlock_item (_L (display).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_copy edinode="),
		       MOMOUT_VALUE (_L (edinode)),
		       MOMOUT_LITERAL (" curval="),
		       MOMOUT_VALUE (_L (curval)), NULL);
	  }
	else
	  MOM_FATAPRINTF ("ajax_edit bad idvalstr=%s", idvalstr);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_edit_codmom editval_copy edinode="),
		   MOMOUT_VALUE (_L (edinode)));
	/// here we got the correct curval
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
	mom_unlock_item (_L (webx).pitem);
	mom_item_tasklet_push_frame	//
	  (momtasklet_, (momval_t) _C (display_value),
	   MOMPFR_FIVE_VALUES (_C (editors), _L (webx), _L (curval),
			       /*orig: */ (momval_t) mom_named__buffer,
			       /*olddisplay: */ MOM_NULLV),
	   MOMPFR_INT ((intptr_t) 0), NULL);
	_SET_STATE (dideditcopy);
      }				// end if todov is mom_menuitem_editval_copy
    //

    /***** todo= mom_menuitem_editval_replace ****/
    else if (mom_string_same (todov, "mom_menuitem_editval_replace"))
      {
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom editval_replace idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	_L (display) = MOM_NULLV;
	MOM_DEBUGPRINTF (run, "ajax_edit_codmom editval_replace idvalstr=%s",
			 idvalstr);
	if (idvalstr
	    && !strncmp (idvalstr, "momdisplay", strlen ("momdisplay"))
	    && (_L (display) =
		(momval_t) (mom_get_item_of_identcstr
			    (idvalstr + strlen ("momdisplay")))).ptr)
	  {
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_replace display="),
		       MOMOUT_VALUE ((const momval_t) _L (display)),
		       MOMOUT_LITERAL (" "),
		       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					       _L (display).pitem), NULL);
	  }
	else
	  MOM_FATAPRINTF ("ajax_edit bad idvalstr=%s", idvalstr);
	{
	  momval_t nowv = mom_make_double (mom_clock_time (CLOCK_REALTIME));
	  {
	    mom_should_lock_item (_L (display).pitem);
	    _L (origin) =
	      mom_item_get_attribute (_L (display).pitem, mom_named__origin);
	    mom_item_put_attribute (_L (display).pitem, mom_named__display,
				    (momval_t) mom_named__input);
	    mom_item_remove_attribute (_L (display).pitem, mom_named__val);
	    mom_unlock_item (_L (display).pitem);
	  }
	  {
	    mom_should_lock_item (_L (origin).pitem);
	    mom_item_put_attribute (_L (origin).pitem,
				    mom_named__updated, (momval_t) nowv);
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom editval_replace origin="),
		       MOMOUT_VALUE ((const momval_t) _L (origin)),
		       MOMOUT_LITERAL (" "),
		       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					       _L (origin).pitem), NULL);
	    mom_unlock_item (_L (origin).pitem);
	  }
	}
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("{ \"momedit_do\": \"momedit_replacebyinput\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL ("  \"momedit_displayid\": \""),
		      MOMOUT_LITERALV ((const char *)
				       mom_string_cstr ((momval_t)
							mom_item_get_idstr (_L
									    (display).pitem))),
		      MOMOUT_LITERAL ("\", "), MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL (" \"momedit_inputhtml\": \""),
		      MOMOUT_JS_LITERAL
		      ("<input type='text' class='mom_newvalinput_cl' id='momvalinp"),
		      MOMOUT_LITERALV ((const char *)
				       mom_string_cstr ((momval_t)
							mom_item_get_idstr (_L
									    (display).pitem))),
		      MOMOUT_JS_LITERAL ("'/>"), MOMOUT_LITERAL ("\" }"),
		      MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom editval_replace done idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv),
		   MOMOUT_LITERAL (" webx="),
		   MOMOUT_VALUE ((const momval_t) _L (webx)), NULL);
	goto end;
      }				// end if todov is mom_menuitem_editval_replace
    //

    /***** todo= mom_menuitem_edititem_copy ****/
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
	  (momtasklet_, (momval_t) _C (display_value),
	   MOMPFR_FIVE_VALUES (_C (editors), _L (webx), _L (curval),
			       /*orig: */ (momval_t) mom_named__buffer,
			       /*olddisplay: */ MOM_NULLV),
	   MOMPFR_INT ((intptr_t) 0), NULL);
	mom_unlock_item (_L (webx).pitem);
	_SET_STATE (dideditcopy);
      }				//// end if todo is mom_menuitem_edititem_copy
    //
    /***** todo= mom_prepare_editval_menu ****/
    else if (mom_string_same (todov, "mom_prepare_editval_menu"))
      {
	momval_t idvalv = mom_webx_post_arg (_L (webx).pitem, "idval_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom prepareditvalmenu idvalv="),
		   MOMOUT_VALUE ((const momval_t) idvalv));
	const char *idvalstr = mom_string_cstr (idvalv);
	char dispidbuf[MOM_IDSTRING_LEN + 8];
	memset (dispidbuf, 0, sizeof (dispidbuf));
	const char *end = NULL;
	if (idvalstr
	    && !strncmp (idvalstr, "momdisplay", strlen ("momdisplay"))
	    && mom_looks_like_random_id_cstr (idvalstr +
					      strlen ("momdisplay"), &end)
	    && end)
	  {
	    strncpy (dispidbuf, idvalstr + strlen ("momdisplay"),
		     MOM_IDSTRING_LEN);
	    _L (display) = (momval_t) (mom_get_item_of_identcstr (dispidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom  prepareditvalmenu dispidbuf="),
		       MOMOUT_LITERALV ((const char *) dispidbuf),
		       MOMOUT_LITERAL ("; display="),
		       MOMOUT_VALUE ((const momval_t) _L (display)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end), NULL);
	    {
	      mom_should_lock_item (_L (display).pitem);
	      MOM_DEBUG (run,
			 MOMOUT_LITERAL
			 ("ajax_edit_codmom  prepareditvalmenu locked display="),
			 MOMOUT_VALUE ((const momval_t) _L (display)),
			 MOMOUT_LITERAL (" !: "),
			 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t
						  *) (_L (display).pitem)),
			 NULL);
	      _L (dispnode) =
		mom_item_get_attribute (_L (display).pitem,
					mom_named__display);
	      _L (editor) =
		mom_item_get_attribute (_L (display).pitem,
					mom_named__editor);
	      _L (origin) =
		mom_item_get_attribute (_L (display).pitem,
					mom_named__origin);
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom prepareditvalmenu "),	//
		       MOMOUT_LITERAL (" dispnode="), MOMOUT_VALUE (_L (dispnode)),	//
		       MOMOUT_LITERAL ("; display="), MOMOUT_VALUE (_L (display)),	//
		       MOMOUT_LITERAL ("; editor="), MOMOUT_VALUE (_L (editor)),	//
		       MOMOUT_LITERAL ("; curval="), MOMOUT_VALUE (_L (curval)),	//
		       MOMOUT_LITERAL ("; origin="), MOMOUT_VALUE (_L (origin)),	//
		       NULL);
	  }
	else
	  MOM_FATAPRINTF ("ajax_edit bad idvalstr=%s end=%s", idvalstr, end);
	/// here we got the correct dispnode. 
	assert (_L (dispnode).ptr != NULL);
	if (mom_node_conn (_L (dispnode)) == mom_named__attr)
	  {			// node: *attr(<item>,<attr>,<disp>)
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
	else if (mom_node_conn (_L (dispnode)) == mom_named__node)
	  {			/* *node(<conn>,<tuple-disp>) */
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
	else
	  {
	    MOM_WARNING (MOMOUT_LITERAL
			 ("ajax_exit prepareditvalmenu unhandled dispnode="),
			 MOMOUT_VALUE (_L (dispnode)));
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_add_to_editval_menu\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_menuval\": ["),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"<li>-</li>\" ]"),
			  MOMOUT_NEWLINE (), MOMOUT_LITERAL ("}"),
			  MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
      }				//// end if todo is mom_prepare_editval_menu
    //
    /***** todo= mom_menuitem_editval_replaceson ****/
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
    //
    /***** todo= mom_menuitem_editval_replaceattr ****/
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
    //
    /***** todo= momedit_newinput ****/
    else if (mom_string_same (todov, "momedit_newinput"))
      {
	_L (display) = MOM_NULLV;
	_L (editor) = MOM_NULLV;
	_L (dispnode) = MOM_NULLV;
	_L (origin) = MOM_NULLV;
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
	MOM_DEBUGPRINTF (run,
			 "ajax_edit_codmom edit_newinput idvalstr=%s inputstr=%s",
			 idvalstr, inputstr);
	char dispidbuf[MOM_IDSTRING_LEN + 8];
	memset (dispidbuf, 0, sizeof (dispidbuf));
	const char *end = NULL;
	if (idvalstr && !strncmp (idvalstr, "momvalinp", strlen ("momvalinp"))
	    && mom_looks_like_random_id_cstr (idvalstr + strlen ("momvalinp"),
					      &end) && end)
	  {
	    strncpy (dispidbuf, idvalstr + strlen ("momvalinp"),
		     MOM_IDSTRING_LEN);
	    _L (display) = (momval_t) (mom_get_item_of_identcstr (dispidbuf));
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom edit_newinput dispidbuf="),
		       MOMOUT_LITERALV ((const char *) dispidbuf),
		       MOMOUT_LITERAL ("; display="),
		       MOMOUT_VALUE ((const momval_t) _L (display)),
		       MOMOUT_LITERAL ("; end="),
		       MOMOUT_LITERALV ((const char *) end), NULL);
	  }
	else
	  MOM_FATAPRINTF
	    ("ajax_edit_codmom edit_newinput bad idval=%s input=%s", idvalstr,
	     inputstr);
	if (!_L (display).ptr)
	  MOM_FATAPRINTF ("ajax_edit_codmom edit_newinput invalid idval=%s",
			  idvalstr);
	{
	  mom_should_lock_item (_L (display).pitem);
	  _L (editor) =
	    mom_item_get_attribute (_L (display).pitem, mom_named__editor);
	  _L (dispnode) =
	    mom_item_get_attribute (_L (display).pitem, mom_named__display);
	  _L (origin) =
	    mom_item_get_attribute (_L (display).pitem, mom_named__origin);
	  mom_unlock_item (_L (display).pitem);
	}
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_edit_codmom edit_newinput editor="),
		   MOMOUT_VALUE ((const momval_t) _L (editor)),
		   MOMOUT_LITERAL ("; dispnode="),
		   MOMOUT_VALUE ((const momval_t) _L (dispnode)),
		   MOMOUT_LITERAL ("; origin="),
		   MOMOUT_VALUE ((const momval_t) _L (origin)), NULL);
	_L (curval) = MOM_NULLV;
	_N (good_input) = 0;
	if (isdigit (inputstr[0])
	    || ((inputstr[0] == '+' || inputstr[0] == '-')
		&& isdigit (inputstr[1])))
	  {
	    _N (num) = strtoll (inputstr, &endp, 0);
	    if (endp && *endp == (char) 0)
	      {
		MOM_DEBUG (run,
			   MOMOUT_LITERAL
			   ("ajax_edit_codmom edit_newinput got num="),
			   MOMOUT_DEC_INT ((int) _N (num)), NULL);
		_L (curval) = mom_make_integer (_N (num));
		_N (good_input) = __LINE__;
	      }
	  }
	else if (inputstr[0] == '"' && inputstr[1])
	  {
	    _L (curval) = (momval_t) mom_make_string (inputstr + 1);
	    _N (good_input) = __LINE__;
	  }
	else if (isalpha (inputstr[0])
		 && (_L (curval) =
		     (momval_t) mom_get_item_of_name (inputstr)).pitem !=
		 NULL)
	  {
	    _N (good_input) = __LINE__;
	  }
	// explicit nil
	else if ((inputstr[0] == '_' || inputstr[0] == '~') && !inputstr[1])
	  {
	    _L (curval) = MOM_NULLV;
	    _N (good_input) = __LINE__;
	  }
	else if (inputstr[0] == '_' && isdigit (inputstr[1])
		 && (_L (curval) =
		     (momval_t) mom_get_item_of_identcstr (inputstr)).pitem !=
		 NULL)
	  {
	    _N (good_input) = __LINE__;
	  }
	else if (sscanf (inputstr, "*%70[a-zA-Z0-9_]/%d", nambuf, &arity) > 0
		 && (_L (curitem) =
		     (momval_t) mom_get_item_of_name (nambuf)).pitem)
	  {
	    _L (curval) =
	      (momval_t) mom_make_node_from_array (_L (curitem).pitem, arity,
						   NULL);
	    if (_L (curval).ptr)
	      _N (good_input) = __LINE__;
	  }
	else if (sscanf (inputstr, "*%40[0-9_]/%d", nambuf, &arity) > 0
		 && (_L (curitem) =
		     (momval_t) mom_get_item_of_identcstr (nambuf)).pitem)
	  {
	    _L (curval) =
	      (momval_t) mom_make_node_from_array (_L (curitem).pitem, arity,
						   NULL);
	    if (_L (curval).ptr)
	      _N (good_input) = __LINE__;
	  }
	else if (inputstr[0] == '{')
	  {
	    _L (curitem) = MOM_NULLV;
	    if (isalpha (inputstr[1]))
	      _L (curitem) = (momval_t) mom_get_item_of_name (inputstr + 1);
	    else if (inputstr[1] == '_')
	      _L (curitem) =
		(momval_t) mom_get_item_of_identcstr (inputstr + 1);
	    if (_L (curitem).pitem)
	      _L (curval) =
		(momval_t) mom_make_set_sized (1, _L (curitem).pitem);
	    else
	      _L (curval) = (momval_t) mom_make_set_sized (0);
	    _N (good_input) = __LINE__;
	  }
	else if (inputstr[0] == '[')
	  {
	    _L (curitem) = MOM_NULLV;
	    if (isalpha (inputstr[1]))
	      _L (curitem) = (momval_t) mom_get_item_of_name (inputstr + 1);
	    else if (inputstr[1] == '_')
	      _L (curitem) =
		(momval_t) mom_get_item_of_identcstr (inputstr + 1);
	    if (_L (curitem).pitem)
	      _L (curval) =
		(momval_t) mom_make_tuple_sized (1, _L (curitem).pitem);
	    else
	      _L (curval) = (momval_t) mom_make_tuple_sized (0);
	    _N (good_input) = __LINE__;
	  }
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom good_input="), MOMOUT_DEC_INT ((int) _N (good_input)),	//
		   MOMOUT_LITERAL ("; curval="), MOMOUT_VALUE (_L (curval)),	//
		   MOMOUT_LITERAL ("; origin="), MOMOUT_VALUE (_L (origin)),	//
		   MOMOUT_LITERAL (" !: "), MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) mom_value_to_item ((_L (origin)))),	//
		   MOMOUT_SPACE (48),	//
		   MOMOUT_LITERAL ("; display="), MOMOUT_VALUE (_L (display)),	//
		   MOMOUT_LITERAL (" !: "), MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) mom_value_to_item ((_L (display)))),	//
		   MOMOUT_NEWLINE (), MOMOUT_LITERAL ("webx="), MOMOUT_VALUE (_L (webx)),	//
		   MOMOUT_LITERAL ("; display_value="), MOMOUT_VALUE (_C (display_value)),	//
		   NULL);
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("{ \"momedit_do\": \"momedit_replaceinput\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL ("  \"momedit_replacebyhtml\": \""),
		      NULL);
	assert (_L (webx).pitem != NULL);
	mom_item_tasklet_clear_res (momtasklet_);
	mom_unlock_item (_L (webx).pitem);
	mom_item_tasklet_push_frame	//
	  (momtasklet_, (momval_t) _C (display_value),
	   MOMPFR_FIVE_VALUES (_L (editor), _L (webx), _L (curval),
			       /*orig: */ (momval_t) _L (origin),
			       /*olddisplay: */ _L (display)),
	   MOMPFR_INT ((intptr_t) 0), NULL);
	_SET_STATE (didputvalue);
      }				/* end if todo mom_newinput */
    //
    /***** todo= mom_add_attribute ****/
    else if (mom_string_same (todov, "mom_add_attribute"))
      {
	momval_t attrstrv = mom_webx_post_arg (_L (webx).pitem, "attr_mom");
	momval_t editoridv =
	  mom_webx_post_arg (_L (webx).pitem, "editor_mom");
	_L (curattr) =
	  (momval_t) mom_get_item_of_name_or_ident_string (attrstrv);
	_L (editor) = (momval_t) (mom_get_item_of_ident (editoridv.pstring));
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_edit_codmom add_attribute attrstrv="),
		   MOMOUT_VALUE ((const momval_t) attrstrv),
		   MOMOUT_LITERAL (" editoridv="),
		   MOMOUT_VALUE ((const momval_t) editoridv),
		   MOMOUT_LITERAL (" curattr="), MOMOUT_VALUE (_L (curattr)),
		   MOMOUT_LITERAL (" editor="), MOMOUT_VALUE (_L (editor)),
		   NULL);
	if (_L (curattr).pitem && _L (editor).pitem)
	  {
	    int newnum = -1;
	    momval_t newnodv = MOM_NULLV;
	    momitem_t *curitem = NULL;
	    const char *editoridstr =
	      mom_string_cstr ((momval_t)
			       mom_item_get_idstr (_L (editor).pitem));
	    const char *attridstr =
	      mom_string_cstr ((momval_t)
			       mom_item_get_idstr (_L (curattr).pitem));
	    {
	      mom_should_lock_item (_L (editor).pitem);
	      curitem =
		mom_value_to_item (mom_item_get_attribute
				   (_L (editor).pitem, mom_named__item));
	      newnum = mom_item_vector_count (_L (editor).pitem);
	      _N (rank) = newnum;
	      _L (origin) = (momval_t) mom_make_item ();
	      mom_item_put_attribute (_L (origin).pitem, mom_named__editor,
				      _L (editor));
	      mom_item_put_attribute (_L (origin).pitem, mom_named__rank,
				      mom_make_integer (_N (rank)));
	      momval_t nowtimv =
		mom_make_double (mom_clock_time (CLOCK_REALTIME));
	      mom_item_put_attribute (_L (origin).pitem, mom_named__updated,
				      nowtimv);
	      mom_item_vector_append1 (_L (editor).pitem, _L (origin));
	      _L (display) = (momval_t) mom_make_item ();
	      mom_item_put_attribute (_L (display).pitem, mom_named__editor,
				      _L (editor));
	      mom_item_put_attribute (_L (display).pitem, mom_named__rank,
				      mom_make_integer (1 + _N (rank)));
	      mom_item_put_attribute (_L (display).pitem, mom_named__display,
				      (momval_t) mom_named__input);
	      mom_item_put_attribute (_L (display).pitem, mom_named__origin,
				      _L (origin));
	      mom_item_vector_append1 (_L (editor).pitem, _L (display));
	      newnodv =
		(momval_t) mom_make_node_sized (mom_named__attr, 3,
						(momval_t) curitem,
						_L (curattr), _L (display));
	      mom_item_put_attribute (_L (origin).pitem, mom_named__display,
				      newnodv);
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_edit_codmom add_attribute newnodv="),
		       MOMOUT_VALUE ((const momval_t) newnodv),
		       MOMOUT_LITERAL ("; newnum="), MOMOUT_DEC_INT (newnum),
		       MOMOUT_LITERAL ("; origin="),
		       MOMOUT_VALUE (_L (origin)),
		       MOMOUT_LITERAL ("; display="),
		       MOMOUT_VALUE (_L (display)), MOMOUT_SPACE (48),
		       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					       mom_value_to_item (_L
								  (display))),
		       NULL);
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_dispnewattr\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_editorid\": \""),
			  MOMOUT_LITERALV (editoridstr),
			  MOMOUT_LITERAL ("\", "), MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_attrid\": \""),
			  MOMOUT_LITERALV (attridstr),
			  MOMOUT_LITERAL ("\", "), MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_inputid\": \""),
			  MOMOUT_LITERALV (mom_string_cstr
					   ((momval_t)
					    mom_item_get_idstr (_L
								(display).pitem))),
			  MOMOUT_LITERAL ("\", "), MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_attrlihtml\": \""),
			  MOMOUT_JS_LITERAL
			  ("<li class='mom_display_new_attr_entry_cl' id='momdisplay"),
			  MOMOUT_LITERALV ((const char *)
					   mom_string_cstr ((momval_t)
							    mom_item_get_idstr
							    (_L
							     (origin).pitem))),
			  MOMOUT_JS_LITERAL ("'>"));
	    display_item_occ_mom (_L (webx).pitem, _L (curattr).pitem);
	    MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JS_LITERAL (" " "&#8674;"
							      /* U+21E2 RIGHTWARDS DASHED ARROW ⇢ */
							      " "),
			  MOMOUT_JS_LITERAL
			  ("<input type='text' class='mom_newvalinput_cl' id='momvalinp"),
			  MOMOUT_LITERALV ((const char *)
					   mom_string_cstr ((momval_t)
							    mom_item_get_idstr
							    (_L
							     (display).pitem))),
			  MOMOUT_JS_LITERAL ("'/></li>"),
			  MOMOUT_LITERAL ("\" }"), MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
	else
	  {
	    const char *editoridstr =
	      mom_string_cstr ((momval_t)
			       mom_item_get_idstr (_L (editor).pitem));
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL
			  ("{ \"momedit_do\": \"momedit_badnewattr\","),
			  MOMOUT_NEWLINE (),
			  MOMOUT_LITERAL (" \"momedit_editorid\": \""),
			  MOMOUT_LITERALV (editoridstr),
			  MOMOUT_LITERAL ("\" } "), MOMOUT_NEWLINE (), NULL);
	    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	    goto end;
	  }
      }				/* end if todo mom_add_attribute */

    /***** todo= momedit_update ****/
    else if (mom_string_same (todov, "momedit_update"))
      {
	{
	  momval_t editoridv =
	    mom_webx_post_arg (_L (webx).pitem, "editorid_mom");
	  _L (editor) =
	    (momval_t) mom_get_item_of_ident (mom_to_string (editoridv));
	}
	{
	  mom_should_lock_item (_L (editor).pitem);
	  _N (size) = mom_item_vector_count (_L (editor).pitem);
	  mom_unlock_item (_L (editor).pitem);
	}
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax edit edit_update editor="),
		   MOMOUT_VALUE ((const momval_t) _L (editor)),
		   MOMOUT_LITERAL (", size="),
		   MOMOUT_DEC_INT ((int) _N (size)),
		   MOMOUT_LITERAL (", update_display_value="),
		   MOMOUT_VALUE ((const momval_t) _C (update_display_value)),
		   NULL);
	for (_N (ix) = 0; _N (ix) < _N (size); _N (ix)++)
	  {
	    {
	      mom_should_lock_item (_L (editor).pitem);
	      _L (display) = mom_item_vector_nth (_L (editor).pitem, _N (ix));
	      mom_unlock_item (_L (editor).pitem);
	    }
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL ("ajax_edit edit_update ix="),
		       MOMOUT_DEC_INT ((int) _N (ix)),
		       MOMOUT_LITERAL (" display="),
		       MOMOUT_VALUE (_L (display)),
		       MOMOUT_LITERAL (" "),
		       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					       mom_value_to_item (_L
								  (display))),
		       NULL);
	    {
	      mom_should_lock_item (_L (display).pitem);
	      _L (updated) =
		mom_item_get_attribute (_L (display).pitem,
					mom_named__updated);
	      _L (dispnode) =
		mom_item_get_attribute (_L (display).pitem,
					mom_named__display);
	      mom_unlock_item (_L (display).pitem);
	    }
	    _L (subval) = MOM_NULLV;
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL ("ajax_edit edit_update ix="),
		       MOMOUT_DEC_INT ((int) _N (ix)),
		       MOMOUT_LITERAL (" updated="),
		       MOMOUT_VALUE (_L (updated)),
		       MOMOUT_LITERAL (" dispnode="),
		       MOMOUT_VALUE (_L (dispnode)), NULL);
	    if (_L (updated).ptr)
	      {
		mom_item_tasklet_push_frame
		  (momtasklet_, (momval_t) _C (update_display_value),
		   MOMPFR_VALUE (_L (display)), NULL);
		mom_item_tasklet_clear_res (momtasklet_);
		_SET_STATE (didupdatedisplayvalue);
		////
	      ajaxedit_lab_didupdatedisplayvalue:	////// ****************
		_L (subval) = mom_item_tasklet_res1 (momtasklet_);
		mom_item_tasklet_clear_res (momtasklet_);
		MOM_DEBUG (run,
			   MOMOUT_LITERAL
			   ("ajax_edit didupdatedisplayvalue ix="),
			   MOMOUT_DEC_INT ((int) _N (ix)),
			   MOMOUT_LITERAL (" display="),
			   MOMOUT_VALUE (_L (display)),
			   MOMOUT_LITERAL (" !: "),
			   MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
						   _L (display).pitem),
			   MOMOUT_SPACE (32), MOMOUT_LITERAL ("subval="),
			   MOMOUT_VALUE (_L (subval)), NULL);
		{
		  mom_should_lock_item (_L (display).pitem);
		  mom_item_put_attribute (_L (display).pitem, mom_named__val,
					  _L (subval));
		  mom_item_remove_attribute (_L (display).pitem,
					     mom_named__updated);
		  MOM_DEBUG (run,
			     MOMOUT_LITERAL
			     ("ajax_edit didupdatedisplayvalue ix="),
			     MOMOUT_DEC_INT ((int) _N (ix)),
			     MOMOUT_LITERAL (" updated display="),
			     MOMOUT_VALUE (_L (display)),
			     MOMOUT_LITERAL (" !: "),
			     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
						     _L (display).pitem),
			     NULL);
		  mom_unlock_item (_L (display).pitem);
		}
	      }
	  }			/* end for _N(ix) */
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax edit edit_update done editor="),
		   MOMOUT_VALUE ((const momval_t) _L (editor)), NULL);
	const char *editoridstr =
	  mom_string_cstr ((momval_t) mom_item_get_idstr (_L (editor).pitem));
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("{ \"momedit_do\": \"momedit_updated\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL (" \"momedit_editorid\": \""),
		      MOMOUT_LITERALV (editoridstr),
		      MOMOUT_LITERAL ("\" } "), MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	goto end;
      }

    /***** todo= momedit_revert ****/
    else if (mom_string_same (todov, "momedit_revert"))
      {
	{
	  momval_t editoridv =
	    mom_webx_post_arg (_L (webx).pitem, "editorid_mom");
	  _L (editor) =
	    (momval_t) mom_get_item_of_ident (mom_to_string (editoridv));
	}
	const char *editoridstr =
	  mom_string_cstr ((momval_t) mom_item_get_idstr (_L (editor).pitem));
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("{ \"momedit_do\": \"momedit_reverted\","),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL (" \"momedit_editorid\": \""),
		      MOMOUT_LITERALV (editoridstr),
		      MOMOUT_LITERAL ("\" } "), MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
	goto end;
      }

    else
      MOM_FATAL (MOMOUT_LITERAL ("ajax edit bad todov="),
		 MOMOUT_VALUE (todov), MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL (" postjsob="),
		 MOMOUT_VALUE ((const momval_t)
			       mom_webx_jsob_post (_L (webx).pitem)),
		 MOMOUT_NEWLINE (), MOMOUT_LITERAL (" webx="),
		 MOMOUT_VALUE (_L (webx)), NULL);
#warning ajax_edit incomplete
    goto end;
  end:
    mom_unlock_item (_L (webx).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom return pop"));
    return momroutres_pop;
  }
  ;
  ////
ajaxedit_lab_dideditcopy:	///// **********
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
ajaxedit_lab_didputvalue:	///// **********
  {
    _L (display) = mom_item_tasklet_res1 (momtasklet_);
    mom_should_lock_item (_L (webx).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit_codmom didputvalue curval="), MOMOUT_VALUE (_L (curval)),	//
	       MOMOUT_LITERAL (" display="), MOMOUT_VALUE (_L (display)),	//
	       MOMOUT_SPACE (48), MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) mom_value_to_item (_L (display))),	//
	       MOMOUT_LITERAL (" webx="), MOMOUT_VALUE (_L (webx)),	//
	       MOMOUT_LITERAL (" editor="), MOMOUT_VALUE (_L (editor)),	//
	       NULL);
    MOM_WEBX_OUT (_L (webx).pitem,
		  MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE (),
		  MOMOUT_LITERAL (" \"momeditor_displayid\": \""),
		  MOMOUT_JS_LITERALV ((const char *)
				      mom_string_cstr ((momval_t)
						       mom_item_get_idstr
						       (_L
							(display).pitem))),
		  MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE (),
		  MOMOUT_LITERAL (" \"momeditor_editorid\": \""),
		  MOMOUT_JS_LITERALV ((const char *)
				      mom_string_cstr ((momval_t)
						       mom_item_get_idstr
						       (_L
							(editor).pitem))),
		  MOMOUT_LITERAL ("\" }"), MOMOUT_NEWLINE (), NULL);
    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
    mom_unlock_item (_L (webx).pitem);
    return momroutres_pop;
  }
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
///// ajax_objects
enum ajax_objects_valindex_en
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
  ajaxobjs_v_orig,
  ajaxobjs_v_display,
  ajaxobjs_v__lastval
};

enum ajax_objects_closure_en
{
  ajaxobjs_c_editors,
  ajaxobjs_c_editvalueclos,
  ajaxobjs_c_displayvalueclos,
  ajaxobjs_c__lastclosure
};

enum ajax_objects_numbers_en
{
  ajaxobjs_n_nbattrs,
  ajaxobjs_n_atix,
  ajaxobjs_n_rank,
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
    ajaxobjs_s_begindisplay,
    ajaxobjs_s_didattrdisplay,
    ajaxobjs_s_didcontentdisplay,
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
      case ajaxobjs_s_begindisplay:
	goto ajaxobjs_lab_begindisplay;
	//
      case ajaxobjs_s_didattrdisplay:
	goto ajaxobjs_lab_didattrdisplay;
	//
      case ajaxobjs_s_didcontentdisplay:
	goto ajaxobjs_lab_didcontentdisplay;
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
    if (mom_string_same (todov, "mom_menuitem_obj_dispnamed"))
      {
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom dispnamed"));
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("Display an existing <b>named</b> item <small>at </i>"),
		      MOMOUT_DOUBLE_TIME ((const char *) "%c",
					  mom_clock_time (CLOCK_REALTIME)),
		      MOMOUT_LITERAL ("</i></small><br/>"), MOMOUT_SPACE (32),
		      MOMOUT_LITERAL
		      ("<label for='mom_display_name_input'>Display name:</label>"
		       " <input id='mom_display_name_input' class='mom_nameinput_cl' name='mom_name' onChange='mom_display_name_input_changed(this)'/>"
		       " <input type='submit' id='mom_cancel' class='mom_cancel_cl' value='cancel' onclick='mom_erase_maindiv()'/>"),
		      MOMOUT_NEWLINE (),
		      MOMOUT_LITERAL
		      ("<script>mom_set_name_entry($('#mom_display_name_input'));"),
		      MOMOUT_SPACE (32), MOMOUT_LITERAL ("</script>"),
		      MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
	goto end;
      }
    else if (mom_string_same (todov, "mom_menuitem_obj_dispnew"))
      {
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom dispnew"));
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_LITERAL
		      ("Display a <b>new</b> item <small>at </i>"),
		      MOMOUT_DOUBLE_TIME ((const char *) "%c",
					  mom_clock_time (CLOCK_REALTIME)),
		      MOMOUT_LITERAL ("</i></small><br/>"), MOMOUT_SPACE (32),
		      MOMOUT_LITERAL
		      ("<label for='mom_name_new'>Name:</label>"
		       " <input id='mom_name_new' class='mom_newname_cl' name='mom_new_name' pattern='[A-Za-z_][A-Za-z0-9_]*' /> "
		       "<label for='mom_comment'>Comment:</label>"
		       " <input id='mom_comment' class='mom_newcomment_cl' name='mom_new_comment'/>"
		       "<br/>"
		       " <input type='submit' id='mom_make_named' class='mom_make_named_cl' value='make' onclick='mom_make_disp_named()'/>"
		       " <input type='submit' id='mom_cancel' class='mom_cancel_cl' value='cancel' onclick='mom_erase_maindiv()'/>"),
		      MOMOUT_NEWLINE (), NULL);
	mom_webx_reply (_L (webx).pitem, "text/html", HTTP_OK);
	goto end;
      }

    else if (mom_string_same (todov, "mom_dodisplaynamed"))
      {
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom displaynamed"));
	momval_t namev = mom_webx_post_arg (_L (webx).pitem, "name_mom");
	momval_t idv = mom_webx_post_arg (_L (webx).pitem, "id_mom");
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom dodisplaynamed namev="),
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
		   ("ajax_objects_codmom dodisplaynamed editeditm="),
		   MOMOUT_VALUE ((const momval_t) _L (editeditm)));

	if (_L (editeditm).ptr)
	  {
	    mom_unlock_item (_L (webx).pitem);
	    _SET_STATE (begindisplay);
	  }
	else
	  {
	    MOM_WEBX_OUT (_L (webx).pitem,
			  MOMOUT_LITERAL ("Unknown item <tt>"),
			  MOMOUT_HTML (mom_string_cstr (namev)),
			  MOMOUT_LITERAL ("</tt> to display"));
	    mom_webx_reply (_L (webx).pitem, "text/html", HTTP_NOT_FOUND);
	    MOM_WARNPRINTF ("unknown item to display named %s",
			    mom_string_cstr (namev));
	    goto end;
	  }
      }
    else if (mom_string_same (todov, "mom_domakedispnamed"))
      {
	MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_objects_codmom makedispnamed"));
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
	  _SET_STATE (begindisplay);
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
ajaxobjs_lab_begindisplay:
  _L (editor) = (momval_t) mom_make_item ();
  mom_item_start_vector (_L (editor).pitem);
  mom_item_vector_reserve (_L (editor).pitem, 16);
  mom_item_put_attribute (_L (editor).pitem, mom_named__item, _L (editeditm));
  MOM_DEBUG (run,
	     MOMOUT_LITERAL ("ajax_objects_codmom begindisplay editor="),
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
  /////
  {
    mom_lock_item (_L (editeditm).pitem);
    _L (setattrs) = (momval_t) mom_item_set_attributes (_L (editeditm).pitem);
    _N (nbattrs) = mom_set_cardinal (_L (setattrs));
    mom_unlock_item (_L (editeditm).pitem);
  }
  {
    momval_t namidv =
      (momval_t) mom_item_get_name_or_idstr (_L (editeditm).pitem);
    momval_t idv = (momval_t) mom_item_get_idstr (_L (editeditm).pitem);
    bool anonymous = (namidv.ptr == idv.ptr);
    mom_should_lock_item (_L (webx).pitem);
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
		      ("<span class='mom_displaytabanon_cl' id='momeditab"),
		      MOMOUT_LITERALV (editoridstr),
		      MOMOUT_JS_LITERAL ("'>"),
		      MOMOUT_JS_LITERALV ((const char *)
					  mom_string_cstr ((idv))),
		      MOMOUT_JS_LITERAL ("</span>"));
      else
	MOM_WEBX_OUT (_L (webx).pitem,
		      MOMOUT_JS_LITERAL
		      ("<span class='mom_displaytabnamed_cl' id='momeditab"),
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
		      MOMOUT_JS_LITERAL ("</tt>"), MOMOUT_JS_RAW_NEWLINE ());
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
		  ("<span class='mom_editdate_cl'>displayed on "),
		  MOMOUT_DOUBLE_TIME ((const char *) "%c",
				      mom_clock_time (CLOCK_REALTIME)),
		  MOMOUT_JS_LITERAL ("</span></p>"));
    MOM_WEBX_OUT (_L (webx).pitem,
		  //
		  MOMOUT_JS_LITERAL ("<div class='mom_attributes_cl'>"),
		  MOMOUT_JS_LITERAL ("<p class='mom_attrtitle_cl'>"),
		  MOMOUT_DEC_INT ((int) _N (nbattrs)),
		  MOMOUT_JS_LITERAL
		  ("<span class='mom_moreattributes_cl'></span>"),
		  MOMOUT_JS_LITERAL (" attribute[s]:"),
		  MOMOUT_JS_LITERAL ("</p>"), MOMOUT_JS_RAW_NEWLINE ());
    //
    MOM_WEBX_OUT (_L (webx).pitem,
		  MOMOUT_JS_LITERAL ("<ul class='mom_attrlist_cl'>"),
		  MOMOUT_JS_RAW_NEWLINE ());
    mom_unlock_item (_L (webx).pitem);
    for (_N (atix) = 0; _N (atix) < _N (nbattrs); _N (atix)++)
      {
	_L (curattritm) =
	  (momval_t) mom_set_nth_item (_L (setattrs), _N (atix));
	/// retrieve the attribute value
	{
	  assert (mom_is_item (_L (editeditm)));
	  assert (mom_is_item (_L (curattritm)));
	  mom_lock_item (_L (editeditm).pitem);
	  _L (curvalattr) = mom_item_get_attribute (_L (editeditm).pitem,
						    _L (curattritm).pitem);
	  mom_unlock_item (_L (editeditm).pitem);
	}
	/// create the orig display
	{
	  mom_should_lock_item (_L (editor).pitem);
	  _N (rank) = mom_item_vector_count (_L (editor).pitem);
	  _L (orig) = (momval_t) mom_make_item ();
	  mom_item_put_attribute (_L (orig).pitem, mom_named__editor,
				  _L (editor));
	  mom_item_put_attribute (_L (orig).pitem, mom_named__rank,
				  mom_make_integer (_N (rank)));
	  mom_item_vector_append1 (_L (editor).pitem, _L (orig));
	  mom_unlock_item (_L (editor).pitem);
	}
	///
	MOM_DEBUG (run,
		   MOMOUT_LITERAL ("ajax_objects_codmom display atix="),
		   MOMOUT_DEC_INT ((int) _N (atix)),
		   MOMOUT_LITERAL ("; curattritm="),
		   MOMOUT_VALUE ((const momval_t) _L (curattritm)),
		   MOMOUT_LITERAL ("; curvalattr="),
		   MOMOUT_VALUE ((const momval_t) _L (curvalattr)),
		   MOMOUT_LITERAL ("; displayvalueclos="),
		   MOMOUT_VALUE ((const momval_t) _C (displayvalueclos)),
		   MOMOUT_LITERAL ("; orig="),
		   MOMOUT_VALUE ((const momval_t) _L (orig)), NULL);
	{
	  mom_should_lock_item (_L (webx).pitem);
	  MOM_WEBX_OUT (_L (webx).pitem,
			MOMOUT_JS_LITERAL
			("<li class='mom_display_attr_entry_cl' id='momdisplay"),
			MOMOUT_LITERALV ((const char *)
					 mom_string_cstr ((momval_t)
							  mom_item_get_idstr
							  (_L (orig).pitem))),
			MOMOUT_JS_LITERAL ("'>"));
	  display_item_occ_mom (_L (webx).pitem, _L (curattritm).pitem);
	  MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JS_LITERAL (" " "&#9659;"	/*U+25BB WHITE RIGHT-POINTING POINTER ▻ */
							    " "));
	  mom_unlock_item (_L (webx).pitem);
	}
	mom_item_tasklet_push_frame	///
	  (momtasklet_, _C (displayvalueclos),	//
	   MOMPFR_FIVE_VALUES (_L (editor), _L (webx), _L (curvalattr),	//curval
			       _L (orig),	//orig
			       /*olddisplay: */ MOM_NULLV
	   ), MOMPFR_INT ((intptr_t) 0), NULL);
	mom_item_tasklet_clear_res (momtasklet_);
	_SET_STATE (didattrdisplay);
	///
      ajaxobjs_lab_didattrdisplay:	////// **********************
	//////
	_L (display) = mom_item_tasklet_res1 (momtasklet_);
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom didattrdisplay display="),
		   MOMOUT_VALUE ((const momval_t) _L (display)));
	mom_item_tasklet_clear_res (momtasklet_);
	{
	  mom_should_lock_item (_L (orig).pitem);
	  mom_item_put_attribute
	    (_L (orig).pitem, mom_named__display,
	     (momval_t) mom_make_node_sized (mom_named__attr, 3,
					     _L (editeditm),
					     _L (curattritm), _L (display)));
	  mom_unlock_item (_L (orig).pitem);
	}
	{
	  mom_should_lock_item (_L (webx).pitem);
	  MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JS_LITERAL ("</li>"),
			MOMOUT_JS_RAW_NEWLINE ());
	  mom_unlock_item (_L (webx).pitem);
	}
	_L (orig).ptr = NULL;
      }				// end for _N(atix)
  }
  //// create an orig display for the content
  {
    mom_should_lock_item (_L (editor).pitem);
    _N (rank) = mom_item_vector_count (_L (editor).pitem);
    _L (orig) = (momval_t) mom_make_item ();
    mom_item_put_attribute (_L (orig).pitem, mom_named__editor, _L (editor));
    mom_item_put_attribute (_L (orig).pitem, mom_named__rank,
			    mom_make_integer (_N (rank)));
    mom_item_vector_append1 (_L (editor).pitem, _L (orig));
    mom_unlock_item (_L (editor).pitem);
  }
  {
    mom_lock_item (_L (editeditm).pitem);
    _L (curcontent) = mom_item_content (_L (editeditm).pitem);
    mom_unlock_item (_L (editeditm).pitem);
  }
  {
    mom_should_lock_item (_L (webx).pitem);
    MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JS_LITERAL ("</ul>"), MOMOUT_JS_LITERAL ("<p class='mom_displaycontent_cl' id='momdisplay"), MOMOUT_LITERALV ((const char *) mom_string_cstr ((momval_t) mom_item_get_idstr (_L (orig).pitem))), MOMOUT_JS_LITERAL ("'>" "&#8258;"	/* U+2042 ASTERISM ⁂ */
																															      " "));
    mom_unlock_item (_L (webx).pitem);
  }
  MOM_DEBUG (run,
	     MOMOUT_LITERAL ("ajax_objects_codmom display content="),
	     MOMOUT_VALUE (_L (curcontent)),
	     MOMOUT_LITERAL ("; orig="), MOMOUT_VALUE (_L (orig)));
  mom_item_tasklet_push_frame	///
    (momtasklet_, _C (displayvalueclos),	//
     MOMPFR_FIVE_VALUES (_L (editor), _L (webx), _L (curcontent),	//curval
			 _L (orig),	//orig
			 /*olddisplay: */ MOM_NULLV
     ), MOMPFR_INT ((intptr_t) 0), NULL);
  mom_item_tasklet_clear_res (momtasklet_);
  _SET_STATE (didcontentdisplay);
  ////
ajaxobjs_lab_didcontentdisplay:
  _L (display) = mom_item_tasklet_res1 (momtasklet_);
  mom_item_tasklet_clear_res (momtasklet_);
  MOM_DEBUG (run,
	     MOMOUT_LITERAL
	     ("ajax_objects_codmom didcontentdisplay content="),
	     MOMOUT_VALUE (_L (curcontent)),
	     MOMOUT_LITERAL (" display="), MOMOUT_VALUE (_L (display)),
	     MOMOUT_SPACE (48),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				     mom_value_to_item (_L (display))),
	     MOMOUT_NEWLINE (), NULL);
  ////// finalize the editor
  {
    int sizedit = -1;
    mom_should_lock_item (_L (editor).pitem);
    mom_item_put_attribute
      (_L (orig).pitem, mom_named__display,
       (momval_t) mom_make_node_sized (mom_named__content, 2,
				       _L (editeditm), _L (display)));
    sizedit = mom_item_vector_count (_L (editor).pitem);
    mom_item_put_attribute (_L (editor).pitem, mom_named__size,
			    mom_make_integer (sizedit));
    if (MOM_IS_DEBUGGING (run))
      {
	MOM_DEBUG (run,
		   MOMOUT_LITERAL
		   ("ajax_objects_codmom didcontentdisplay editor:"),
		   MOMOUT_VALUE (_L (editor)),
		   MOMOUT_LITERAL (" sizedit="), MOMOUT_DEC_INT (sizedit));
	for (int eix = 0; eix < sizedit; eix++)
	  {
	    MOM_DEBUG (run,
		       MOMOUT_LITERAL
		       ("ajax_objects_codmom didcontentdisplay eix#"),
		       MOMOUT_DEC_INT (eix),
		       MOMOUT_LITERAL (" editorcomp: "),
		       MOMOUT_VALUE ((mom_item_vector_nth
				      (_L (editor).pitem, eix))), NULL);
	  }
      }
    mom_unlock_item (_L (editor).pitem);
  }
  _L (orig).ptr = NULL;
  ////// end the content para, and the entire tab, and send the JSON
  {
    mom_should_lock_item (_L (webx).pitem);
    MOM_WEBX_OUT (_L (webx).pitem, MOMOUT_JS_LITERAL ("</p>"),
		  MOMOUT_JS_RAW_NEWLINE (),
		  MOMOUT_JS_LITERAL ("</div>"), MOMOUT_JS_RAW_NEWLINE (),
		  MOMOUT_LITERAL ("\" "),
		  MOMOUT_NEWLINE (),
		  MOMOUT_LITERAL ("}") /* to end the JSON */ ,
		  MOMOUT_NEWLINE ());
    mom_webx_reply (_L (webx).pitem, "application/json", HTTP_OK);
    mom_unlock_item (_L (webx).pitem);
  }
  MOM_DEBUG (run,
	     MOMOUT_LITERAL
	     ("ajax_objects_codmom didcontentdisplay returning"));
  return momroutres_pop;
  ////
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
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
enum edit_value_valindex_en
{
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
  MOM_FATAPRINTF ("edit_value is obsolete");
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



/**************************************************************/
////////////////////////////////////////////////////////////////
///// update_display_value
enum update_display_value_valindex_en
{
  update_display_value_v_display,
  update_display_value_v_spare,
  update_display_value_v_dispnode,
  update_display_value_v_dispconn,
  update_display_value_v_curitem,
  update_display_value_v_curattr,
  update_display_value_v_subdisplay,
  update_display_value_v_curval,
  update_display_value_v_sondisplays,
  update_display_value_v_vectvalues,
  update_display_value_v__lastval
};

enum update_display_value_closure_en
{
  update_display_value_c__lastclosure
};

enum update_display_value_numbers_en
{
  update_display_value_n_nbsons,
  update_display_value_n_ix,
  update_display_value_n__lastnum
};


static int
update_display_value_codmom (int momstate_, momitem_t *momtasklet_,
			     const momnode_t *momclosure_,
			     momval_t *momlocvals_, intptr_t * momlocnums_,
			     double *momlocdbls_)
{
#define _L(Nam) (momlocvals_[update_display_value_v_##Nam])
#define _C(Nam) (momclosure_->sontab[update_display_value_c_##Nam])
#define _N(Nam) (momlocnums_[update_display_value_n_##Nam])
#define _DO_ON_STATES(A)			\
  A(start)					\
    A(impossible)				\
    A(didupdateattrdisp)			\
    A(didupdatesondisp)
  enum update_display_value_state_en
  {
#define UpdateDisplayValue_DefineState(S) update_display_value_s_##S,
    _DO_ON_STATES (UpdateDisplayValue_DefineState)
      update_display_value_s__laststate
#undef UpdateDisplayValue_DefineState
  };
#define _SET_STATE(St) do {						\
    MOM_DEBUGPRINTF (run,						\
		     "update_display_value_codmom setstate " #St " = %d", \
		     (int)update_display_value_s_##St);			\
    return update_display_value_s_##St; } while(0)
  if (momstate_ >= 0 && momstate_ < update_display_value_s__laststate)
    switch ((enum update_display_value_state_en) momstate_)
      {
#define UpdateDisplayValue_JumpState(S) case update_display_value_s_##S: goto update_display_value_lab_##S;
	_DO_ON_STATES (UpdateDisplayValue_JumpState)
#undef UpdateDisplayValue_JumpState
      case update_display_value_s__laststate:;
      }
  MOM_FATAPRINTF ("update_display_value invalid state #%d", momstate_);
update_display_value_lab_start:
  if (_L (display).ptr == MOM_EMPTY)
    _SET_STATE (impossible);
  {
    mom_should_lock_item (_L (display).pitem);
    MOM_DEBUG (run, MOMOUT_LITERAL ("update_display_value start display="),
	       MOMOUT_VALUE ((const momval_t) _L (display)),
	       MOMOUT_LITERAL (" !: "),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       mom_value_to_item (_L (display))),
	       NULL);
    _L (dispnode) =
      mom_item_get_attribute (_L (display).pitem, mom_named__display);
    _L (curval) = mom_item_get_attribute (_L (display).pitem, mom_named__val);
    mom_unlock_item (_L (display).pitem);
  }
  _L (dispconn) = (momval_t) mom_node_conn (_L (dispnode));
  //
  MOM_DEBUG (run, MOMOUT_LITERAL ("update_display_value dispnode="),
	     MOMOUT_VALUE(_L(dispnode)),
	     MOMOUT_LITERAL (" curval="),
	     MOMOUT_VALUE(_L(curval)),
	     MOMOUT_LITERAL (" dispconn="),
	     MOMOUT_VALUE(_L(dispconn)),
	     NULL);
  //	     
  if (_L (dispnode).pitem == mom_named__empty)
    {
      MOM_DEBUG (run, MOMOUT_LITERAL ("update_display_value empty"), NULL);
      _L (curval) = MOM_NULLV;
      mom_item_tasklet_set_1res (momtasklet_, MOM_NULLV);
      return momroutres_pop;
    }
  else if (_L (dispnode).pitem == mom_named__integer
	   || _L (dispnode).pitem == mom_named__item
	   || _L (dispnode).pitem == mom_named__double
	   || _L (dispnode).pitem == mom_named__string)
    {
      MOM_DEBUG (run, MOMOUT_LITERAL ("update_display_value scalar curval="),
		 MOMOUT_VALUE ((const momval_t) _L (curval)), NULL);
      assert (_L (curval).ptr != NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (curval));
      return momroutres_pop;
    }
  else if (_L (dispnode).pitem == mom_named__set)
    {
      MOM_DEBUG (run, MOMOUT_LITERAL ("update_display_value set curval="),
		 MOMOUT_VALUE ((const momval_t) _L (curval)), NULL);
      assert (mom_is_set (_L (curval)));
      mom_item_tasklet_set_1res (momtasklet_, _L (curval));
      return momroutres_pop;
    }
  else if (_L (dispnode).pitem == mom_named__tuple)
    {
      MOM_DEBUG (run, MOMOUT_LITERAL ("update_display_value tuple curval="),
		 MOMOUT_VALUE ((const momval_t) _L (curval)), NULL);
      assert (mom_is_tuple (_L (curval)));
      mom_item_tasklet_set_1res (momtasklet_, _L (curval));
      return momroutres_pop;
    }
  else if (_L (dispconn).pitem == mom_named__attr
	   && mom_node_arity (_L (dispnode)) == 3)
    {
      _L (curitem) = mom_node_nth (_L (dispnode), 0);
      _L (curattr) = mom_node_nth (_L (dispnode), 1);
      _L (subdisplay) = mom_node_nth (_L (dispnode), 2);
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("update_display_value start attr subdisplay="),
		 MOMOUT_VALUE ((const momval_t) _L (subdisplay)),
		 MOMOUT_LITERAL (" !: "),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					 mom_value_to_item (_L (subdisplay))),
		 NULL);

      mom_item_tasklet_push_frame (momtasklet_, (momval_t) momclosure_,
				   MOMPFR_VALUE (_L (subdisplay)), NULL);
      mom_item_tasklet_clear_res (momtasklet_);
      _SET_STATE (didupdateattrdisp);
      ////// ********
    update_display_value_lab_didupdateattrdisp:
      _L (curval) = mom_item_tasklet_res1 (momtasklet_);
      mom_item_tasklet_clear_res (momtasklet_);
      MOM_DEBUG (run,
		 MOMOUT_LITERAL
		 ("update_display_value didupdateattrdisp curval="),
		 MOMOUT_VALUE ((const momval_t) _L (curval)),
		 MOMOUT_LITERAL (" curitem="),
		 MOMOUT_VALUE ((const momval_t) _L (curitem)),
		 MOMOUT_LITERAL (" curattr="),
		 MOMOUT_VALUE ((const momval_t) _L (curattr)), NULL);
      {
	mom_should_lock_item (_L (curitem).pitem);
	mom_item_put_attribute (_L (curitem).pitem, _L (curattr).pitem,
				_L (curval));
	mom_unlock_item (_L (curitem).pitem);
      }
      return momroutres_pop;
    }
  else if (_L (dispconn).pitem == mom_named__node
	   && mom_node_arity (_L (dispnode)) == 2)
    {
      _L (curitem) = mom_node_nth (_L (dispnode), 0);
      _L (sondisplays) = mom_node_nth (_L (dispnode), 1);
      _N (nbsons) = mom_tuple_length (_L (sondisplays));
      _L (vectvalues) = (momval_t) mom_make_item ();
      mom_item_start_vector (_L (vectvalues).pitem);
      mom_item_vector_reserve (_L (vectvalues).pitem, _N (nbsons));
      MOM_DEBUG (run,
		 MOMOUT_LITERAL ("update_display_value start node curitem="),
		 MOMOUT_VALUE (_L (curitem)),
		 MOMOUT_LITERAL (" sondisplays="),
		 MOMOUT_VALUE (_L (sondisplays)),
		 MOMOUT_LITERAL (" nbsons="),
		 MOMOUT_DEC_INT ((int) _N (nbsons)),
		 MOMOUT_LITERAL (" vectvalues="),
		 MOMOUT_VALUE (_L (vectvalues)), NULL);
      for (_N (ix) = 0; _N (ix) < _N (nbsons); _N (ix)++)
	{
	  _L (subdisplay) =
	    (momval_t) mom_tuple_nth_item (_L (sondisplays), _N (ix));

	  MOM_DEBUG (run,
		     MOMOUT_LITERAL ("update_display_value node ix="),
		     MOMOUT_DEC_INT ((int) _N (ix)),
		     MOMOUT_LITERAL (" subdisplay="),
		     MOMOUT_VALUE ((const momval_t) _L (subdisplay)),
		     MOMOUT_LITERAL (" !: "),
		     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					     mom_value_to_item (_L
								(subdisplay))),
		     NULL);

	  mom_item_tasklet_push_frame (momtasklet_, (momval_t) momclosure_,
				       MOMPFR_VALUE (_L (subdisplay)), NULL);
	  mom_item_tasklet_clear_res (momtasklet_);
	  _SET_STATE (didupdatesondisp);
	  ////// ********
	update_display_value_lab_didupdatesondisp:
	  _L (curval) = mom_item_tasklet_res1 (momtasklet_);
	  mom_item_tasklet_clear_res (momtasklet_);
	  MOM_DEBUG (run,
		     MOMOUT_LITERAL ("update_display_value node ix="),
		     MOMOUT_DEC_INT ((int) _N (ix)),
		     MOMOUT_LITERAL (" curval="),
		     MOMOUT_VALUE ((const momval_t) _L (curval)), NULL);
	  {
	    mom_should_lock_item (_L (vectvalues).pitem);
	    mom_item_vector_append1 (_L (vectvalues).pitem, _L (curval));
	    mom_unlock_item (_L (vectvalues).pitem);
	  }
	}			/* end for _N(ix) */
      _L (curval) = MOM_NULLV;
      {
	mom_should_lock_item (_L (vectvalues).pitem);
	_L (curval) =
	  mom_make_node_from_item_vector (_L (curitem).pitem,
					  _L (vectvalues).pitem);
	mom_unlock_item (_L (vectvalues).pitem);
      }
      MOM_DEBUG (run,
		 MOMOUT_LITERAL ("update_display_value node result curval="),
		 MOMOUT_VALUE (_L (curval)), NULL);
      mom_item_tasklet_set_1res (momtasklet_, _L (curval));
      return momroutres_pop;
    }
  else
    MOM_FATAL (MOMOUT_LITERAL
	       ("update_display_value fail to handle dispnode="),
	       MOMOUT_VALUE ((const momval_t) _L (dispnode)),
	       MOMOUT_LITERAL (" in display="), MOMOUT_VALUE (_L (display)),
	       NULL);
  return momroutres_pop;
update_display_value_lab_impossible:
  MOM_FATAPRINTF ("update_display_value impossible state reached!");
#undef _L
#undef _C
#undef _N
#undef _SET_STATE
#undef _DO_ON_STATES
  return momroutres_pop;
}

const struct momroutinedescr_st momrout_update_display_value = {
  .rout_magic = MOM_ROUTINE_MAGIC,	//
  .rout_minclosize = update_display_value_c__lastclosure,	//
  .rout_frame_nbval = update_display_value_v__lastval,	//
  .rout_frame_nbnum = update_display_value_n__lastnum,	//
  .rout_frame_nbdbl = 0,	//
  .rout_name = "update_display_value",	//
  .rout_module = MONIMELT_CURRENT_MODULE,	//
  .rout_codefun = update_display_value_codmom,	//
  .rout_timestamp = __DATE__ "@" __TIME__
};



////////////////////////////////////////////////////////////////
///// ajax_complete_name
enum ajax_complete_name_valindex_en
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
		   MOMOUT_NEWLINE (), MOMOUT_LITERAL (" jsarrnames="),
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
///// noop
enum noop_valindex_en
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
