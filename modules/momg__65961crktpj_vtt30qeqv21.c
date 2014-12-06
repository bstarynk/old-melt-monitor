// MONIMELT module first_module
// generated file momg__65961crktpj_vtt30qeqv21.c
///**** DO NOT EDIT! ***

////////////////////////////////////////////////////////////////++

/// generated file momg__65961crktpj_vtt30qeqv21.c ** DO NOT EDIT

/// Copyright (C) 2014 Free Software Foundation, Inc.
// MONIMELT is a monitor for MELT - see http://gcc-melt.org/
// This file is part of GCC.

// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.

// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.   If not see
//  <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////--




//// **** head of momg__65961crktpj_vtt30qeqv21.c



//// header part

#include "monimelt.h"

////++++ declaration of 4 routines:

////!AJAX for appl menu

// declare tasklet function ajax_appl rank#0
static int momfuncod__06uk4pppvx9_huv0v11v18j(int, momitem_t*, momval_t, momval_t*, intptr_t*, double*);

////!todo procedure on exit

// declare procedure _07zti91e4kd_952zqsd03fz rank#1
void momprocfun__07zti91e4kd_952zqsd03fz (momcstr_t);

////!procedure to show on webx the HTML code for some item

// declare procedure show_html_for_item_proc rank#2
void momprocfun__0z0rsvwfkcj_dcpkx68i074 (momval_t, momval_t);

////!todo procedure on dump

// declare procedure _7x6as13park_w64mrkx2xtm rank#3
void momprocfun__7x6as13park_w64mrkx2xtm (momcstr_t);

static momitem_t* momfconstitems__06uk4pppvx9_huv0v11v18j[1]; // constant items of tasklet function ajax_appl
// constant items of proc. _07zti91e4kd_952zqsd03fz
static const momitem_t* mompconstitems__07zti91e4kd_952zqsd03fz[1]; //! for _07zti91e4kd_952zqsd03fz
// constant items of proc. show_html_for_item_proc
static const momitem_t* mompconstitems__0z0rsvwfkcj_dcpkx68i074[3]; //! for show_html_for_item_proc
// constant items of proc. _7x6as13park_w64mrkx2xtm
static const momitem_t* mompconstitems__7x6as13park_w64mrkx2xtm[11]; //! for _7x6as13park_w64mrkx2xtm

// declare module md5sum for first_module
const char mommd5mod__65961crktpj_vtt30qeqv21[] = MONIMELT_MD5_MODULE; // Makefile generated

// declare module routines descriptor array for first_module
static const union momrout_un momdroutarr__65961crktpj_vtt30qeqv21[5];




//// **** body of momg__65961crktpj_vtt30qeqv21.c



//// body part

////++++ implementation of 4 routines:


// implement tasklet function ajax_appl rank#0
static int momfuncod__06uk4pppvx9_huv0v11v18j
(int momstate,
	 momitem_t* restrict momtasklet, const momval_t momclosure,
	 momval_t* restrict momvals,
	 intptr_t* restrict momnums_ MOM_UNUSED,
	 double* restrict momdbls_ MOM_UNUSED)
{ // start of tasklet function ajax_appl
 assert (mom_item_payload_kind (momtasklet) == mompayk_tasklet);
 if (MOM_UNLIKELY(momstate==0)) {
  momstate = 3;
  mom_item_tasklet_top_frame_set_state (momtasklet, 3);
 };
 assert (mom_is_item (momclosure));
 momval_t* momclovals = mom_item_closure_values (momclosure.pitem);
 assert (momclovals != NULL);
 assert (momvals != NULL); // 2 values.
  MOM_DEBUG(run,
   MOMOUT_LITERAL("start tasklet="), MOMOUT_ITEM((const momitem_t*)momtasklet),
   MOMOUT_LITERAL(" state#"), MOMOUT_DEC_INT((int)momstate),
   MOMOUT_LITERAL(" taskfunc ajax_appl"));
 switch (momstate) {
 case 1: goto momfblo_1; // block _7yyaydvyhpr_teuchcqzs7k
 case 2: goto momfblo_2; // block _8t137w1z1s9_2tea9xp64s6
 case 3: goto momfblo_3; // block _8y756mef2ca_w8cj58726vj
 case 4: goto momfblo_4; // block _91471ta1047_pra9zfqc2y1
 case 5: goto momfblo_5; // block _9u6a6xy2e1p_qeapfc73cm4
 default: return momroutres_pop;
 }; // end switch state
 
 // +++++++ function block #1 _7yyaydvyhpr_teuchcqzs7k
 momfblo_1:
 {
  ////!block to handle ajax_appl when whatv=dump
  
  
  //! instr#1 in block _7yyaydvyhpr_teuchcqzs7k ::
  /*!do*/ /*!primitive-void mom_stop_work_with_todo*/ mom_stop_work_with_todo((mom_todoafterstop_fun_t *)
  momprocfun__7x6as13park_w64mrkx2xtm, /*!litstr:*/ ".")/*!endvoidprimitive mom_stop_work_with_todo*/  /*!done*/;
  
  //! instr#2 in block _7yyaydvyhpr_teuchcqzs7k ::
  
  /*!jump _91471ta1047_pra9zfqc2y1*/
    return 5 /*!func.block _91471ta1047_pra9zfqc2y1*/;
 }; // -------- end function block _7yyaydvyhpr_teuchcqzs7k
 return momroutres_pop;
 
 // +++++++ function block #2 _8t137w1z1s9_2tea9xp64s6
 momfblo_2:
 {
  ////!block to handle appl with what=exit
  
  
  // locked-item webx in block _8t137w1z1s9_2tea9xp64s6
  momitem_t* momlockeditem_1 = mom_value_to_item ( momvals[0/*:webx*/]) /* locked-item */;
  if (!mom_lock_item (momlockeditem_1)) goto momendblock_1;
  
  //! instr#1 in block _8t137w1z1s9_2tea9xp64s6 ::
  /*!do*/ /*!primitive-void mom_stop_work_with_todo*/ mom_stop_work_with_todo((mom_todoafterstop_fun_t *)
  momprocfun__07zti91e4kd_952zqsd03fz, /*!litstr:*/ ".")/*!endvoidprimitive mom_stop_work_with_todo*/  /*!done*/;
  
  //! instr#2 in block _8t137w1z1s9_2tea9xp64s6 ::
  /*!do*/ /*!primitive-void MOM_WEBX_OUT*/ MOM_WEBX_OUT (mom_value_to_item(
  momvals[0/*:webx*/]), /*!litoutstr*/MOMOUTDO_LITERAL, "<em>Monimelt</em> <b>save then exit</b> at <i>",
  /*!outexp MOMOUT_DOUBLE_TIME*/ MOMOUTDO_DOUBLE_TIME,
  /*!litstr:*/ "%c", (/*!primitive mom_clock_time*/ mom_clock_time(
  CLOCK_REALTIME)/*!endprimitive mom_clock_time*/) /*!endoutexp MOMOUT_DOUBLE_TIME*/,
   /*!litoutstr*/MOMOUTDO_LITERAL, "</i>", /*!outputend*/NULL
  , NULL)/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
  //! instr#3 in block _8t137w1z1s9_2tea9xp64s6 ::
  /*!do*/ /*!primitive-void mom_webx_reply*/ mom_webx_reply (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "text/html",
  HTTP_OK)/*!endvoidprimitive mom_webx_reply*/  /*!done*/;
  
  /*! epilogue for lock */
  mom_unlock_item (momlockeditem_1); // unlock webx
  momendblock_1:;
  
 }; // -------- end function block _8t137w1z1s9_2tea9xp64s6
 return momroutres_pop;
 
 // +++++++ function block #3 _8y756mef2ca_w8cj58726vj
 momfblo_3:
 {
  ////!starting block for ajax_appl.
  
  
  // locked-item webx in block _8y756mef2ca_w8cj58726vj
  momitem_t* momlockeditem_2 = mom_value_to_item ( momvals[0/*:webx*/]) /* locked-item */;
  if (!mom_lock_item (momlockeditem_2)) goto momendblock_2;
  
  //! instr#1 in block _8y756mef2ca_w8cj58726vj ::
  /*!do*/ /*!primitive-void debug_run*/ MOM_DEBUG(run,  /*!litoutstr*/MOMOUTDO_LITERAL, "ajax_appl webx:",
   /*!outvalvar*/MOMOUTDO_VALUE, (momval_t) momvals[0/*:webx*/],/*!outputend*/NULL
  )/*!endvoidprimitive debug_run*/  /*!done*/;
  
  //! instr#2 in block _8y756mef2ca_w8cj58726vj ::
  /*!assign*/  momvals[1/*:whatv*/] = (/*!primitive mom_webx_post_arg*/ mom_webx_post_arg (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "what_mom")/*!endprimitive mom_webx_post_arg*/) ;
  
  //! instr#3 in block _8y756mef2ca_w8cj58726vj ::
  /*!do*/ /*!primitive-void debug_run*/ MOM_DEBUG(run,  /*!litoutstr*/MOMOUTDO_LITERAL, "ajax_appl whatv:",
   /*!outvalvar*/MOMOUTDO_VALUE, (momval_t) momvals[1/*:whatv*/],/*!outputend*/NULL
  )/*!endvoidprimitive debug_run*/  /*!done*/;
  
  //! instr#4 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[1/*:whatv*/]), ( /*!litstr:*/ "exit"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_2);  return 2 /*!func.block _8t137w1z1s9_2tea9xp64s6*/;
   }; //!unlocked momlockeditem_2
    
  //! instr#5 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[1/*:whatv*/]), ( /*!litstr:*/ "quit"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_2);  return 3 /*!func.block _9u6a6xy2e1p_qeapfc73cm4*/;
   }; //!unlocked momlockeditem_2
    
  //! instr#6 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[1/*:whatv*/]), ( /*!litstr:*/ "dump"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_2);  return 4 /*!func.block _7yyaydvyhpr_teuchcqzs7k*/;
   }; //!unlocked momlockeditem_2
    
  /*! epilogue for lock */
  mom_unlock_item (momlockeditem_2); // unlock webx
  momendblock_2:;
  
 }; // -------- end function block _8y756mef2ca_w8cj58726vj
 return momroutres_pop;
 
 // +++++++ function block #4 _91471ta1047_pra9zfqc2y1
 momfblo_4:
 {
  ////!block for ajax_appl continue after dump
  
  
  //! instr#1 in block _91471ta1047_pra9zfqc2y1 ::
  /*!do*/ /*!primitive-void MOM_WEBX_OUT*/ MOM_WEBX_OUT (mom_value_to_item(
  momvals[0/*:webx*/]), /*!litoutstr*/MOMOUTDO_LITERAL, "<em>Monimelt</em> <b>dump then continue</b> at<i>",
  /*!outexp MOMOUT_DOUBLE_TIME*/ MOMOUTDO_DOUBLE_TIME,
  /*!litstr:*/ "%c", (/*!primitive mom_clock_time*/ mom_clock_time(
  CLOCK_REALTIME)/*!endprimitive mom_clock_time*/) /*!endoutexp MOMOUT_DOUBLE_TIME*/,
   /*!litoutstr*/MOMOUTDO_LITERAL, "</i>", /*!outputend*/NULL
  , NULL)/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
  //! instr#2 in block _91471ta1047_pra9zfqc2y1 ::
  /*!do*/ /*!primitive-void mom_webx_reply*/ mom_webx_reply (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "text/html",
  HTTP_OK)/*!endvoidprimitive mom_webx_reply*/  /*!done*/;
  
 }; // -------- end function block _91471ta1047_pra9zfqc2y1
 return momroutres_pop;
 
 // +++++++ function block #5 _9u6a6xy2e1p_qeapfc73cm4
 momfblo_5:
 {
  ////!block to handle ajax_appl when what=quit
  
  
  // locked-item webx in block _9u6a6xy2e1p_qeapfc73cm4
  momitem_t* momlockeditem_3 = mom_value_to_item ( momvals[0/*:webx*/]) /* locked-item */;
  if (!mom_lock_item (momlockeditem_3)) goto momendblock_3;
  
  //! instr#1 in block _9u6a6xy2e1p_qeapfc73cm4 ::
  /*!do*/ /*!primitive-void MOM_WEBX_OUT*/ MOM_WEBX_OUT (mom_value_to_item(
  momvals[0/*:webx*/]), /*!litoutstr*/MOMOUTDO_LITERAL, "<em>Monimelt</em> <b>quitting</b> without saving at <i>",
  /*!outexp MOMOUT_DOUBLE_TIME*/ MOMOUTDO_DOUBLE_TIME,
  /*!litstr:*/ "%c", (/*!primitive mom_clock_time*/ mom_clock_time(
  CLOCK_REALTIME)/*!endprimitive mom_clock_time*/) /*!endoutexp MOMOUT_DOUBLE_TIME*/,
   /*!litoutstr*/MOMOUTDO_LITERAL, "</i>", /*!outputend*/NULL
  , NULL)/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
  //! instr#2 in block _9u6a6xy2e1p_qeapfc73cm4 ::
  /*!do*/ /*!primitive-void mom_webx_reply*/ mom_webx_reply (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "text/html",
  HTTP_OK)/*!endvoidprimitive mom_webx_reply*/  /*!done*/;
  
  //! instr#3 in block _9u6a6xy2e1p_qeapfc73cm4 ::
  /** chunk **/
   MOM_INFORMPRINTF("quitting Monimelt per web request");
 exit(EXIT_SUCCESS); // per web request

  /*! epilogue for lock */
  mom_unlock_item (momlockeditem_3); // unlock webx
  momendblock_3:;
  
 }; // -------- end function block _9u6a6xy2e1p_qeapfc73cm4
 return momroutres_pop;
 } // end function ajax_appl

static const char* const momfconstid__06uk4pppvx9_huv0v11v18j[1] = { // constant ids of function ajax_appl
}; // end of function constants of ajax_appl

static const char* const momfblockid__06uk4pppvx9_huv0v11v18j[7] = { // block ids of function ajax_appl
 [0] = "_7yyaydvyhpr_teuchcqzs7k", //!block _7yyaydvyhpr_teuchcqzs7k
 [1] = "_8t137w1z1s9_2tea9xp64s6", //!block _8t137w1z1s9_2tea9xp64s6
 [2] = "_8y756mef2ca_w8cj58726vj", //!block _8y756mef2ca_w8cj58726vj
 [3] = "_91471ta1047_pra9zfqc2y1", //!block _91471ta1047_pra9zfqc2y1
 [4] = "_9u6a6xy2e1p_qeapfc73cm4", //!block _9u6a6xy2e1p_qeapfc73cm4
}; // end of function block-ids of ajax_appl


static const char* const momflocvalid__06uk4pppvx9_huv0v11v18j[3] = { // value var.ids of function ajax_appl
  [0] = "_16cd0fvmdrh_r77ajpy26za", //!! val.var. webx
  [1] = "_350hj5kfymd_145tfc1sevi", //!! val.var. whatv
}; // end val.var. of ajax_appl

// no number.var.ids for ajax_appl

// no dbl.var.ids for ajax_appl

const struct momtfundescr_st momrout__06uk4pppvx9_huv0v11v18j = { // tasklet function descriptor ajax_appl
 .tfun_magic = MOM_TFUN_MAGIC,
 .tfun_minclosize = 0,
 .tfun_nbconstants = 0,
 .tfun_nbblocks = 5,
 .tfun_frame_nbval = 2,
 .tfun_frame_nbnum = 0,
 .tfun_frame_nbdbl = 0,
 .tfun_constantids = momfconstid__06uk4pppvx9_huv0v11v18j,
 .tfun_constantitems = (const momitem_t*const*) momfconstitems__06uk4pppvx9_huv0v11v18j,
 .tfun_ident = "_06uk4pppvx9_huv0v11v18j",
 .tfun_blockids = momfblockid__06uk4pppvx9_huv0v11v18j,
 .tfun_valids = momflocvalid__06uk4pppvx9_huv0v11v18j,
 .tfun_module = MONIMELT_CURRENT_MODULE,
 .tfun_codefun = momfuncod__06uk4pppvx9_huv0v11v18j,
 .tfun_timestamp = __DATE__ "@" __TIME__
 
}; // end function descriptor




// implementation of procedure #1 = _07zti91e4kd_952zqsd03fz
void momprocfun__07zti91e4kd_952zqsd03fz (momcstr_t momparg_0/*!formal:cstr*/)
{
 /// 0 integer locals
 /// 0 double locals
 /// 0 values locals
 static momitem_t* momprocitem; //! prologue of proc. _07zti91e4kd_952zqsd03fz
 if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_07zti91e4kd_952zqsd03fz");
 /// starting procedure _07zti91e4kd_952zqsd03fz
 goto mompblo_1; // start at _1kj1j3878fe_duw3ts10hev
 /*!! 1 blocks in proc. _07zti91e4kd_952zqsd03fz */
 
 
 /******** block#1: _1kj1j3878fe_duw3ts10hev ********/
  mompblo_1: { //! start procedure block _1kj1j3878fe_duw3ts10hev
  ////!start block of todo proc on exit
  
  
  //! instr#1 in block _1kj1j3878fe_duw3ts10hev ::
  /*!do*/ /*!primitive-void _1iyd2es3u59_x6uq7vhecjj*/ mom_full_dump("todo dump-at-exit", 
  momparg_0/*cstr*/ , NULL); mom_stop_event_loop();/*!endvoidprimitive _1iyd2es3u59_x6uq7vhecjj*/  /*!done*/;
  
 }; //! end procedure block _1kj1j3878fe_duw3ts10hev
 
 /*!defreturn*/return ;
} // end proc.code _07zti91e4kd_952zqsd03fz


static const char* const mompconstid__07zti91e4kd_952zqsd03fz[1] = { //! for _07zti91e4kd_952zqsd03fz
 
 NULL }; // end constid proc _07zti91e4kd_952zqsd03fz

const struct momprocrout_st momprocdescr__07zti91e4kd_952zqsd03fz// proc.descriptor _07zti91e4kd_952zqsd03fz
 = { .prout_magic = MOM_PROCROUT_MAGIC,
 .prout_resty = momtypenc__none,.prout_len = 0,
 .prout_id = "_07zti91e4kd_952zqsd03fz",
 .prout_module = "_65961crktpj_vtt30qeqv21",
 .prout_constantids = mompconstid__07zti91e4kd_952zqsd03fz,
 .prout_constantitems = mompconstitems__07zti91e4kd_952zqsd03fz,
 .prout_addr = (void*)momprocfun__07zti91e4kd_952zqsd03fz,
 .prout_argsig = "s",
 .prout_timestamp= __DATE__ "@" __TIME__
}; // end proc descriptor




// implementation of procedure #2 = show_html_for_item_proc
void momprocfun__0z0rsvwfkcj_dcpkx68i074 (momval_t momparg_0/*!formal:webx*/,
momval_t momparg_1/*!formal:itmv*/)
{
 /// 0 integer locals
 /// 0 double locals
 /// 5 values locals
 momval_t mompval_2 = MOM_NULLV; //!! local value namv
 momval_t mompval_0 = MOM_NULLV; //!! local value webses
 momval_t mompval_4 = MOM_NULLV; //!! local value strv
 momval_t mompval_3 = MOM_NULLV; //!! local value commv
 momval_t mompval_1 = MOM_NULLV; //!! local value hitmv
 static momitem_t* momprocitem; //! prologue of proc. show_html_for_item_proc
 if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_0z0rsvwfkcj_dcpkx68i074");
 /// starting procedure show_html_for_item_proc
 goto mompblo_1; // start at _02yd241wh4z_tca7i6iamf3
 /*!! 6 blocks in proc. show_html_for_item_proc */
 
 
 /******** block#1: _02yd241wh4z_tca7i6iamf3 ********/
  mompblo_1: { //! start procedure block _02yd241wh4z_tca7i6iamf3
  ////!start of show_html_for_item..` (block)
  
  
  // locked-item webx in block _02yd241wh4z_tca7i6iamf3
  momitem_t* momlockeditem_4 = mom_value_to_item ( momparg_0/*webx*/ ) /* locked-item */;
  if (!mom_lock_item (momlockeditem_4)) goto momendblock_4;
  
  //! instr#1 in block _02yd241wh4z_tca7i6iamf3 ::
  /*!assign*/  (mompval_0/*:webses*/) = (/*!primitive mom_webx_session*/  (momval_t) mom_webx_session (
  momparg_0/*webx*/ .pitem) /*!endprimitive mom_webx_session*/) ;
  
  //! instr#2 in block _02yd241wh4z_tca7i6iamf3 ::
  /*!assign*/  (mompval_1/*:hitmv*/) = (/*!primitive mom_item_websession_get*/  mom_item_websession_get (
  (mompval_0/*:webses*/).ptr,  ((momval_t) mompconstitems__0z0rsvwfkcj_dcpkx68i074[1] /*hset*/).ptr)/*!endprimitive mom_item_websession_get*/) ;
  
  //! instr#3 in block _02yd241wh4z_tca7i6iamf3 ::
  /*!if*/ if ((/*!primitive is_nil*/  (intptr_t) (NULL == ( (mompval_1/*:hitmv*/)).ptr)/*!endprimitive is_nil*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_4); 
   goto mompblo_2 /*!proc.block _34w40p46vwp_pw6u3d75ww6*/;
   }; //!unlocked momlockeditem_4
    
  //! instr#4 in block _02yd241wh4z_tca7i6iamf3 ::
  
  /*!jump _9xhd87mskav_rqcu5eqk5te*/
  
  /*!unlock-goto*/ { mom_unlock_item (momlockeditem_4); 
  goto mompblo_3 /*!proc.block _9xhd87mskav_rqcu5eqk5te*/;
  }; //!unlocked momlockeditem_4
  
  /*! epilogue for lock */
  mom_unlock_item (momlockeditem_4); // unlock webx
  momendblock_4:;
  
 }; //! end procedure block _02yd241wh4z_tca7i6iamf3
 
 
 
 /******** block#2: _1witua6ujek_6jadsxtv4cd ********/
  mompblo_2: { //! start procedure block _1witua6ujek_6jadsxtv4cd
  ////!show uncommented anon in show_html_for_item..
  
  
 }; //! end procedure block _1witua6ujek_6jadsxtv4cd
 
 
 
 /******** block#3: _34w40p46vwp_pw6u3d75ww6 ********/
  mompblo_3: { //! start procedure block _34w40p46vwp_pw6u3d75ww6
  ////!create hset in show_html_for_item..
  ////+ block to create the item hset in the web session
  
  
  // locked-item webses in block _34w40p46vwp_pw6u3d75ww6
  momitem_t* momlockeditem_5 = mom_value_to_item ( (mompval_0/*:webses*/)) /* locked-item */;
  if (!mom_lock_item (momlockeditem_5)) goto momendblock_5;
  
  //! instr#1 in block _34w40p46vwp_pw6u3d75ww6 ::
  /*!assign*/  (mompval_1/*:hitmv*/) = (/*!primitive mom_make_item*/  (momval_t)mom_make_item ()/*!endprimitive mom_make_item*/) ;
  
  //! instr#2 in block _34w40p46vwp_pw6u3d75ww6 ::
  /*!do*/ /*!primitive-void mom_item_start_hset*/  mom_item_start_hset (
  (mompval_1/*:hitmv*/).pitem)/*!endvoidprimitive mom_item_start_hset*/  /*!done*/;
  
  //! instr#3 in block _34w40p46vwp_pw6u3d75ww6 ::
  /*!do*/ /*!primitive-void mom_item_websession_put*/  mom_item_websession_put (
  (mompval_0/*:webses*/).pitem,  ((momval_t) mompconstitems__0z0rsvwfkcj_dcpkx68i074[1] /*hset*/).pitem, 
  (mompval_1/*:hitmv*/))/*!endvoidprimitive mom_item_websession_put*/  /*!done*/;
  
  //! instr#4 in block _34w40p46vwp_pw6u3d75ww6 ::
  
  /*!jump _02yd241wh4z_tca7i6iamf3*/
  
  /*!unlock-goto*/ { mom_unlock_item (momlockeditem_5); 
  goto mompblo_1 /*!proc.block _02yd241wh4z_tca7i6iamf3*/;
  }; //!unlocked momlockeditem_5
  
  /*! epilogue for lock */
  mom_unlock_item (momlockeditem_5); // unlock webses
  momendblock_5:;
  
 }; //! end procedure block _34w40p46vwp_pw6u3d75ww6
 
 
 
 /******** block#4: _591chiicj6r_512iim4cf4m ********/
  mompblo_4: { //! start procedure block _591chiicj6r_512iim4cf4m
  ////!show anonymous in show_html_for_item..
  
  
  //! instr#1 in block _591chiicj6r_512iim4cf4m ::
  /*!assign*/  (mompval_3/*:commv*/) = (/*!primitive get_attribute*/  mom_item_get_attribute(
  momparg_1/*itmv*/ .ptr,  ((momval_t) mompconstitems__0z0rsvwfkcj_dcpkx68i074[0] /*comment*/).ptr)/*!endprimitive get_attribute*/) ;
  
  //! instr#2 in block _591chiicj6r_512iim4cf4m ::
  /*!if*/ if ((/*!primitive is_nil*/  (intptr_t) (NULL == ( (mompval_3/*:commv*/)).ptr)/*!endprimitive is_nil*/) ) 
   goto mompblo_6 /*!proc.block _1witua6ujek_6jadsxtv4cd*/;
  
  //! instr#3 in block _591chiicj6r_512iim4cf4m ::
  /** chunk **/
  /* put into strv the prefix of commv to be shown. */ const char* commvstr = mom_string_cstr (
  (mompval_3/*:commv*/));
 const char* backquotestr = strchr(commvstr, '`');
 const char* eolstr = strchr(commvstr, '\n'); if (backquotestr && (!eolstr || backquotestr < eolstr))
  
  (mompval_4/*:strv*/) = (momval_t) mom_make_string_len (commvstr, backquotestr - commvstr);
 else if (eolstr && eolstr < commvstr + 72)
  
  (mompval_4/*:strv*/) = (momval_t) mom_make_string_len (commvstr, eolstr - commvstr);
 else 
  (mompval_4/*:strv*/) = (momval_t) mom_make_string_len (commvstr, 70);

 }; //! end procedure block _591chiicj6r_512iim4cf4m
 
 
 
 /******** block#5: _7rxfx0rruqm_kdtk9pk7wtj ********/
  mompblo_5: { //! start procedure block _7rxfx0rruqm_kdtk9pk7wtj
  ////!get namv from itmv in show_html_for_item..
  
  
  //! instr#1 in block _7rxfx0rruqm_kdtk9pk7wtj ::
  /*!assign*/  (mompval_2/*:namv*/) = (/*!primitive mom_item_get_name*/  (momval_t) mom_item_get_name (
  momparg_1/*itmv*/ .ptr)/*!endprimitive mom_item_get_name*/) ;
  
  //! instr#2 in block _7rxfx0rruqm_kdtk9pk7wtj ::
  /*!if*/ if ((/*!primitive is_nil*/  (intptr_t) (NULL == ( (mompval_2/*:namv*/)).ptr)/*!endprimitive is_nil*/) ) 
   goto mompblo_5 /*!proc.block _591chiicj6r_512iim4cf4m*/;
  
  //! instr#3 in block _7rxfx0rruqm_kdtk9pk7wtj ::
  /*!do*/ /*!primitive-void MOM_WEBX_OUT*/ MOM_WEBX_OUT (mom_value_to_item(
  momparg_0/*webx*/ ), /*!litoutstr*/MOMOUTDO_LITERAL, "<span class='mom_named_item_cl' data-momitemid='",
   /*!outcstrexp*/MOMOUTDO_LITERAL, (const char*)(/*!primitive mom_string_cstr*/  mom_string_cstr ((/*!primitive mom_item_get_idstr*/  (momval_t) mom_item_get_idstr (
  momparg_1/*itmv*/ .ptr)/*!endprimitive mom_item_get_idstr*/) )/*!endprimitive mom_string_cstr*/) ,
   /*!litoutstr*/MOMOUTDO_LITERAL, "'>",  /*!outcstrexp*/MOMOUTDO_LITERAL, (const char*)(/*!primitive mom_string_cstr*/  mom_string_cstr (
  (mompval_2/*:namv*/))/*!endprimitive mom_string_cstr*/) ,
   /*!litoutstr*/MOMOUTDO_LITERAL, "</span>", /*!outputend*/NULL
  , NULL)/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
 }; //! end procedure block _7rxfx0rruqm_kdtk9pk7wtj
 
 
 
 /******** block#6: _9xhd87mskav_rqcu5eqk5te ********/
  mompblo_6: { //! start procedure block _9xhd87mskav_rqcu5eqk5te
  ////!put itmv into hset in show_html_form_item..` (block)
  
  
  //! instr#1 in block _9xhd87mskav_rqcu5eqk5te ::
  /*!do*/ (void) (/*!primitive mom_item_hset_add*/  (intptr_t) mom_item_hset_add (
  (mompval_1/*:hitmv*/).pitem,  momparg_1/*itmv*/ )/*!endprimitive mom_item_hset_add*/)  /*!done*/;
  
  //! instr#2 in block _9xhd87mskav_rqcu5eqk5te ::
  
  /*!jump _7rxfx0rruqm_kdtk9pk7wtj*/
   
  goto mompblo_4 /*!proc.block _7rxfx0rruqm_kdtk9pk7wtj*/;
 }; //! end procedure block _9xhd87mskav_rqcu5eqk5te
 
 /*!defreturn*/return ;
} // end proc.code show_html_for_item_proc


static const char* const mompconstid__0z0rsvwfkcj_dcpkx68i074[3] = { //! for show_html_for_item_proc
 
 "_41u1utcxyek_22cftxt3xxm", //=comment
 "_7fafkrcdjpd_dy0zpqsshr6", //=hset
 NULL }; // end constid proc show_html_for_item_proc

const struct momprocrout_st momprocdescr__0z0rsvwfkcj_dcpkx68i074// proc.descriptor show_html_for_item_proc
 = { .prout_magic = MOM_PROCROUT_MAGIC,
 .prout_resty = momtypenc__none,.prout_len = 2,
 .prout_id = "_0z0rsvwfkcj_dcpkx68i074",
 .prout_module = "_65961crktpj_vtt30qeqv21",
 .prout_constantids = mompconstid__0z0rsvwfkcj_dcpkx68i074,
 .prout_constantitems = mompconstitems__0z0rsvwfkcj_dcpkx68i074,
 .prout_addr = (void*)momprocfun__0z0rsvwfkcj_dcpkx68i074,
 .prout_argsig = "vv",
 .prout_timestamp= __DATE__ "@" __TIME__
}; // end proc descriptor




// implementation of procedure #3 = _7x6as13park_w64mrkx2xtm
void momprocfun__7x6as13park_w64mrkx2xtm (momcstr_t momparg_0/*!formal:cstr*/)
{
 /// 0 integer locals
 /// 0 double locals
 /// 0 values locals
 static momitem_t* momprocitem; //! prologue of proc. _7x6as13park_w64mrkx2xtm
 if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_7x6as13park_w64mrkx2xtm");
 /// starting procedure _7x6as13park_w64mrkx2xtm
 goto mompblo_1; // start at _39hpqv0jqj6_9sa2v0vhfm6
 /*!! 1 blocks in proc. _7x6as13park_w64mrkx2xtm */
 
 
 /******** block#1: _39hpqv0jqj6_9sa2v0vhfm6 ********/
  mompblo_1: { //! start procedure block _39hpqv0jqj6_9sa2v0vhfm6
  ////!start block of todo on dump
  
  
  // locked-item dump_data in block _39hpqv0jqj6_9sa2v0vhfm6
  momitem_t* momlockeditem_6 = mom_value_to_item ( ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[3] /*dump_data*/)) /* locked-item */;
  if (!mom_lock_item (momlockeditem_6)) goto momendblock_6;
  
  //! instr#1 in block _39hpqv0jqj6_9sa2v0vhfm6 ::
  /*!do*/ /*!primitive-void _87r5zd69i6m_zr0hupaer90*/ /* primitive _87r5zd... in start-block of todo-on-dump */
 struct mom_dumpoutcome_st doutc;
 memset(&doutc, 0, sizeof(doutc)); mom_full_dump("todo-dump-with-outcome", 
  momparg_0/*cstr*/ , &doutc);
 MOM_INFORMPRINTF(" dumped with outcome %d items into %s", doutc.odmp_nbdumpeditems, 
  momparg_0/*cstr*/ );
 mom_item_put_attribute (/*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[6] /*elapsed_time*/).pitem, mom_let_transient(mom_make_double(doutc.odmp_elapsedtime)));
 mom_item_put_attribute (/*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[2] /*cpu_time*/).pitem, mom_let_transient(mom_make_double(doutc.odmp_cputime)));
 mom_item_put_attribute (/*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[1] /*nb_dumped_items*/).pitem, mom_let_transient(mom_make_integer(doutc.odmp_nbdumpeditems)));
 mom_item_put_attribute (/*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[8] /*notice*/).pitem, mom_let_transient(doutc.odmp_nodenotice));
 mom_item_put_attribute (/*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[0] /*predefined*/).pitem, mom_let_transient(doutc.odmp_setpredef));
 mom_item_put_attribute (/*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
  ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[9] /*module*/).pitem, mom_let_transient(doutc.odmp_nodemodules));
 mom_continue_working();
/*!endvoidprimitive _87r5zd69i6m_zr0hupaer90*/  /*!done*/;
  
  /*! epilogue for lock */
  mom_unlock_item (momlockeditem_6); // unlock dump_data
  momendblock_6:;
  
 }; //! end procedure block _39hpqv0jqj6_9sa2v0vhfm6
 
 /*!defreturn*/return ;
} // end proc.code _7x6as13park_w64mrkx2xtm


static const char* const mompconstid__7x6as13park_w64mrkx2xtm[11] = { //! for _7x6as13park_w64mrkx2xtm
 
 "_133zjf1f9zp_jq8kti38sd7", //=predefined
 "_1tzf3q2dix5_jqxphp9ivcw", //=nb_dumped_items
 "_50623j9vemk_1hp2q2czrhi", //=cpu_time
 "_5tihf27p4rj_t80tzx4fxrf", //=dump_data
 "_5xw5qm751tv_jvm099ita0w", //=named_items
 "_6rs26jmh9ya_jv0aiqf4kvx", //=dump_state
 "_6u6cp2a2tsz_ses4qchc3y3", //=elapsed_time
 "_6vrzjdj7ij8_dupds6c9895", //=names
 "_7diyc1cwj8z_x630afccr8e", //=notice
 "_7sqk8vh89xr_6tj8dq7vqju", //=module
 NULL }; // end constid proc _7x6as13park_w64mrkx2xtm

const struct momprocrout_st momprocdescr__7x6as13park_w64mrkx2xtm// proc.descriptor _7x6as13park_w64mrkx2xtm
 = { .prout_magic = MOM_PROCROUT_MAGIC,
 .prout_resty = momtypenc__none,.prout_len = 10,
 .prout_id = "_7x6as13park_w64mrkx2xtm",
 .prout_module = "_65961crktpj_vtt30qeqv21",
 .prout_constantids = mompconstid__7x6as13park_w64mrkx2xtm,
 .prout_constantitems = mompconstitems__7x6as13park_w64mrkx2xtm,
 .prout_addr = (void*)momprocfun__7x6as13park_w64mrkx2xtm,
 .prout_argsig = "s",
 .prout_timestamp= __DATE__ "@" __TIME__
}; // end proc descriptor

// define module routines descriptor array for first_module
static const union momrout_un momdroutarr__65961crktpj_vtt30qeqv21[5] = {
 [0]= {.rtfun= &momrout__06uk4pppvx9_huv0v11v18j}, // taskletfun ajax_appl
 [1]= {.rproc= &momprocdescr__07zti91e4kd_952zqsd03fz}, // procedure _07zti91e4kd_952zqsd03fz
 [2]= {.rproc= &momprocdescr__0z0rsvwfkcj_dcpkx68i074}, // procedure show_html_for_item_proc
 [3]= {.rproc= &momprocdescr__7x6as13park_w64mrkx2xtm}, // procedure _7x6as13park_w64mrkx2xtm
 
}; // end of module routines descriptor array for first_module


// module initialization for first_module
void mominitmodule__65961crktpj_vtt30qeqv21 (void) {
 mom_module_internal_initialize ("_65961crktpj_vtt30qeqv21" /*!module first_module*/,
       MONIMELT_MD5_MODULE /*see Makefile*/,  4,  momdroutarr__65961crktpj_vtt30qeqv21);
 MOM_INFORMPRINTF("module first_module of md5 " MONIMELT_MD5_MODULE " initialized.");
} // end of module initialization




//// **** license info of momg__65961crktpj_vtt30qeqv21.c
const char mom_module_GPL_compatible[]=
   "GPLv3+, generated module first_module; commit " MONIMELT_LAST_COMMITID;


///// for Emacs :::
////=== Local Variables:
////=== mode: C
////=== End:


//// **** eof momg__65961crktpj_vtt30qeqv21.c
