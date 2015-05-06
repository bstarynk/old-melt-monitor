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
  struct momhashset_st *cg_funvariableset;	/* the set of variable items */
  struct momattributes_st *cg_blockassoc;	/* the association of
						   c_block-s to a node ^c_block(<function>,<intruction-tuple>) */
  struct momqueueitems_st cg_blockqueue;	/* the queue of c_blocks to be scanned */
  momitem_t *cg_curblockitm;	/* the current block */
  momitem_t *cg_curstmtitm;	/* the current statement */
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


#define CGEN_ERROR_RESULT_AT_BIS_MOM(Lin,Cg,Res,Fmt,...) do {	\
  struct codegen_mom_st *cg_##Lin = (Cg);			\
  assert (cg_##Lin && cg_##Lin->cg_magic == CODEGEN_MAGIC_MOM);	\
  cg_##Lin->cg_errormsg =					\
    mom_make_string_sprintf(Fmt,__VA_ARGS__);			\
  mom_warnprintf_at(__FILE__,Lin,"CODEGEN FAILURE: %s",		\
		    cg_##Lin->cg_errormsg->cstr);		\
  return (Res);							\
 }while(0)
#define CGEN_ERROR_RESULT_AT_MOM(Lin,Cg,Res,Fmt,...)	\
  CGEN_ERROR_RESULT_AT_BIS_MOM(Lin,Cg,Res,Fmt,__VA_ARGS__)
#define CGEN_ERROR_RESULT_MOM(Cg,Res,Fmt,...)			\
  CGEN_ERROR_RESULT_AT_MOM(__LINE__,(Cg),(Res),(Fmt),__VA_ARGS__)


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

static void cgen_scan_statement_first_mom (struct codegen_mom_st *cg,
					   momitem_t *itminstr);

static void
cgen_bind_formals_mom (struct codegen_mom_st *cg, momitem_t *itmsignature,
		       momvalue_t vformals);


static void
cgen_bind_constants_mom (struct codegen_mom_st *cg, momvalue_t vconstants);

static void
cgen_bind_variables_mom (struct codegen_mom_st *cg, momvalue_t vvariables);

static void
cgen_bind_closed_variables_mom (struct codegen_mom_st *cg,
				momvalue_t vclosed);

static void
cgen_bind_closed_item_mom (struct codegen_mom_st *cg, momitem_t *itmc);

static void
cgen_scan_function_first_mom (struct codegen_mom_st *cg, momitem_t *itmfun)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmfun != NULL);
  cg->cg_curfunitm = NULL;
  cg->cg_funbind = NULL;
  cg->cg_funconstset = NULL;
  cg->cg_funclosedset = NULL;
  cg->cg_funvariableset = NULL;
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
  /////
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
  }
  if (cg->cg_errormsg)
    return;
  /////
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
  /////
  {
    /* bind the explicit variable-s */
    momvalue_t vvariable =	//
      mom_item_unsync_get_attribute (itmfun,
				     MOM_PREDEFINED_NAMED (variable));
    if (vvariable.typnum == momty_tuple || vvariable.typnum == momty_set)
      cgen_bind_variables_mom (cg, vvariable);
    else if (vvariable.typnum != momty_null)
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has bad `variable` %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (itmfun),
			     mom_output_gcstring (vvariable));
  }
  if (cg->cg_errormsg)
    return;
  /////
  {				/* bind the explicit closed */
    momvalue_t vclosed =	//
      mom_item_unsync_get_attribute (itmfun,
				     MOM_PREDEFINED_NAMED (closed));
    if (vclosed.typnum == momty_tuple || vclosed.typnum == momty_set)
      cgen_bind_closed_variables_mom (cg, vclosed);
    else if (vclosed.typnum != momty_null)
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has bad `closed` %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (itmfun),
			     mom_output_gcstring (vclosed));
  }
  if (cg->cg_errormsg)
    return;
  //// 
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
  cg->cg_curstmtitm = NULL;
  MOM_DEBUGPRINTF (gencod, "in function %s scanning block %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmblock));
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
      cgen_scan_statement_first_mom (cg, instritm);
      cg->cg_curstmtitm = NULL;
    }
}				/* end cgen_scan_block_first_mom */


static void cgen_bind_variable_item_mom (struct codegen_mom_st *cg,
					 momitem_t *itmv);

static void cgen_bind_constant_item_mom (struct codegen_mom_st *cg,
					 momitem_t *itmk, momvalue_t vconst);

static momitem_t *
cgen_type_of_scanned_item_mom (struct codegen_mom_st *cg, momitem_t *itm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itm != NULL);
  assert (mom_hashset_contains (cg->cg_lockeditemset, itm));
  struct momentry_st *ent = mom_attributes_find_entry (cg->cg_funbind, itm);
  if (ent != NULL)
    {
      momvalue_t vbind = ent->ent_val;
      assert (vbind.typnum == momty_node);
      const momnode_t *nodbind = mom_value_to_node (vbind);
      assert (nodbind != NULL);
      momitem_t *nodconnitm = mom_node_conn (nodbind);
      assert (nodconnitm != NULL);
      switch (mom_item_hash (nodconnitm))
	{
	case MOM_PREDEFINED_NAMED_CASE (formals, nodconnitm, otherwiseconnlab):
	  {
	    momvalue_t vres = mom_node_nth (nodbind, 3);
	    MOM_DEBUGPRINTF (gencod, "in function %s item %s is formal of %s",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itm),
			     mom_output_gcstring (vres));
	    assert (vres.typnum == momty_item);
	    return mom_value_to_item (vres);
	  }
	  break;
	case MOM_PREDEFINED_NAMED_CASE (constants, nodconnitm, otherwiseconnlab):
	  {
	    MOM_DEBUGPRINTF (gencod,
			     "in function %s item %s is constant value",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itm));
	    return MOM_PREDEFINED_NAMED (value);
	  }
	  break;
	case MOM_PREDEFINED_NAMED_CASE (closed, nodconnitm, otherwiseconnlab):
	  {
	    MOM_DEBUGPRINTF (gencod, "in function %s item %s is closed value",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itm));
	    return MOM_PREDEFINED_NAMED (value);
	  }
	  break;
	case MOM_PREDEFINED_NAMED_CASE (variable, nodconnitm, otherwiseconnlab):
	  {
	    momvalue_t vres = mom_node_nth (nodbind, 2);
	    MOM_DEBUGPRINTF (gencod,
			     "in function %s item %s is variable of %s",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itm),
			     mom_output_gcstring (vres));
	    assert (vres.typnum == momty_item);
	    return mom_value_to_item (vres);
	  }
	  break;
	default:
	otherwiseconnlab:
	  // this should never happen
	  MOM_FATAPRINTF
	    ("codgen: module item %s : function %s has item %s with unexpected binding %s",
	     mom_item_cstring (cg->cg_moduleitm),
	     mom_item_cstring (cg->cg_curfunitm), mom_item_cstring (itm),
	     mom_output_gcstring (vbind));
	}			/* end swith conn vbindnod */
    }
  //// unbound item:
  momitem_t *itmkind = itm->itm_kind;
  MOM_DEBUGPRINTF (gencod, "in function %s item %s (of kind %s) is unbound",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itm),
		   itmkind ? mom_item_cstring (itmkind) : "~");
  switch (mom_item_hash (itmkind))
    {
    case MOM_PREDEFINED_NAMED_CASE (closed, itmkind, otherwisekindlab):
      {
	cgen_bind_closed_item_mom (cg, itm);
	MOM_DEBUGPRINTF (gencod, "function %s has new closed value item %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (itm));
	return MOM_PREDEFINED_NAMED (value);
      }
      break;
    case MOM_PREDEFINED_NAMED_CASE (variable, itmkind, otherwisekindlab):
      {
	momitem_t *itmctyp = NULL;
	itmctyp = mom_value_to_item (mom_item_unsync_get_attribute (itm,
								    MOM_PREDEFINED_NAMED
								    (c_type)));
	if (!itmctyp)
	  CGEN_ERROR_RESULT_MOM (cg, NULL,
				 "module item %s : function %s has block %s with statment %s with untyped variable %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (cg->cg_curstmtitm),
				 mom_item_cstring (itm));
	if (cg->cg_errormsg)
	  return NULL;
	cgen_bind_variable_item_mom (cg, itm);
	if (cg->cg_errormsg)
	  return NULL;
	MOM_DEBUGPRINTF (gencod,
			 "function %s has new variable item %s of type %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (itm), mom_item_cstring (itmctyp));
	return itmctyp;
      }
      break;
    default:
    otherwisekindlab:
    case MOM_PREDEFINED_NAMED_CASE (value, itmkind, otherwisekindlab):
      // handle the item as a constant
      {
	momvalue_t valconst = MOM_NONEV;
	if (itmkind == MOM_PREDEFINED_NAMED (value))
	  {
	    valconst =		//
	      mom_item_unsync_get_attribute (itm,
					     MOM_PREDEFINED_NAMED (value));
	  }
	if (valconst.typnum == momty_null)
	  valconst = mom_itemv (itm);
	cgen_bind_constant_item_mom (cg, itm, valconst);
	if (cg->cg_errormsg)
	  return NULL;
	MOM_DEBUGPRINTF (gencod,
			 "function %s has new constant item %s of kind %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (itm),
			 itmkind ? mom_item_cstring (itmkind) : "~");
	return MOM_PREDEFINED_NAMED (value);
      }
      break;
    }
  CGEN_ERROR_RESULT_MOM (cg, NULL,
			 "module item %s : function %s has block %s with statment %s with unexpected item %s",
			 mom_item_cstring (cg->cg_moduleitm),
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (cg->cg_curblockitm),
			 mom_item_cstring (cg->cg_curstmtitm),
			 mom_item_cstring (itm));
}				/* end cgen_type_of_scanned_item_mom */


static momitem_t *
cgen_type_of_scanned_expr_mom (struct codegen_mom_st *cg, momvalue_t vexpr)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  switch ((enum momvaltype_en) vexpr.typnum)
    {
    case momty_null:
      return NULL;
    case momty_delim:
    case momty_tuple:
    case momty_set:
      CGEN_ERROR_RESULT_MOM (cg,
			     (momitem_t *) NULL,
			     "module item %s : function %s has block %s with statment %s with bad expression %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (cg->cg_curblockitm),
			     mom_item_cstring (cg->cg_curstmtitm),
			     mom_output_gcstring (vexpr));
    case momty_int:
      return MOM_PREDEFINED_NAMED (integer);
    case momty_double:
      return MOM_PREDEFINED_NAMED (double);
    case momty_item:
      cgen_lock_item_mom (cg, vexpr.vitem);
      return cgen_type_of_scanned_item_mom (cg, vexpr.vitem);
    }
}

static void
cgen_scan_statement_first_mom (struct codegen_mom_st *cg, momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt != NULL);
  cg->cg_curstmtitm = itmstmt;
  MOM_DEBUGPRINTF (gencod, "in function %s start scanning statement %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmstmt));
  assert (mom_hashset_contains (cg->cg_lockeditemset, itmstmt));
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  if (itmstmt->itm_kind != MOM_PREDEFINED_NAMED (code_statement))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s with block %s with bad statement %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (itmstmt));
  unsigned stmtlen = mom_components_count (stmtcomps);
  if (stmtlen < 1)
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s with block %s with empty statement %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (itmstmt));
  momitem_t *itmop = mom_value_to_item (mom_components_nth (stmtcomps, 0));
  if (!itmop)
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s with block %s with opless statement %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (itmstmt));
  switch (mom_item_hash (itmop))
    {
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (set, itmop, otherwiseoplab):
      {				/// set <lvar> <rexpr>
	momitem_t *itmlvar = NULL;
	if (stmtlen != 3
	    || !(itmlvar =
		 mom_value_to_item (mom_components_nth (stmtcomps, 1))))
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with bad set statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
	momvalue_t rexprv = mom_components_nth (stmtcomps, 2);
	MOM_DEBUGPRINTF (gencod,
			 "in function %s block %s set statement %s lvar %s rexpr %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (cg->cg_curblockitm),
			 mom_item_cstring (itmstmt),
			 mom_item_cstring (itmlvar),
			 mom_output_gcstring (rexprv));
	momitem_t *lvarctypitm = cgen_type_of_scanned_item_mom (cg, itmlvar);
	if (cg->cg_errormsg)
	  return;
	MOM_DEBUGPRINTF (gencod,
			 "in function %s block %s set statement %s lvar %s lvarctyp %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (cg->cg_curblockitm),
			 mom_item_cstring (itmstmt),
			 mom_item_cstring (itmlvar),
			 mom_item_cstring (lvarctypitm));
	if (!lvarctypitm)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with set statement %s with bad lvar %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt),
				 mom_item_cstring (itmlvar));
	momitem_t *rexpctypitm = cgen_type_of_scanned_expr_mom (cg, rexprv);
	MOM_DEBUGPRINTF (gencod,
			 "in function %s block %s set statement %s rexpr %s of ctype %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (cg->cg_curblockitm),
			 mom_item_cstring (itmstmt),
			 mom_output_gcstring (rexprv),
			 mom_item_cstring (rexpctypitm));
	if (cg->cg_errormsg)
	  return;
	if (!rexpctypitm)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with set statement %s with bad rexpr %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt),
				 mom_output_gcstring (rexprv));
	if (lvarctypitm == rexpctypitm)
	  break;
	if (cg->cg_errormsg)
	  return;
	if ((lvarctypitm == MOM_PREDEFINED_NAMED (item)
	     || lvarctypitm == MOM_PREDEFINED_NAMED (locked_item))
	    && (rexpctypitm == MOM_PREDEFINED_NAMED (item)
		|| rexpctypitm == MOM_PREDEFINED_NAMED (locked_item)))
	  break;
	CGEN_ERROR_RETURN_MOM (cg,
			       "module item %s : function %s with block %s with set statement %s : leftvar %s of type %s is incompatible with rightexpr %s of type %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm),
			       mom_item_cstring (cg->cg_curblockitm),
			       mom_item_cstring (itmstmt),
			       mom_item_cstring (itmlvar),
			       mom_item_cstring (lvarctypitm),
			       mom_output_gcstring (rexprv),
			       mom_item_cstring (rexpctypitm));

      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (chunk, itmop, otherwiseoplab):
      {				/// chunk ...
	for (unsigned ix = 1; ix < stmtlen && !cg->cg_errormsg; ix++)
	  {
	    momvalue_t vchkarg = mom_components_nth (stmtcomps, ix);
	    momitem_t *itmarg = mom_value_to_item (vchkarg);
	    if (itmarg != NULL)
	      {
		cgen_lock_item_mom (cg, itmarg);
		if (itmarg->itm_kind == MOM_PREDEFINED_NAMED (c_block))
		  {
		    cgen_scan_block_first_mom (cg, itmarg);
		    continue;
		  }
		else
		  {
		    momitem_t *itmtyp =
		      cgen_type_of_scanned_item_mom (cg, itmarg);
		    if (!itmtyp)
		      goto strangechunkarglab;
		  }
	      }
	    else if (vchkarg.typnum != momty_null)
	      {
		momitem_t *itmtyparg =
		  cgen_type_of_scanned_expr_mom (cg, vchkarg);
		if (!itmtyparg)
		strangechunkarglab:
		  CGEN_ERROR_RETURN_MOM (cg,
					 "module item %s : function %s with block %s with chunk statement %s with strange arg  %s",
					 mom_item_cstring (cg->cg_moduleitm),
					 mom_item_cstring (cg->cg_curfunitm),
					 mom_item_cstring
					 (cg->cg_curblockitm),
					 mom_item_cstring (itmstmt),
					 mom_output_gcstring (vchkarg));
	      }
	  }
      }
      break;
      ////////////////
      case MOM_PREDEFINED_NAMED_CASE (if, itmop, otherwiseoplab)
    :
	{			/// if <expr-cond> <block>
	  momvalue_t vtestexpr = MOM_NONEV;
	  momitem_t *testctypitm = NULL;
	  momitem_t *thenitm = NULL;
	  if (stmtlen != 3
	      || (vtestexpr =
		  mom_components_nth (stmtcomps, 1)).typnum == momty_null
	      || !(testctypitm =
		   cgen_type_of_scanned_expr_mom (cg, vtestexpr))
	      || !(thenitm =
		   mom_value_to_item (mom_components_nth (stmtcomps, 2))))
	    CGEN_ERROR_RETURN_MOM (cg,
				   "module item %s : function %s with block %s with bad if statement %s",
				   mom_item_cstring (cg->cg_moduleitm),
				   mom_item_cstring (cg->cg_curfunitm),
				   mom_item_cstring (cg->cg_curblockitm),
				   mom_item_cstring (itmstmt));
	  if (cg->cg_errormsg)
	    return;
	  if (thenitm->itm_kind == MOM_PREDEFINED_NAMED (c_block))
	    {
	      cgen_lock_item_mom (cg, thenitm);
	      cgen_scan_block_first_mom (cg, thenitm);
	    }
	  else
	    CGEN_ERROR_RETURN_MOM (cg,
				   "module item %s : function %s with block %s with wrong if statement %s - non block then %s",
				   mom_item_cstring (cg->cg_moduleitm),
				   mom_item_cstring (cg->cg_curfunitm),
				   mom_item_cstring (cg->cg_curblockitm),
				   mom_item_cstring (itmstmt),
				   mom_item_cstring (thenitm));
	}
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (apply, itmop, otherwiseoplab):
      {				/// apply <signature> <results...> <fun> <args...> [<else-block>]
	if (stmtlen < 3)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with too short apply statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
	momitem_t *sigitm =
	  mom_value_to_item (mom_components_nth (stmtcomps, 1));
	if (!sigitm
	    || (cgen_lock_item_mom (cg, sigitm),
		sigitm->itm_kind !=
		MOM_PREDEFINED_NAMED (function_signature)))
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with  apply statement %s with bad signature %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt),
				 mom_item_cstring (sigitm));
	const momseq_t *intyptup =
	  mom_value_to_tuple (mom_item_unsync_get_attribute
			      (sigitm, MOM_PREDEFINED_NAMED (input_types)));
	const momseq_t *outyptup =
	  mom_value_to_tuple (mom_item_unsync_get_attribute
			      (sigitm, MOM_PREDEFINED_NAMED (output_types)));
	if (!intyptup || !outyptup)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with  apply statement %s with mistyped signature %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt),
				 mom_item_cstring (sigitm));
	unsigned nbin = mom_seq_length (intyptup);
	unsigned nbout = mom_seq_length (outyptup);
	if (stmtlen < 3 + nbin + nbout || stmtlen > 4 + nbin + nbout)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with  apply statement %s for signature %s of bad length %d",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt),
				 mom_item_cstring (sigitm), stmtlen);
	for (unsigned inix = 0; inix < nbin && !cg->cg_errormsg; inix++)
	  {
	    momitem_t *incuritm =
	      mom_value_to_item (mom_components_nth (stmtcomps, 2 + inix));
	    if (!incuritm)
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s with bad result#%d",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), inix);
	    cgen_lock_item_mom (cg, incuritm);
	    momitem_t *insigtypitm = mom_seq_nth (intyptup, inix);
	    momitem_t *incurtypitm =
	      cgen_type_of_scanned_item_mom (cg, incuritm);
	    if (insigtypitm && insigtypitm == incuritm)
	      continue;
	    else
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s with invalid result#%d type",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), inix);
	  };
#warning should test the function argument and shift
	for (unsigned outix = 0; outix < nbout && !cg->cg_errormsg; outix++)
	  {
	    momvalue_t outcurv =
	      mom_components_nth (stmtcomps, 2 + nbin + outix);
	    momitem_t *outsigtypitm = mom_seq_nth (outyptup, outix);
	    momitem_t *outcurtypitm =
	      cgen_type_of_scanned_expr_mom (cg, outcurv);
	    if (outsigtypitm && outsigtypitm == outcurtypitm)
	      continue;
	    else
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s with invalid argument#%d type",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), outix);
	  };
	if (stmtlen == 4 + nbin + nbout)
	  {
	    momitem_t *itmelse =
	      mom_value_to_item (mom_components_nth
				 (stmtcomps, 3 + nbin + nbout));
	    if (!itmelse)
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s without else block",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt));
	    cgen_lock_item_mom (cg, itmelse);
	    if (itmelse->itm_kind == MOM_PREDEFINED_NAMED (c_block))
	      {
		cgen_scan_block_first_mom (cg, itmelse);
	      }
	    else
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s with bad else block %s",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt),
				     mom_item_cstring (itmelse));
	  }
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (int_switch, itmop, otherwiseoplab):
      {				// int_switch <expr> <case....>
	if (stmtlen < 2)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with too short int_switch statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
	momvalue_t exprv = mom_components_nth (stmtcomps, 1);
	momitem_t *typxitm = cgen_type_of_scanned_expr_mom (cg, exprv);
      };
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (item_switch, itmop, otherwiseoplab):
      {
	if (stmtlen < 2)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with too short item_switch statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
      };
      break;
    default:
    otherwiseoplab:
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s with block %s with statement %s with strange op %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (cg->cg_curblockitm),
			     mom_item_cstring (itmstmt),
			     mom_item_cstring (itmop));
      break;
    }
#warning cgen_scan_statement_first_mom unimplemented
  MOM_DEBUGPRINTF (gencod, "in function %s end scanning statement %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmstmt));
}				/* end cgen_scan_statement_first_mom */



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


static void
cgen_bind_variable_item_mom (struct codegen_mom_st *cg, momitem_t *itmv)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmv != NULL);
  assert (mom_hashset_contains (cg->cg_lockeditemset, itmv));
  if (mom_hashset_contains (cg->cg_funvariableset, itmv))
    return;
  if (itmv->itm_kind != MOM_PREDEFINED_NAMED (variable))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has bad variable %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (itmv));
  momvalue_t vctyp =		//
    mom_item_unsync_get_attribute (itmv,
				   MOM_PREDEFINED_NAMED (c_type));
  momitem_t *itmctyp = mom_value_to_item (vctyp);
  if (!itmctyp || itmctyp->itm_kind != MOM_PREDEFINED_NAMED (c_type))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has variable %s with bad `c_type` %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (itmv),
			   mom_output_gcstring (vctyp));
  momvalue_t vvarbind =		//
    mom_nodev_new (MOM_PREDEFINED_NAMED (variable),
		   3,
		   mom_itemv (cg->cg_curfunitm),
		   mom_intv (mom_hashset_count (cg->cg_funvariableset)),
		   mom_itemv (itmctyp));
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_variable_item function %s variable item %s bound to %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmv), mom_output_gcstring (vvarbind));
  cg->cg_funvariableset = mom_hashset_put (cg->cg_funvariableset, itmv);
  cgen_bind_new_mom (cg, itmv, vvarbind);
}				/* end of cgen_bind_variable_item_mom */


static void
cgen_bind_variables_mom (struct codegen_mom_st *cg, momvalue_t vvariables)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod, "in function %s binding variables %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_output_gcstring (vvariables));
  const momseq_t *seqvariable = mom_value_to_sequ (vvariables);
  assert (seqvariable != NULL);
  unsigned nbvariable = seqvariable->slen;
  for (unsigned ixv = 0; ixv < nbvariable && !cg->cg_errormsg; ixv++)
    {
      momitem_t *itmv = (momitem_t *) seqvariable->arritm[ixv];
      assert (itmv != NULL);
      cgen_lock_item_mom (cg, itmv);
      cgen_bind_variable_item_mom (cg, itmv);
    }
}				/* end cgen_bind_variables_mom */


static void
cgen_bind_closed_item_mom (struct codegen_mom_st *cg, momitem_t *itmc)
{
  MOM_DEBUGPRINTF (gencod, "in function %s binding closed item %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmc));
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmc != NULL);
  assert (mom_hashset_contains (cg->cg_lockeditemset, itmc));
  if (mom_hashset_contains (cg->cg_funclosedset, itmc))
    return;
  momvalue_t vclobind =		//
    mom_nodev_new (MOM_PREDEFINED_NAMED (closed),
		   2,
		   mom_itemv (cg->cg_curfunitm),
		   mom_intv (mom_hashset_count (cg->cg_funclosedset)));
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_constant_item function %s closed item %s bound to %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmc), mom_output_gcstring (vclobind));
  cg->cg_funclosedset = mom_hashset_put (cg->cg_funclosedset, itmc);
  cgen_bind_new_mom (cg, itmc, vclobind);
}				/* end cgen_bind_closed_item_mom */


static void
cgen_bind_closed_variables_mom (struct codegen_mom_st *cg, momvalue_t vclosed)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod, "in function %s binding closed variables %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_output_gcstring (vclosed));
  const momseq_t *seqclosed = mom_value_to_sequ (vclosed);
  assert (seqclosed != NULL);
  unsigned nbclosed = seqclosed->slen;
  for (unsigned ixc = 0; ixc < nbclosed && !cg->cg_errormsg; ixc++)
    {
      momitem_t *itmc = (momitem_t *) seqclosed->arritm[ixc];
      assert (itmc != NULL);
      cgen_lock_item_mom (cg, itmc);
      cgen_bind_closed_item_mom (cg, itmc);
    };
  MOM_DEBUGPRINTF (gencod, "in function %s done closed variables %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_output_gcstring (vclosed));
}				/* end cgen_bind_closed_variables_mom */

/// eof codgen.c
