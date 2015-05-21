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
      || mom_unsync_item_components_count (mom_funcitm)<3
       ))
  return false;
  // 0 output results:
  // 2 variables:
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar1 = (momlockeditem_t*)0;
// variable varclo of type value
  momvalue_t momvar0 = MOM_NONEV;
  // 3 constants:
  // constant _07BHLcwhp_48ka0t9bq
  const momvalue_t momconst_0 = 
    mom_raw_item_get_indexed_component (mom_funcitm, 0);
  // constant fill_agenda
  momitem_t* momconst_1 =
    mom_value_to_item(mom_raw_item_get_indexed_component (mom_funcitm, 1));
  // constant the_agenda
  momitem_t* momconst_2 = MOM_PREDEFINED_NAMED(the_agenda);
  // 0 closed:
  goto momblocklab__7MF947fC8_8ChefReMD;
 // block #0 : _0HvrqAHte_4ryaA4drs
 momblocklab__0HvrqAHte_4ryaA4drs: {
// 1 statements in block _0HvrqAHte_4ryaA4drs
//: block chunk-wait agenda-changed
// statement #0 _5a8Hrecb3_9evcyu8sh
  // chunk of 16 components
/*chunk wait agenda changed*/
  struct timespec ts__5a8Hrecb3_9evcyu8sh = {0,0};
  clock_gettime(CLOCK_REALTIME, &ts__5a8Hrecb3_9evcyu8sh);
  ts__5a8Hrecb3_9evcyu8sh.tv_sec += MOM_AGENDA_WAIT_SEC;
  pthread_cond_timedwait(&mom_agenda_changed_condvar, &momvar1 /*var:lkitm_agenda*/->itm_mtx, & ts__5a8Hrecb3_9evcyu8sh);
 ;

  }; // end block _0HvrqAHte_4ryaA4drs
  goto momepilog_agenda_step;
////----
 // block #1 : _14MzMbJ9v_627D0CIiA
 momblocklab__14MzMbJ9v_627D0CIiA: {
// 1 statements in block _14MzMbJ9v_627D0CIiA
//: block to apply varclo
// statement #0 _4KKszvz3w_1HLUHLsru
// apply with 1 input arguments and 0 output results, radix 1itm_to_void
   if (!mom_applval_1itm_to_void (momvar0 /*var:varclo*/,  /*constant#2:*/MOM_PREDEFINED_NAMED(the_agenda)))
     return false;

  }; // end block _14MzMbJ9v_627D0CIiA
  goto momepilog_agenda_step;
////----
 // block #2 : _4xAIqB3tj_97IrD41UP
 momblocklab__4xAIqB3tj_97IrD41UP: {
// 3 statements in block _4xAIqB3tj_97IrD41UP
//: refill empty agenda
// statement #0 _60nmad63F_1Jd336xAL
// set into varclo
  momvar0 /*var:varclo*/ =
   /*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar1 /*var:lkitm_agenda*/, /*constant-item:fill_agenda*/momconst_1);
// statement #1 _7vj8eaIrt_9bLBC6eKB
// if testing on integer
    if (/*value_is_node:*/ (momvar0 /*var:varclo*/).typnum == momty_node)
      goto momblocklab__14MzMbJ9v_627D0CIiA;
// statement #2 _6kDqRmkjk_6CxUEu2hk
// if testing on integer
    if (/*queue_item_is_empty:*/momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind == MOM_PREDEFINED_NAMED(item_queue)  && mom_queueitem_size (momvar1 /*var:lkitm_agenda*/->itm_data1) == 0)
      goto momblocklab__0HvrqAHte_4ryaA4drs;

  }; // end block _4xAIqB3tj_97IrD41UP
  goto momepilog_agenda_step;
////----
 // block #3 : _7MF947fC8_8ChefReMD
 momblocklab__7MF947fC8_8ChefReMD: {
// 2 statements in block _7MF947fC8_8ChefReMD
//: starting block of agenda_step
// statement #0 _1c462DJmx_68zwnz1Ua
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__1c462DJmx_68zwnz1Ua = momvar1 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__1c462DJmx_68zwnz1Ua =  /*constant#2:*/MOM_PREDEFINED_NAMED(the_agenda);
  if (momoldlocked__1c462DJmx_68zwnz1Ua != momnewlocked__1c462DJmx_68zwnz1Ua) {
    if (momoldlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_unlock (momoldlocked__1c462DJmx_68zwnz1Ua);
    if (momnewlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_lock (momnewlocked__1c462DJmx_68zwnz1Ua);
  } // end lock test _1c462DJmx_68zwnz1Ua
    momvar1 /*var:lkitm_agenda*/ = momnewlocked__1c462DJmx_68zwnz1Ua;
  momoldlocked__1c462DJmx_68zwnz1Ua = NULL;
  momnewlocked__1c462DJmx_68zwnz1Ua = NULL;
  } // end locked set into lkitm_agenda
// statement #1 _44tmkDkKa_8nUhJEeay
// if testing on integer
    if (/*queue_item_is_empty:*/momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind == MOM_PREDEFINED_NAMED(item_queue)  && mom_queueitem_size (momvar1 /*var:lkitm_agenda*/->itm_data1) == 0)
      goto momblocklab__4xAIqB3tj_97IrD41UP;

  }; // end block _7MF947fC8_8ChefReMD
  goto momepilog_agenda_step;
////----
//////
// epilogue of agenda_step
    momsuccess_agenda_step = true;
    goto momepilog_agenda_step;
 momepilog_agenda_step:
   if (momvar1 != NULL) mom_item_unlock(momvar1);
// give 0 outputs
  return momsuccess_agenda_step;
} // end of momfunc_void_to_void_agenda_step 



/***** end 1 functions *****/


//// end of generated module file momg_the_base_module.c

