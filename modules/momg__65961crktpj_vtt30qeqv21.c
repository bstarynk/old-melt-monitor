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

////++++ declaration of 2 routines:

////!AJAX for appl menu

// declare tasklet function ajax_appl rank#0
static int momfuncod__06uk4pppvx9_huv0v11v18j(int, momitem_t*, momval_t, momval_t*, intptr_t*, double*);

////!todo procedure on exit

// declare procedure _07zti91e4kd_952zqsd03fz rank#1
void momprocfun__07zti91e4kd_952zqsd03fz (momcstr_t);

static momitem_t* momfconstitems__06uk4pppvx9_huv0v11v18j[1]; // constant items of tasklet function ajax_appl

static const momitem_t* mompconstitems__07zti91e4kd_952zqsd03fz[1]; // define constant items of procedure _07zti91e4kd_952zqsd03fz

// declare module md5sum for first_module
const char mommd5mod__65961crktpj_vtt30qeqv21[] = MONIMELT_MD5_MODULE; // Makefile generated

// declare module routines descriptor array for first_module
static const union momrout_un momdroutarr__65961crktpj_vtt30qeqv21[3];



//// implementations


////++++ implementation of 2 routines:


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
 switch (momstate) {
 case 1: goto momfblo_1;
 case 2: goto momfblo_2;
 case 3: goto momfblo_3;
 case 4: goto momfblo_4;
 default: return momroutres_pop;
 }; // end switch state
 
 // function block #1 _7yyaydvyhpr_teuchcqzs7k
 momfblo_1:
 {
  
 }; // end function block _7yyaydvyhpr_teuchcqzs7k
 return momroutres_pop;
 
 // function block #2 _8t137w1z1s9_2tea9xp64s6
 momfblo_2:
 {
  
 }; // end function block _8t137w1z1s9_2tea9xp64s6
 return momroutres_pop;
 
 // function block #3 _8y756mef2ca_w8cj58726vj
 momfblo_3:
 {
  
  // locked-item webx in block _8y756mef2ca_w8cj58726vj
  momitem_t* momlockeditem_1 = mom_value_to_item ( momvals[0/*:webx*/]) /* locked-item */;
  if (!mom_lock_item (momlockeditem_1)) goto momendblock_1;
  
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
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_1);  return 1 /*!func.block _8t137w1z1s9_2tea9xp64s6*/;
   }; //!unlocked momlockeditem_1
    
  //! instr#5 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[2/*:whatv*/]), ( /*!litstr:*/ "quit"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_1);  return 3 /*!func.block _9u6a6xy2e1p_qeapfc73cm4*/;
   }; //!unlocked momlockeditem_1
    
  //! instr#6 in block _8y756mef2ca_w8cj58726vj ::
  /*!if*/ if ((/*!primitive mom_string_same*/ (intptr_t) mom_string_same((
  momvals[2/*:whatv*/]), ( /*!litstr:*/ "dump"))/*!endprimitive mom_string_same*/) )
   /*!unlock-goto*/ { mom_unlock_item (momlockeditem_1);  return 0 /*!func.block _7yyaydvyhpr_teuchcqzs7k*/;
   }; //!unlocked momlockeditem_1
    
  /*! epilogue for lock */
  momendblock_1:;
  
 }; // end function block _8y756mef2ca_w8cj58726vj
 return momroutres_pop;
 
 // function block #4 _9u6a6xy2e1p_qeapfc73cm4
 momfblo_4:
 {
  
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
 
 // define module routines descriptor array for first_module
 static const union momrout_un momdroutarr__65961crktpj_vtt30qeqv21[3] = {
  [0]= {.rtfun= &momrout__06uk4pppvx9_huv0v11v18j}, // taskletfun ajax_appl
  [1]= {.rproc= &momprocdescr__07zti91e4kd_952zqsd03fz}, // procedure _07zti91e4kd_952zqsd03fz
  
 }; // end of module routines descriptor array for first_module
 
 
 // module initialization for first_module
 void mominitmodule__65961crktpj_vtt30qeqv21 (void) {
  mom_module_internal_initialize ("_65961crktpj_vtt30qeqv21" /*!module first_module*/,
        MONIMELT_MD5_MODULE /*see Makefile*/,  2,  momdroutarr__65961crktpj_vtt30qeqv21);
  MOM_INFORMPRINTF("module first_module of md5 " MONIMELT_MD5_MODULE " initialized.");
 } // end of module initialization
 
 

 // module license
const char mom_module_GPL_compatible[]=
	"GPLv3+, generated module first_module; commit " MONIMELT_LAST_COMMITID;



//// end of generated file momg__65961crktpj_vtt30qeqv21.c
