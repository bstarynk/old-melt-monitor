// file codgen.c - manage the persistent state load & dump

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

////////////////

#define CODEGEN_MAGIC_MOM 0x386182db	/* codegen magic 945914587 */
struct codegen_mom_st
{
  unsigned cg_magic;		/* always CODEGEN_MAGIC_MOM */
  momitem_t *cg_codgenitm;
  momitem_t *cg_moduleitm;	/* the module item */
  struct momhashset_st *cg_functionhset;	/* the set of c functions */
  const momstring_t *cg_errormsg;	/* the error message */
  struct momhashset_st *cg_lockeditemset;	/* the set of locked items */
  momitem_t *cg_curfunitm;	/* the current function item */
  struct momattributes_st *cg_funbind;	/* the function's bindings */
  struct momhashset_st *cg_funconstset;	/* the set of constant items */
  struct momhashset_st *cg_funclosedset;	/* the set of closed items */
  struct momattributes_st *cg_blockassoc;	/* the association of
						   c_block-s to a node ^c_block(<function>,<intruction-tuple>) */
  struct momqueueitems_st cg_blockqueue;	/* the queue of c_blocks to be scanned */
  momitem_t *cg_curblockitm;	/* the current block */
};				/* end struct codegen_mom_st */


#define CGEN_ERROR_RETURN_MOM_AT_BIS(Lin,Cg,Fmt,...) do {       \
  struct codegen_mom_st *cg_##Lin = (Cg);                       \
  assert (cg_##Lin && cg_##Lin->cg_magic == CODEGEN_MAGIC_MOM); \
  cg_##Lin->cg_errormsg =                                       \
    mom_make_string_sprintf(Fmt,__VA_ARGS__);                   \
  mom_warnprintf_at(__FILE__,Lin,"CODEGEN ERROR: %s",           \
                    cg_##Lin->cg_errormsg->cstr);               \
  return;                                                       \
 }while(0)
#define CGEN_ERROR_RETURN_MOM_AT(Lin,Cg,Fmt,...) \
  CGEN_ERROR_RETURN_MOM_AT_BIS(Lin,Cg,Fmt,__VA_ARGS__)
#define CGEN_ERROR_RETURN_MOM(Cg,Fmt,...) \
  CGEN_ERROR_RETURN_MOM_AT(__LINE__,(Cg),(Fmt),__VA_ARGS__)


#define CGEN_ERROR_FAILURE_AT_BIS(Lin,Cg,Fmt,...) do {		\
  struct codegen_mom_st *cg_##Lin = (Cg);			\
  assert (cg_##Lin && cg_##Lin->cg_magic == CODEGEN_MAGIC_MOM);	\
  cg_##Lin->cg_errormsg =					\
    mom_make_string_sprintf(Fmt,__VA_ARGS__);			\
  mom_warnprintf_at(__FILE__,Lin,"CODEGEN FAILURE: %s",		\
		    cg_##Lin->cg_errormsg->cstr);		\
  return false;							\
 }while(0)
#define CGEN_ERROR_FAILURE_AT(Lin,Cg,Fmt,...) \
  CGEN_ERROR_FAILURE_AT_BIS(Lin,Cg,Fmt,__VA_ARGS__)
#define CGEN_ERROR_FAILURE(Cg,Fmt,...) \
  CGEN_ERROR_FAILURE_AT(__LINE__,(Cg),(Fmt),__VA_ARGS__)


static void
cgen_lock_item_mom (struct codegen_mom_st *cg, momitem_t *itm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  if (itm && !mom_hashset_contains (cg->cg_lockeditemset, itm))
    {
      mom_item_lock (itm);
      cg->cg_lockeditemset = mom_hashset_put (cg->cg_lockeditemset, itm);
    }
}

static void
cgen_unlock_all_items_mom (struct codegen_mom_st *cg)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  struct momhashset_st *hset = cg->cg_lockeditemset;
  assert (hset);
  unsigned len = hset->hset_len;
  for (unsigned ix = 0; ix < len; ix++)
    {
      momitem_t *itm = (momitem_t *) hset->hset_elems[ix];
      if (!itm || itm == MOM_EMPTY)
	continue;
      mom_item_unlock (itm);
    }
}

static void cgen_first_pass_mom (momitem_t *itmcgen);

bool
  momfun_1itm_to_val_generate_c_module
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (gencod,
		   "generate_c_module itm=%s closure=%s start",
		   mom_item_cstring (itm),
		   mom_output_gcstring (mom_nodev (clonode)));
  if (!itm || itm == MOM_EMPTY)
    return false;
  *res = MOM_NONEV;
  momitem_t *itmcgen = mom_make_anonymous_item ();
  itmcgen->itm_space = momspa_transient;
  struct codegen_mom_st *cg =
    MOM_GC_ALLOC ("codegenerator", sizeof (struct codegen_mom_st));
  cg->cg_magic = CODEGEN_MAGIC_MOM;
  cg->cg_moduleitm = itm;
  cg->cg_codgenitm = itmcgen;
  mom_item_lock (itmcgen);
  itmcgen->itm_kind = MOM_PREDEFINED_NAMED (c_code_generation);
  itmcgen->itm_data1 = (void *) cg;
  itmcgen->itm_data2 = NULL;
  cgen_lock_item_mom (cg, itm);
  cgen_first_pass_mom (itmcgen);
  if (cg->cg_errormsg)
    goto end;
end:
  cgen_unlock_all_items_mom (cg);
  mom_item_unlock (itmcgen);
  if (cg->cg_errormsg)
    *res = mom_stringv (cg->cg_errormsg);
  else
    *res = mom_itemv (itm);
  return true;
}				/* end generate_c_module */

static void
cgen_scan_function_first_mom (struct codegen_mom_st *cg, momitem_t *itmfun);

void
cgen_first_pass_mom (momitem_t *itmcgen)
{
  assert (itmcgen
	  && itmcgen->itm_kind == MOM_PREDEFINED_NAMED (c_code_generation));
  struct codegen_mom_st *cg = (struct codegen_mom_st *) itmcgen->itm_data1;
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  momitem_t *itmmod = cg->cg_moduleitm;
  assert (itmmod);
  if (itmmod->itm_kind != MOM_PREDEFINED_NAMED (c_module))
    CGEN_ERROR_RETURN_MOM (cg, "module item %s is not a `c_module`",
			   mom_item_cstring (itmmod));
  ///// prepare the module using its c_preparation
  momvalue_t resprepv = MOM_NONEV;
  momvalue_t prepv =		//
    mom_item_unsync_get_attribute (itmmod,
				   MOM_PREDEFINED_NAMED (c_preparation));
  MOM_DEBUGPRINTF (gencod, "c_preparation of %s is %s",
		   mom_item_cstring (itmmod), mom_output_gcstring (prepv));
  if (prepv.typnum == momty_node
      && (!mom_applval_2itm_to_val (prepv, itmmod, itmcgen, &resprepv)
	  || resprepv.typnum == momty_string))
    {
      MOM_DEBUGPRINTF (gencod, "preparation of %s gave %s",
		       mom_item_cstring (itmmod),
		       mom_output_gcstring (resprepv));
      if (resprepv.typnum == momty_string)
	cg->cg_errormsg = mom_value_to_string (resprepv);
      else
	CGEN_ERROR_RETURN_MOM (cg, "module item %s preparation failed",
			       mom_item_cstring (itmmod));
    }
  else if (prepv.typnum != momty_null)
    CGEN_ERROR_RETURN_MOM (cg, "module item %s has bad preparation %s",
			   mom_item_cstring (itmmod),
			   mom_output_gcstring (prepv));
  ///// compute into cg_functionhset the hashed set of functions
  momvalue_t funsv =		//
    mom_item_unsync_get_attribute (itmmod, MOM_PREDEFINED_NAMED (functions));
  MOM_DEBUGPRINTF (gencod, "in module %s funsv= %s",
		   mom_item_cstring (itmmod), mom_output_gcstring (funsv));
  if (funsv.typnum == momty_node)
    {
      momvalue_t funsetv = MOM_NONEV;
      if (!mom_applval_2itm_to_val (prepv, itmmod, itmcgen, &funsetv)
	  || (funsv.typnum != momty_set || funsv.typnum != momty_tuple))
	CGEN_ERROR_RETURN_MOM (cg,
			       "module item %s : application of `functions` clsosure %s gave non-sequence result %s",
			       mom_item_cstring (itmmod),
			       mom_output_gcstring (funsv),
			       mom_output_gcstring (funsetv));
      MOM_DEBUGPRINTF (gencod, "in module %s funsv= %s gave %s",
		       mom_item_cstring (itmmod), mom_output_gcstring (funsv),
		       mom_output_gcstring (funsetv));
      funsv = funsetv;
    };
  if (funsv.typnum != momty_set && funsv.typnum != momty_tuple)
    CGEN_ERROR_RETURN_MOM (cg, "module item %s : functions %s are not a set",
			   mom_item_cstring (itmmod),
			   mom_output_gcstring (funsv));
  cg->cg_functionhset =
    mom_hashset_add_sized_items (NULL, funsv.vsequ->slen,
				 funsv.vsequ->arritm);
  {
    const momseq_t *fseq = mom_hashset_elements_set (cg->cg_functionhset);
    assert (fseq != NULL);
    unsigned nbfun = fseq->slen;
    for (unsigned ix = 0; ix < nbfun; ix++)
      {
	cgen_lock_item_mom (cg, (momitem_t *) fseq->arritm[ix]);
	cgen_scan_function_first_mom (cg, (momitem_t *) fseq->arritm[ix]);
	if (cg->cg_errormsg)
	  return;
      }
  }
}				/* end cgen_first_pass_mom */


static void cgen_scan_block_first_mom (struct codegen_mom_st *cg,
				       momitem_t *itmblock);

static void cgen_scan_instr_first_mom (struct codegen_mom_st *cg,
				       momitem_t *itminstr);

static void
cgen_bind_formals_mom (struct codegen_mom_st *cg, momitem_t *itmsignature,
		       momvalue_t vformals);


static void
cgen_bind_constants_mom (struct codegen_mom_st *cg, momvalue_t vconstants);

static void
cgen_scan_function_first_mom (struct codegen_mom_st *cg, momitem_t *itmfun)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmfun != NULL);
  cg->cg_curfunitm = NULL;
  cg->cg_funbind = NULL;
  cg->cg_funconstset = NULL;
  cg->cg_funclosedset = NULL;
  momitem_t *itmsignature = NULL;
  MOM_DEBUGPRINTF (gencod, "scanning function %s", mom_item_cstring (itmfun));
  cg->cg_curfunitm = itmfun;
  memset (&cg->cg_blockqueue, 0, sizeof (cg->cg_blockqueue));
  {
    momitem_t *itmfunkind = itmfun->itm_kind;
    if (itmfunkind)
      {
	mom_item_lock (itmfunkind);
	if (itmfunkind->itm_kind == MOM_PREDEFINED_NAMED (function_signature))
	  {
	    itmsignature = itmfunkind;
	    MOM_DEBUGPRINTF (gencod, "scanning function %s itmsignature %s",
			     mom_item_cstring (itmfun),
			     mom_item_cstring (itmsignature));
	  }
	mom_item_unlock (itmfunkind);
      }
    momvalue_t vfunctionsig = MOM_NONEV;
    if (!itmsignature)
      {
	vfunctionsig =
	  mom_item_unsync_get_attribute (itmfun,
					 MOM_PREDEFINED_NAMED
					 (function_signature));
	MOM_DEBUGPRINTF (gencod, "scanning function %s vfunctionsig=%s",
			 mom_item_cstring (itmfun),
			 mom_output_gcstring (vfunctionsig));
	itmsignature = mom_value_to_item (vfunctionsig);
      }
  }
  if (!itmsignature
      || itmsignature->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s without signature",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (itmfun));
  cgen_lock_item_mom (cg, itmsignature);
  momvalue_t vstart =
    mom_item_unsync_get_attribute (itmfun, MOM_PREDEFINED_NAMED (start));
  if (vstart.typnum != momty_item)
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has bad `start` %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (itmfun),
			   mom_output_gcstring (vstart));
  {				/* bind the formals */
    momvalue_t vformals =	//
      mom_item_unsync_get_attribute (itmfun, MOM_PREDEFINED_NAMED (formals));
    if (vformals.typnum != momty_tuple)
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has bad `formals` %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (itmfun),
			     mom_output_gcstring (vformals));
    cgen_bind_formals_mom (cg, itmsignature, vformals);
    if (cg->cg_errormsg)
      return;
  }
  {				/* bind the explicit constants */
    momvalue_t vconstants =	//
      mom_item_unsync_get_attribute (itmfun,
				     MOM_PREDEFINED_NAMED (constants));
    if (vconstants.typnum == momty_tuple || vconstants.typnum == momty_set)
      cgen_bind_constants_mom (cg, vconstants);
    else if (vconstants.typnum != momty_null)
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has bad `constants` %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (itmfun),
			     mom_output_gcstring (vconstants));
  }
  if (cg->cg_errormsg)
    return;
  momitem_t *itmstart = vstart.vitem;
  cgen_lock_item_mom (cg, itmstart);
  cgen_scan_block_first_mom (cg, itmstart);
  while (!cg->cg_errormsg && mom_queueitem_size (&cg->cg_blockqueue) > 0)
    {
      momitem_t *itmblock =
	(momitem_t *) mom_queueitem_pop_front (&cg->cg_blockqueue);
      assert (itmblock != NULL);
      cgen_lock_item_mom (cg, itmblock);
      cgen_scan_block_first_mom (cg, itmblock);
    };
  if (cg->cg_errormsg)
    return;
#warning incomplete cgen_scan_function_first_mom
  MOM_WARNPRINTF ("unimplemented cgen_scan_function_first_mom");
}


static void
cgen_scan_block_first_mom (struct codegen_mom_st *cg, momitem_t *itmblock)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmblock != NULL);
  momvalue_t vcinstrs = MOM_NONEV;
  cg->cg_curblockitm = NULL;
  MOM_DEBUGPRINTF (gencod, "scanning block %s", mom_item_cstring (itmblock));
  if (itmblock->itm_kind != MOM_PREDEFINED_NAMED (c_block))
    CGEN_ERROR_RETURN_MOM
      (cg, "module item %s : function %s has invalid block %s",
       mom_item_cstring (cg->cg_moduleitm),
       mom_item_cstring (cg->cg_curfunitm), mom_item_cstring (itmblock));
  {
    momvalue_t vablock = MOM_NONEV;
    struct momentry_st *entblock =
      mom_attributes_find_entry (cg->cg_blockassoc, itmblock);
    if (entblock)
      vablock = entblock->ent_val;
    assert (vablock.typnum == momty_null || vablock.typnum == momty_node);
    if (vablock.typnum == momty_node)
      {
	assert (vablock.vnode->slen == 2
		&& vablock.vnode->conn == MOM_PREDEFINED_NAMED (c_block));
	momitem_t *blockfunitm =
	  mom_value_to_item (vablock.vnode->arrsons[0]);
	assert (blockfunitm);
	if (blockfunitm != cg->cg_curfunitm)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s has block %s already inside other function %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_output_gcstring (vablock),
				 mom_item_cstring (blockfunitm));
	else
	  return;
      }
    vcinstrs = mom_item_unsync_get_attribute (itmblock,
					      MOM_PREDEFINED_NAMED
					      (c_instructions));
    if (vcinstrs.typnum == momty_node)
      {
	momvalue_t newvcinstrs = MOM_NONEV;
	if (mom_applval_2itm_to_val (vcinstrs, itmblock, cg->cg_codgenitm,
				     &newvcinstrs))
	  vcinstrs = newvcinstrs;
	else
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s has block %s with bad `c_instructions` closure %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_output_gcstring (vablock),
				 mom_output_gcstring (vcinstrs));
      };
    if (vcinstrs.typnum == momty_tuple)
      {
	vablock = mom_nodev_new (MOM_PREDEFINED_NAMED (c_block), 2,
				 mom_itemv (cg->cg_curfunitm), vcinstrs);
	cg->cg_blockassoc = mom_attributes_put (cg->cg_blockassoc,
						itmblock, &vablock);
      }
    else
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has block %s with bad `c_instructions` %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itmblock),
			     mom_output_gcstring (vcinstrs));
  }
  cg->cg_curblockitm = itmblock;
  assert (vcinstrs.typnum == momty_tuple);
  unsigned nbinstrs = vcinstrs.vtuple->slen;
  for (unsigned ix = 0; ix < nbinstrs && !cg->cg_errormsg; ix++)
    {
      momitem_t *instritm = (momitem_t *) vcinstrs.vtuple->arritm[ix];
      assert (instritm != NULL);
      cgen_lock_item_mom (cg, instritm);
      cgen_scan_instr_first_mom (cg, instritm);
    }
}				/* end cgen_scan_block_first_mom */


static void
cgen_scan_instr_first_mom (struct codegen_mom_st *cg, momitem_t *itminstr)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itminstr != NULL);
#warning cgen_scan_instr_first_mom incomplete
  MOM_FATAPRINTF ("gen_scan_instr_first itminstr=%s unimplemented",
		  mom_item_cstring (itminstr));
}

static void
cgen_bind_new_mom (struct codegen_mom_st *cg, momitem_t *itm,
		   momvalue_t vbind)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itm != NULL);
  assert (mom_hashset_contains (cg->cg_lockeditemset, itm));
  assert (vbind.typnum != momty_null);
  struct momentry_st *ent = mom_attributes_find_entry (cg->cg_funbind, itm);
  if (MOM_UNLIKELY (ent != NULL))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s with already bound item %s to %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (itm),
			   mom_output_gcstring (ent->ent_val));
  MOM_DEBUGPRINTF (gencod, "cgen_bind_new func.%s itm=%s vbind=%s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itm), mom_output_gcstring (vbind));
  cg->cg_funbind = mom_attributes_put (cg->cg_funbind, itm, &vbind);
}


static void
cgen_bind_formals_mom (struct codegen_mom_st *cg, momitem_t *itmsignature,
		       momvalue_t vformals)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmsignature != NULL);
  assert (mom_hashset_contains (cg->cg_lockeditemset, itmsignature));
  assert (vformals.typnum == momty_tuple && vformals.vtuple);
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_formals start itmsignature=%s vformals=%s",
		   mom_item_cstring (itmsignature),
		   mom_output_gcstring (vformals));
  unsigned nbformals = vformals.vtuple->slen;
  if (itmsignature->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has bad signature %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (itmsignature));
  momvalue_t vinputy =		//
    mom_item_unsync_get_attribute (itmsignature,
				   MOM_PREDEFINED_NAMED (input_types));
  momvalue_t voutputy =		//
    mom_item_unsync_get_attribute (itmsignature,
				   MOM_PREDEFINED_NAMED (output_types));
  unsigned nbins = 0, nbouts = 0;
  const struct momseq_st *tupins = NULL;
  const struct momseq_st *tupouts = NULL;
  if (vinputy.typnum != momty_tuple || voutputy.typnum != momty_tuple
      || !(tupins = mom_value_to_tuple (vinputy))
      || !(tupouts = mom_value_to_tuple (voutputy))
      || (nbins = tupins->slen) + (nbouts = tupouts->slen) != nbformals)
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s formals %s mismatch with input_types %s & output_types %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_output_gcstring (vformals),
			   mom_output_gcstring (vinputy),
			   mom_output_gcstring (voutputy));
  //// process input formals
  for (unsigned inix = 0; inix < nbins; inix++)
    {
      if (cg->cg_errormsg)
	return;
      momitem_t *intypitm = (momitem_t *) tupins->arritm[inix];
      assert (intypitm);
      momitem_t *informalitm = (momitem_t *) vformals.vtuple->arritm[inix];
      assert (informalitm);
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s inix#%d intypitm %s informalitm %s",
		       mom_item_cstring (cg->cg_curfunitm), inix,
		       mom_item_cstring (intypitm),
		       mom_item_cstring (informalitm));
      if (intypitm->itm_kind != MOM_PREDEFINED_NAMED (c_type))
	CGEN_ERROR_RETURN_MOM (cg,
			       "module item %s : function %s bad input type #%d %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm), inix,
			       mom_item_cstring (intypitm));
      momvalue_t valbind = mom_nodev_new (MOM_PREDEFINED_NAMED (formals),
					  4,
					  mom_itemv (cg->cg_curfunitm),
					  mom_intv (inix),
					  MOM_PREDEFINED_NAMED (input_types),
					  mom_itemv (intypitm));
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s inix#%d informalitm %s valbind %s",
		       mom_item_cstring (cg->cg_curfunitm), inix,
		       mom_item_cstring (informalitm),
		       mom_output_gcstring (valbind));
      cgen_bind_new_mom (cg, informalitm, valbind);
      if (cg->cg_errormsg)
	return;
    }
  //// process output formals
  for (unsigned outix = 0; outix < nbouts; outix++)
    {
      if (cg->cg_errormsg)
	return;
      momitem_t *outtypitm = (momitem_t *) tupouts->arritm[outix];
      assert (outtypitm);
      momitem_t *outformalitm =
	(momitem_t *) vformals.vtuple->arritm[outix + nbins];
      assert (outformalitm);
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s outix#%d outtypitm %s outformalitm %s",
		       mom_item_cstring (cg->cg_curfunitm), outix,
		       mom_item_cstring (outtypitm),
		       mom_item_cstring (outformalitm));
      if (outtypitm->itm_kind != MOM_PREDEFINED_NAMED (c_type))
	CGEN_ERROR_RETURN_MOM (cg,
			       "module item %s : function %s bad output type #%d %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm), outix,
			       mom_item_cstring (outtypitm));
      momvalue_t valbind = mom_nodev_new (MOM_PREDEFINED_NAMED (formals),
					  4,
					  mom_itemv (cg->cg_curfunitm),
					  mom_intv (outix),
					  MOM_PREDEFINED_NAMED (output_types),
					  mom_itemv (outtypitm));
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s outix#%d outformalitm %s valbind %s",
		       mom_item_cstring (cg->cg_curfunitm), outix,
		       mom_item_cstring (outformalitm),
		       mom_output_gcstring (valbind));
      cgen_bind_new_mom (cg, outformalitm, valbind);
      if (cg->cg_errormsg)
	return;
    }
}				/* end cgen_bind_formals_mom */


static void
cgen_bind_constant_item_mom (struct codegen_mom_st *cg, momitem_t *itmk,
			     momvalue_t vconst)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmk != NULL);
  assert (vconst.typnum != momty_null);
  assert (mom_hashset_contains (cg->cg_lockeditemset, itmk));
  if (mom_hashset_contains (cg->cg_funconstset, itmk))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has duplicate constant %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (itmk));
  momvalue_t vconstbind =	//
    mom_nodev_new (MOM_PREDEFINED_NAMED (constants),
		   3,
		   mom_itemv (cg->cg_curfunitm),
		   mom_intv (mom_hashset_count (cg->cg_funconstset)),
		   vconst);
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_constant_item function %s constant item %s bound to %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmk), mom_output_gcstring (vconst));
  cg->cg_funconstset = mom_hashset_put (cg->cg_funconstset, itmk);
  cgen_bind_new_mom (cg, itmk, vconstbind);
}

static void
cgen_bind_constants_mom (struct codegen_mom_st *cg, momvalue_t vconstants)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod, "in function %s binding constants %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_output_gcstring (vconstants));
  const momseq_t *seqconstants = mom_value_to_sequ (vconstants);
  assert (seqconstants != NULL);
  unsigned nbconstants = seqconstants->slen;
  for (unsigned ixk = 0; ixk < nbconstants && !cg->cg_errormsg; ixk++)
    {
      momitem_t *itmk = (momitem_t *) seqconstants->arritm[ixk];
      assert (itmk != NULL);
      cgen_lock_item_mom (cg, itmk);
      momvalue_t valconst =	//
	mom_item_unsync_get_attribute (itmk,
				       MOM_PREDEFINED_NAMED (value));
      if (valconst.typnum == momty_null)
	valconst = mom_itemv (itmk);
      MOM_DEBUGPRINTF (gencod, "binding constant ixk=%d itmk=%s valconst=%s",
		       ixk, mom_item_cstring (itmk),
		       mom_output_gcstring (valconst));
      cgen_bind_constant_item_mom (cg, itmk, valconst);
      if (cg->cg_errormsg)
	return;
    }
  MOM_DEBUGPRINTF (gencod, "in function %s done bind constants %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_output_gcstring (vconstants));
}				/* end cgen_bind_constants_mom */

/// eof codgen.c
