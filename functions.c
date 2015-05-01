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
  momvalue_t vclos =
    mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_magic_attribute),
		   2,
		   mom_nodev ((momnode_t *) itm->itm_data1),
		   mom_nodev ((momnode_t *) itm->itm_data2));
  MOM_DEBUGPRINTF (dump, "emitter_of_magic_attribute vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_magic_attribute */




bool
  momfun_1itm_to_void_scanner_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (magic_attribute));
  momvalue_t valgetclos = mom_nodev ((momnode_t *) itm->itm_data1);
  momvalue_t valputclos = mom_nodev ((momnode_t *) itm->itm_data2);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute itm=%s valgetclos=%s valputclos=%s",
		   mom_item_cstring (itm), mom_output_gcstring (valgetclos),
		   mom_output_gcstring (valputclos));
  mom_scan_dumped_value (valgetclos);
  mom_scan_dumped_value (valputclos);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute end itm=%s",
		   mom_item_cstring (itm));
  return true;
}				/* end scanner_of_magic_attribute */


bool
  momfun_1itm_to_void_filler_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  if (!clonode || clonode->slen < 2)
    MOM_FATAPRINTF ("filler_of_magic_attribute %s has bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));
  momvalue_t vgetclos = clonode->arrsons[0];
  momvalue_t vputclos = clonode->arrsons[1];
  if (vgetclos.typnum != momty_node)
    MOM_FATAPRINTF ("filler_of_magic_attribute %s has bad getter %s",
		    mom_item_cstring (itm), mom_output_gcstring (vgetclos));
  if (vputclos.typnum != momty_node)
    MOM_FATAPRINTF ("filler_of_magic_attribute %s has bad getter %s",
		    mom_item_cstring (itm), mom_output_gcstring (vputclos));
  itm->itm_kind = MOM_PREDEFINED_NAMED (magic_attribute);
  itm->itm_data1 = (void *) vgetclos.vnode;
  itm->itm_data2 = (void *) vputclos.vnode;
  MOM_DEBUGPRINTF (dump,
		   "filler_of_magic_attribute itm=%s done vgetclos=%s vputclos=%s",
		   mom_item_cstring (itm),
		   mom_output_gcstring (vgetclos),
		   mom_output_gcstring (vputclos));
  return true;
}				/* end of filler_of_magic_attribute */
