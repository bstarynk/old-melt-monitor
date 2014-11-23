// generated MONIMELT module test_module
// file momg__4q1v3ax0ffi_hv7fwpsv6uf.c
// DO NOT EDIT


///// declarations
// generated monimelt module file momg__4q1v3ax0ffi_hv7fwpsv6uf.c ** DO NOT EDIT **

////////////////////////////////////////////////////////////////++

/// generated file momg__4q1v3ax0ffi_hv7fwpsv6uf.c ** DO NOT EDIT

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

// declare procedure test_proc1 rank#0
void momprocfun__682ush7cppa_s7vzfd2rsxp ();

// declare tasklet function test_fun1 rank#1
static int momfuncod__8qmqy249w63_fxdr6rdz48m(int, momitem_t*, momval_t, momval_t*, intptr_t*, double*);

static const momitem_t* mompconstitems__682ush7cppa_s7vzfd2rsxp[2]; // define constant items of procedure test_proc1

static momitem_t* momfconstitems__8qmqy249w63_fxdr6rdz48m[3]; // constant items of tasklet function test_fun1

// declare module md5sum for test_module
const char mommd5mod__4q1v3ax0ffi_hv7fwpsv6uf[] = MONIMELT_MD5_MODULE; // Makefile generated

// declare module routines descriptor array for test_module
static const union momrout_un momdroutarr__4q1v3ax0ffi_hv7fwpsv6uf[3];



//// implementations


////++++ implementation of 2 routines:



// implementation of procedure #0 = test_proc1
void momprocfun__682ush7cppa_s7vzfd2rsxp ()
{
 static momitem_t* momprocitem;
 if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id("_682ush7cppa_s7vzfd2rsxp");
 /// starting:
 goto mompblo_1; // start at _5xa08a3ittw_imt86y9q33c
 
  mompblo_1:
 { // procedure block _5xa08a3ittw_imt86y9q33c
  
 }; // end procedure block _5xa08a3ittw_imt86y9q33c
 return;
 
 } // end of procedure momprocfun_test_proc1
 
 
 static const char* const mompconstid__682ush7cppa_s7vzfd2rsxp[2] = {
  [0] = "_0yyp8vmw4si_wf49m4d4zwq", // code
 }; // end of procedure constant item ids of test_proc1
 
 
 const struct momprocrout_st momprocdescr__682ush7cppa_s7vzfd2rsxp = { .prout_magic = MOM_PROCROUT_MAGIC,
  .prout_resty = momtypenc__none,
  .prout_len = 1,
  .prout_id = "_682ush7cppa_s7vzfd2rsxp",
  .prout_module = "_4q1v3ax0ffi_hv7fwpsv6uf",
  .prout_constantids = mompconstid__682ush7cppa_s7vzfd2rsxp,
  .prout_constantitems = mompconstitems__682ush7cppa_s7vzfd2rsxp,
  .prout_addr = (void*)momprocfun__682ush7cppa_s7vzfd2rsxp,
  .prout_argsig = "",
  .prout_timestamp= __DATE__ "@" __TIME__
 }; // end proc descriptor
 
 
 
 // implement tasklet function test_fun1 rank#1
 static int momfuncod__8qmqy249w63_fxdr6rdz48m
 (int momstate,
 	 momitem_t* restrict momtasklet, const momval_t momclosure,
 	 momval_t* restrict momvals_ MOM_UNUSED,
 	 intptr_t* restrict momnums_ MOM_UNUSED,
 	 double* restrict momdbls_ MOM_UNUSED)
 { // start of tasklet function test_fun1
  // declared 0 locals, 0 arguments.
  if (MOM_UNLIKELY(momstate==0)) return 1;
  assert (mom_is_item (momclosure));
  momval_t* momclovals = mom_item_closure_values (momclosure.pitem);
  assert (momclovals != NULL);
  assert (mom_item_payload_kind(momtasklet)== mompayk_tasklet);
  switch (momstate) {
  case 1: goto momfblo_1;
  default: return momroutres_pop;
  }; // end switch state
  
  // function block #1 _2ky10qvckv2_kqa0pr8z29z
  momfblo_1:
  {
   
  }; // end function block _2ky10qvckv2_kqa0pr8z29z
  return momroutres_pop;
  } // end function test_fun1

static const char* const momfconstid__8qmqy249w63_fxdr6rdz48m[3] = { // constant ids of function test_fun1
 [0] = "_41u1utcxyek_22cftxt3xxm", // comment
 [1] = "_7wk9y7e7r0z_575esi8ys5x", // val
}; // end of function constants of test_fun1

const struct momtfundescr_st momrout__8qmqy249w63_fxdr6rdz48m = { // tasklet function descriptor test_fun1
 .tfun_magic = MOM_TFUN_MAGIC,
 .tfun_minclosize = 0,
 .tfun_nbconstants = 2,
 .tfun_frame_nbval = 0,
 .tfun_frame_nbnum = 0,
 .tfun_frame_nbdbl = 0,
 .tfun_constantids = momfconstid__8qmqy249w63_fxdr6rdz48m,
 .tfun_constantitems = (const momitem_t*const*) momfconstitems__8qmqy249w63_fxdr6rdz48m,
 .tfun_ident = "_8qmqy249w63_fxdr6rdz48m",
 .tfun_module = MONIMELT_CURRENT_MODULE,
 .tfun_codefun = momfuncod__8qmqy249w63_fxdr6rdz48m,
 .tfun_timestamp = __DATE__ "@" __TIME__
 
}; // end function descriptor

// define module routines descriptor array for test_module
static const union momrout_un momdroutarr__4q1v3ax0ffi_hv7fwpsv6uf[3] = {
 [0]= {.rproc= &momprocdescr__682ush7cppa_s7vzfd2rsxp}, // procedure test_proc1
 [1]= {.rtfun= &momrout__8qmqy249w63_fxdr6rdz48m}, // taskletfun test_fun1
 
}; // end of module routines descriptor array for test_module


// module initialization for test_module
void mominitmodule__4q1v3ax0ffi_hv7fwpsv6uf (void) {
 mom_module_internal_initialize ("_4q1v3ax0ffi_hv7fwpsv6uf" /*!module test_module*/,
       MONIMELT_MD5_MODULE /*see Makefile*/,  2,  momdroutarr__4q1v3ax0ffi_hv7fwpsv6uf);
 MOM_INFORMPRINTF("module test_module of md5 " MONIMELT_MD5_MODULE " initialized.");
} // end of module initialization



 // module license
const char mom_module_GPL_compatible[]=
	"GPLv3+, generated module _4q1v3ax0ffi_hv7fwpsv6uf";



//// end of generated file momg__4q1v3ax0ffi_hv7fwpsv6uf.c
