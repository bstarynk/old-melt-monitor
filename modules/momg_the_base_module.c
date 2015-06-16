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



/***** declaring 4 functions *****/


/// declare function #0: agenda_push_back
extern bool momfunc_1val_to_void_agenda_push_back (const momnode_t *, momvalue_t);


/// declare function #1: agenda_push_front
extern bool momfunc_1val_to_void_agenda_push_front (const momnode_t *, momvalue_t);


/// declare function #2: agenda_step
extern bool momfunc_void_to_void_agenda_step (const momnode_t *);


/// declare function #3: append_function_to_closed_module
extern bool momfunc_1itm_to_void_append_function_to_closed_module (const momnode_t *, momitem_t*);


/***** implementing 4 functions *****/


/// implement function #0: agenda_push_back
bool momfunc_1val_to_void_agenda_push_back (const momnode_t *mom_node, momvalue_t momarg0)
{ // body of function agenda_push_back
  bool momsuccess_agenda_push_back = false;
  momitem_t* mom_funcitm = NULL;
  if (MOM_UNLIKELY(!mom_node
      || !(mom_funcitm = mom_node_conn (mom_node))
      || mom_unsync_item_components_count (mom_funcitm)<4
       ))
  return false;
  // 0 output results:
  // 4 variables:
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar0  /*declvar:lkitm_agenda*/ = (momlockeditem_t*)0;
// variable lkitm_tasklet of type locked_item
  momlockeditem_t* momvar1  /*declvar:lkitm_tasklet*/ = (momlockeditem_t*)0;
// variable varix of type integer
  intptr_t momvar3  /*declvar:varix*/ = (intptr_t)0;
// variable varlen of type integer
  intptr_t momvar2  /*declvar:varlen*/ = (intptr_t)0;
  // 4 constants:
  // constant hook_closure
  const momvalue_t momconst_0 /*const:hook_closure*/ = 
    mom_raw_item_get_indexed_component (mom_funcitm, 0);
  // constant item_queue
  momitem_t* momconst_1 /*const:item_queue*/ = MOM_PREDEFINED_NAMED(item_queue);
  // constant tasklet
  momitem_t* momconst_2 /*const:tasklet*/ = MOM_PREDEFINED_NAMED(tasklet);
  // constant the_agenda
  momitem_t* momconst_3 /*const:the_agenda*/ = MOM_PREDEFINED_NAMED(the_agenda);
  // 0 closed:
  goto momblocklab__0y7wu372C_3JH4an3L2;

/// block #0: _0y7wu372C_3JH4an3L2
//: start block of agenda_push_back
 momblocklab__0y7wu372C_3JH4an3L2: {
// 4 statements in block _0y7wu372C_3JH4an3L2
// statement #0 :: _2ur1uJ52w_1RmsKK5vb; set lkitm_agenda := the_agenda; in agenda_push_back
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__2ur1uJ52w_1RmsKK5vb = momvar0 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__2ur1uJ52w_1RmsKK5vb =  /*constant#3:*/MOM_PREDEFINED_NAMED(the_agenda);
  if (momoldlocked__2ur1uJ52w_1RmsKK5vb != momnewlocked__2ur1uJ52w_1RmsKK5vb) {
    if (momoldlocked__2ur1uJ52w_1RmsKK5vb != NULL) mom_item_unlock (momoldlocked__2ur1uJ52w_1RmsKK5vb);
    if (momnewlocked__2ur1uJ52w_1RmsKK5vb != NULL) mom_item_lock (momnewlocked__2ur1uJ52w_1RmsKK5vb);
  } // end lock test _2ur1uJ52w_1RmsKK5vb
    momvar0 /*var:lkitm_agenda*/ = momnewlocked__2ur1uJ52w_1RmsKK5vb;
  momoldlocked__2ur1uJ52w_1RmsKK5vb = NULL;
  momnewlocked__2ur1uJ52w_1RmsKK5vb = NULL;
  } // end locked set into lkitm_agenda
// statement #1 :: _6e0q8qCzJ_3ycbtIr58; set lkitm_tasklet := value_to_item(argtasklets); in agenda_push_back
  { // locked set into lkitm_tasklet
  momlockeditem_t* momoldlocked__6e0q8qCzJ_3ycbtIr58 = momvar1 /*var:lkitm_tasklet*/;
  momlockeditem_t* momnewlocked__6e0q8qCzJ_3ycbtIr58 = /*value_to_item:*/ mom_value_to_item(momarg0 /*formalarg:argtasklets*/);
  if (momoldlocked__6e0q8qCzJ_3ycbtIr58 != momnewlocked__6e0q8qCzJ_3ycbtIr58) {
    if (momoldlocked__6e0q8qCzJ_3ycbtIr58 != NULL) mom_item_unlock (momoldlocked__6e0q8qCzJ_3ycbtIr58);
    if (momnewlocked__6e0q8qCzJ_3ycbtIr58 != NULL) mom_item_lock (momnewlocked__6e0q8qCzJ_3ycbtIr58);
  } // end lock test _6e0q8qCzJ_3ycbtIr58
    momvar1 /*var:lkitm_tasklet*/ = momnewlocked__6e0q8qCzJ_3ycbtIr58;
  momoldlocked__6e0q8qCzJ_3ycbtIr58 = NULL;
  momnewlocked__6e0q8qCzJ_3ycbtIr58 = NULL;
  } // end locked set into lkitm_tasklet
// statement #2 :: _1m2L2Rncp_4uuzUDCqz; if lkitm_tasklet, push it back; in agenda_push_back
// if testing on locked_item
    if (momvar1 /*var:lkitm_tasklet*/)
      goto momblocklab__50yPKJAK2_4Ksihqd4c;
// statement #3 :: _01KbUac4A_3Jk2sUhJp; if argtasklets is sequence push them in back then notify; in agenda_push_back
// if testing on integer
    if (/*value_is_sequence:*/ mom_value_to_sequ(momarg0 /*formalarg:argtasklets*/) != NULL)
      goto momblocklab__27PBmP9xP_3Hh1aedqr;

  }; // end block _0y7wu372C_3JH4an3L2
  goto momepilog_agenda_push_back;
 ////----

/// block #1: _27PBmP9xP_3Hh1aedqr
//: block to push every lkitm_tasklet in sequence argtasklets in back of agenda and notify
 momblocklab__27PBmP9xP_3Hh1aedqr: {
// 3 statements in block _27PBmP9xP_3Hh1aedqr
// statement #0 :: _5JFtUFwBC_1v2BUA42z; varlen := length of argtasklets; in agenda_push_back
// set into varlen
  momvar2 /*var:varlen*/ =
   /*value_sequence_length:*/ mom_seq_length(mom_value_to_sequ(momarg0 /*formalarg:argtasklets*/));
// statement #1 :: _60v4pLRCu_55xI9BKJD; varix := 0; in agenda_push_back
// set into varix
  momvar3 /*var:varix*/ =
   0;
// statement #2 :: _4bmJK3Ip5_22Dnmu5Hu; jump to seqloop on argtasklets; in agenda_push_back
// jump to _3JddiuKnc_1nzuB6bss
  goto momblocklab__3JddiuKnc_1nzuB6bss;

  }; // end block _27PBmP9xP_3Hh1aedqr
////----++++

/// block #2: _2I2UdbsCL_6c82fexBk
//: blockepilog to notify in agenda_push_back
 momblocklab__2I2UdbsCL_6c82fexBk: {
// 2 statements in block _2I2UdbsCL_6c82fexBk
// statement #0 :: _27iRH5wiL_0Rmz4hwxv; broadcast agenda changed after pushing tasklet in agenda_push_back
  // chunk of 2 components
  pthread_cond_broadcast(&mom_agenda_changed_condvar);
 ;
// statement #1 :: _9asfCiefD_1pRiIn679; success of agenda_push_back
// success
  momsuccess_agenda_push_back = true;
  goto momepilog_agenda_push_back;

  }; // end block _2I2UdbsCL_6c82fexBk
  goto momepilog_agenda_push_back;
 ////----

/// block #3: _3JddiuKnc_1nzuB6bss
//: block seqloop on argtasklets; in agenda_push_back
 momblocklab__3JddiuKnc_1nzuB6bss: {
// 5 statements in block _3JddiuKnc_1nzuB6bss
// statement #0 :: _61DxdHIjP_2U7hhsKDL; if (varix>=varlen) goto epilogpushback; in agenda_push_back
// if testing on integer
    if (/*integer_greater_or_equal:*/((momvar3 /*var:varix*/) >= (momvar2 /*var:varlen*/)) )
      goto momblocklab__2I2UdbsCL_6c82fexBk;
// statement #1 :: _4qKLvc6ce_24ym1K7qm; set lkitm_tasklet := value_sequence_nth(argtasklets, varix); for agenda_push_back
  { // locked set into lkitm_tasklet
  momlockeditem_t* momoldlocked__4qKLvc6ce_24ym1K7qm = momvar1 /*var:lkitm_tasklet*/;
  momlockeditem_t* momnewlocked__4qKLvc6ce_24ym1K7qm = /*value_sequence_nth:*/ (momitem_t*)mom_seq_nth(mom_value_to_sequ(momarg0 /*formalarg:argtasklets*/), (int)(momvar3 /*var:varix*/));
  if (momoldlocked__4qKLvc6ce_24ym1K7qm != momnewlocked__4qKLvc6ce_24ym1K7qm) {
    if (momoldlocked__4qKLvc6ce_24ym1K7qm != NULL) mom_item_unlock (momoldlocked__4qKLvc6ce_24ym1K7qm);
    if (momnewlocked__4qKLvc6ce_24ym1K7qm != NULL) mom_item_lock (momnewlocked__4qKLvc6ce_24ym1K7qm);
  } // end lock test _4qKLvc6ce_24ym1K7qm
    momvar1 /*var:lkitm_tasklet*/ = momnewlocked__4qKLvc6ce_24ym1K7qm;
  momoldlocked__4qKLvc6ce_24ym1K7qm = NULL;
  momnewlocked__4qKLvc6ce_24ym1K7qm = NULL;
  } // end locked set into lkitm_tasklet
// statement #2 :: _60fFrKH58_32pCM7kxK; push lkitm_tasklet in back of lkitm_tasklet; in agenda_push_back seq...
/*push_back_queue_item:*/
{if (momvar0 /*var:lkitm_agenda*/ && momvar0 /*var:lkitm_agenda*/->itm_kind ==  /*constant#1:*/MOM_PREDEFINED_NAMED(item_queue))
   mom_queueitem_push_back((struct momqueueitems_st*)momvar0 /*var:lkitm_agenda*/->itm_data1, momvar1 /*var:lkitm_tasklet*/);}
// statement #3 :: _6UmR4hJ3a_9A2d4BdrJ; increment varix ; in agenda_push_back seq...
// set into varix
  momvar3 /*var:varix*/ =
   /*integer_add:*/((momvar3 /*var:varix*/) + (1)) ;
// statement #4 :: _7qRm75s5I_9vzdpzm0K; jump block seqloop; in agenda_push_back seq... 
// jump to _3JddiuKnc_1nzuB6bss
  goto momblocklab__3JddiuKnc_1nzuB6bss;

  }; // end block _3JddiuKnc_1nzuB6bss
////----++++

/// block #4: _50yPKJAK2_4Ksihqd4c
//: block to push lkitm_tasklet in back of agenda and epilog; in agenda_push_back
 momblocklab__50yPKJAK2_4Ksihqd4c: {
// 3 statements in block _50yPKJAK2_4Ksihqd4c
// statement #0 :: _24sBAv42k_3Ly5hw0vj; push lkitm_tasklet at back of lkitm_agenda; in agenda_push_back
/*push_back_queue_item:*/
{if (momvar0 /*var:lkitm_agenda*/ && momvar0 /*var:lkitm_agenda*/->itm_kind ==  /*constant#1:*/MOM_PREDEFINED_NAMED(item_queue))
   mom_queueitem_push_back((struct momqueueitems_st*)momvar0 /*var:lkitm_agenda*/->itm_data1, momvar1 /*var:lkitm_tasklet*/);}
// statement #1 :: _1R1KvIhCR_6ak3tU4ep; debug-printf pushed fron lkitm_tasklet in agenda_push_back
  // chunk of 5 components
  MOM_DEBUGPRINTF(run, "agenda_push_back: pushed tasklet %s in back of the_agenda",
   mom_item_cstring(momvar1 /*var:lkitm_tasklet*/));
 ;
// statement #2 :: _6pte4hEqb_94D0cHswb; jump to epilog of agenda_push_back
// jump to _2I2UdbsCL_6c82fexBk
  goto momblocklab__2I2UdbsCL_6c82fexBk;

  }; // end block _50yPKJAK2_4Ksihqd4c
////----++++

//////
// epilogue of agenda_push_back
    momsuccess_agenda_push_back = true;
    goto momepilog_agenda_push_back;
 momepilog_agenda_push_back:
   if (momvar0 != NULL) mom_item_unlock(momvar0);
   if (momvar1 != NULL) mom_item_unlock(momvar1);
// give 0 outputs
  return momsuccess_agenda_push_back;
} // end of momfunc_1val_to_void_agenda_push_back 




/// implement function #1: agenda_push_front
bool momfunc_1val_to_void_agenda_push_front (const momnode_t *mom_node, momvalue_t momarg0)
{ // body of function agenda_push_front
  bool momsuccess_agenda_push_front = false;
  momitem_t* mom_funcitm = NULL;
  if (MOM_UNLIKELY(!mom_node
      || !(mom_funcitm = mom_node_conn (mom_node))
      || mom_unsync_item_components_count (mom_funcitm)<4
       ))
  return false;
  // 0 output results:
  // 4 variables:
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar0  /*declvar:lkitm_agenda*/ = (momlockeditem_t*)0;
// variable lkitm_tasklet of type locked_item
  momlockeditem_t* momvar1  /*declvar:lkitm_tasklet*/ = (momlockeditem_t*)0;
// variable varix of type integer
  intptr_t momvar3  /*declvar:varix*/ = (intptr_t)0;
// variable varlen of type integer
  intptr_t momvar2  /*declvar:varlen*/ = (intptr_t)0;
  // 4 constants:
  // constant hook_closure
  const momvalue_t momconst_0 /*const:hook_closure*/ = 
    mom_raw_item_get_indexed_component (mom_funcitm, 0);
  // constant item_queue
  momitem_t* momconst_1 /*const:item_queue*/ = MOM_PREDEFINED_NAMED(item_queue);
  // constant tasklet
  momitem_t* momconst_2 /*const:tasklet*/ = MOM_PREDEFINED_NAMED(tasklet);
  // constant the_agenda
  momitem_t* momconst_3 /*const:the_agenda*/ = MOM_PREDEFINED_NAMED(the_agenda);
  // 0 closed:
  goto momblocklab__02MHbyAxU_563vpdUu2;

/// block #0: _02MHbyAxU_563vpdUu2
//: start block of agenda_push_front
 momblocklab__02MHbyAxU_563vpdUu2: {
// 4 statements in block _02MHbyAxU_563vpdUu2
// statement #0 :: _2P84CtHfA_4kF28e70H; set lkitm_agenda := the_agenda; in agenda_push_front
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__2P84CtHfA_4kF28e70H = momvar0 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__2P84CtHfA_4kF28e70H =  /*constant#3:*/MOM_PREDEFINED_NAMED(the_agenda);
  if (momoldlocked__2P84CtHfA_4kF28e70H != momnewlocked__2P84CtHfA_4kF28e70H) {
    if (momoldlocked__2P84CtHfA_4kF28e70H != NULL) mom_item_unlock (momoldlocked__2P84CtHfA_4kF28e70H);
    if (momnewlocked__2P84CtHfA_4kF28e70H != NULL) mom_item_lock (momnewlocked__2P84CtHfA_4kF28e70H);
  } // end lock test _2P84CtHfA_4kF28e70H
    momvar0 /*var:lkitm_agenda*/ = momnewlocked__2P84CtHfA_4kF28e70H;
  momoldlocked__2P84CtHfA_4kF28e70H = NULL;
  momnewlocked__2P84CtHfA_4kF28e70H = NULL;
  } // end locked set into lkitm_agenda
// statement #1 :: _7zujDCE1i_9m9ccD6C7; set lkitm_tasklet := value_to_item(argtasklets); in agenda_push_front
  { // locked set into lkitm_tasklet
  momlockeditem_t* momoldlocked__7zujDCE1i_9m9ccD6C7 = momvar1 /*var:lkitm_tasklet*/;
  momlockeditem_t* momnewlocked__7zujDCE1i_9m9ccD6C7 = /*value_to_item:*/ mom_value_to_item(momarg0 /*formalarg:argtasklets*/);
  if (momoldlocked__7zujDCE1i_9m9ccD6C7 != momnewlocked__7zujDCE1i_9m9ccD6C7) {
    if (momoldlocked__7zujDCE1i_9m9ccD6C7 != NULL) mom_item_unlock (momoldlocked__7zujDCE1i_9m9ccD6C7);
    if (momnewlocked__7zujDCE1i_9m9ccD6C7 != NULL) mom_item_lock (momnewlocked__7zujDCE1i_9m9ccD6C7);
  } // end lock test _7zujDCE1i_9m9ccD6C7
    momvar1 /*var:lkitm_tasklet*/ = momnewlocked__7zujDCE1i_9m9ccD6C7;
  momoldlocked__7zujDCE1i_9m9ccD6C7 = NULL;
  momnewlocked__7zujDCE1i_9m9ccD6C7 = NULL;
  } // end locked set into lkitm_tasklet
// statement #2 :: _8UcKkiiIk_2ur4xDt59; if lkitm_tasklet, push it front; in agenda_push_front
// if testing on locked_item
    if (momvar1 /*var:lkitm_tasklet*/)
      goto momblocklab__0LHqbyzz9_2J9v5p6zc;
// statement #3 :: _4kFw4Pe2j_7uh2m7JuM; if argtasklets is sequence push them in front then notify; in agenda_push_front
// if testing on integer
    if (/*value_is_sequence:*/ mom_value_to_sequ(momarg0 /*formalarg:argtasklets*/) != NULL)
      goto momblocklab__9m9aprPJz_4hExeiqIi;

  }; // end block _02MHbyAxU_563vpdUu2
  goto momepilog_agenda_push_front;
 ////----

/// block #1: _0LHqbyzz9_2J9v5p6zc
//: block to push lkitm_tasklet in front of agenda and epilog; in agenda_push_front
 momblocklab__0LHqbyzz9_2J9v5p6zc: {
// 3 statements in block _0LHqbyzz9_2J9v5p6zc
// statement #0 :: _2AAHn6xi7_1R19bsnhn; push lkitm_tasklet in front of lkitm_agenda; in agenda_push_front
/*push_front_queue_item:*/
{if (momvar0 /*var:lkitm_agenda*/ && momvar0 /*var:lkitm_agenda*/->itm_kind ==  /*constant#1:*/MOM_PREDEFINED_NAMED(item_queue))
  mom_queueitem_push_front((struct momqueueitems_st*)momvar0 /*var:lkitm_agenda*/->itm_data1, momvar1 /*var:lkitm_tasklet*/);}
// statement #1 :: _9jMHbym6i_27j6eCiEL; debug-printf pushed fron lkitm_tasklet in agenda_push_front
  // chunk of 5 components
  MOM_DEBUGPRINTF(run, "agenda_push_front: pushed tasklet %s in front of the_agenda",
   mom_item_cstring(momvar1 /*var:lkitm_tasklet*/));
 ;
// statement #2 :: _6npzcxMDM_2I2dUIfMn; jump to epilog of agenda_push_front
// jump to _4h7BHLiLB_9askynfAC
  goto momblocklab__4h7BHLiLB_9askynfAC;

  }; // end block _0LHqbyzz9_2J9v5p6zc
////----++++

/// block #2: _4h7BHLiLB_9askynfAC
//: blockepilog to notify in agenda_push_front
 momblocklab__4h7BHLiLB_9askynfAC: {
// 2 statements in block _4h7BHLiLB_9askynfAC
// statement #0 :: _1FU0vnqP4_6LLqBkKnt; broadcast agenda changed after pushing tasklet in agenda_push_front
  // chunk of 2 components
  pthread_cond_broadcast(&mom_agenda_changed_condvar);
 ;
// statement #1 :: _35ALykwdw_1UiPefUMH; success of agenda_push_front
// success
  momsuccess_agenda_push_front = true;
  goto momepilog_agenda_push_front;

  }; // end block _4h7BHLiLB_9askynfAC
  goto momepilog_agenda_push_front;
 ////----

/// block #3: _7jkswmJf4_89dmJmBc2
//: block seqloop on argtasklets; in agenda_push_front
 momblocklab__7jkswmJf4_89dmJmBc2: {
// 5 statements in block _7jkswmJf4_89dmJmBc2
// statement #0 :: _6PukHeHkL_5rxkpnFsJ; if (varix>=varlen) goto epilogpushfront; in agenda_push_front
// if testing on integer
    if (/*integer_greater_or_equal:*/((momvar3 /*var:varix*/) >= (momvar2 /*var:varlen*/)) )
      goto momblocklab__4h7BHLiLB_9askynfAC;
// statement #1 :: _24z256FyK_7r8ApkvqK; set lkitm_tasklet := value_sequence_nth(argtasklets, varix); for agenda_push_front
  { // locked set into lkitm_tasklet
  momlockeditem_t* momoldlocked__24z256FyK_7r8ApkvqK = momvar1 /*var:lkitm_tasklet*/;
  momlockeditem_t* momnewlocked__24z256FyK_7r8ApkvqK = /*value_sequence_nth:*/ (momitem_t*)mom_seq_nth(mom_value_to_sequ(momarg0 /*formalarg:argtasklets*/), (int)(momvar3 /*var:varix*/));
  if (momoldlocked__24z256FyK_7r8ApkvqK != momnewlocked__24z256FyK_7r8ApkvqK) {
    if (momoldlocked__24z256FyK_7r8ApkvqK != NULL) mom_item_unlock (momoldlocked__24z256FyK_7r8ApkvqK);
    if (momnewlocked__24z256FyK_7r8ApkvqK != NULL) mom_item_lock (momnewlocked__24z256FyK_7r8ApkvqK);
  } // end lock test _24z256FyK_7r8ApkvqK
    momvar1 /*var:lkitm_tasklet*/ = momnewlocked__24z256FyK_7r8ApkvqK;
  momoldlocked__24z256FyK_7r8ApkvqK = NULL;
  momnewlocked__24z256FyK_7r8ApkvqK = NULL;
  } // end locked set into lkitm_tasklet
// statement #2 :: _32pAfaaDd_1jeEvt083; push lkitm_tasklet in front of lkitm_tasklet; in agenda_push_front seq...
/*push_front_queue_item:*/
{if (momvar0 /*var:lkitm_agenda*/ && momvar0 /*var:lkitm_agenda*/->itm_kind ==  /*constant#1:*/MOM_PREDEFINED_NAMED(item_queue))
  mom_queueitem_push_front((struct momqueueitems_st*)momvar0 /*var:lkitm_agenda*/->itm_data1, momvar1 /*var:lkitm_tasklet*/);}
// statement #3 :: _9A2akip87_18CiHx1db; increment varix ; in agenda_push_front seq...
// set into varix
  momvar3 /*var:varix*/ =
   /*integer_add:*/((momvar3 /*var:varix*/) + (1)) ;
// statement #4 :: _8HabUa62b_311b5M6qe; jump block seqloop; in agenda_push_front 
// jump to _7jkswmJf4_89dmJmBc2
  goto momblocklab__7jkswmJf4_89dmJmBc2;

  }; // end block _7jkswmJf4_89dmJmBc2
////----++++

/// block #4: _9m9aprPJz_4hExeiqIi
//: block to push every lkitm_tasklet in sequence argtasklets in front of agenda and notify
 momblocklab__9m9aprPJz_4hExeiqIi: {
// 3 statements in block _9m9aprPJz_4hExeiqIi
// statement #0 :: _4sni2unK9_1AqEEbhcc; varlen := length of argtasklets; in agenda_push_front
// set into varlen
  momvar2 /*var:varlen*/ =
   /*value_sequence_length:*/ mom_seq_length(mom_value_to_sequ(momarg0 /*formalarg:argtasklets*/));
// statement #1 :: _67HCKpy4n_17u81x1Cz; varix := 0; in agenda_push_front
// set into varix
  momvar3 /*var:varix*/ =
   0;
// statement #2 :: _7e0I1v1zU_9Ibu5v8Iw; jump to seqloop on argtasklets; in agenda_push_front
// jump to _7jkswmJf4_89dmJmBc2
  goto momblocklab__7jkswmJf4_89dmJmBc2;

  }; // end block _9m9aprPJz_4hExeiqIi
////----++++

//////
// epilogue of agenda_push_front
    momsuccess_agenda_push_front = true;
    goto momepilog_agenda_push_front;
 momepilog_agenda_push_front:
   if (momvar0 != NULL) mom_item_unlock(momvar0);
   if (momvar1 != NULL) mom_item_unlock(momvar1);
// give 0 outputs
  return momsuccess_agenda_push_front;
} // end of momfunc_1val_to_void_agenda_push_front 




/// implement function #2: agenda_step
bool momfunc_void_to_void_agenda_step (const momnode_t *mom_node)
{ // body of function agenda_step
  bool momsuccess_agenda_step = false;
  momitem_t* mom_funcitm = NULL;
  if (MOM_UNLIKELY(!mom_node
      || !(mom_funcitm = mom_node_conn (mom_node))
      || mom_unsync_item_components_count (mom_funcitm)<7
       ))
  return false;
  // 0 output results:
  // 4 variables:
// variable itmvar_tasklet of type item
  momitem_t* momvar2  /*declvar:itmvar_tasklet*/ = (momitem_t*)0;
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar1  /*declvar:lkitm_agenda*/ = (momlockeditem_t*)0;
// variable lkitm_tasklet of type locked_item
  momlockeditem_t* momvar3  /*declvar:lkitm_tasklet*/ = (momlockeditem_t*)0;
// variable varclo of type value
  momvalue_t momvar0 /*declvar:varclo*/ = MOM_NONEV;
  // 7 constants:
  // constant _07BHLcwhp_48ka0t9bq
  const momvalue_t momconst_1 /*const:_07BHLcwhp_48ka0t9bq*/ = 
    mom_raw_item_get_indexed_component (mom_funcitm, 1);
  // constant fill_agenda
  momitem_t* momconst_2 /*const:fill_agenda*/ =
    mom_value_to_item(mom_raw_item_get_indexed_component (mom_funcitm, 2));
  // constant hook_closure
  const momvalue_t momconst_0 /*const:hook_closure*/ = 
    mom_raw_item_get_indexed_component (mom_funcitm, 0);
  // constant item_queue
  momitem_t* momconst_3 /*const:item_queue*/ = MOM_PREDEFINED_NAMED(item_queue);
  // constant runner
  momitem_t* momconst_4 /*const:runner*/ = MOM_PREDEFINED_NAMED(runner);
  // constant tasklet
  momitem_t* momconst_5 /*const:tasklet*/ = MOM_PREDEFINED_NAMED(tasklet);
  // constant the_agenda
  momitem_t* momconst_6 /*const:the_agenda*/ = MOM_PREDEFINED_NAMED(the_agenda);
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
   if (!mom_applval_1itm_to_void (momvar0 /*var:varclo*/,  /*constant#6:*/MOM_PREDEFINED_NAMED(the_agenda)))
     return false;

  }; // end block _14MzMbJ9v_627D0CIiA
  goto momepilog_agenda_step;
 ////----

/// block #2: _2CMuyktFw_5rt7PbyCD
//: block succeed agenda_step since stopping
 momblocklab__2CMuyktFw_5rt7PbyCD: {
// 1 statements in block _2CMuyktFw_5rt7PbyCD
// statement #0 :: _7H4dmCuFI_1C4PP5IBx; succeed agenda_step when stopping
// success
  momsuccess_agenda_step = true;
  goto momepilog_agenda_step;

  }; // end block _2CMuyktFw_5rt7PbyCD
  goto momepilog_agenda_step;
 ////----

/// block #3: _2tz000b51_1AEBtnP4P
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
   /*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar3 /*var:lkitm_tasklet*/, /*constant#4:*/MOM_PREDEFINED_NAMED(runner));
// statement #2 :: _1dhK6Bj47_4vvDyBKAR; if varclo is runner node, apply it to lkitm_tasklet
// if testing on integer
    if (/*value_is_node:*/ (momvar0 /*var:varclo*/).typnum == momty_node)
      goto momblocklab__8aqavIeMK_2EHL44cnU;

  }; // end block _2tz000b51_1AEBtnP4P
  goto momepilog_agenda_step;
 ////----

/// block #4: _4xAIqB3tj_97IrD41UP
//: refill empty agenda
 momblocklab__4xAIqB3tj_97IrD41UP: {
// 3 statements in block _4xAIqB3tj_97IrD41UP
// statement #0 :: _60nmad63F_1Jd336xAL; set varclo := fill_agenda(lkitm_agenda)
// set into varclo
  momvar0 /*var:varclo*/ =
   /*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar1 /*var:lkitm_agenda*/, /*constant-item:fill_agenda*/momconst_2);
// statement #1 :: _7vj8eaIrt_9bLBC6eKB; if varclo is node apply it
// if testing on integer
    if (/*value_is_node:*/ (momvar0 /*var:varclo*/).typnum == momty_node)
      goto momblocklab__14MzMbJ9v_627D0CIiA;
// statement #2 :: _6kDqRmkjk_6CxUEu2hk; if lkitm_agenda empty queue wait agenda-changed
// if testing on integer
    if (/*queue_item_is_empty:*/momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind == /*constant#3:*/MOM_PREDEFINED_NAMED(item_queue) && mom_queueitem_size (momvar1 /*var:lkitm_agenda*/->itm_data1) == 0)
      goto momblocklab__0HvrqAHte_4ryaA4drs;

  }; // end block _4xAIqB3tj_97IrD41UP
  goto momepilog_agenda_step;
 ////----

/// block #5: _7MF947fC8_8ChefReMD
//: starting block of agenda_step
 momblocklab__7MF947fC8_8ChefReMD: {
// 5 statements in block _7MF947fC8_8ChefReMD
// statement #0 :: _1c462DJmx_68zwnz1Ua; set lkitm_agenda := the_agenda in agenda_step
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__1c462DJmx_68zwnz1Ua = momvar1 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__1c462DJmx_68zwnz1Ua =  /*constant#6:*/MOM_PREDEFINED_NAMED(the_agenda);
  if (momoldlocked__1c462DJmx_68zwnz1Ua != momnewlocked__1c462DJmx_68zwnz1Ua) {
    if (momoldlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_unlock (momoldlocked__1c462DJmx_68zwnz1Ua);
    if (momnewlocked__1c462DJmx_68zwnz1Ua != NULL) mom_item_lock (momnewlocked__1c462DJmx_68zwnz1Ua);
  } // end lock test _1c462DJmx_68zwnz1Ua
    momvar1 /*var:lkitm_agenda*/ = momnewlocked__1c462DJmx_68zwnz1Ua;
  momoldlocked__1c462DJmx_68zwnz1Ua = NULL;
  momnewlocked__1c462DJmx_68zwnz1Ua = NULL;
  } // end locked set into lkitm_agenda
// statement #1 :: _9kCFcc1xu_2s0E2MiAa; if should_stop succeed in agenda_step
// if testing on integer
    if (/*should_stop:*/ mom_should_stop())
      goto momblocklab__2CMuyktFw_5rt7PbyCD;
// statement #2 :: _44tmkDkKa_8nUhJEeay; if empty agenda refill it
// if testing on integer
    if (/*queue_item_is_empty:*/momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind == /*constant#3:*/MOM_PREDEFINED_NAMED(item_queue) && mom_queueitem_size (momvar1 /*var:lkitm_agenda*/->itm_data1) == 0)
      goto momblocklab__4xAIqB3tj_97IrD41UP;
// statement #3 :: _0p7zBDIku_3njudUFtp; itmvar_tasklet <- pop_front(lkitm_agenda)
/*pop_front_queue_item:*/
momvar2 /*var:itmvar_tasklet*/ = (momitem_t*)NULL;
{if (momvar1 /*var:lkitm_agenda*/ && momvar1 /*var:lkitm_agenda*/->itm_kind ==  /*constant#3:*/MOM_PREDEFINED_NAMED(item_queue))
  momvar2 /*var:itmvar_tasklet*/ = mom_queueitem_pop_front((struct momqueueitems_st*)momvar1 /*var:lkitm_agenda*/->itm_data1);}
// statement #4 :: _9MCjbp28L_6zLA3KERH; if itmvar_tasklet is a tasklet run it
// if testing on integer
    if (/*item_has_kind:*/momvar2 /*var:itmvar_tasklet*/ && momvar2 /*var:itmvar_tasklet*/->itm_kind == /*constant#5:*/MOM_PREDEFINED_NAMED(tasklet))
      goto momblocklab__2tz000b51_1AEBtnP4P;

  }; // end block _7MF947fC8_8ChefReMD
  goto momepilog_agenda_step;
 ////----

/// block #6: _8aqavIeMK_2EHL44cnU
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




/// implement function #3: append_function_to_closed_module
bool momfunc_1itm_to_void_append_function_to_closed_module (const momnode_t *mom_node, momitem_t* momarg0)
{ // body of function append_function_to_closed_module
  bool momsuccess_append_function_to_closed_module = false;
  momitem_t* mom_funcitm = NULL;
  if (MOM_UNLIKELY(!mom_node
      || mom_node_arity (mom_node) < 1
      || !(mom_funcitm = mom_node_conn (mom_node))
      || mom_unsync_item_components_count (mom_funcitm)<3
       ))
  return false;
  // 0 output results:
  // 5 variables:
// variable itmvar_funsig of type item
  momitem_t* momvar0  /*declvar:itmvar_funsig*/ = (momitem_t*)0;
// variable lkitm_fun of type locked_item
  momlockeditem_t* momvar3  /*declvar:lkitm_fun*/ = (momlockeditem_t*)0;
// variable lkitm_module of type locked_item
  momlockeditem_t* momvar4  /*declvar:lkitm_module*/ = (momlockeditem_t*)0;
// variable valvar_funcset of type value
  momvalue_t momvar1 /*declvar:valvar_funcset*/ = MOM_NONEV;
// variable valvar_newfuncset of type value
  momvalue_t momvar2 /*declvar:valvar_newfuncset*/ = MOM_NONEV;
  // 3 constants:
  // constant code_module
  momitem_t* momconst_1 /*const:code_module*/ = MOM_PREDEFINED_NAMED(code_module);
  // constant function_signature
  momitem_t* momconst_0 /*const:function_signature*/ = MOM_PREDEFINED_NAMED(function_signature);
  // constant functions
  momitem_t* momconst_2 /*const:functions*/ = MOM_PREDEFINED_NAMED(functions);
  // 1 closed:
  const momvalue_t momclosed_0 = mom_node_nth(mom_node, 0);
  goto momblocklab__9MARH5tjh_28MKpzxv3;

/// block #0: _4he2bnU93_5b5bkfLER
//: testmodule block of append_function_to_closed_module
 momblocklab__4he2bnU93_5b5bkfLER: {
// 2 statements in block _4he2bnU93_5b5bkfLER
// statement #0 :: _4cDEyFzEy_8frx4MEAH; if lkitm_module is a code_module ... in appfuntoclomod
// if testing on integer
    if (/*item_has_kind:*/momvar4 /*var:lkitm_module*/ && momvar4 /*var:lkitm_module*/->itm_kind == /*constant#1:*/MOM_PREDEFINED_NAMED(code_module))
      goto momblocklab__8xBt9K6Ch_2v6P4PEtw;
// statement #1 :: _20wqLcAtc_09ciRnJLc; otherwise -no itmvar_funsig- fail ... in appfuntoclomod
// fail
  momsuccess_append_function_to_closed_module = false;
  goto momepilog_append_function_to_closed_module;

  }; // end block _4he2bnU93_5b5bkfLER
  goto momepilog_append_function_to_closed_module;
 ////----

/// block #1: _8xBt9K6Ch_2v6P4PEtw
//: get in valvar_funcset functions block of append_function_to_closed_module
 momblocklab__8xBt9K6Ch_2v6P4PEtw: {
// 5 statements in block _8xBt9K6Ch_2v6P4PEtw
// statement #0 :: _8ePmCtD84_0yERvJzrB; get in valvar_funcset functions of module ... in appfuntoclomod
// set into valvar_funcset
  momvar1 /*var:valvar_funcset*/ =
   /*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar4 /*var:lkitm_module*/, /*constant#2:*/MOM_PREDEFINED_NAMED(functions));
// statement #1 :: _50yPKJrza_50B8EJ0LA; set valvar_newfuncset augmenting vavar_funcset ... in appfuntoclomod
// set into valvar_newfuncset
  momvar2 /*var:valvar_newfuncset*/ =
   /*collect_set:*/ mom_collect_setv( /*variadic-count count:*/5, /*variadic-rest restval:*/ /*rest#0*/(momvar1 /*var:valvar_funcset*/),  /*rest#1*/(/*value_of_item:*/ mom_itemv(momvar3 /*var:lkitm_fun*/)), NULL);
// statement #2 :: _2Pjxm7fux_60FU0zy46; put in lkitm_module attribute `functions` the valvar_newfuncset 
/*unsync_put_attribute:*/ (void)mom_item_unsync_put_attribute((momvar4 /*var:lkitm_module*/), ( /*constant#2:*/MOM_PREDEFINED_NAMED(functions)), (momvar2 /*var:valvar_newfuncset*/));// statement #3 :: _50yPKJtuB_0LhHHcP39; chunk inform about the updated module in appfuntoclomod
  // chunk of 12 components
MOM_INFORMPRINTF("updated functions of module %s from %s to %s", 
    mom_item_cstring(momvar4 /*var:lkitm_module*/), mom_output_gcstring(momvar1 /*var:valvar_funcset*/),  mom_output_gcstring(momvar2 /*var:valvar_newfuncset*/));
 ;
// statement #4 :: _6xfMIeeHE_66yBKz8u6; succeed in appfuntoclomod
// success
  momsuccess_append_function_to_closed_module = true;
  goto momepilog_append_function_to_closed_module;

  }; // end block _8xBt9K6Ch_2v6P4PEtw
  goto momepilog_append_function_to_closed_module;
 ////----

/// block #2: _9MARH5tjh_28MKpzxv3
//: start block of append_function_to_closed_module
 momblocklab__9MARH5tjh_28MKpzxv3: {
// 5 statements in block _9MARH5tjh_28MKpzxv3
// statement #0 :: _1mhPemeyd_4k5enfKC3; set lkitm_fun <- itm1 ... in appfuntoclomod
  { // locked set into lkitm_fun
  momlockeditem_t* momoldlocked__1mhPemeyd_4k5enfKC3 = momvar3 /*var:lkitm_fun*/;
  momlockeditem_t* momnewlocked__1mhPemeyd_4k5enfKC3 = momarg0 /*formalarg:itm1*/;
  if (momoldlocked__1mhPemeyd_4k5enfKC3 != momnewlocked__1mhPemeyd_4k5enfKC3) {
    if (momoldlocked__1mhPemeyd_4k5enfKC3 != NULL) mom_item_unlock (momoldlocked__1mhPemeyd_4k5enfKC3);
    if (momnewlocked__1mhPemeyd_4k5enfKC3 != NULL) mom_item_lock (momnewlocked__1mhPemeyd_4k5enfKC3);
  } // end lock test _1mhPemeyd_4k5enfKC3
    momvar3 /*var:lkitm_fun*/ = momnewlocked__1mhPemeyd_4k5enfKC3;
  momoldlocked__1mhPemeyd_4k5enfKC3 = NULL;
  momnewlocked__1mhPemeyd_4k5enfKC3 = NULL;
  } // end locked set into lkitm_fun
// statement #1 :: _5Icpjuev3_8xCdCtMdD; set lkitm_module <- val2 ..., closed in appfuntoclomod
  { // locked set into lkitm_module
  momlockeditem_t* momoldlocked__5Icpjuev3_8xCdCtMdD = momvar4 /*var:lkitm_module*/;
  momlockeditem_t* momnewlocked__5Icpjuev3_8xCdCtMdD = /*value_to_item:*/ mom_value_to_item( /*closed:val2*/momclosed_0);
  if (momoldlocked__5Icpjuev3_8xCdCtMdD != momnewlocked__5Icpjuev3_8xCdCtMdD) {
    if (momoldlocked__5Icpjuev3_8xCdCtMdD != NULL) mom_item_unlock (momoldlocked__5Icpjuev3_8xCdCtMdD);
    if (momnewlocked__5Icpjuev3_8xCdCtMdD != NULL) mom_item_lock (momnewlocked__5Icpjuev3_8xCdCtMdD);
  } // end lock test _5Icpjuev3_8xCdCtMdD
    momvar4 /*var:lkitm_module*/ = momnewlocked__5Icpjuev3_8xCdCtMdD;
  momoldlocked__5Icpjuev3_8xCdCtMdD = NULL;
  momnewlocked__5Icpjuev3_8xCdCtMdD = NULL;
  } // end locked set into lkitm_module
// statement #2 :: _5hCHeHf33_2y4IR7sHP; set itmvar_funsig <- get(lkitm_fun, function_signature) ... in appfuntoclomod
// set into itmvar_funsig
  momvar0 /*var:itmvar_funsig*/ =
   /*value_to_item:*/ mom_value_to_item(/*unsync_get_attribute:*/mom_item_unsync_get_attribute (momvar3 /*var:lkitm_fun*/, /*constant#0:*/MOM_PREDEFINED_NAMED(function_signature)));
// statement #3 :: _2ijiEhdMr_0E10qj6s8; if itmvar_funsig test-module ... in appfuntoclomod
// if testing on item
    if (momvar0 /*var:itmvar_funsig*/)
      goto momblocklab__4he2bnU93_5b5bkfLER;
// statement #4 :: _0t73aJCyK_6IRtxxHRn; otherwise -no funcset- fail ... in appfuntoclomod
// fail
  momsuccess_append_function_to_closed_module = false;
  goto momepilog_append_function_to_closed_module;

  }; // end block _9MARH5tjh_28MKpzxv3
  goto momepilog_append_function_to_closed_module;
 ////----

//////
// epilogue of append_function_to_closed_module
    momsuccess_append_function_to_closed_module = true;
    goto momepilog_append_function_to_closed_module;
 momepilog_append_function_to_closed_module:
   if (momvar3 != NULL) mom_item_unlock(momvar3);
   if (momvar4 != NULL) mom_item_unlock(momvar4);
// give 0 outputs
  return momsuccess_append_function_to_closed_module;
} // end of momfunc_1itm_to_void_append_function_to_closed_module 




/***** end 4 functions *****/



/***** old function addresses of module the_base_module *****/
static void* momoldad_agenda_push_back;
static void* momoldad_agenda_push_front;
static void* momoldad_agenda_step;
static void* momoldad_append_function_to_closed_module;


/***** loading constructor of module the_base_module *****/
static void momloadcons_the_base_module (void) __attribute__((constructor));
static void momloadcons_the_base_module (void)
{ // loading constructor
  MOM_DEBUGPRINTF(run, "loading constructor of the_base_module with 4 functions");
 // load constructor #0 for agenda_push_back
  momoldad_agenda_push_back
  = mom_dynload_function("agenda_push_back", "signature_1val_to_void", (void*) &momfunc_1val_to_void_agenda_push_back);
 // load constructor #1 for agenda_push_front
  momoldad_agenda_push_front
  = mom_dynload_function("agenda_push_front", "signature_1val_to_void", (void*) &momfunc_1val_to_void_agenda_push_front);
 // load constructor #2 for agenda_step
  momoldad_agenda_step
  = mom_dynload_function("agenda_step", "signature_void_to_void", (void*) &momfunc_void_to_void_agenda_step);
 // load constructor #3 for append_function_to_closed_module
  momoldad_append_function_to_closed_module
  = mom_dynload_function("append_function_to_closed_module", "signature_1itm_to_void", (void*) &momfunc_1itm_to_void_append_function_to_closed_module);

} // end loading constructor momloadcons_the_base_module



/***** unloading desstructor of module the_base_module *****/
static void momunloaddestr_the_base_module (void) __attribute__((destructor));
static void momunloaddestr_the_base_module (void)
{ // unloading destructor
  MOM_DEBUGPRINTF(run, "unloading destructor of the_base_module with 4 functions");
 // unload destructor #0 for agenda_push_back
  mom_dynunload_function ("agenda_push_back", "signature_1val_to_void", momoldad_agenda_push_back);
  momoldad_agenda_push_back = NULL;
 // unload destructor #1 for agenda_push_front
  mom_dynunload_function ("agenda_push_front", "signature_1val_to_void", momoldad_agenda_push_front);
  momoldad_agenda_push_front = NULL;
 // unload destructor #2 for agenda_step
  mom_dynunload_function ("agenda_step", "signature_void_to_void", momoldad_agenda_step);
  momoldad_agenda_step = NULL;
 // unload destructor #3 for append_function_to_closed_module
  mom_dynunload_function ("append_function_to_closed_module", "signature_1itm_to_void", momoldad_append_function_to_closed_module);
  momoldad_append_function_to_closed_module = NULL;

} // end unloading destructor momunloaddestr_the_base_module



//// end of generated module file momg_the_base_module.c

