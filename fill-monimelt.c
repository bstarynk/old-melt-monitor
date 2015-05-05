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
// item emitter_of_association of kind signature_1itm_to_val
  MOM_PREDEFINED_NAMED (emitter_of_association)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_val);
// item emitter_of_function of kind signature_1itm_to_val
  MOM_PREDEFINED_NAMED (emitter_of_function)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_val);
// item emitter_of_magic_attribute of kind signature_1itm_to_val
  MOM_PREDEFINED_NAMED (emitter_of_magic_attribute)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_val);
// item emitter_of_plain_kind of kind signature_1itm_to_val
  MOM_PREDEFINED_NAMED (emitter_of_plain_kind)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_val);
// item filler_of_association of kind signature_1itm_to_void
  MOM_PREDEFINED_NAMED (filler_of_association)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
// item filler_of_function of kind signature_1itm_to_void
  MOM_PREDEFINED_NAMED (filler_of_function)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
// item filler_of_magic_attribute of kind signature_1itm_to_void
  MOM_PREDEFINED_NAMED (filler_of_magic_attribute)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
// item filler_of_plain_kind of kind signature_1itm_to_void
  MOM_PREDEFINED_NAMED (filler_of_plain_kind)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
// item generate_c_module of kind signature_1itm_to_val
  MOM_PREDEFINED_NAMED (generate_c_module)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_val);
// item integer of kind c_type
  MOM_PREDEFINED_NAMED (integer)->itm_kind = MOM_PREDEFINED_NAMED (c_type);
// item item of kind c_type
  MOM_PREDEFINED_NAMED (item)->itm_kind = MOM_PREDEFINED_NAMED (c_type);
// item scanner_of_association of kind signature_1itm_to_void
  MOM_PREDEFINED_NAMED (scanner_of_association)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
// item scanner_of_magic_attribute of kind signature_1itm_to_void
  MOM_PREDEFINED_NAMED (scanner_of_magic_attribute)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
// item signature_1itm_to_val of kind function_signature
  MOM_PREDEFINED_NAMED (signature_1itm_to_val)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
// item signature_1itm_to_void of kind function_signature
  MOM_PREDEFINED_NAMED (signature_1itm_to_void)->itm_kind
    = MOM_PREDEFINED_NAMED (function_signature);
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
// item value of kind c_type
  MOM_PREDEFINED_NAMED (value)->itm_kind = MOM_PREDEFINED_NAMED (c_type);
// function item emitter_of_association of signature_1itm_to_val:
  MOM_PREDEFINED_NAMED (emitter_of_association)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_val_emitter_of_association");
// function item emitter_of_function of signature_1itm_to_val:
  MOM_PREDEFINED_NAMED (emitter_of_function)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_val_emitter_of_function");
// function item emitter_of_magic_attribute of signature_1itm_to_val:
  MOM_PREDEFINED_NAMED (emitter_of_magic_attribute)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_val_emitter_of_magic_attribute");
// function item emitter_of_plain_kind of signature_1itm_to_val:
  MOM_PREDEFINED_NAMED (emitter_of_plain_kind)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_val_emitter_of_plain_kind");
// function item filler_of_association of signature_1itm_to_void:
  MOM_PREDEFINED_NAMED (filler_of_association)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_filler_of_association");
// function item filler_of_function of signature_1itm_to_void:
  MOM_PREDEFINED_NAMED (filler_of_function)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_filler_of_function");
// function item filler_of_magic_attribute of signature_1itm_to_void:
  MOM_PREDEFINED_NAMED (filler_of_magic_attribute)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_filler_of_magic_attribute");
// function item filler_of_plain_kind of signature_1itm_to_void:
  MOM_PREDEFINED_NAMED (filler_of_plain_kind)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_filler_of_plain_kind");
// function item generate_c_module of signature_1itm_to_val:
  MOM_PREDEFINED_NAMED (generate_c_module)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_val_generate_c_module");
// function item scanner_of_association of signature_1itm_to_void:
  MOM_PREDEFINED_NAMED (scanner_of_association)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_scanner_of_association");
// function item scanner_of_magic_attribute of signature_1itm_to_void:
  MOM_PREDEFINED_NAMED (scanner_of_magic_attribute)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_scanner_of_magic_attribute");

}				/* end mom_predefined_items_fill */

 // end of generated file fill-monimelt.c
