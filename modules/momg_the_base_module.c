/// *** generated file momg_the_base_module.c - DO NOT EDIT ///
/// Copyright (C) 2015 Free Software Foundation, Inc. ///
/// MONIMELT is a monitor for MELT - see http://gcc-melt.org/ ///
/// This generated file momg_the_base_module.c is part of MONIMELT, part of GCC ///
//////
/// GCC is free software; you can redistribute it and/or modify ///
/// it under the terms of the GNU General Public License as published by ///
/// the Free Software Foundation; either version 3, or (at your option) ///
/// any later version. ///
//////
///  GCC is distributed in the hope that it will be useful, ///
///  but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ///
///  GNU General Public License for more details. ///
///  You should have received a copy of the GNU General Public License ///
///  along with GCC; see the file COPYING3.   If not see ///
///  <http://www.gnu.org/licenses/>. ///
//////


#include "monimelt.h"



/***** declaring 1 functions *****/


/// declare function #0: agenda_step
extern bool momfunc_void_to_void_agenda_step (const momnode_t *);


/***** implementing 1 functions *****/


/// implement function #0: agenda_step
bool momfunc_void_to_void_agenda_step (const momnode_t *mom_node)
{ // body of function agenda_step
  bool momsuccess_agenda_step = false;
  momitem_t* mom_funcitm = NULL;
  if (MOM_UNLIKELY(!mom_node
      || !(mom_funcitm = mom_node_conn (mom_node))
      || mom_unsync_item_components_count (mom_funcitm)<2
       ))
  return false;
  // 0 output results:
  // 1 variables:
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar0 = (momlockeditem_t*)0;
  // 2 constants:
  // constant _07BHLcwhp_48ka0t9bq
  const momvalue_t momconst_0 = 
    mom_raw_item_get_indexed_component (mom_funcitm, 0);
  // constant the_agenda
  momitem_t* momconst_1 = MOM_PREDEFINED_NAMED(the_agenda);
  // 0 closed:
  goto momblocklab__7MF947fC8_8ChefReMD;
 // block #0 : _7MF947fC8_8ChefReMD
 momblocklab__7MF947fC8_8ChefReMD: {
// 1 statements in block _7MF947fC8_8ChefReMD
// statement #0 _1c462DJmx_68zwnz1Ua
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__1c462DJmx_68zwnz1Ua = momvar0 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__1c462DJmx_68zwnz1Ua =  /*constant#1:*/MOM_PREDEFINED_NAMED(the_agenda);
  if (momoldlocked__1c462DJmx_68zwnz1Ua != momnewlocked__1c462DJmx_68zwnz1Ua) {
    if (momoldlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_unlock (momoldlocked__1c462DJmx_68zwnz1Ua);
    if (momnewlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_lock (momnewlocked__1c462DJmx_68zwnz1Ua);
  } // end lock test _1c462DJmx_68zwnz1Ua
    momvar0 /*var:lkitm_agenda*/ = momnewlocked__1c462DJmx_68zwnz1Ua;
  momoldlocked__1c462DJmx_68zwnz1Ua = NULL;
  momnewlocked__1c462DJmx_68zwnz1Ua = NULL;
  } // end locked set into lkitm_agenda

  }; // end block _7MF947fC8_8ChefReMD
//////
// epilogue of agenda_step
    momsuccess_agenda_step = true;
    goto momepilog_agenda_step;
 momepilog_agenda_step:
// give 0 outputs
  return momsuccess_agenda_step;
} // end of momfunc_void_to_void_agenda_step 



/***** end 1 functions *****/


//// end of generated module file momg_the_base_module.c

