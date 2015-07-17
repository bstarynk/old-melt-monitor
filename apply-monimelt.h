/// *** generated file apply-monimelt.h - DO NOT EDIT ***
/// Copyright (C) 2015 Free Software Foundation, Inc. ***
/// MONIMELT is a monitor for MELT - see http://gcc-melt.org/ ***
/// This generated file apply-monimelt.h is part of MONIMELT, part of GCC ***
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


// signature application support for signature_1itm1int_to_item
typedef bool mom_1itm1int_to_item_sig_t (const momnode_t*nod_mom /* 2 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		 intptr_t arg1_mom,
		  momitem_t** res0_mom);


#define MOM_PREFIXFUN_1itm1int_to_item "momfunc_1itm1int_to_item"
static inline mom_1itm1int_to_item_sig_t mom_applclos_1itm1int_to_item;
static inline bool mom_applval_1itm1int_to_item(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 intptr_t arg1_mom,
		  momitem_t** res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm1int_to_item (clo_mom.vnode, arg0_mom, arg1_mom, res0_mom);
} // end of mom_applval_1itm1int_to_item 

static inline bool
mom_applclos_1itm1int_to_item(const momnode_t* nod_mom, momitem_t* arg0_mom, intptr_t arg1_mom,  momitem_t** res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm1int_to_item))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm1int_to_item__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm1int_to_item",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm1int_to_item_sig_t* fun_mom = (mom_1itm1int_to_item_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm1int_to_item




// signature application support for signature_1itm1val1int_to_void
typedef bool mom_1itm1val1int_to_void_sig_t (const momnode_t*nod_mom /* 3 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		 intptr_t arg2_mom);


#define MOM_PREFIXFUN_1itm1val1int_to_void "momfunc_1itm1val1int_to_void"
static inline mom_1itm1val1int_to_void_sig_t mom_applclos_1itm1val1int_to_void;
static inline bool mom_applval_1itm1val1int_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		 intptr_t arg2_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm1val1int_to_void (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom);
} // end of mom_applval_1itm1val1int_to_void 

static inline bool
mom_applclos_1itm1val1int_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momvalue_t arg1_mom, intptr_t arg2_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm1val1int_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm1val1int_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm1val1int_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm1val1int_to_void_sig_t* fun_mom = (mom_1itm1val1int_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm1val1int_to_void




// signature application support for signature_1itm1val_to_item
typedef bool mom_1itm1val_to_item_sig_t (const momnode_t*nod_mom /* 2 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		  momitem_t** res0_mom);


#define MOM_PREFIXFUN_1itm1val_to_item "momfunc_1itm1val_to_item"
static inline mom_1itm1val_to_item_sig_t mom_applclos_1itm1val_to_item;
static inline bool mom_applval_1itm1val_to_item(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		  momitem_t** res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm1val_to_item (clo_mom.vnode, arg0_mom, arg1_mom, res0_mom);
} // end of mom_applval_1itm1val_to_item 

static inline bool
mom_applclos_1itm1val_to_item(const momnode_t* nod_mom, momitem_t* arg0_mom, momvalue_t arg1_mom,  momitem_t** res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm1val_to_item))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm1val_to_item__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm1val_to_item",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm1val_to_item_sig_t* fun_mom = (mom_1itm1val_to_item_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm1val_to_item




// signature application support for signature_1itm1val_to_void
typedef bool mom_1itm1val_to_void_sig_t (const momnode_t*nod_mom /* 2 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom);


#define MOM_PREFIXFUN_1itm1val_to_void "momfunc_1itm1val_to_void"
static inline mom_1itm1val_to_void_sig_t mom_applclos_1itm1val_to_void;
static inline bool mom_applval_1itm1val_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm1val_to_void (clo_mom.vnode, arg0_mom, arg1_mom);
} // end of mom_applval_1itm1val_to_void 

static inline bool
mom_applclos_1itm1val_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momvalue_t arg1_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm1val_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm1val_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm1val_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm1val_to_void_sig_t* fun_mom = (mom_1itm1val_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm1val_to_void




// signature application support for signature_1itm2val_to_item
typedef bool mom_1itm2val_to_item_sig_t (const momnode_t*nod_mom /* 3 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		 momvalue_t arg2_mom,
		  momitem_t** res0_mom);


#define MOM_PREFIXFUN_1itm2val_to_item "momfunc_1itm2val_to_item"
static inline mom_1itm2val_to_item_sig_t mom_applclos_1itm2val_to_item;
static inline bool mom_applval_1itm2val_to_item(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		 momvalue_t arg2_mom,
		  momitem_t** res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm2val_to_item (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom, res0_mom);
} // end of mom_applval_1itm2val_to_item 

static inline bool
mom_applclos_1itm2val_to_item(const momnode_t* nod_mom, momitem_t* arg0_mom, momvalue_t arg1_mom, momvalue_t arg2_mom,  momitem_t** res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm2val_to_item))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm2val_to_item__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm2val_to_item",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm2val_to_item_sig_t* fun_mom = (mom_1itm2val_to_item_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm2val_to_item




// signature application support for signature_1itm2val_to_void
typedef bool mom_1itm2val_to_void_sig_t (const momnode_t*nod_mom /* 3 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		 momvalue_t arg2_mom);


#define MOM_PREFIXFUN_1itm2val_to_void "momfunc_1itm2val_to_void"
static inline mom_1itm2val_to_void_sig_t mom_applclos_1itm2val_to_void;
static inline bool mom_applval_1itm2val_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momvalue_t arg1_mom,
		 momvalue_t arg2_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm2val_to_void (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom);
} // end of mom_applval_1itm2val_to_void 

static inline bool
mom_applclos_1itm2val_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momvalue_t arg1_mom, momvalue_t arg2_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm2val_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm2val_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm2val_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm2val_to_void_sig_t* fun_mom = (mom_1itm2val_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm2val_to_void




// signature application support for signature_1itm_to_item
typedef bool mom_1itm_to_item_sig_t (const momnode_t*nod_mom /* 1 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		  momitem_t** res0_mom);


#define MOM_PREFIXFUN_1itm_to_item "momfunc_1itm_to_item"
static inline mom_1itm_to_item_sig_t mom_applclos_1itm_to_item;
static inline bool mom_applval_1itm_to_item(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		  momitem_t** res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm_to_item (clo_mom.vnode, arg0_mom, res0_mom);
} // end of mom_applval_1itm_to_item 

static inline bool
mom_applclos_1itm_to_item(const momnode_t* nod_mom, momitem_t* arg0_mom,  momitem_t** res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm_to_item))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm_to_item__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm_to_item",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm_to_item_sig_t* fun_mom = (mom_1itm_to_item_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm_to_item




// signature application support for signature_1itm_to_val
typedef bool mom_1itm_to_val_sig_t (const momnode_t*nod_mom /* 1 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		  momvalue_t* res0_mom);


#define MOM_PREFIXFUN_1itm_to_val "momfunc_1itm_to_val"
static inline mom_1itm_to_val_sig_t mom_applclos_1itm_to_val;
static inline bool mom_applval_1itm_to_val(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		  momvalue_t* res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm_to_val (clo_mom.vnode, arg0_mom, res0_mom);
} // end of mom_applval_1itm_to_val 

static inline bool
mom_applclos_1itm_to_val(const momnode_t* nod_mom, momitem_t* arg0_mom,  momvalue_t* res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm_to_val))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm_to_val__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm_to_val",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm_to_val_sig_t* fun_mom = (mom_1itm_to_val_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm_to_val




// signature application support for signature_1itm_to_void
typedef bool mom_1itm_to_void_sig_t (const momnode_t*nod_mom /* 1 inputs, 0 outputs: */,
		 momitem_t* arg0_mom);


#define MOM_PREFIXFUN_1itm_to_void "momfunc_1itm_to_void"
static inline mom_1itm_to_void_sig_t mom_applclos_1itm_to_void;
static inline bool mom_applval_1itm_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1itm_to_void (clo_mom.vnode, arg0_mom);
} // end of mom_applval_1itm_to_void 

static inline bool
mom_applclos_1itm_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1itm_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1itm_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1itm_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1itm_to_void_sig_t* fun_mom = (mom_1itm_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1itm_to_void




// signature application support for signature_1val1int_to_void
typedef bool mom_1val1int_to_void_sig_t (const momnode_t*nod_mom /* 2 inputs, 0 outputs: */,
		 momvalue_t arg0_mom,
		 intptr_t arg1_mom);


#define MOM_PREFIXFUN_1val1int_to_void "momfunc_1val1int_to_void"
static inline mom_1val1int_to_void_sig_t mom_applclos_1val1int_to_void;
static inline bool mom_applval_1val1int_to_void(const momvalue_t clo_mom,
		 momvalue_t arg0_mom,
		 intptr_t arg1_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1val1int_to_void (clo_mom.vnode, arg0_mom, arg1_mom);
} // end of mom_applval_1val1int_to_void 

static inline bool
mom_applclos_1val1int_to_void(const momnode_t* nod_mom, momvalue_t arg0_mom, intptr_t arg1_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1val1int_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1val1int_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1val1int_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1val1int_to_void_sig_t* fun_mom = (mom_1val1int_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1val1int_to_void




// signature application support for signature_1val_to_val
typedef bool mom_1val_to_val_sig_t (const momnode_t*nod_mom /* 1 inputs, 1 outputs: */,
		 momvalue_t arg0_mom,
		  momvalue_t* res0_mom);


#define MOM_PREFIXFUN_1val_to_val "momfunc_1val_to_val"
static inline mom_1val_to_val_sig_t mom_applclos_1val_to_val;
static inline bool mom_applval_1val_to_val(const momvalue_t clo_mom,
		 momvalue_t arg0_mom,
		  momvalue_t* res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1val_to_val (clo_mom.vnode, arg0_mom, res0_mom);
} // end of mom_applval_1val_to_val 

static inline bool
mom_applclos_1val_to_val(const momnode_t* nod_mom, momvalue_t arg0_mom,  momvalue_t* res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1val_to_val))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1val_to_val__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1val_to_val",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1val_to_val_sig_t* fun_mom = (mom_1val_to_val_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1val_to_val




// signature application support for signature_1val_to_void
typedef bool mom_1val_to_void_sig_t (const momnode_t*nod_mom /* 1 inputs, 0 outputs: */,
		 momvalue_t arg0_mom);


#define MOM_PREFIXFUN_1val_to_void "momfunc_1val_to_void"
static inline mom_1val_to_void_sig_t mom_applclos_1val_to_void;
static inline bool mom_applval_1val_to_void(const momvalue_t clo_mom,
		 momvalue_t arg0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_1val_to_void (clo_mom.vnode, arg0_mom);
} // end of mom_applval_1val_to_void 

static inline bool
mom_applclos_1val_to_void(const momnode_t* nod_mom, momvalue_t arg0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_1val_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_1val_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_1val_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_1val_to_void_sig_t* fun_mom = (mom_1val_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_1val_to_void




// signature application support for signature_2itm1int_to_item
typedef bool mom_2itm1int_to_item_sig_t (const momnode_t*nod_mom /* 3 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 intptr_t arg2_mom,
		  momvalue_t* res0_mom);


#define MOM_PREFIXFUN_2itm1int_to_item "momfunc_2itm1int_to_item"
static inline mom_2itm1int_to_item_sig_t mom_applclos_2itm1int_to_item;
static inline bool mom_applval_2itm1int_to_item(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 intptr_t arg2_mom,
		  momvalue_t* res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm1int_to_item (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom, res0_mom);
} // end of mom_applval_2itm1int_to_item 

static inline bool
mom_applclos_2itm1int_to_item(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom, intptr_t arg2_mom,  momvalue_t* res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm1int_to_item))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm1int_to_item__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm1int_to_item",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm1int_to_item_sig_t* fun_mom = (mom_2itm1int_to_item_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm1int_to_item




// signature application support for signature_2itm1int_to_void
typedef bool mom_2itm1int_to_void_sig_t (const momnode_t*nod_mom /* 3 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 intptr_t arg2_mom);


#define MOM_PREFIXFUN_2itm1int_to_void "momfunc_2itm1int_to_void"
static inline mom_2itm1int_to_void_sig_t mom_applclos_2itm1int_to_void;
static inline bool mom_applval_2itm1int_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 intptr_t arg2_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm1int_to_void (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom);
} // end of mom_applval_2itm1int_to_void 

static inline bool
mom_applclos_2itm1int_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom, intptr_t arg2_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm1int_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm1int_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm1int_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm1int_to_void_sig_t* fun_mom = (mom_2itm1int_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm1int_to_void




// signature application support for signature_2itm1val_to_val
typedef bool mom_2itm1val_to_val_sig_t (const momnode_t*nod_mom /* 3 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 momvalue_t arg2_mom,
		  momvalue_t* res0_mom);


#define MOM_PREFIXFUN_2itm1val_to_val "momfunc_2itm1val_to_val"
static inline mom_2itm1val_to_val_sig_t mom_applclos_2itm1val_to_val;
static inline bool mom_applval_2itm1val_to_val(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 momvalue_t arg2_mom,
		  momvalue_t* res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm1val_to_val (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom, res0_mom);
} // end of mom_applval_2itm1val_to_val 

static inline bool
mom_applclos_2itm1val_to_val(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom, momvalue_t arg2_mom,  momvalue_t* res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm1val_to_val))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm1val_to_val__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm1val_to_val",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm1val_to_val_sig_t* fun_mom = (mom_2itm1val_to_val_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm1val_to_val




// signature application support for signature_2itm1val_to_void
typedef bool mom_2itm1val_to_void_sig_t (const momnode_t*nod_mom /* 3 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 momvalue_t arg2_mom);


#define MOM_PREFIXFUN_2itm1val_to_void "momfunc_2itm1val_to_void"
static inline mom_2itm1val_to_void_sig_t mom_applclos_2itm1val_to_void;
static inline bool mom_applval_2itm1val_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 momvalue_t arg2_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm1val_to_void (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom);
} // end of mom_applval_2itm1val_to_void 

static inline bool
mom_applclos_2itm1val_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom, momvalue_t arg2_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm1val_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm1val_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm1val_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm1val_to_void_sig_t* fun_mom = (mom_2itm1val_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm1val_to_void




// signature application support for signature_2itm2val_to_void
typedef bool mom_2itm2val_to_void_sig_t (const momnode_t*nod_mom /* 4 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 momvalue_t arg2_mom,
		 momvalue_t arg3_mom);


#define MOM_PREFIXFUN_2itm2val_to_void "momfunc_2itm2val_to_void"
static inline mom_2itm2val_to_void_sig_t mom_applclos_2itm2val_to_void;
static inline bool mom_applval_2itm2val_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		 momvalue_t arg2_mom,
		 momvalue_t arg3_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm2val_to_void (clo_mom.vnode, arg0_mom, arg1_mom, arg2_mom, arg3_mom);
} // end of mom_applval_2itm2val_to_void 

static inline bool
mom_applclos_2itm2val_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom, momvalue_t arg2_mom, momvalue_t arg3_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm2val_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm2val_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm2val_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm2val_to_void_sig_t* fun_mom = (mom_2itm2val_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, arg2_mom, arg3_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm2val_to_void




// signature application support for signature_2itm_to_val
typedef bool mom_2itm_to_val_sig_t (const momnode_t*nod_mom /* 2 inputs, 1 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		  momvalue_t* res0_mom);


#define MOM_PREFIXFUN_2itm_to_val "momfunc_2itm_to_val"
static inline mom_2itm_to_val_sig_t mom_applclos_2itm_to_val;
static inline bool mom_applval_2itm_to_val(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom,
		  momvalue_t* res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm_to_val (clo_mom.vnode, arg0_mom, arg1_mom, res0_mom);
} // end of mom_applval_2itm_to_val 

static inline bool
mom_applclos_2itm_to_val(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom,  momvalue_t* res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm_to_val))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm_to_val__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm_to_val",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm_to_val_sig_t* fun_mom = (mom_2itm_to_val_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm_to_val




// signature application support for signature_2itm_to_void
typedef bool mom_2itm_to_void_sig_t (const momnode_t*nod_mom /* 2 inputs, 0 outputs: */,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom);


#define MOM_PREFIXFUN_2itm_to_void "momfunc_2itm_to_void"
static inline mom_2itm_to_void_sig_t mom_applclos_2itm_to_void;
static inline bool mom_applval_2itm_to_void(const momvalue_t clo_mom,
		 momitem_t* arg0_mom,
		 momitem_t* arg1_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2itm_to_void (clo_mom.vnode, arg0_mom, arg1_mom);
} // end of mom_applval_2itm_to_void 

static inline bool
mom_applclos_2itm_to_void(const momnode_t* nod_mom, momitem_t* arg0_mom, momitem_t* arg1_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2itm_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2itm_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2itm_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2itm_to_void_sig_t* fun_mom = (mom_2itm_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2itm_to_void




// signature application support for signature_2val_to_val
typedef bool mom_2val_to_val_sig_t (const momnode_t*nod_mom /* 2 inputs, 1 outputs: */,
		 momvalue_t arg0_mom,
		 momvalue_t arg1_mom,
		  momvalue_t* res0_mom);


#define MOM_PREFIXFUN_2val_to_val "momfunc_2val_to_val"
static inline mom_2val_to_val_sig_t mom_applclos_2val_to_val;
static inline bool mom_applval_2val_to_val(const momvalue_t clo_mom,
		 momvalue_t arg0_mom,
		 momvalue_t arg1_mom,
		  momvalue_t* res0_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_2val_to_val (clo_mom.vnode, arg0_mom, arg1_mom, res0_mom);
} // end of mom_applval_2val_to_val 

static inline bool
mom_applclos_2val_to_val(const momnode_t* nod_mom, momvalue_t arg0_mom, momvalue_t arg1_mom,  momvalue_t* res0_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_2val_to_val))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_2val_to_val__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_2val_to_val",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_2val_to_val_sig_t* fun_mom = (mom_2val_to_val_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom, arg0_mom, arg1_mom, res0_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_2val_to_val




// signature application support for signature_void_to_void
typedef bool mom_void_to_void_sig_t (const momnode_t*nod_mom /* 0 inputs, 0 outputs: */);


#define MOM_PREFIXFUN_void_to_void "momfunc_void_to_void"
static inline mom_void_to_void_sig_t mom_applclos_void_to_void;
static inline bool mom_applval_void_to_void(const momvalue_t clo_mom)
{
 if (clo_mom.typnum != momty_node) return false;
 return mom_applclos_void_to_void (clo_mom.vnode);
} // end of mom_applval_void_to_void 

static inline bool
mom_applclos_void_to_void(const momnode_t* nod_mom)
{
 bool ok_mom= false; //// generated in state.c
  if (!nod_mom) return false;
  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;
  assert (connitm_mom != NULL);
  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind
      != MOM_PREDEFINED_NAMED(signature_void_to_void))) goto end_mom;
  void* data1_mom = connitm_mom->itm_data1;
  if (MOM_UNLIKELY(data1_mom==NULL)) {    char nambuf_mom[256];
    memset (nambuf_mom, 0, sizeof(nambuf_mom));
    if (snprintf(nambuf_mom, sizeof(nambuf_mom), "momfunc_void_to_void__%s",
                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))
       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);
    else MOM_FATAPRINTF("too long function name %s for signature_void_to_void",  mom_item_cstring(connitm_mom));
  };
  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {
     mom_void_to_void_sig_t* fun_mom = (mom_void_to_void_sig_t*) data1_mom;
     ok_mom = (*fun_mom) (nod_mom);
  };
 end_mom:
  mom_item_unlock(connitm_mom);
  return ok_mom;
} // end of mom_applclos_void_to_void


static inline bool
momhook_agenda_push_back(momvalue_t mom_arg0)
{
  momvalue_t mom_clos = MOM_NONEV;
  momitem_t* mom_itm =  MOM_PREDEFINED_NAMED(agenda_push_back);
  mom_item_lock(mom_itm);
  mom_clos = mom_components_nth(mom_itm->itm_comps, 0);
  mom_item_unlock(mom_itm);
  return mom_applval_1val_to_void(mom_clos, mom_arg0);
} // end momhook_agenda_push_back of signature_1val_to_void

static inline bool
momhook_agenda_push_front(momvalue_t mom_arg0)
{
  momvalue_t mom_clos = MOM_NONEV;
  momitem_t* mom_itm =  MOM_PREDEFINED_NAMED(agenda_push_front);
  mom_item_lock(mom_itm);
  mom_clos = mom_components_nth(mom_itm->itm_comps, 0);
  mom_item_unlock(mom_itm);
  return mom_applval_1val_to_void(mom_clos, mom_arg0);
} // end momhook_agenda_push_front of signature_1val_to_void

static inline bool
momhook_agenda_step(void)
{
  momvalue_t mom_clos = MOM_NONEV;
  momitem_t* mom_itm =  MOM_PREDEFINED_NAMED(agenda_step);
  mom_item_lock(mom_itm);
  mom_clos = mom_components_nth(mom_itm->itm_comps, 0);
  mom_item_unlock(mom_itm);
  return mom_applval_void_to_void(mom_clos);
} // end momhook_agenda_step of signature_void_to_void

static inline bool
momhook_generate_c_module(momitem_t* mom_arg0, momvalue_t* mom_res0)
{
  momvalue_t mom_clos = MOM_NONEV;
  momitem_t* mom_itm =  MOM_PREDEFINED_NAMED(generate_c_module);
  mom_item_lock(mom_itm);
  mom_clos = mom_components_nth(mom_itm->itm_comps, 0);
  mom_item_unlock(mom_itm);
  return mom_applval_1itm_to_val(mom_clos, mom_arg0, mom_res0);
} // end momhook_generate_c_module of signature_1itm_to_val


 // end of generated apply-header file apply-monimelt.h
