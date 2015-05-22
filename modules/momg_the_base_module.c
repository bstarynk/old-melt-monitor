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



/***** declaring 3 functions *****/


/// declare function #0: agenda_push_back
extern bool momfunc_1val_to_void_agenda_push_back (const momnode_t *, momvalue_t);


/// declare function #1: agenda_push_front
extern bool momfunc_1val_to_void_agenda_push_front (const momnode_t *, momvalue_t);


/// declare function #2: agenda_step
extern bool momfunc_void_to_void_agenda_step (const momnode_t *);


/***** implementing 3 functions *****/


/// implement function #0: agenda_push_back
bool momfunc_1val_to_void_agenda_push_back (const momnode_t *mom_node, momvalue_t momarg0)
{ // body of function agenda_push_back
  bool momsuccess_agenda_push_back = false;
  momitem_t* mom_funcitm = NULL;
  if (MOM_UNLIKELY(!mom_node
      || !(mom_funcitm = mom_node_conn (mom_node))
      || mom_unsync_item_components_count (mom_funcitm)<3
       ))
  return false;
  // 0 output results:
  // 2 variables:
// variable lkitm_agenda of type locked_item
  momlockeditem_t* momvar0 = (momlockeditem_t*)0;
// variable lkitm_tasklet of type locked_item
  momlockeditem_t* momvar1 = (momlockeditem_t*)0;
  // 3 constants:
  // constant item_queue
  momitem_t* momconst_0 = MOM_PREDEFINED_NAMED(item_queue);
  // constant tasklet
  momitem_t* momconst_1 = MOM_PREDEFINED_NAMED(tasklet);
  // constant the_agenda
  momitem_t* momconst_2 = MOM_PREDEFINED_NAMED(the_agenda);
  // 0 closed:
  goto momblocklab__0y7wu372C_3JH4an3L2;

/// block #0: _0y7wu372C_3JH4an3L2
//: start block of agenda_push_back
 momblocklab__0y7wu372C_3JH4an3L2: {
// 4 statements in block _0y7wu372C_3JH4an3L2
// statement #0 :: _2ur1uJ52w_1RmsKK5vb; set lkitm_agenda := the_agenda; in agenda_push_back
  { // locked set into lkitm_agenda
  momlockeditem_t* momoldlocked__2ur1uJ52w_1RmsKK5vb = momvar0 /*var:lkitm_agenda*/;
  momlockeditem_t* momnewlocked__2ur1uJ52w_1RmsKK5vb =  /*constant#2:*/MOM_PREDEFINED_NAMED(the_agenda);
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
// 0 statements in block _27PBmP9xP_3Hh1aedqr

  }; // end block _27PBmP9xP_3Hh1aedqr
////----++++

/// block #2: _50yPKJAK2_4Ksihqd4c
//: block to push lkitm_tasklet in back of agenda and notify; in agenda_push_back
 momblocklab__50yPKJAK2_4Ksihqd4c: {
// 3 statements in block _50yPKJAK2_4Ksihqd4c
// statement #0 :: _24sBAv42k_3Ly5hw0vj; push lkitm_tasklet at back of lkitm_agenda; in agenda_push_back
/*push_back_queue_item:*/

  }; // end block _50yPKJAK2_4Ksihqd4c
  goto momepilog_agenda_push_back;
 ////----

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




/***** end 3 functions *****/


//// end of generated module file momg_the_base_module.c

