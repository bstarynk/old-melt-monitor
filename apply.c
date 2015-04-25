// file apply.c - manage function application

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
mom_applyclos_void_to_void (const momnode_t *closnode)
{
  bool ok = false;
  if (!closnode)
    return false;
  momitem_t *connitm = (momitem_t *) closnode->conn;
  assert (connitm);
  mom_item_lock (connitm);
  if (MOM_UNLIKELY ((const momitem_t *) connitm->itm_kind
		    != MOM_PREDEFINED_NAMED (signature_1val_to_void)))
    goto end;
  void *data1 = connitm->itm_data1;
  if (MOM_UNLIKELY (data1 == NULL))
    {
      char nambuf[256];
      memset (nambuf, 0, sizeof (nambuf));
      if (snprintf (nambuf, sizeof (nambuf), MOM_PREFIXFUN_void_to_void "_%s",
		    connitm->itm_str->cstr) < (int) sizeof (nambuf))
	((momitem_t *) connitm)->itm_data1 = data1 =
	  mom_dynload_symbol (nambuf);
      else
	MOM_FATAPRINTF ("too long function name %s for 1val to void",
			connitm->itm_str->cstr);
    }
  if (MOM_LIKELY (data1 != NULL && data1 != MOM_EMPTY))
    {
      mom_void_to_void_sig_t *fun = (mom_void_to_void_sig_t *) data1;
      ok = (*fun) (closnode);
    }
end:
  mom_item_unlock (connitm);
  return ok;
}

bool
mom_applyclos_1val_to_void (const momnode_t *closnode, const momvalue_t arg0)
{
  bool ok = false;
  if (!closnode)
    return false;
  momitem_t *connitm = (momitem_t *) closnode->conn;
  assert (connitm);
  mom_item_lock (connitm);
  if (MOM_UNLIKELY ((const momitem_t *) connitm->itm_kind
		    != MOM_PREDEFINED_NAMED (signature_1val_to_void)))
    goto end;
  void *data1 = connitm->itm_data1;
  if (MOM_UNLIKELY (data1 == NULL))
    {
      char nambuf[256];
      memset (nambuf, 0, sizeof (nambuf));
      if (snprintf (nambuf, sizeof (nambuf), MOM_PREFIXFUN_1val_to_void "_%s",
		    connitm->itm_str->cstr) < (int) sizeof (nambuf))
	((momitem_t *) connitm)->itm_data1 = data1 =
	  mom_dynload_symbol (nambuf);
      else
	MOM_FATAPRINTF ("too long function name %s for 1val to void",
			connitm->itm_str->cstr);
    }
  if (MOM_LIKELY (data1 != NULL && data1 != MOM_EMPTY))
    {
      mom_1val_to_void_sig_t *fun = (mom_1val_to_void_sig_t *) data1;
      ok = (*fun) (closnode, arg0);
    }
end:
  mom_item_unlock (connitm);
  return ok;
}

bool
mom_applyclos_1val_to_val (const momnode_t *closnode, const momvalue_t arg0,
			   momvalue_t *resptr)
{
  bool ok = false;
  if (!closnode)
    return false;
  assert (resptr);
  *resptr = MOM_NONEV;
  momitem_t *connitm = (momitem_t *) closnode->conn;
  assert (connitm);
  mom_item_lock (connitm);
  if (MOM_UNLIKELY ((const momitem_t *) connitm->itm_kind
		    != MOM_PREDEFINED_NAMED (signature_1val_to_val)))
    goto end;
  void *data1 = connitm->itm_data1;
  if (MOM_UNLIKELY (data1 == NULL))
    {
      char nambuf[256];
      memset (nambuf, 0, sizeof (nambuf));
      if (snprintf (nambuf, sizeof (nambuf), MOM_PREFIXFUN_1val_to_val "_%s",
		    connitm->itm_str->cstr) < (int) sizeof (nambuf))
	((momitem_t *) connitm)->itm_data1 = data1 =
	  mom_dynload_symbol (nambuf);
      else
	MOM_FATAPRINTF ("too long function name %s for 1val to val",
			connitm->itm_str->cstr);
    }
  if (MOM_LIKELY (data1 != NULL && data1 != MOM_EMPTY))
    {
      mom_1val_to_val_sig_t *fun = (mom_1val_to_val_sig_t *) data1;
      ok = (*fun) (closnode, arg0, resptr);
    }
end:
  mom_item_unlock (connitm);
  return ok;
}
