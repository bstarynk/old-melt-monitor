// generated MONIMELT module first_module
// file momg__65961crktpj_vtt30qeqv21.c
// DO NOT EDIT


///// declarations
// generated monimelt module file momg__65961crktpj_vtt30qeqv21.c ** DO NOT EDIT **

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

static const momitem_t* mompconstitems__07zti91e4kd_952zqsd03fz[1]; // define constant items of procedure _07zti91e4kd_952zqsd03fz

static const momitem_t* mompconstitems__0z0rsvwfkcj_dcpkx68i074[1]; // define constant items of procedure show_html_for_item_proc

static const momitem_t* mompconstitems__7x6as13park_w64mrkx2xtm[11]; // define constant items of procedure _7x6as13park_w64mrkx2xtm

// declare module md5sum for first_module
const char mommd5mod__65961crktpj_vtt30qeqv21[] = MONIMELT_MD5_MODULE; // Makefile generated

// declare module routines descriptor array for first_module
static const union momrout_un momdroutarr__65961crktpj_vtt30qeqv21[5];



//// implementations


////++++ implementation of 4 routines:


// implement tasklet function ajax_appl rank#0
static int momfuncod__06uk4pppvx9_huv0v11v18j
(int momstate,
	 momitem_t* restrict momtasklet, const momval_t momclosure,
	 momval_t* restrict momvals,
	 intptr_t* restrict momnums_ MOM_UNUSED,
	 double* restrict momdbls_ MOM_UNUSED)
{ // start of tasklet function ajax_appl
 // declared 1 locals, 1 arguments.
 if (MOM_UNLIKELY(momstate==0)) return 3;
 assert (mom_is_item (momclosure));
 momval_t* momclovals = mom_item_closure_values (momclosure.pitem);
 assert (momclovals != NULL);
 assert (mom_item_payload_kind(momtasklet)== mompayk_tasklet);
 assert (momvals != NULL); // 2 values.
  MOM_DEBUG(run, MOMOUT_LITERAL("start tasklet="), MOMOUT_ITEM((const momitem_t*)momtasklet),
 	 MOMOUT_LITERAL(" state#"), MOMOUT_DEC_INT((int)momstate), MOMOUT_LITERAL(" taskfunc ajax_appl"));
 switch (momstate) {
 case 1: goto momfblo_1; // block _7yyaydvyhpr_teuchcqzs7k
 case 2: goto momfblo_2; // block _8t137w1z1s9_2tea9xp64s6
 case 3: goto momfblo_3; // block _8y756mef2ca_w8cj58726vj
 case 4: goto momfblo_4; // block _91471ta1047_pra9zfqc2y1
 case 5: goto momfblo_5; // block _9u6a6xy2e1p_qeapfc73cm4
 default: return momroutres_pop;
 }; // end switch state
 
 // function block #1 _7yyaydvyhpr_teuchcqzs7k
 momfblo_1:
 {
  ////!block to handle ajax_appl when whatv=dump
  
  
  //! instr#1 in block _7yyaydvyhpr_teuchcqzs7k ::
  /*!do*/ /*!primitive-void mom_stop_work_with_todo*/ mom_stop_work_with_todo (
  momprocfun__7x6as13park_w64mrkx2xtm, /*!litstr:*/ ".")/*!endvoidprimitive mom_stop_work_with_todo*/  /*!done*/;
  
  //! instr#2 in block _7yyaydvyhpr_teuchcqzs7k ::
  
  /*!jump */
    return 4 /*!func.block _91471ta1047_pra9zfqc2y1*/;
 }; // end function block _7yyaydvyhpr_teuchcqzs7k
 return momroutres_pop;
 
 // function block #2 _8t137w1z1s9_2tea9xp64s6
 momfblo_2:
 {
  ////!block to handle appl with what=exit
  
  
  // locked-item webx in block _8t137w1z1s9_2tea9xp64s6
  momitem_t* momlockeditem_1 = mom_value_to_item ( momvals[0/*:webx*/]) /* locked-item */;
  if (!mom_lock_item (momlockeditem_1)) goto momendblock_1;
  
  //! instr#1 in block _8t137w1z1s9_2tea9xp64s6 ::
  /*!do*/ /*!primitive-void mom_stop_work_with_todo*/ mom_stop_work_with_todo (
  momprocfun__07zti91e4kd_952zqsd03fz, /*!litstr:*/ ".")/*!endvoidprimitive mom_stop_work_with_todo*/  /*!done*/;
  
  //! instr#2 in block _8t137w1z1s9_2tea9xp64s6 ::
  /*!do*/ /*!primitive-void MOM_WEBX_OUT*/ MOM_WEBX_OUT (mom_value_to_item(
  momvals[0/*:webx*/]), /*!litoutstr*/MOMOUTDO_LITERAL, "<em>Monimelt</em> <b>save then exit</b> at <i>",
  /*!outexp MOMOUT_DOUBLE_TIME*/ MOMOUTDO_DOUBLE_TIME,
  /*!litstr:*/ "%c", (/*!primitive mom_clock_time*/ mom_clock_time(
  CLOCK_REALTIME)/*!endprimitive mom_clock_time*/) /*!endoutexp MOMOUT_DOUBLE_TIME*/,
   /*!litoutstr*/MOMOUTDO_LITERAL, "</i>", /*!outputend*/NULL
  )/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
  //! instr#3 in block _8t137w1z1s9_2tea9xp64s6 ::
  /*!do*/ /*!primitive-void mom_webx_reply*/ mom_webx_reply (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "text/html",
  HTTP_OK)/*!endvoidprimitive mom_webx_reply*/  /*!done*/;
  
  /*! epilogue for lock */
  mom_unlock_item(momlockeditem_1); // unlock webx
  momendblock_1:;
  
 }; // end function block _8t137w1z1s9_2tea9xp64s6
 return momroutres_pop;
 
 // function block #3 _8y756mef2ca_w8cj58726vj
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
  /*!assign*/  momvals[2/*:whatv*/] = (/*!primitive mom_webx_post_arg*/ mom_webx_post_arg (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "what_mom")/*!endprimitive mom_webx_post_arg*/) ;
  
  //! instr#3 in block _8y756mef2ca_w8cj58726vj ::
  /*!do*/ /*!primitive-void debug_run*/ MOM_DEBUG(run,  /*!litoutstr*/MOMOUTDO_LITERAL, "ajax_appl whatv:",
   /*!outvalvar*/MOMOUTDO_VALUE, (momval_t) momvals[2/*:whatv*/],/*!outputend*/NULL
  )/*!endvoidprimitive debug_run*/  /*!done*/;
  
  //! instr#4 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[2/*:whatv*/]), ( /*!litstr:*/ "exit"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_2);  return 2 /*!func.block _8t137w1z1s9_2tea9xp64s6*/;
   }; //!unlocked momlockeditem_2
    
  //! instr#5 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[2/*:whatv*/]), ( /*!litstr:*/ "quit"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_2);  return 5 /*!func.block _9u6a6xy2e1p_qeapfc73cm4*/;
   }; //!unlocked momlockeditem_2
    
  //! instr#6 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[2/*:whatv*/]), ( /*!litstr:*/ "dump"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_2);  return 1 /*!func.block _7yyaydvyhpr_teuchcqzs7k*/;
   }; //!unlocked momlockeditem_2
    
  /*! epilogue for lock */
  mom_unlock_item(momlockeditem_2); // unlock webx
  momendblock_2:;
  
 }; // end function block _8y756mef2ca_w8cj58726vj
 return momroutres_pop;
 
 // function block #4 _91471ta1047_pra9zfqc2y1
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
  )/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
  //! instr#2 in block _91471ta1047_pra9zfqc2y1 ::
  /*!do*/ /*!primitive-void mom_webx_reply*/ mom_webx_reply (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "text/html",
  HTTP_OK)/*!endvoidprimitive mom_webx_reply*/  /*!done*/;
  
 }; // end function block _91471ta1047_pra9zfqc2y1
 return momroutres_pop;
 
 // function block #5 _9u6a6xy2e1p_qeapfc73cm4
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
  )/*!endvoidprimitive MOM_WEBX_OUT*/  /*!done*/;
  
  //! instr#2 in block _9u6a6xy2e1p_qeapfc73cm4 ::
  /*!do*/ /*!primitive-void mom_webx_reply*/ mom_webx_reply (mom_value_to_item (
  momvals[0/*:webx*/]),  /*!litstr:*/ "text/html",
  HTTP_OK)/*!endvoidprimitive mom_webx_reply*/  /*!done*/;
  
  //! instr#3 in block _9u6a6xy2e1p_qeapfc73cm4 ::
  /** chunk **/
   MOM_INFORMPRINTF("quitting Monimelt per web request");
 exit(EXIT_SUCCESS); // per web request

  /*! epilogue for lock */
  mom_unlock_item(momlockeditem_3); // unlock webx
  momendblock_3:;
  
 }; // end function block _9u6a6xy2e1p_qeapfc73cm4
 return momroutres_pop;
 } // end function ajax_appl

static const char* const momfconstid__06uk4pppvx9_huv0v11v18j[1] = { // constant ids of function ajax_appl
}; // end of function constants of ajax_appl

const struct momtfundescr_st momrout__06uk4pppvx9_huv0v11v18j = { // tasklet function descriptor ajax_appl
 .tfun_magic = MOM_TFUN_MAGIC,
 .tfun_minclosize = 0,
 .tfun_nbconstants = 0,
 .tfun_frame_nbval = 2,
 .tfun_frame_nbnum = 0,
 .tfun_frame_nbdbl = 0,
 .tfun_constantids = momfconstid__06uk4pppvx9_huv0v11v18j,
 .tfun_constantitems = (const momitem_t*const*) momfconstitems__06uk4pppvx9_huv0v11v18j,
 .tfun_ident = "_06uk4pppvx9_huv0v11v18j",
 .tfun_module = MONIMELT_CURRENT_MODULE,
 .tfun_codefun = momfuncod__06uk4pppvx9_huv0v11v18j,
 .tfun_timestamp = __DATE__ "@" __TIME__
 
}; // end function descriptor




// implementation of procedure #1 = _07zti91e4kd_952zqsd03fz
void momprocfun__07zti91e4kd_952zqsd03fz (momcstr_t momparg_0 ////!_389t77v85ej_fwpy6exy62x
)
{
 static momitem_t* momprocitem;
 if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_07zti91e4kd_952zqsd03fz");
 /// starting:
 goto mompblo_1; // start at _1kj1j3878fe_duw3ts10hev
 
  mompblo_1:
 { // procedure block _1kj1j3878fe_duw3ts10hev
  ////!start block of todo proc on exit
  
  
  //! instr#1 in block _1kj1j3878fe_duw3ts10hev ::
  /*!do*/ /*!primitive-void _1iyd2es3u59_x6uq7vhecjj*/ mom_full_dump("todo dump-at-exit", 
  momparg_0, NULL); mom_stop_event_loop();/*!endvoidprimitive _1iyd2es3u59_x6uq7vhecjj*/  /*!done*/;
  
 }; // end procedure block _1kj1j3878fe_duw3ts10hev
 return;
 
 } // end of procedure momprocfun__07zti91e4kd_952zqsd03fz
 
 
 static const char* const mompconstid__07zti91e4kd_952zqsd03fz[1] = {
 }; // end of procedure constant item ids of _07zti91e4kd_952zqsd03fz
 
 
 const struct momprocrout_st momprocdescr__07zti91e4kd_952zqsd03fz = { .prout_magic = MOM_PROCROUT_MAGIC,
  .prout_resty = momtypenc__none,
  .prout_len = 0,
  .prout_id = "_07zti91e4kd_952zqsd03fz",
  .prout_module = "_65961crktpj_vtt30qeqv21",
  .prout_constantids = mompconstid__07zti91e4kd_952zqsd03fz,
  .prout_constantitems = mompconstitems__07zti91e4kd_952zqsd03fz,
  .prout_addr = (void*)momprocfun__07zti91e4kd_952zqsd03fz,
  .prout_argsig = "s",
  .prout_timestamp= __DATE__ "@" __TIME__
 }; // end proc descriptor
 
 
 
 
 // implementation of procedure #2 = show_html_for_item_proc
 void momprocfun__0z0rsvwfkcj_dcpkx68i074 (momval_t momparg_0 ////!_16cd0fvmdrh_r77ajpy26za
 , momval_t momparg_1 ////!_1a2aavj5vir_2hz681zdfqd
 )
 {
  momval_t mompval_0 = MOM_NULLV; ////!webses
  
  static momitem_t* momprocitem;
  if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_0z0rsvwfkcj_dcpkx68i074");
  /// starting:
  goto mompblo_1; // start at _02yd241wh4z_tca7i6iamf3
  
   mompblo_1:
  { // procedure block _02yd241wh4z_tca7i6iamf3
   ////!start block of show_html_for_item_proc
   
   
   // locked-item webx in block _02yd241wh4z_tca7i6iamf3
   momitem_t* momlockeditem_4 = mom_value_to_item ( momparg_0) /* locked-item */;
   if (!mom_lock_item (momlockeditem_4)) goto momendblock_4;
   
   //! instr#1 in block _02yd241wh4z_tca7i6iamf3 ::
   /*!assign*/  (mompval_0/*:webses*/) = (/*!primitive mom_webx_session*/  (momval_t) mom_webx_session (
   momparg_0.pitem) /*!endprimitive mom_webx_session*/) ;
   
   /*! epilogue for lock */
   mom_unlock_item(momlockeditem_4); // unlock webx
   momendblock_4:;
   
  }; // end procedure block _02yd241wh4z_tca7i6iamf3
  return;
  
  } // end of procedure momprocfun_show_html_for_item_proc
  
  
  static const char* const mompconstid__0z0rsvwfkcj_dcpkx68i074[1] = {
  }; // end of procedure constant item ids of show_html_for_item_proc
  
  
  const struct momprocrout_st momprocdescr__0z0rsvwfkcj_dcpkx68i074 = { .prout_magic = MOM_PROCROUT_MAGIC,
   .prout_resty = momtypenc__none,
   .prout_len = 0,
   .prout_id = "_0z0rsvwfkcj_dcpkx68i074",
   .prout_module = "_65961crktpj_vtt30qeqv21",
   .prout_constantids = mompconstid__0z0rsvwfkcj_dcpkx68i074,
   .prout_constantitems = mompconstitems__0z0rsvwfkcj_dcpkx68i074,
   .prout_addr = (void*)momprocfun__0z0rsvwfkcj_dcpkx68i074,
   .prout_argsig = "vv",
   .prout_timestamp= __DATE__ "@" __TIME__
  }; // end proc descriptor
  
  
  
  
  // implementation of procedure #3 = _7x6as13park_w64mrkx2xtm
  void momprocfun__7x6as13park_w64mrkx2xtm (momcstr_t momparg_0 ////!_389t77v85ej_fwpy6exy62x
  )
  {
   static momitem_t* momprocitem;
   if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_7x6as13park_w64mrkx2xtm");
   /// starting:
   goto mompblo_1; // start at _39hpqv0jqj6_9sa2v0vhfm6
   
    mompblo_1:
   { // procedure block _39hpqv0jqj6_9sa2v0vhfm6
    ////!start block of todo on dump
    
    
    // locked-item dump_data in block _39hpqv0jqj6_9sa2v0vhfm6
    momitem_t* momlockeditem_5 = mom_value_to_item ( ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[3] /*dump_data*/)) /* locked-item */;
    if (!mom_lock_item (momlockeditem_5)) goto momendblock_5;
    
    //! instr#1 in block _39hpqv0jqj6_9sa2v0vhfm6 ::
    /*!do*/ /*!primitive-void _87r5zd69i6m_zr0hupaer90*/ /* primitive _87r5zd... in start-block of todo-on-dump */
 struct mom_dumpoutcome_st doutc;
 memset(&doutc, 0, sizeof(doutc)); mom_full_dump("todo-dump-with-outcome", 
    momparg_0, &doutc);
 MOM_INFORMPRINTF(" dumped with outcome %d items into %s", doutc.odmp_nbdumpeditems, 
    momparg_0);
 mom_item_put_attribute (/*!outsidechunk*/ ((momval_t) mompconstitems__7x6as13park_w64mrkx2xtm[5] /*dump_state*/).pitem, /*!outsidechunk*/
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
    mom_unlock_item(momlockeditem_5); // unlock dump_data
    momendblock_5:;
    
   }; // end procedure block _39hpqv0jqj6_9sa2v0vhfm6
   return;
   
   } // end of procedure momprocfun__7x6as13park_w64mrkx2xtm
   
   
   static const char* const mompconstid__7x6as13park_w64mrkx2xtm[11] = {
    [0] = "_133zjf1f9zp_jq8kti38sd7", // predefined
    [1] = "_1tzf3q2dix5_jqxphp9ivcw", // nb_dumped_items
    [2] = "_50623j9vemk_1hp2q2czrhi", // cpu_time
    [3] = "_5tihf27p4rj_t80tzx4fxrf", // dump_data
    [4] = "_5xw5qm751tv_jvm099ita0w", // named_items
    [5] = "_6rs26jmh9ya_jv0aiqf4kvx", // dump_state
    [6] = "_6u6cp2a2tsz_ses4qchc3y3", // elapsed_time
    [7] = "_6vrzjdj7ij8_dupds6c9895", // names
    [8] = "_7diyc1cwj8z_x630afccr8e", // notice
    [9] = "_7sqk8vh89xr_6tj8dq7vqju", // module
   }; // end of procedure constant item ids of _7x6as13park_w64mrkx2xtm
   
   
   const struct momprocrout_st momprocdescr__7x6as13park_w64mrkx2xtm = { .prout_magic = MOM_PROCROUT_MAGIC,
    .prout_resty = momtypenc__none,
    .prout_len = 10,
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
   
   

 // module license
const char mom_module_GPL_compatible[]=
	"GPLv3+, generated module first_module; commit " MONIMELT_LAST_COMMITID;



//// end of generated file momg__65961crktpj_vtt30qeqv21.c
