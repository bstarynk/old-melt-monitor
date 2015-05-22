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
      || mom_unsync_item_components_count (mom_funcitm)<6
       ))
  return false;
  // 0 output results:
  // 4 variables:
// variable itmvar_tasklet of type item
  momitem_t* momvar2 = (momitem_t*)0;
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar1 = (momlockeditem_t*)0;
// variable lkitm_tasklet of type locked_item
  momlockeditem_t* momvar3 = (momlockeditem_t*)0;
// variable varclo of type value
  momvalue_t momvar0 = MOM_NONEV;
  // 6 constants:
  // constant _07BHLcwhp_48ka0t9bq
  const momvalue_t momconst_0 = 
    mom_raw_item_get_indexed_component (mom_funcitm, 0);
  // constant fill_agenda
  momitem_t* momconst_1 =
    mom_value_to_item(mom_raw_item_get_indexed_component (mom_funcitm, 1));
  // constant item_queue
  momitem_t* momconst_2 = MOM_PREDEFINED_NAMED(item_queue);
  // constant runner
  momitem_t* momconst_3 = MOM_PREDEFINED_NAMED(runner);
  // constant tasklet
  momitem_t* momconst_4 = MOM_PREDEFINED_NAMED(tasklet);
  // constant the_agenda
  momitem_t* momconst_5 = MOM_PREDEFINED_NAMED(the_agenda);
  // 0 closed:
  goto momblocklab__7MF947fC8_8ChefReMD;

/// block #0: _0HvrqAHte_4ryaA4drs
//: block chunk-wait agenda-changed
 momblocklab__0HvrqAHte_4ryaA4drs: {
// 3 statements in block _0HvrqAHte_4ryaA4drs
// statement #0 :: _5a8Hrecb3_9evcyu8sh; instr chunk-wait agenda changed
  // chunk of 16 components
/*chunk wait agenda changed*/
  struct timespec ts__5a8Hrecb3_9evcyu8sh = {0,0};
  clock_gettime(CLOCK_REALTIME, &ts__5a8Hrecb3_9evcyu8sh);
  ts__5a8Hrecb3_9evcyu8sh.tv_sec += MOM_AGENDA_WAIT_SEC;
  pthread_cond_timedwait(&mom_agenda_changed_condvar, &momvar1 /*var:lkitm_agenda*/->itm_mtx, & ts__5a8Hrecb3_9evcyu8sh);
 ;
// statement #1 :: _46231jt3F_9IUe8xbwK; clear lkitm_agenda to unlock it
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__46231jt3F_9IUe8xbwK = momvar1 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__46231jt3F_9IUe8xbwK =  (momlockeditem_t*)NULL;
  if (momoldlocked__46231jt3F_9IUe8xbwK != momnewlocked__46231jt3F_9IUe8xbwK) {
    if (momoldlocked__46231jt3F_9IUe8xbwK != NULL) mom_item_unlock (momoldlocked__46231jt3F_9IUe8xbwK);
    if (momnewlocked__46231jt3F_9IUe8xbwK != NULL) mom_item_lock (momnewlocked__46231jt3F_9IUe8xbwK);
  } // end lock test _46231jt3F_9IUe8xbwK
    momvar1 /*var:lkitm_agenda*/ = momnewlocked__46231jt3F_9IUe8xbwK;
  momoldlocked__46231jt3F_9IUe8xbwK = NULL;
  momnewlocked__46231jt3F_9IUe8xbwK = NULL;
  } // end locked set into lkitm_agenda
// statement #2 :: _51n8x1e9t_76B1hJrwI; jump start agenda_step
// jump to _7MF947fC8_8ChefReMD
  goto momblocklab__7MF947fC8_8ChefReMD;

  }; // end block _0HvrqAHte_4ryaA4drs
////----++++

/// block #1: _14MzMbJ9v_627D0CIiA
//: block to apply varclo to lkitm_agenda
 momblocklab__14MzMbJ9v_627D0CIiA: {
// 1 statements in block _14MzMbJ9v_627D0CIiA
// statement #0 :: _4KKszvz3w_1HLUHLsru; apply varclo to lkitm_agenda
// apply with 1 input arguments and 0 output results, radix 1itm_to_void
   if (!mom_applval_1itm_to_void (momvar0 /*var:varclo*/,  /*constant#5:*/MOM_PREDEFINED_NAMED(the_agenda)))
     return false;

  }; // end block _14MzMbJ9v_627D0CIiA
  goto momepilog_agenda_step;
 ////----

/// block #2: _2tz000b51_1AEBtnP4P
//: lock and run the tasklet
 momblocklab__2tz000b51_1AEBtnP4P: {
// 3 statements in block _2tz000b51_1AEBtnP4P
// statement #0 :: _4zMbIKH13_6KmkUPLpd; set lkitm_tasklet := itmvar_tasklet
  { // locked set into lkitm_tasklet
  momlockeditem_t* momoldlocked__4zMbIKH13_6KmkUPLpd = momvar3 /*var:lkitm_tasklet*/;
  momlockeditem_t* momnewlocked__4zMbIKH13_6KmkUPLpd = momvar2 /*var:itmvar_tasklet*/;
  if (momoldlocked__4zMbIKH13_6KmkUPLpd != momnewlocked__4zMbIKH13_6KmkUPLpd) {
    if (momoldlocked__4zMbIKH13_6KmkUPLpd != NULL) mom_item_unlock (momoldlocked__4zMbIKH13_6KmkUPLpd);
    if (momnewlocked__4zMbIKH13_6KmkUPLpd != NULL) mom_item_lock (momnewlocked__4zMbIKH13_6KmkUPLpd);
  } // end lock test _4zMbIKH13_6KmkUPLpd
    momvar3 /*var:lkitm_tasklet*/ = momnewlocked__4zMbIKH13_6KmkUPLpd;
  momoldlocked__4zMbIKH13_6KmkUPLpd = NULL;
  momnewlocked__4zMbIKH13_6KmkUPLpd = NULL;
  } // end locked set into lkitm_tasklet
// statement #1 :: _2u1LykBw5_7IRf5Cjub; set varclo := get_attr(lkitm_tasklet, runner)
// set into varclo
  momvar0 /*var:varclo*/ =
   /*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar3 /*var:lkitm_tasklet*/, /*constant#3:*/MOM_PREDEFINED_NAMED(runner));
// statement #2 :: _1dhK6Bj47_4vvDyBKAR; if varclo is runner node, apply it to lkitm_tasklet
// if testing on integer
    if (/*value_is_node:*/ (momvar0 /*var:varclo*/).typnum == momty_node)
      goto momblocklab__8aqavIeMK_2EHL44cnU;

  }; // end block _2tz000b51_1AEBtnP4P
  goto momepilog_agenda_step;
 ////----

/// block #3: _4xAIqB3tj_97IrD41UP
//: refill empty agenda
 momblocklab__4xAIqB3tj_97IrD41UP: {
// 3 statements in block _4xAIqB3tj_97IrD41UP
// statement #0 :: _60nmad63F_1Jd336xAL; set varclo := fill_agenda(lkitm_agenda)
// set into varclo
  momvar0 /*var:varclo*/ =
   /*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar1 /*var:lkitm_agenda*/, /*constant-item:fill_agenda*/momconst_1);
// statement #1 :: _7vj8eaIrt_9bLBC6eKB; if varclo is node apply it
// if testing on integer
    if (/*value_is_node:*/ (momvar0 /*var:varclo*/).typnum == momty_node)
      goto momblocklab__14MzMbJ9v_627D0CIiA;
// statement #2 :: _6kDqRmkjk_6CxUEu2hk; if lkitm_agenda empty queue wait agenda-changed
// if testing on integer
    if (/*queue_item_is_empty:*/momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind == /*constant#2:*/MOM_PREDEFINED_NAMED(item_queue) && mom_queueitem_size (momvar1 /*var:lkitm_agenda*/->itm_data1) == 0)
      goto momblocklab__0HvrqAHte_4ryaA4drs;

  }; // end block _4xAIqB3tj_97IrD41UP
  goto momepilog_agenda_step;
 ////----

/// block #4: _7MF947fC8_8ChefReMD
//: starting block of agenda_step
 momblocklab__7MF947fC8_8ChefReMD: {
// 4 statements in block _7MF947fC8_8ChefReMD
// statement #0 :: _1c462DJmx_68zwnz1Ua; set lkitm_agenda := the_agenda
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__1c462DJmx_68zwnz1Ua = momvar1 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__1c462DJmx_68zwnz1Ua =  /*constant#5:*/MOM_PREDEFINED_NAMED(the_agenda);
  if (momoldlocked__1c462DJmx_68zwnz1Ua != momnewlocked__1c462DJmx_68zwnz1Ua) {
    if (momoldlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_unlock (momoldlocked__1c462DJmx_68zwnz1Ua);
    if (momnewlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_lock (momnewlocked__1c462DJmx_68zwnz1Ua);
  } // end lock test _1c462DJmx_68zwnz1Ua
    momvar1 /*var:lkitm_agenda*/ = momnewlocked__1c462DJmx_68zwnz1Ua;
  momoldlocked__1c462DJmx_68zwnz1Ua = NULL;
  momnewlocked__1c462DJmx_68zwnz1Ua = NULL;
  } // end locked set into lkitm_agenda
// statement #1 :: _44tmkDkKa_8nUhJEeay; if empty agenda refill it
// if testing on integer
    if (/*queue_item_is_empty:*/momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind == /*constant#2:*/MOM_PREDEFINED_NAMED(item_queue) && mom_queueitem_size (momvar1 /*var:lkitm_agenda*/->itm_data1) == 0)
      goto momblocklab__4xAIqB3tj_97IrD41UP;
// statement #2 :: _0p7zBDIku_3njudUFtp; itmvar_tasklet <- pop_front(lkitm_agenda)
/*pop_front_queue_item:*/
momvar2 /*var:itmvar_tasklet*/ = (momitem_t*)NULL;
{if (momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind ==  /*constant#2:*/MOM_PREDEFINED_NAMED(item_queue))
  momvar2 /*var:itmvar_tasklet*/ = mom_queueitem_pop_front((struct momqueueitems_st*)momvar1 /*var:lkitm_agenda*/->itm_data1);}
// statement #3 :: _9MCjbp28L_6zLA3KERH; if itmvar_tasklet is a tasklet run it
// if testing on integer
    if (/*item_has_kind:*/momvar2 /*var:itmvar_tasklet*/ && momvar2 /*var:itmvar_tasklet*/->itm_kind == /*constant#4:*/MOM_PREDEFINED_NAMED(tasklet))
      goto momblocklab__2tz000b51_1AEBtnP4P;

  }; // end block _7MF947fC8_8ChefReMD
  goto momepilog_agenda_step;
 ////----

/// block #5: _8aqavIeMK_2EHL44cnU
//: apply varclo to lkitm_tasklet and succeed
 momblocklab__8aqavIeMK_2EHL44cnU: {
// 4 statements in block _8aqavIeMK_2EHL44cnU
// statement #0 :: _3243Pxefq_0KLe76cvy; debug-run lkitm_tasklet & varclo before running
  // chunk of 7 components
  MOM_DEBUGPRINTF(run, "before running tasklet %s using closure %s",
    mom_item_cstring(momvar3 /*var:lkitm_tasklet*/),  mom_output_gcstring(momvar0 /*var:varclo*/));
 ;
// statement #1 :: _6r8vRwBre_0c5UpIM8y; apply varclo to lkitm_tasklet for running
// apply with 1 input arguments and 0 output results, radix 1itm_to_void
   if (!mom_applval_1itm_to_void (momvar0 /*var:varclo*/, momvar3 /*var:lkitm_tasklet*/))
     return false;
// statement #2 :: _7s4IHrpuh_90FeBfefy; debug-run lkitm_tasklet after running
  // chunk of 5 components
  MOM_DEBUGPRINTF(run, "after running tasklet %s\n",
    mom_item_cstring(momvar3 /*var:lkitm_tasklet*/));
 ;
// statement #3 :: _55xk4tfhF_5sUF6zmde; succeed agenda_step after tasklet running
// success
  momsuccess_agenda_step = true;
  goto momepilog_agenda_step;

  }; // end block _8aqavIeMK_2EHL44cnU
  goto momepilog_agenda_step;
 ////----

//////
// epilogue of agenda_step
    momsuccess_agenda_step = true;
    goto momepilog_agenda_step;
 momepilog_agenda_step:
   if (momvar1 != NULL) mom_item_unlock(momvar1);
   if (momvar3 != NULL) mom_item_unlock(momvar3);
// give 0 outputs
  return momsuccess_agenda_step;
} // end of momfunc_void_to_void_agenda_step 




/***** end 1 functions *****/


//// end of generated module file momg_the_base_module.c

