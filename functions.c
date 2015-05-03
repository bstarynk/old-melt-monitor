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
  assert (clonode);
  assert (itm);
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
  assert (clonode);
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



bool
  momfun_1itm_to_val_emitter_of_function
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  momitem_t *itmkind = itm->itm_kind;
  assert (itmkind);
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_function itm=%s kind %s (of kind %s)",
		   mom_item_cstring (itm), mom_item_cstring (itmkind),
		   mom_item_cstring (itmkind->itm_kind));
  assert (itmkind->itm_kind == MOM_PREDEFINED_NAMED (function_signature));
  momvalue_t vclos = mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_function),
				    1,
				    mom_itemv (itmkind));
  MOM_DEBUGPRINTF (dump, "emitter_of_function vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_function */



bool
  momfun_1itm_to_void_filler_of_function
  (const momnode_t *clonode, momitem_t *itm)
{
  char bufnam[256];
  memset (bufnam, 0, sizeof (bufnam));
  MOM_DEBUGPRINTF (dump, "filler_of_function itm=%s", mom_item_cstring (itm));
  if (!clonode || clonode->slen < 1)
    MOM_FATAPRINTF ("filler_of_function %s has bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));
  momitem_t *itmsig = mom_value_to_item (clonode->arrsons[0]);
  if (!itmsig
      || itmsig->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    MOM_FATAPRINTF ("filler_of_function %s has bad closed signature %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (clonode->arrsons[0]));
  momvalue_t cfunprefv = MOM_NONEV;
  {
    mom_item_lock (itmsig);
    cfunprefv =
      mom_item_unsync_get_attribute (itmsig,
				     MOM_PREDEFINED_NAMED
				     (c_function_prefix));
    mom_item_unlock (itmsig);
  }
  if (cfunprefv.typnum != momty_string)
    MOM_FATAPRINTF
      ("filler_of_function %s with kind %s and bad `c_function_prefix` %s",
       mom_item_cstring (itm), mom_item_cstring (itmsig),
       mom_output_gcstring (cfunprefv));
  if (snprintf
      (bufnam, sizeof (bufnam), "%s_%s", mom_value_cstr (cfunprefv),
       mom_item_cstring (itm)) >= (int) sizeof (bufnam))
    MOM_FATAPRINTF ("filler_of_function %s with kind %s too long name %s",
		    mom_item_cstring (itm), mom_item_cstring (itmsig),
		    bufnam);
  void *adfun = mom_dynload_symbol (bufnam);
  if (!adfun)
    MOM_FATAPRINTF
      ("filler_of_function %s with kind %s failed to find C function %s",
       mom_item_cstring (itm), mom_item_cstring (itmsig), bufnam);
  {
    mom_item_lock (itm);
    itm->itm_kind = itmsig;
    itm->itm_data1 = adfun;
    mom_item_unlock (itm);
  }
  MOM_DEBUGPRINTF (load, "filler_of_function %s done kind %s function %s @%p",
		   mom_item_cstring (itm), mom_item_cstring (itmsig), bufnam,
		   adfun);
  return true;
}				/* end of filler_of_function */

bool
  momfun_1itm_to_val_emitter_of_plain_kind
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  momitem_t *itmclokind = NULL;
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_plain_kind itm=%s", mom_item_cstring (itm));
  if (!clonode || clonode->slen == 0
      || !(itmclokind = mom_value_to_item (clonode->arrsons[0])))
    MOM_FATAPRINTF ("emitter_of_plain_kind itm=%s bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));

  momvalue_t vclos =
    mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_plain_kind),
		   1, mom_itemv (itmclokind));
  MOM_DEBUGPRINTF (dump, "emitter_of_plain_kind vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_plain_kind */

bool
  momfun_1itm_to_void_filler_of_plain_kind
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_plain_kind itm=%s", mom_item_cstring (itm));
  if (!clonode || clonode->slen < 1)
    MOM_FATAPRINTF ("filler_of_plain_kind %s has bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));
  momitem_t *itmkind = mom_value_to_item (clonode->arrsons[0]);
  if (!itmkind
      || itmkind->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    MOM_FATAPRINTF ("filler_of_plain_kind %s has bad kind %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (clonode->arrsons[0]));
  assert (!itm->itm_kind || itm->itm_kind == itmkind);
  {
    mom_item_lock (itm);
    itm->itm_kind = itmkind;
    itm->itm_data1 = NULL;
    itm->itm_data2 = NULL;
    mom_item_unlock (itm);
  }
  MOM_DEBUGPRINTF (load, "filler_of_plain_kind %s done kind %s",
		   mom_item_cstring (itm), mom_item_cstring (itmkind));
  return true;

}
