// file functions.c

/**   Copyright (C)  2015 Free Software Foundation, Inc.
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "monimelt.h"

bool
  momfun_1itm_to_val_emitter_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (magic_attribute));
#warning should make a node of filler_of_magic_attribute
}				/* end emitter_of_magic_attribute */




bool
  momfun_1itm_to_void_scanner_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (magic_attribute));
  momvalue_t valgetclos = mom_nodev (itm->itm_data1);
  momvalue_t valputclos = mom_nodev (itm->itm_data2);
  mom_scan_dumped_value (valgetclos);
  mom_scan_dumped_value (valputclos);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute end itm=%s",
		   mom_item_cstring (itm));
  return true;
}				/* end scanner_of_magic_attribute */
