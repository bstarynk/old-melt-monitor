/// *** generated file fill-monimelt.c - DO NOT EDIT ***
/// Copyright (C) 2015 Free Software Foundation, Inc. ***
/// MONIMELT is a monitor for MELT - see http://gcc-melt.org/ ***
/// This generated file fill-monimelt.c is part of MONIMELT, part of GCC ***
///***
/// GCC is free software; you can redistribute it and/or modify ***
/// it under the terms of the GNU General Public License as published by ***
/// the Free Software Foundation; either version 3, or (at your option) ***
/// any later version. ***
///***
///  GCC is distributed in the hope that it will be useful, ***
///  but WITHOUT ANY WARRANTY; without even the implied warranty of ***
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the ***
///  GNU General Public License for more details. ***
///  You should have received a copy of the GNU General Public License ***
///  along with GCC; see the file COPYING3.   If not see ***
///  <http://www.gnu.org/licenses/>. ***
///***

#include "monimelt.h"

void
mom_predefined_items_fill (void)
{
  //// assign predefined kinds
// item signature_1val_to_val of kind function_signature
  MOM_PREDEFINED_NAMED (signature_1val_to_val)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_1val_to_void of kind function_signature
  MOM_PREDEFINED_NAMED (signature_1val_to_void)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_2itm1val_to_val of kind function_signature
  MOM_PREDEFINED_NAMED (signature_2itm1val_to_val)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_2itm1val_to_void of kind function_signature
  MOM_PREDEFINED_NAMED (signature_2itm1val_to_void)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_2itm_to_val of kind function_signature
  MOM_PREDEFINED_NAMED (signature_2itm_to_val)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_2itm_to_void of kind function_signature
  MOM_PREDEFINED_NAMED (signature_2itm_to_void)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_void_to_void of kind function_signature
  MOM_PREDEFINED_NAMED (signature_void_to_void)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);

}				/* end mom_predefined_items_fill */

 // end of generated file fill-monimelt.c
