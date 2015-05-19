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
      || mom_unsync_item_components_count (mom_funcitm)<1
       ))
  return false;
  // 0 output results:
  // 0 variables:
  // 1 constants:
  // constant the_agenda
  const momvalue_t momconst_0 =  mom_itemv(MOM_PREDEFINED_NAMED(the_agenda));
  // 0 closed:
  goto momblocklab__9rwzfFAJ2_0J3c749dM;
 // block #0 : _9rwzfFAJ2_0J3c749dM
 momblocklab__9rwzfFAJ2_0J3c749dM: {
// 0 statements in block _9rwzfFAJ2_0J3c749dM

  }; // end block _9rwzfFAJ2_0J3c749dM
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

