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
  struct momattributes_st *cg_functionassoc;	/* associate each function with a node
						   function_info(<itm-signature>,<associtm-blocks>,<associtm-bindings>,<startitem>, <set-constants>,<set-closed>,<set-variables>) */
  const momstring_t *cg_errormsg;	/* the error message */
  struct momhashset_st *cg_lockeditemset;	/* the set of locked items */
  char *cg_emitbuffer;
  size_t cg_emitsize;
  FILE *cg_emitfile;		/* the emitted file */
  momitem_t *cg_curfunitm;	/* the current function item */
  momitem_t *cg_funsigitm;	/* the signature of the current function */
  struct momattributes_st *cg_funbind;	/* the function's bindings */
  struct momhashset_st *cg_funconstset;	/* the set of constant items */
  struct momhashset_st *cg_funclosedset;	/* the set of closed items */
  struct momhashset_st *cg_funvariableset;	/* the set of variable items */
  struct momattributes_st *cg_blockassoc;	/* the association of
						   block-s to a node ^block(<function>,<instruction-tuple>) */
  struct momqueueitems_st cg_blockqueue;	/* the queue of blocks to be scanned */
  momnode_t *cg_funinfonod;	/* information node about the function */
  momitem_t *cg_curblockitm;	/* the current block */
  momitem_t *cg_curstmtitm;	/* the current statement */
};				/* end struct codegen_mom_st */


enum funinfoindex_mom_en
{
  funinfo_signature = 0,
  funinfo_blocks = 1,
  funinfo_bindings = 2,
  funinfo_start = 3,
  funinfo_const = 4,
  funinfo_closed = 5,
  funinfo_vars = 6,
  funinfo_formals = 7,
  funinfo__last
};

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

static void cgen_first_scanning_pass_mom (momitem_t *itmcgen);

static void cgen_second_emitting_pass_mom (momitem_t *itmcgen);

static void cgen_third_decorating_pass_mom (momitem_t *itmcgen);

bool
  momfunc_1itm_to_val_generate_c_module
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
  itmcgen->itm_kind = MOM_PREDEFINED_NAMED (code_generation);
  itmcgen->itm_data1 = (void *) cg;
  itmcgen->itm_data2 = NULL;
  cgen_lock_item_mom (cg, itm);
  cgen_first_scanning_pass_mom (itmcgen);
  if (cg->cg_errormsg)
    goto end;
  cgen_second_emitting_pass_mom (itmcgen);
  if (cg->cg_errormsg)
    goto end;
  cgen_third_decorating_pass_mom (itmcgen);
  if (cg->cg_errormsg)
    goto end;
end:
  cgen_unlock_all_items_mom (cg);
  if (cg->cg_emitfile)
    fclose (cg->cg_emitfile);
  if (cg->cg_emitbuffer)
    {
      free (cg->cg_emitbuffer);
      cg->cg_emitbuffer = NULL;
      cg->cg_emitsize = 0;
    };
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
cgen_first_scanning_pass_mom (momitem_t *itmcgen)
{
  assert (itmcgen
	  && itmcgen->itm_kind == MOM_PREDEFINED_NAMED (code_generation));
  struct codegen_mom_st *cg = (struct codegen_mom_st *) itmcgen->itm_data1;
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  momitem_t *itmmod = cg->cg_moduleitm;
  assert (itmmod);
  if (itmmod->itm_kind != MOM_PREDEFINED_NAMED (code_module))
    CGEN_ERROR_RETURN_MOM (cg, "module item %s is not a `code_module`",
			   mom_item_cstring (itmmod));
  ///// prepare the module using its preparation
  momvalue_t resprepv = MOM_NONEV;
  momvalue_t prepv =		//
    mom_item_unsync_get_attribute (itmmod,
				   MOM_PREDEFINED_NAMED (preparation));
  MOM_DEBUGPRINTF (gencod, "preparation of %s is %s",
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
}				/* end cgen_first_scanning_pass_mom */


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
  MOM_DEBUGPRINTF (gencod, "cgen_scan_function_first scanning function %s",
		   mom_item_cstring (itmfun));
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
  MOM_DEBUGPRINTF (gencod, "scanning function %s itmsignature %s",
		   mom_item_cstring (itmfun),
		   mom_item_cstring (itmsignature));
  momvalue_t vstart =
    mom_item_unsync_get_attribute (itmfun, MOM_PREDEFINED_NAMED (start));
  if (vstart.typnum != momty_item)
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has bad `start` %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (itmfun),
			   mom_output_gcstring (vstart));
  momitem_t *itmstart = mom_value_to_item (vstart);
  MOM_DEBUGPRINTF (gencod, "scanning function %s itmstart %s",
		   mom_item_cstring (itmfun), mom_item_cstring (itmstart));
  /////
  momvalue_t vformals = MOM_NONEV;
  {				/* bind the formals */
    vformals =			//
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
  cg->cg_funsigitm = itmsignature;
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
  //// scan start and all the queued blocks 
  itmstart = vstart.vitem;
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
  momitem_t *itmbindings = mom_make_anonymous_item ();
  itmbindings->itm_space = momspa_transient;
  itmbindings->itm_kind = MOM_PREDEFINED_NAMED (association);
  itmbindings->itm_data1 = cg->cg_funbind;
  cgen_lock_item_mom (cg, itmbindings);
  MOM_DEBUGPRINTF (gencod, "for scanned function %s itmbindings %s",
		   mom_item_cstring (itmfun), mom_item_cstring (itmbindings));
  momitem_t *itmblocks = mom_make_anonymous_item ();
  itmblocks->itm_space = momspa_transient;
  itmblocks->itm_kind = MOM_PREDEFINED_NAMED (association);
  itmblocks->itm_data1 = cg->cg_blockassoc;
  cgen_lock_item_mom (cg, itmblocks);
  MOM_DEBUGPRINTF (gencod, "for scanned function %s itmblocks %s",
		   mom_item_cstring (itmfun), mom_item_cstring (itmblocks));
  momvalue_t vconstset = mom_hashset_elements_value (cg->cg_funconstset);
  MOM_DEBUGPRINTF (gencod, "for scanned function %s vconstset %s",
		   mom_item_cstring (itmfun),
		   mom_output_gcstring (vconstset));
  momvalue_t vclosedset = mom_hashset_elements_value (cg->cg_funclosedset);
  MOM_DEBUGPRINTF (gencod, "for scanned function %s vclosedset %s",
		   mom_item_cstring (itmfun),
		   mom_output_gcstring (vclosedset));
  momvalue_t vvarset = mom_hashset_elements_value (cg->cg_funvariableset);
  MOM_DEBUGPRINTF (gencod, "for scanned function %s vvarset %s",
		   mom_item_cstring (itmfun), mom_output_gcstring (vvarset));
  momvalue_t vfuninfo =		//
    mom_nodev_new (MOM_PREDEFINED_NAMED (function_info), funinfo__last,
		   mom_itemv (itmsignature),
		   mom_itemv (itmblocks),
		   mom_itemv (itmbindings),
		   mom_itemv (itmstart),
		   vconstset,
		   vclosedset,
		   vvarset,
		   vformals);
  vfuninfo.istransient = true;
  MOM_DEBUGPRINTF (gencod, "for scanned function %s vfuninfo %s",
		   mom_item_cstring (itmfun), mom_output_gcstring (vfuninfo));
  cg->cg_functionassoc =
    mom_attributes_put (cg->cg_functionassoc, itmfun, &vfuninfo);
  cg->cg_errormsg = NULL;
  cg->cg_curfunitm = NULL;
  cg->cg_funbind = NULL;
  cg->cg_funconstset = NULL;
  cg->cg_funvariableset = NULL;
  cg->cg_blockassoc = NULL;
  memset (&cg->cg_blockqueue, 0, sizeof (cg->cg_blockqueue));
  cg->cg_curblockitm = NULL;
  cg->cg_curstmtitm = NULL;
}				/* end cgen_scan_function_first_mom */



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
  if (itmblock->itm_kind != MOM_PREDEFINED_NAMED (block))
    CGEN_ERROR_RETURN_MOM
      (cg, "module item %s : function %s has invalid block %s (of kind %s)",
       mom_item_cstring (cg->cg_moduleitm),
       mom_item_cstring (cg->cg_curfunitm), mom_item_cstring (itmblock),
       mom_item_cstring (itmblock->itm_kind));
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
		&& vablock.vnode->conn == MOM_PREDEFINED_NAMED (block));
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
					      (instructions));
    if (vcinstrs.typnum == momty_node)
      {
	momvalue_t newvcinstrs = MOM_NONEV;
	if (mom_applval_2itm_to_val (vcinstrs, itmblock, cg->cg_codgenitm,
				     &newvcinstrs))
	  vcinstrs = newvcinstrs;
	else
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s has block %s with bad `instructions` closure %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_output_gcstring (vablock),
				 mom_output_gcstring (vcinstrs));
      };
    if (vcinstrs.typnum == momty_tuple)
      {
	vablock = mom_nodev_new (MOM_PREDEFINED_NAMED (block), 2,
				 mom_itemv (cg->cg_curfunitm), vcinstrs);
	cg->cg_blockassoc = mom_attributes_put (cg->cg_blockassoc,
						itmblock, &vablock);
      }
    else
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has block %s with bad `instructions` %s",
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

static int
cmp_intptr_mom (const void *p1, const void *p2)
{
  intptr_t i1 = *(const intptr_t *) p1;
  intptr_t i2 = *(const intptr_t *) p2;
  if (i1 < i2)
    return -1;
  else if (i1 > i2)
    return 1;
  return 0;
}

static momitem_t *cgen_type_of_scanned_nodexpr_mom (struct codegen_mom_st *cg,
						    momvalue_t vnodexpr);

static momitem_t *
cgen_type_of_scanned_item_mom (struct codegen_mom_st *cg, momitem_t *itm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itm != NULL);
  MOM_DEBUGPRINTF (gencod, "start cgen_type_of_scanned_item itm %s",
		   mom_item_cstring (itm));
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
		   mom_item_cstring (itm), mom_item_cstring (itmkind));
  if (!itmkind)
    CGEN_ERROR_RESULT_MOM (cg, NULL,
			   "module item %s : function %s has block %s with statement %s with bad kindless item %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (cg->cg_curstmtitm),
			   mom_item_cstring (itm));
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
								    (type)));
	if (!itmctyp)
	  CGEN_ERROR_RESULT_MOM (cg, NULL,
				 "module item %s : function %s has block %s with statement %s with untyped variable %s",
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
			 mom_item_cstring (itm), mom_item_cstring (itmkind));
	return MOM_PREDEFINED_NAMED (value);
      }
      break;
    }
  MOM_DEBUGPRINTF (gencod,
		   "function %s has unexpected item %s (%s)",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itm), mom_item_cstring (itmkind));
  CGEN_ERROR_RESULT_MOM (cg, NULL,
			 "module item %s : function %s has block %s with statement %s with unexpected item %s",
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
			     "module item %s : function %s has block %s with statement %s with bad expression %s",
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
    case momty_node:
      return cgen_type_of_scanned_nodexpr_mom (cg, vexpr);
    }
  return NULL;
}

static momitem_t *
cgen_type_of_scanned_nodexpr_mom (struct codegen_mom_st *cg,
				  const momvalue_t vnodexpr)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  const momnode_t *nod = mom_value_to_node (vnodexpr);
  MOM_DEBUGPRINTF (gencod, "start cgen_type_of_scanned_nodexpr_mom vnode %s",
		   mom_output_gcstring (vnodexpr));
  assert (nod != NULL);
  momitem_t *connitm = mom_node_conn (nod);
  cgen_lock_item_mom (cg, connitm);
  momvalue_t vtypscan = mom_item_unsync_get_attribute (connitm,
						       MOM_PREDEFINED_NAMED
						       (code_type_scanner));
  momvalue_t vcodemit = mom_item_unsync_get_attribute (connitm,
						       MOM_PREDEFINED_NAMED
						       (code_emitter));
  MOM_DEBUGPRINTF (gencod,
		   "start cgen_type_of_scanned_nodexpr_mom connitm %s vtypscan %s vcodemit %s",
		   mom_item_cstring (connitm), mom_output_gcstring (vtypscan),
		   mom_output_gcstring (vcodemit));
  if (vtypscan.typnum != momty_node)
    CGEN_ERROR_RESULT_MOM (cg, NULL,
			   "module item %s : function %s has block %s with expression %s with connective %s with bad `code_type_scanner`",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_output_gcstring (vnodexpr),
			   mom_item_cstring (connitm));
  if (vcodemit.typnum != momty_node)
    CGEN_ERROR_RESULT_MOM (cg, NULL,
			   "module item %s : function %s has block %s with expression %s with connective %s with bad `code_emitter`",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_output_gcstring (vnodexpr),
			   mom_item_cstring (connitm));
  momitem_t *itmtyp = NULL;
  // apply vtypscan to cgitem, vnodexpr
  if (!mom_applval_1itm1val_to_item (vtypscan, cg->cg_codgenitm, vnodexpr,
				     &itmtyp) || !itmtyp || cg->cg_errormsg)
    {
      if (cg->cg_errormsg)
	return NULL;
      CGEN_ERROR_RESULT_MOM (cg, NULL,
			     "module item %s : function %s has block %s with expression %s with connective %s failing to type",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (cg->cg_curblockitm),
			     mom_output_gcstring (vnodexpr),
			     mom_item_cstring (connitm));
    }
  MOM_DEBUGPRINTF (gencod,
		   "type_of_scanned_nodexpr blockitm %s connitm %s vnodexpr %s gives type %s",
		   mom_item_cstring (cg->cg_curblockitm),
		   mom_item_cstring (connitm), mom_output_gcstring (vnodexpr),
		   mom_item_cstring (itmtyp));
  return itmtyp;
}				/* end of cgen_type_of_scanned_nodexpr_mom */


////////////////
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
			   "module item %s : function %s with block %s with bad statement %s (%s)",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (itmstmt),
			   mom_item_cstring (itmstmt->itm_kind));
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
    case MOM_PREDEFINED_NAMED_CASE (jump, itmop, otherwiseoplab):
      {				/// jump <blockitm>
	momitem_t *itmjump = NULL;
	if (stmtlen != 2
	    || !(itmjump =
		 mom_value_to_item (mom_components_nth (stmtcomps, 1)))
	    || itmjump->itm_kind != MOM_PREDEFINED_NAMED (block))
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with bad jump statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
	cgen_lock_item_mom (cg, itmjump);
	cgen_scan_block_first_mom (cg, itmjump);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (success, itmop, otherwiseoplab):
      {				/// success
	if (stmtlen != 1)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with bad success statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (fail, itmop, otherwiseoplab):
      {				/// fail
	if (stmtlen != 1)
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with bad fail statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt));
      }
      break;
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
	cgen_lock_item_mom (cg, itmlvar);
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
			 "in function %s block %s set statement %s lvar %s lvarctypitm %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (cg->cg_curblockitm),
			 mom_item_cstring (itmstmt),
			 mom_item_cstring (itmlvar),
			 mom_item_cstring (lvarctypitm));
	cgen_lock_item_mom (cg, lvarctypitm);
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
	if (cg->cg_errormsg)
	  return;
	if (lvarctypitm == rexpctypitm)
	  break;
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
		if (itmarg->itm_kind == MOM_PREDEFINED_NAMED (block))
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
	  if (thenitm->itm_kind == MOM_PREDEFINED_NAMED (block))
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
	MOM_DEBUGPRINTF (gencod,
			 "in function %s apply statement %s with signature %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (itmstmt),
			 mom_item_cstring (sigitm));
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
	    MOM_DEBUGPRINTF (gencod,
			     "in function %s apply statement %s with result#%d : %s",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itmstmt),
			     inix, mom_item_cstring (incuritm));
	    if (!incuritm)
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s with bad result#%d",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), inix);
	    cgen_lock_item_mom (cg, incuritm);
	    const momitem_t *insigtypitm = mom_seq_nth (intyptup, inix);
	    momitem_t *incurtypitm =
	      cgen_type_of_scanned_item_mom (cg, incuritm);
	    if (insigtypitm && insigtypitm == incurtypitm)
	      continue;
	    else
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s with invalid result#%d type",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), inix);
	  };
	if (cg->cg_errormsg)
	  return;
	momvalue_t vexpfun = mom_components_nth (stmtcomps, 2 + nbin);
	momitem_t *itmtypfun = cgen_type_of_scanned_expr_mom (cg, vexpfun);
	MOM_DEBUGPRINTF (gencod,
			 "in function %s apply statement %s with function %s of type %s",
			 mom_item_cstring (cg->cg_curfunitm),
			 mom_item_cstring (itmstmt),
			 mom_output_gcstring (vexpfun),
			 mom_item_cstring (itmtypfun));
	if (itmtypfun != MOM_PREDEFINED_NAMED (value))
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with apply statement %s with invalid function %s type",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_item_cstring (itmstmt),
				 mom_output_gcstring (vexpfun));
	if (cg->cg_errormsg)
	  return;
	for (unsigned outix = 0; outix < nbout && !cg->cg_errormsg; outix++)
	  {
	    momvalue_t outcurv =
	      mom_components_nth (stmtcomps, 3 + nbin + outix);
	    MOM_DEBUGPRINTF (gencod,
			     "in function %s apply statement %s with argument#%d : %s",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itmstmt),
			     outix, mom_output_gcstring (outcurv));
	    const momitem_t *outsigtypitm = mom_seq_nth (outyptup, outix);
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
	if (cg->cg_errormsg)
	  return;
	if (stmtlen == 5 + nbin + nbout)
	  {
	    momitem_t *itmelse =
	      mom_value_to_item (mom_components_nth
				 (stmtcomps, 4 + nbin + nbout));
	    MOM_DEBUGPRINTF (gencod,
			     "in function %s apply statement %s with else %s",
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (itmstmt),
			     mom_item_cstring (itmelse));
	    if (!itmelse)
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with apply statement %s without else block",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt));
	    cgen_lock_item_mom (cg, itmelse);
	    if (itmelse->itm_kind == MOM_PREDEFINED_NAMED (block))
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
	if (cg->cg_errormsg)
	  return;
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
	intptr_t *intarr =
	  MOM_GC_SCALAR_ALLOC ("intarr", stmtlen * sizeof (intptr_t));
	if (typxitm != MOM_PREDEFINED_NAMED (integer))
	  CGEN_ERROR_RETURN_MOM (cg,
				 "module item %s : function %s with block %s with non-integer selector %s in int_switch statement %s",
				 mom_item_cstring (cg->cg_moduleitm),
				 mom_item_cstring (cg->cg_curfunitm),
				 mom_item_cstring (cg->cg_curblockitm),
				 mom_output_gcstring (exprv),
				 mom_item_cstring (itmstmt));
	unsigned nbcases = stmtlen - 2;
	for (unsigned ixc = 0; ixc < nbcases && !cg->cg_errormsg; ixc++)
	  {
	    momvalue_t casev = mom_components_nth (stmtcomps, ixc + 2);
	    const momnode_t *casnod = mom_value_to_node (casev);
	    momvalue_t casevalv = MOM_NONEV;
	    momvalue_t caseblockv = MOM_NONEV;
	    momitem_t *caseblockitm = NULL;
	    if (!casnod
		|| mom_node_conn (casnod) != MOM_PREDEFINED_NAMED (case)
		|| mom_node_arity (casnod) != 2
		|| ((casevalv = mom_node_nth (casnod, 0)).typnum != momty_int)
		|| ((caseblockv = mom_node_nth (casnod, 1)).typnum !=
		    momty_item)
		|| (!(caseblockitm = mom_value_to_item (caseblockv)))
		|| (cgen_lock_item_mom (cg, caseblockitm),
		    cgen_scan_block_first_mom (cg, caseblockitm),
		    caseblockitm->itm_kind != MOM_PREDEFINED_NAMED (block)))
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with int_switch statement %s with bad case#%d: %s",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), ixc,
				     mom_output_gcstring (casevalv));
	    if (cg->cg_errormsg)
	      return;
	    intarr[ixc] = casevalv.vint;
	  }
	qsort (intarr, nbcases, sizeof (intptr_t), cmp_intptr_mom);
	for (unsigned ix = 0; ix < nbcases - 1 && !cg->cg_errormsg; ix++)
	  if (intarr[ix] == intarr[ix + 1])
	    CGEN_ERROR_RETURN_MOM (cg,
				   "module item %s : function %s with block %s with int_switch statement %s with duplicate case number %lld",
				   mom_item_cstring (cg->cg_moduleitm),
				   mom_item_cstring (cg->cg_curfunitm),
				   mom_item_cstring (cg->cg_curblockitm),
				   mom_item_cstring (itmstmt),
				   (long long) intarr[ix]);
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
	struct momhashset_st *keyhset = NULL;
	unsigned nbcases = stmtlen - 2;
	for (unsigned ixc = 0; ixc < nbcases && !cg->cg_errormsg; ixc++)
	  {
	    momvalue_t casev = mom_components_nth (stmtcomps, ixc + 2);
	    const momnode_t *casnod = mom_value_to_node (casev);
	    momvalue_t casevalv = MOM_NONEV;
	    momvalue_t caseblockv = MOM_NONEV;
	    momitem_t *casevalitm = NULL;
	    momitem_t *caseblockitm = NULL;
	    if (!casnod
		|| mom_node_conn (casnod) != MOM_PREDEFINED_NAMED (case)
		|| mom_node_arity (casnod) != 2
		|| ((casevalv = mom_node_nth (casnod, 0)).typnum !=
		    momty_item)
		|| !(casevalitm = mom_value_to_item (casevalv))
		|| ((caseblockv = mom_node_nth (casnod, 1)).typnum !=
		    momty_item)
		|| (!(caseblockitm = mom_value_to_item (caseblockv)))
		|| (cgen_lock_item_mom (cg, caseblockitm),
		    cgen_scan_block_first_mom (cg, caseblockitm),
		    caseblockitm->itm_kind != MOM_PREDEFINED_NAMED (block)))
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with item_switch statement %s with bad case#%d: %s",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt), ixc,
				     mom_output_gcstring (casevalv));
	    cgen_lock_item_mom (cg, casevalitm);
	    if (mom_hashset_contains (keyhset, casevalitm))
	      CGEN_ERROR_RETURN_MOM (cg,
				     "module item %s : function %s with block %s with item_switch statement %s with duplicate case for %s",
				     mom_item_cstring (cg->cg_moduleitm),
				     mom_item_cstring (cg->cg_curfunitm),
				     mom_item_cstring (cg->cg_curblockitm),
				     mom_item_cstring (itmstmt),
				     mom_item_cstring (casevalitm));
	    keyhset = mom_hashset_put (keyhset, casevalitm);
	    cgen_bind_constant_item_mom (cg, casevalitm,
					 mom_itemv (casevalitm));
	  };
      };
      break;
    default:
    otherwiseoplab:
      if (itmop->itm_kind == MOM_PREDEFINED_NAMED (code_operation))
	{
	  cgen_lock_item_mom (cg, itmop);
	  momvalue_t codscanv = mom_item_unsync_get_attribute (itmop,
							       MOM_PREDEFINED_NAMED
							       (statement_scanner));
	  momvalue_t codemitv = mom_item_unsync_get_attribute (itmop,
							       MOM_PREDEFINED_NAMED
							       (statement_emitter));
	  if (codscanv.typnum != momty_node || codemitv.typnum != momty_node)
	    CGEN_ERROR_RETURN_MOM (cg,
				   "module item %s : function %s with block %s with statement %s with bad code operation %s",
				   mom_item_cstring (cg->cg_moduleitm),
				   mom_item_cstring (cg->cg_curfunitm),
				   mom_item_cstring (cg->cg_curblockitm),
				   mom_item_cstring (itmstmt),
				   mom_item_cstring (itmop));
	  if (!mom_applval_2itm_to_void (codscanv, cg->cg_codgenitm, itmstmt))
	    CGEN_ERROR_RETURN_MOM (cg,
				   "module item %s : function %s with block %s with statement %s failed to scan code operation %s",
				   mom_item_cstring (cg->cg_moduleitm),
				   mom_item_cstring (cg->cg_curfunitm),
				   mom_item_cstring (cg->cg_curblockitm),
				   mom_item_cstring (itmstmt),
				   mom_item_cstring (itmop));
	}
      else
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
  MOM_DEBUGPRINTF (gencod, "cgen_bind_new itm=%s vbind=%s",
		   mom_item_cstring (itm), mom_output_gcstring (vbind));
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
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_formals fun %s itmsignature=%s vinputy %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmsignature),
		   mom_output_gcstring (vinputy));
  momvalue_t voutputy =		//
    mom_item_unsync_get_attribute (itmsignature,
				   MOM_PREDEFINED_NAMED (output_types));
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_formals fun %s itmsignature=%s vinputy %s voutputy %s",
		   mom_item_cstring (cg->cg_curfunitm),
		   mom_item_cstring (itmsignature),
		   mom_output_gcstring (vinputy),
		   mom_output_gcstring (voutputy));
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
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_formals function %s nbins=%d",
		   mom_item_cstring (cg->cg_curfunitm), nbins);
  for (unsigned inix = 0; inix < nbins; inix++)
    {
      if (cg->cg_errormsg)
	return;
      momitem_t *intypitm = (momitem_t *) tupins->arritm[inix];
      assert (intypitm);
      momitem_t *informalitm = (momitem_t *) vformals.vtuple->arritm[inix];
      assert (informalitm);
      cgen_lock_item_mom (cg, intypitm);
      cgen_lock_item_mom (cg, informalitm);
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s inix#%d intypitm %s informalitm %s",
		       mom_item_cstring (cg->cg_curfunitm), inix,
		       mom_item_cstring (intypitm),
		       mom_item_cstring (informalitm));
      if (intypitm->itm_kind != MOM_PREDEFINED_NAMED (type))
	CGEN_ERROR_RETURN_MOM (cg,
			       "module item %s : function %s bad input type #%d %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm), inix,
			       mom_item_cstring (intypitm));
      momvalue_t valbind = mom_nodev_new (MOM_PREDEFINED_NAMED (formals),
					  4,
					  mom_itemv (cg->cg_curfunitm),
					  mom_intv (inix),
					  mom_itemv (MOM_PREDEFINED_NAMED
						     (input_types)),
					  mom_itemv (intypitm));
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s inix#%d informalitm %s valbind %s",
		       mom_item_cstring (cg->cg_curfunitm), inix,
		       mom_item_cstring (informalitm),
		       mom_output_gcstring (valbind));
      cgen_lock_item_mom (cg, informalitm);
      cgen_bind_new_mom (cg, informalitm, valbind);
      if (cg->cg_errormsg)
	return;
    }
  //// process output formals
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_formals function %s nbouts=%d",
		   mom_item_cstring (cg->cg_curfunitm), nbouts);
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
      cgen_lock_item_mom (cg, outtypitm);
      cgen_lock_item_mom (cg, outformalitm);
      if (outtypitm->itm_kind != MOM_PREDEFINED_NAMED (type))
	CGEN_ERROR_RETURN_MOM (cg,
			       "module item %s : function %s bad output type #%d %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm), outix,
			       mom_item_cstring (outtypitm));
      momvalue_t valbind = mom_nodev_new (MOM_PREDEFINED_NAMED (formals),
					  4,
					  mom_itemv (cg->cg_curfunitm),
					  mom_intv (outix),
					  mom_itemv (MOM_PREDEFINED_NAMED
						     (output_types)),
					  mom_itemv (outtypitm));
      MOM_DEBUGPRINTF (gencod,
		       "cgen_bind_formals function %s outix#%d outformalitm %s valbind %s",
		       mom_item_cstring (cg->cg_curfunitm), outix,
		       mom_item_cstring (outformalitm),
		       mom_output_gcstring (valbind));
      cgen_lock_item_mom (cg, outformalitm);
      cgen_bind_new_mom (cg, outformalitm, valbind);
      if (cg->cg_errormsg)
	return;
    }
  MOM_DEBUGPRINTF (gencod,
		   "cgen_bind_formals function %s done",
		   mom_item_cstring (cg->cg_curfunitm));
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
				   MOM_PREDEFINED_NAMED (type));
  momitem_t *itmctyp = mom_value_to_item (vctyp);
  if (!itmctyp || itmctyp->itm_kind != MOM_PREDEFINED_NAMED (type))
    CGEN_ERROR_RETURN_MOM (cg,
			   "module item %s : function %s has variable %s with bad `type` %s",
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


static void
cgen_emit_function_declaration_mom (struct codegen_mom_st *cg,
				    unsigned funix, momitem_t *curfunitm);

static void
cgen_emit_function_code_mom (struct codegen_mom_st *cg,
			     unsigned funix, momitem_t *curfunitm);

static void
cgen_second_emitting_pass_mom (momitem_t *itmcgen)
{
  assert (itmcgen
	  && itmcgen->itm_kind == MOM_PREDEFINED_NAMED (code_generation));
  struct codegen_mom_st *cg = (struct codegen_mom_st *) itmcgen->itm_data1;
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  momitem_t *itmmod = cg->cg_moduleitm;
  assert (itmmod);
  char modbuf[128];
  memset (modbuf, 0, sizeof (modbuf));
  if (snprintf (modbuf, sizeof (modbuf), MOM_SHARED_MODULE_PREFIX "%s.c",
		mom_item_cstring (itmmod)) >= (int) sizeof (modbuf))
    MOM_FATAPRINTF ("too wide module name %s", mom_item_cstring (itmmod));
  assert (cg->cg_emitfile == NULL && cg->cg_emitbuffer == NULL);
  {
    unsigned siz = 24576;
    cg->cg_emitbuffer = malloc (siz);
    if (!cg->cg_emitbuffer)
      MOM_FATAPRINTF
	("when emitting module %s failed to allocate buffer of %d bytes : %m",
	 mom_item_cstring (itmmod), siz);
    memset (cg->cg_emitbuffer, 0, siz);
    cg->cg_emitsize = siz;
    cg->cg_emitfile = open_memstream (&cg->cg_emitbuffer, &cg->cg_emitsize);
    if (!cg->cg_emitfile)
      MOM_FATAPRINTF
	("when emitting module %s failed to open memory output : %m",
	 mom_item_cstring (itmmod));
  }
  mom_output_gplv3_notice (cg->cg_emitfile, "///", "///", modbuf);
  fprintf (cg->cg_emitfile, "\n\n" "#include \"monimelt.h\"\n\n\n");
  const momseq_t *seqfun = mom_hashset_elements_set (cg->cg_functionhset);
  unsigned nbfun = mom_seq_length (seqfun);
  fprintf (cg->cg_emitfile, "\n" "/***** declaring %d functions *****/\n",
	   nbfun);
  for (unsigned funix = 0; funix < nbfun && !cg->cg_errormsg; funix++)
    {
      const momitem_t *curfunitm = mom_seq_nth (seqfun, funix);
      MOM_DEBUGPRINTF (gencod,
		       "emitting signature of curfunitm %s #%d in module %s",
		       mom_item_cstring (curfunitm), funix,
		       mom_item_cstring (itmmod));
      assert (curfunitm);
      cgen_emit_function_declaration_mom (cg, funix, (momitem_t *) curfunitm);
    };
  fprintf (cg->cg_emitfile,
	   "\n\n" "/***** implementing %d functions *****/\n", nbfun);
  for (unsigned funix = 0; funix < nbfun && !cg->cg_errormsg; funix++)
    {
      const momitem_t *curfunitm = mom_seq_nth (seqfun, funix);
      MOM_DEBUGPRINTF (gencod,
		       "emitting signature of curfunitm %s #%d in module %s",
		       mom_item_cstring (curfunitm), funix,
		       mom_item_cstring (itmmod));
      assert (curfunitm);
      cgen_emit_function_code_mom (cg, funix, (momitem_t *) curfunitm);
    };
  fprintf (cg->cg_emitfile, "\n\n" "/***** end %d functions *****/\n", nbfun);
  fprintf (cg->cg_emitfile,
	   "\n\n//// end of generated module file " MOM_SHARED_MODULE_PREFIX
	   "%s.c\n\n", mom_item_cstring (itmmod));
  fflush (cg->cg_emitfile);
  long buflen = ftell (cg->cg_emitfile);
  MOM_DEBUGPRINTF (gencod, "end cgen_second_emitting_pass_mom buffer:\n%s\n",
		   cg->cg_emitbuffer);
  bool same = true;
  {
    char oldpath[256];
    memset (oldpath, 0, sizeof (oldpath));
    if (snprintf
	(oldpath, sizeof (oldpath),
	 MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX "%s.c",
	 mom_item_cstring (itmmod)) < (int) sizeof (oldpath) - 1)
      {
	FILE *fold = fopen (oldpath, "r");
	if (fold)
	  {
	    for (int ix = 0; ix < (int) buflen && same; ix++)
	      {
		int c = fgetc (fold);
		if (c == EOF || c != cg->cg_emitbuffer[ix])
		  same = false;
	      }
	    fclose (fold);
	  }
	else
	  same = false;
      }
  };
  MOM_DEBUGPRINTF (gencod, "end cgen_second_emitting_pass_mom same %s",
		   same ? "true" : "false");
  const momstring_t *pbstr =	//
    mom_make_string_sprintf (MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX
			     "%s.c",
			     mom_item_cstring (itmmod));
  if (!same)
    {
      char backpath[256];
      memset (backpath, 0, sizeof (backpath));
      if (snprintf (backpath, sizeof (backpath), "%s~", pbstr->cstr) <
	  (int) sizeof (backpath))
	rename (pbstr->cstr, backpath);
      FILE *fnew = fopen (pbstr->cstr, "w");
      if (!fnew)
	MOM_FATAPRINTF ("failed to open new emitted C file %s : %m",
			pbstr->cstr);
      fwrite (cg->cg_emitbuffer, buflen, 1, fnew);
      fflush (fnew);
      fclose (fnew);
      MOM_INFORMPRINTF ("emitted file %s", pbstr->cstr);
    }
  else
    MOM_INFORMPRINTF ("keep same file %s", pbstr->cstr);
}				/* end cgen_second_emitting_pass_mom */


static void
cgen_emit_function_declaration_mom (struct codegen_mom_st *cg,
				    unsigned funix, momitem_t *curfunitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (curfunitm);
  momitem_t *itmmod = cg->cg_moduleitm;
  struct momentry_st *entfun =
    mom_attributes_find_entry (cg->cg_functionassoc, curfunitm);
  momvalue_t vfuninfo = MOM_NONEV;
  if (entfun)
    vfuninfo = entfun->ent_val;
  MOM_DEBUGPRINTF (gencod,
		   "cgen_emit_function_declaration funix#%d curfunitm %s vfuninfo %s",
		   funix, mom_item_cstring (curfunitm),
		   mom_output_gcstring (vfuninfo));
  const momnode_t *funinfonod = mom_value_to_node (vfuninfo);
  cg->cg_funinfonod = (momnode_t *) funinfonod;
  momitem_t *funsigitm = NULL;
  if (!funinfonod || mom_node_arity (funinfonod) != funinfo__last
      || mom_node_conn (funinfonod) != MOM_PREDEFINED_NAMED (function_info)
      || !(funsigitm =
	   mom_value_to_item (mom_node_nth (funinfonod, funinfo_signature)))
      || !mom_hashset_contains (cg->cg_lockeditemset, funsigitm)
      || funsigitm->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    MOM_FATAPRINTF
      ("corrupted function info %s for function %s (signature %s) of module %s",
       mom_output_gcstring (vfuninfo), mom_item_cstring (curfunitm),
       mom_item_cstring (funsigitm), mom_item_cstring (itmmod));
  momitem_t *itmstart =
    mom_value_to_item (mom_node_nth (funinfonod, funinfo_start));
  MOM_DEBUGPRINTF (gencod,
		   "emitting declaration signature %s of curfunitm %s",
		   mom_item_cstring (funsigitm),
		   mom_item_cstring (curfunitm));
  const momstring_t *strradix =	//
    mom_value_to_string (mom_item_unsync_get_attribute (funsigitm,
							MOM_PREDEFINED_NAMED
							(function_radix)));
  const momseq_t *seqinputs =	//
    mom_value_to_tuple (mom_item_unsync_get_attribute (funsigitm,
						       MOM_PREDEFINED_NAMED
						       (input_types)));
  const momseq_t *seqoutputs =	//
    mom_value_to_tuple (mom_item_unsync_get_attribute (funsigitm,
						       MOM_PREDEFINED_NAMED
						       (output_types)));
  if (!strradix || !seqinputs || !seqoutputs)
    MOM_FATAPRINTF ("function %s of module %s has bad signature %s",
		    mom_item_cstring (curfunitm),
		    mom_item_cstring (itmmod), mom_item_cstring (funsigitm));
  unsigned nbinputs = mom_seq_length (seqinputs);
  unsigned nboutputs = mom_seq_length (seqoutputs);
  fprintf (cg->cg_emitfile,
	   "\n\n" "/// declare function #%d: %s\n"
	   "extern bool momfunc_%s_%s (const momnode_t *", funix,
	   mom_item_cstring (curfunitm), mom_string_cstr (strradix),
	   mom_item_cstring (curfunitm));
  for (unsigned inix = 0; inix < nbinputs && !cg->cg_errormsg; inix++)
    {
      momitem_t *intypitm = (momitem_t *) mom_seq_nth (seqinputs, inix);
      MOM_DEBUGPRINTF (gencod,
		       "cgen_emit_function_declaration curfunitm %s funsigitm %s inix#%d intypitm %s (%s)",
		       mom_item_cstring (curfunitm),
		       mom_item_cstring (funsigitm), inix,
		       mom_item_cstring (intypitm),
		       mom_item_cstring (intypitm->itm_kind));
      const momstring_t *typstr = NULL;
      if (!intypitm || intypitm->itm_kind != MOM_PREDEFINED_NAMED (type)
	  || !mom_hashset_contains (cg->cg_lockeditemset, intypitm)
	  || !(typstr =
	       mom_value_to_string (mom_item_unsync_get_attribute (intypitm,
								   MOM_PREDEFINED_NAMED
								   (code)))))
	MOM_FATAPRINTF
	  ("function %s of module %s has signature %s with bad input #%d : %s",
	   mom_item_cstring (curfunitm), mom_item_cstring (itmmod),
	   mom_item_cstring (funsigitm), inix, mom_item_cstring (intypitm));
      fprintf (cg->cg_emitfile, ", %s", mom_string_cstr (typstr));
    }
  for (unsigned outix = 0; outix < nboutputs && !cg->cg_errormsg; outix++)
    {
      momitem_t *outtypitm = (momitem_t *) mom_seq_nth (seqoutputs, outix);
      const momstring_t *typstr = NULL;
      if (!outtypitm || outtypitm->itm_kind != MOM_PREDEFINED_NAMED (type)
	  || !mom_hashset_contains (cg->cg_lockeditemset, outtypitm)
	  || !(typstr =
	       mom_value_to_string (mom_item_unsync_get_attribute (outtypitm,
								   MOM_PREDEFINED_NAMED
								   (code)))))
	MOM_FATAPRINTF
	  ("function %s of module %s has signature %s with bad output #%d : %s",
	   mom_item_cstring (curfunitm), mom_item_cstring (itmmod),
	   mom_item_cstring (funsigitm), outix, mom_item_cstring (outtypitm));
      fprintf (cg->cg_emitfile, ", %s*", mom_string_cstr (typstr));
    };
  fprintf (cg->cg_emitfile, ");\n");
  cg->cg_funinfonod = NULL;
}				/* end cgen_emit_function_declaration_mom */


// prefix for labels in emitted code
#define BLOCK_LABEL_PREFIX_MOM "momblocklab"

// prefix for constants in emitted code
#define CONSTANT_PREFIX_MOM "momconst"

// prefix for variables in emitted code
#define VARIABLE_PREFIX_MOM "momvar"

// prefix for closed values in emitted code
#define CLOSED_PREFIX_MOM "momclosed"

// prefix for arguments in emitted code
#define ARGUMENT_PREFIX_MOM "momarg"

// prefix for result-pointer in emitted code
#define RESULT_PREFIX_MOM "mompres"

// prefix for output-result in emitted code
#define OUTPUT_PREFIX_MOM "momout"

// prefix for epilogue
#define EPILOGUE_PREFIX_MOM "momepilog"

// prefix for success-flag
#define SUCCESS_PREFIX_MOM "momsuccess"
static void
cgen_emit_block_mom (struct codegen_mom_st *cg, unsigned bix,
		     momitem_t *blockitm);


static void
cgen_emit_vardecl_mom (struct codegen_mom_st *cg, unsigned varix,
		       momitem_t *varitm);


static void
cgen_emit_outdecl_mom (struct codegen_mom_st *cg, unsigned outix,
		       momitem_t *outypitm, momitem_t *outformalitm);

static void
cgen_emit_constdecl_mom (struct codegen_mom_st *cg, unsigned constix,
			 momitem_t *constitm);


static void
cgen_emit_closeddecl_mom (struct codegen_mom_st *cg, unsigned closix,
			  momitem_t *clositm);

static void cgen_emit_item_mom (struct codegen_mom_st *cg, momitem_t *itm);
static void cgen_emit_expr_mom (struct codegen_mom_st *cg, momvalue_t vexpr);

////////////////
static void
cgen_emit_function_code_mom (struct codegen_mom_st *cg,
			     unsigned funix, momitem_t *curfunitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (curfunitm);
  momitem_t *itmmod = cg->cg_moduleitm;
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code start funix#%d itmmod %s curfunitm %s",
		   funix, mom_item_cstring (itmmod),
		   mom_item_cstring (curfunitm));
  struct momentry_st *entfun =
    mom_attributes_find_entry (cg->cg_functionassoc, curfunitm);
  momvalue_t vfuninfo = MOM_NONEV;
  if (entfun)
    vfuninfo = entfun->ent_val;
  const momnode_t *funinfonod = mom_value_to_node (vfuninfo);
  momitem_t *funsigitm = NULL;
  momitem_t *funblocksitm = NULL;
  momitem_t *funbindingsitm = NULL;
  momitem_t *funstartitm = NULL;
  const momseq_t *funseqconsts = NULL;
  const momseq_t *funseqclosed = NULL;
  const momseq_t *funseqvars = NULL;
  const momseq_t *funseqformals = NULL;
  if (!funinfonod || mom_node_arity (funinfonod) != funinfo__last
      || mom_node_conn (funinfonod) != MOM_PREDEFINED_NAMED (function_info)
      || !(funsigitm =
	   mom_value_to_item (mom_node_nth (funinfonod, funinfo_signature)))
      || !mom_hashset_contains (cg->cg_lockeditemset, funsigitm)
      || funsigitm->itm_kind != MOM_PREDEFINED_NAMED (function_signature)
      || !(funblocksitm =
	   mom_value_to_item (mom_node_nth (funinfonod, funinfo_blocks)))
      || !mom_hashset_contains (cg->cg_lockeditemset, funblocksitm)
      || !(funbindingsitm =
	   mom_value_to_item (mom_node_nth (funinfonod, funinfo_bindings)))
      || !mom_hashset_contains (cg->cg_lockeditemset, funbindingsitm)
      || !(funstartitm =
	   mom_value_to_item (mom_node_nth (funinfonod, funinfo_start)))
      || !mom_hashset_contains (cg->cg_lockeditemset, funstartitm))
    MOM_FATAPRINTF
      ("corrupted function info %s for function %s (signature %s, bindings %s) of module %s",
       mom_output_gcstring (vfuninfo), mom_item_cstring (curfunitm),
       mom_item_cstring (funsigitm), mom_item_cstring (funbindingsitm),
       mom_item_cstring (itmmod));
  cg->cg_funinfonod = (momnode_t *) funinfonod;
  if (funbindingsitm
      && funbindingsitm->itm_kind == MOM_PREDEFINED_NAMED (association))
    cg->cg_funbind = (struct momattributes_st *) funbindingsitm->itm_data1;
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code curfunitm %s funsigitm %s funblocksitm %s funbindingsitm %s funstartitm %s",
		   mom_item_cstring (curfunitm),
		   mom_item_cstring (funsigitm),
		   mom_item_cstring (funblocksitm),
		   mom_item_cstring (funbindingsitm),
		   mom_item_cstring (funstartitm));
  //
  funseqconsts = mom_value_to_set (mom_node_nth (funinfonod, funinfo_const));
  funseqclosed = mom_value_to_set (mom_node_nth (funinfonod, funinfo_closed));
  funseqvars = mom_value_to_set (mom_node_nth (funinfonod, funinfo_vars));
  funseqformals =
    mom_value_to_tuple (mom_node_nth (funinfonod, funinfo_formals));
  assert (funblocksitm);
  MOM_DEBUGPRINTF (gencod, "emit_function_code funblocksitm %s (%s)",
		   mom_item_cstring (funblocksitm),
		   mom_item_cstring (funblocksitm->itm_kind));
  assert (funblocksitm->itm_kind == MOM_PREDEFINED_NAMED (association));
  momvalue_t vblockset = MOM_NONEV;
  if (funblocksitm->itm_kind == MOM_PREDEFINED_NAMED (association))
    vblockset =
      mom_unsafe_setv (mom_attributes_set
		       ((struct momattributes_st *) funblocksitm->itm_data1,
			MOM_NONEV));
  MOM_DEBUGPRINTF (gencod, "emit_function_code funblocksitm %s vblockset %s",
		   mom_item_cstring (funblocksitm),
		   mom_output_gcstring (vblockset));
  //
  cg->cg_curfunitm = curfunitm;
  MOM_DEBUGPRINTF (gencod, "emitting code signature %s of curfunitm %s",
		   mom_item_cstring (funsigitm),
		   mom_item_cstring (curfunitm));
  const momstring_t *strradix =	//
    mom_value_to_string (mom_item_unsync_get_attribute (funsigitm,
							MOM_PREDEFINED_NAMED
							(function_radix)));
  const momseq_t *seqinputs =	//
    mom_value_to_tuple (mom_item_unsync_get_attribute (funsigitm,
						       MOM_PREDEFINED_NAMED
						       (input_types)));
  const momseq_t *seqoutputs =	//
    mom_value_to_tuple (mom_item_unsync_get_attribute (funsigitm,
						       MOM_PREDEFINED_NAMED
						       (output_types)));
  if (!strradix || !seqinputs || !seqoutputs)
    MOM_FATAPRINTF ("function %s of module %s has bad signature %s",
		    mom_item_cstring (curfunitm),
		    mom_item_cstring (itmmod), mom_item_cstring (funsigitm));
  unsigned nbinputs = mom_seq_length (seqinputs);
  unsigned nboutputs = mom_seq_length (seqoutputs);
  unsigned nbconsts = mom_seq_length (funseqconsts);
  unsigned nbclosed = mom_seq_length (funseqclosed);
  unsigned nbvars = mom_seq_length (funseqvars);
  fprintf (cg->cg_emitfile,
	   "\n\n" "/// implement function #%d: %s\n"
	   "bool momfunc_%s_%s (const momnode_t *mom_node", funix,
	   mom_item_cstring (curfunitm), mom_string_cstr (strradix),
	   mom_item_cstring (curfunitm));
  assert (funseqformals);
  for (unsigned inix = 0; inix < nbinputs && !cg->cg_errormsg; inix++)
    {
      momitem_t *intypitm = (momitem_t *) mom_seq_nth (seqinputs, inix);
      const momstring_t *typstr = NULL;
      if (!intypitm || intypitm->itm_kind != MOM_PREDEFINED_NAMED (type)
	  || !mom_hashset_contains (cg->cg_lockeditemset, intypitm)
	  || !(typstr =
	       mom_value_to_string (mom_item_unsync_get_attribute (intypitm,
								   MOM_PREDEFINED_NAMED
								   (code)))))
	MOM_FATAPRINTF
	  ("function %s of module %s has signature %s with bad input #%d : %s",
	   mom_item_cstring (curfunitm), mom_item_cstring (itmmod),
	   mom_item_cstring (funsigitm), inix, mom_item_cstring (intypitm));
      fprintf (cg->cg_emitfile, ", %s " ARGUMENT_PREFIX_MOM "%d",
	       mom_string_cstr (typstr), inix);
    }
  for (unsigned outix = 0; outix < nboutputs && !cg->cg_errormsg; outix++)
    {
      momitem_t *outtypitm = (momitem_t *) mom_seq_nth (seqoutputs, outix);
      const momstring_t *typstr = NULL;
      if (!outtypitm || outtypitm->itm_kind != MOM_PREDEFINED_NAMED (type)
	  || !mom_hashset_contains (cg->cg_lockeditemset, outtypitm)
	  || !(typstr =
	       mom_value_to_string (mom_item_unsync_get_attribute (outtypitm,
								   MOM_PREDEFINED_NAMED
								   (code)))))
	MOM_FATAPRINTF
	  ("function %s of module %s has signature %s with bad output #%d : %s",
	   mom_item_cstring (curfunitm), mom_item_cstring (itmmod),
	   mom_item_cstring (funsigitm), outix, mom_item_cstring (outtypitm));
      fprintf (cg->cg_emitfile, ", %s* " RESULT_PREFIX_MOM "%d",
	       mom_string_cstr (typstr), outix);
    };
  /// emit the early prologue
  fprintf (cg->cg_emitfile, ")\n{ // body of function %s\n",
	   mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, "  bool " SUCCESS_PREFIX_MOM "_%s = false;\n",
	   mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, "  momitem_t* mom_funcitm = NULL;\n");
  fprintf (cg->cg_emitfile, "  if (MOM_UNLIKELY(!mom_node\n");
  if (nbclosed > 0)
    fprintf (cg->cg_emitfile,
	     "      || mom_node_arity(mom_node) < %d)\n", nbclosed);
  fprintf (cg->cg_emitfile,
	   "      || !(mom_funcitm = mom_node_conn(mom_node))\n");
  if (nbconsts > 0)
    fprintf (cg->cg_emitfile,
	     "      || mom_unsync_item_components_count(mom_funcitm)<%d\n",
	     nbconsts);
  fprintf (cg->cg_emitfile, "       ))\n" "  return false;\n");
  /// emit the declaration of output variables
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code function %s has %u output results",
		   mom_item_cstring (curfunitm), nboutputs);
  fprintf (cg->cg_emitfile, "  // %u output results:\n", nboutputs);
  for (unsigned outix = 0; outix < nboutputs; outix++)
    {
      momitem_t *outypitm = (momitem_t *) mom_seq_nth (seqoutputs, outix);
      momitem_t *outformalitm =
	(momitem_t *) mom_seq_nth (funseqformals, nbinputs + outix);
      MOM_DEBUGPRINTF (gencod,
		       "emit_function_code function %s outix#%d outypitm %s outformalitm %s",
		       mom_item_cstring (curfunitm), outix,
		       mom_item_cstring (outypitm),
		       mom_item_cstring (outformalitm));
      cgen_emit_outdecl_mom (cg, outix, outypitm, outformalitm);
    };
  /// emit the variables declaration
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code function %s has %u variables",
		   mom_item_cstring (curfunitm), nbvars);
  fprintf (cg->cg_emitfile, "  // %u variables:\n", nbvars);
  for (unsigned varix = 0; varix < nbvars; varix++)
    {
      momitem_t *varitm = (momitem_t *) mom_seq_nth (funseqvars, varix);
      MOM_DEBUGPRINTF (gencod,
		       "emit_function_code function %s varix#%d varitm %s",
		       mom_item_cstring (curfunitm), varix,
		       mom_item_cstring (varitm));
      cgen_emit_vardecl_mom (cg, varix, varitm);
    }
  /// emit the constants declaration
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code function %s has %u constants",
		   mom_item_cstring (curfunitm), nbconsts);
  fprintf (cg->cg_emitfile, "  // %u constants:\n", nbconsts);
  for (unsigned constix = 0; constix < nbconsts; constix++)
    {
      momitem_t *constitm = (momitem_t *) mom_seq_nth (funseqconsts, constix);
      MOM_DEBUGPRINTF (gencod,
		       "emit_function_code function %s constix#%d constitm %s",
		       mom_item_cstring (curfunitm), constix,
		       mom_item_cstring (constitm));
      cgen_emit_constdecl_mom (cg, constix, constitm);
    }
  /// emit the closed declaration
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code function %s has %u closed",
		   mom_item_cstring (curfunitm), nbclosed);
  fprintf (cg->cg_emitfile, "  // %u closed:\n", nbclosed);
  for (unsigned closix = 0; closix < nbclosed; closix++)
    {
      momitem_t *clositm = (momitem_t *) mom_seq_nth (funseqclosed, closix);
      MOM_DEBUGPRINTF (gencod,
		       "emit_function_code function %s closix#%d clositm %s",
		       mom_item_cstring (curfunitm), closix,
		       mom_item_cstring (clositm));
      cgen_emit_closeddecl_mom (cg, closix, clositm);
    }
  /// emit the goto start
  if (funstartitm)
    fprintf (cg->cg_emitfile, "  goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
	     mom_item_cstring (funstartitm));
  /// emit the blocks
  const momseq_t *blockseq = mom_value_to_set (vblockset);
  unsigned nbblocks = mom_seq_length (blockseq);
  for (unsigned bix = 0; bix < nbblocks; bix++)
    {
      momitem_t *blockitm = (momitem_t *) mom_seq_nth (blockseq, bix);
      fprintf (cg->cg_emitfile, " // block #%d : %s\n", bix,
	       mom_item_cstring (blockitm));
      cg->cg_curblockitm = blockitm;
      cgen_emit_block_mom (cg, bix, blockitm);
      cg->cg_curblockitm = NULL;
    }
  /// emit the epilogue
  fprintf (cg->cg_emitfile, "//////\n"
	   "// epilogue of %s\n", mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, "    " SUCCESS_PREFIX_MOM "_%s = true;\n",
	   mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, "    goto " EPILOGUE_PREFIX_MOM "_%s;\n",
	   mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, " " EPILOGUE_PREFIX_MOM "_%s:\n",
	   mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, "// give %d outputs\n", nboutputs);
  if (nboutputs > 0)
    {
      fprintf (cg->cg_emitfile, "  if (" SUCCESS_PREFIX_MOM "_%s) {\n",
	       mom_item_cstring (curfunitm));
      for (unsigned outix = 0; outix < nboutputs; outix++)
	{
	  momitem_t *outypitm = (momitem_t *) mom_seq_nth (seqoutputs, outix);
	  momitem_t *outformalitm =
	    (momitem_t *) mom_seq_nth (funseqformals, nbinputs + outix);
	  MOM_DEBUGPRINTF (gencod,
			   "emit_function_code function %s epilog outix#%d outypitm %s outformalitm %s",
			   mom_item_cstring (curfunitm), outix,
			   mom_item_cstring (outypitm),
			   mom_item_cstring (outformalitm));
	  fprintf (cg->cg_emitfile,
		   "   if (" RESULT_PREFIX_MOM "%d != NULL)  *"
		   RESULT_PREFIX_MOM "%d = ", outix, outix);
	  cgen_emit_item_mom (cg, outformalitm);
	  fputs (";\n", cg->cg_emitfile);
	}
      fprintf (cg->cg_emitfile, "  }; // end success %s\n",
	       mom_item_cstring (curfunitm));
    }
  fprintf (cg->cg_emitfile, "  return " SUCCESS_PREFIX_MOM "_%s;\n",
	   mom_item_cstring (curfunitm));
  fprintf (cg->cg_emitfile, "} // end of momfunc_%s_%s \n\n",
	   mom_string_cstr (strradix), mom_item_cstring (curfunitm));
  MOM_DEBUGPRINTF (gencod,
		   "emit_function_code done funix#%d itmmod %s curfunitm %s",
		   funix, mom_item_cstring (itmmod),
		   mom_item_cstring (curfunitm));
  cg->cg_funinfonod = NULL;
  cg->cg_funbind = NULL;
}				/* end cgen_emit_function_code_mom */


static void
cgen_emit_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			 momitem_t *insitm);

static void
cgen_emit_block_mom (struct codegen_mom_st *cg, unsigned bix,
		     momitem_t *blockitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (blockitm);
  momitem_t *itmmod = cg->cg_moduleitm;
  momitem_t *curfunitm = cg->cg_curfunitm;
  MOM_DEBUGPRINTF (gencod,
		   "emit_block start itmmod %s curfunitm %s bix#%d blockitm %s",
		   mom_item_cstring (itmmod),
		   mom_item_cstring (curfunitm),
		   bix, mom_item_cstring (blockitm));
  momitem_t *blocksitm =
    mom_value_to_item (mom_node_nth (cg->cg_funinfonod, funinfo_blocks));
  assert (blocksitm
	  && blocksitm->itm_kind == MOM_PREDEFINED_NAMED (association));
  momvalue_t vbindblock = MOM_NONEV;
  if (blocksitm && blocksitm->itm_kind == MOM_PREDEFINED_NAMED (association))
    {
      struct momentry_st *ent =
	mom_attributes_find_entry ((struct momattributes_st *)
				   blocksitm->itm_data1, blockitm);
      if (ent)
	vbindblock = ent->ent_val;
    };
  MOM_DEBUGPRINTF (gencod,
		   "emit_block blockitm %s vbindblock %s",
		   mom_item_cstring (blockitm),
		   mom_output_gcstring (vbindblock));
  fprintf (cg->cg_emitfile, " " BLOCK_LABEL_PREFIX_MOM "_%s: {\n",
	   mom_item_cstring (blockitm));
  const momseq_t *tupblock =
    mom_value_to_tuple (mom_node_nth (mom_value_to_node (vbindblock), 1));
  assert (tupblock != NULL);
  unsigned nbinstr = mom_seq_length (tupblock);
  fprintf (cg->cg_emitfile, "// %u statements in block %s\n", nbinstr,
	   mom_item_cstring (blockitm));
  MOM_DEBUGPRINTF (gencod, "emit_block blockitm %s nbinstr %u",
		   mom_item_cstring (blockitm), nbinstr);
  for (unsigned insix = 0; insix < nbinstr; insix++)
    {
      momitem_t *insitm = (momitem_t *) mom_seq_nth (tupblock, insix);
      MOM_DEBUGPRINTF (gencod, "emit_block blockitm %s insix#%d insitm %s",
		       mom_item_cstring (blockitm), insix,
		       mom_item_cstring (insitm));
      fprintf (cg->cg_emitfile, "// statement #%d %s\n", insix,
	       mom_item_cstring (insitm));
      cg->cg_curstmtitm = insitm;
      cgen_emit_statement_mom (cg, insix, insitm);
      cg->cg_curstmtitm = NULL;
    }
  fprintf (cg->cg_emitfile, "\n  }; // end block %s\n",
	   mom_item_cstring (blockitm));
  MOM_DEBUGPRINTF (gencod,
		   "emit_block done itmmod %s curfunitm %s bix#%d blockitm %s",
		   mom_item_cstring (itmmod),
		   mom_item_cstring (curfunitm), bix,
		   mom_item_cstring (blockitm));
}


void
cgen_emit_vardecl_mom (struct codegen_mom_st *cg, unsigned varix,
		       momitem_t *varitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod,
		   "emit_vardecl start varix#%u varitm %s",
		   varix, mom_item_cstring (varitm));
  momitem_t *binditm =
    mom_value_to_item (mom_node_nth (cg->cg_funinfonod, funinfo_bindings));
  assert (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association));
  momvalue_t vbindvar = MOM_NONEV;
  if (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association))
    {
      struct momentry_st *ent =
	mom_attributes_find_entry ((struct momattributes_st *)
				   binditm->itm_data1, varitm);
      if (ent)
	vbindvar = ent->ent_val;
    };
  MOM_DEBUGPRINTF (gencod,
		   "emit_vardecl varix#%u varitm %s vbindvar %s",
		   varix, mom_item_cstring (varitm),
		   mom_output_gcstring (vbindvar));
  const momnode_t *nodvar = mom_value_to_node (vbindvar);
  assert (nodvar != NULL
	  && mom_node_conn (nodvar) == MOM_PREDEFINED_NAMED (variable));
  intptr_t varrk = mom_value_to_int (mom_node_nth (nodvar, 1), -1);
  momitem_t *vartypitm = mom_value_to_item (mom_node_nth (nodvar, 2));
  MOM_DEBUGPRINTF (gencod,
		   "emit_vardecl varix#%u varitm %s varrk#%ld vartypitm %s",
		   varix, mom_item_cstring (varitm), (long) varrk,
		   mom_item_cstring (vartypitm));
  assert (varrk >= 0);
  fprintf (cg->cg_emitfile, "// variable %s of type %s\n",
	   mom_item_cstring (varitm), mom_item_cstring (vartypitm));
  const momstring_t *typstr =	//
    mom_value_to_string (mom_item_unsync_get_attribute (vartypitm,
							MOM_PREDEFINED_NAMED
							(code)));
  assert (typstr != NULL);
  if (vartypitm == MOM_PREDEFINED_NAMED (value))
    fprintf (cg->cg_emitfile,
	     "  momvalue_t " VARIABLE_PREFIX_MOM "%d = MOM_NONEV;\n",
	     (int) varrk);
  else
    fprintf (cg->cg_emitfile, "  %s " VARIABLE_PREFIX_MOM "%d = (%s)0;\n",
	     typstr->cstr, (int) varrk, typstr->cstr);
}				/* end of cgen_emit_vardecl_mom */



static void
cgen_emit_outdecl_mom (struct codegen_mom_st *cg, unsigned outix,
		       momitem_t *outypitm, momitem_t *outformalitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod,
		   "emit_outdecl start outix#%u outypitm %s outformalitm %s",
		   outix, mom_item_cstring (outypitm),
		   mom_item_cstring (outformalitm));
  momitem_t *binditm =
    mom_value_to_item (mom_node_nth (cg->cg_funinfonod, funinfo_bindings));
  assert (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association));
  momvalue_t vbindout = MOM_NONEV;
  if (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association))
    {
      struct momentry_st *ent =
	mom_attributes_find_entry ((struct momattributes_st *)
				   binditm->itm_data1, outformalitm);
      if (ent)
	vbindout = ent->ent_val;
    };
  MOM_DEBUGPRINTF (gencod,
		   "emit_outdecl outix#%u outformalitm %s outypitm %s vbindout %s",
		   outix, mom_item_cstring (outformalitm),
		   mom_item_cstring (outypitm),
		   mom_output_gcstring (vbindout));
  const momnode_t *nodout = mom_value_to_node (vbindout);
  const momstring_t *typstr =	//
    mom_value_to_string (mom_item_unsync_get_attribute (outypitm,
							MOM_PREDEFINED_NAMED
							(code)));
  assert (nodout != NULL
	  && mom_node_conn (nodout) == MOM_PREDEFINED_NAMED (formals));
  int outrk = (int) mom_value_to_int (mom_node_nth (nodout, 1), -1);
  assert (outrk >= 0);
  if (outypitm == MOM_PREDEFINED_NAMED (value))
    fprintf (cg->cg_emitfile,
	     "   momvalue_t " OUTPUT_PREFIX_MOM "%d = MOM_NONEV;\n", outrk);
  else
    fprintf (cg->cg_emitfile, "  %s " OUTPUT_PREFIX_MOM "%d = (%s)0;\n",
	     typstr->cstr, (int) outrk, typstr->cstr);
}				/* end of cgen_emit_outdecl_mom */

void
cgen_emit_constdecl_mom (struct codegen_mom_st *cg, unsigned constix,
			 momitem_t *constitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod,
		   "emit_constdecl start constix#%u constitm %s",
		   constix, mom_item_cstring (constitm));
  momitem_t *binditm =
    mom_value_to_item (mom_node_nth (cg->cg_funinfonod, funinfo_bindings));
  assert (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association));
  momvalue_t vbindconst = MOM_NONEV;
  if (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association))
    {
      struct momentry_st *ent =
	mom_attributes_find_entry ((struct momattributes_st *)
				   binditm->itm_data1, constitm);
      if (ent)
	vbindconst = ent->ent_val;
    };
  MOM_DEBUGPRINTF (gencod,
		   "emit_constdecl constix#%u constitm %s vbindconst %s",
		   constix, mom_item_cstring (constitm),
		   mom_output_gcstring (vbindconst));
  const momnode_t *nodbindconst = mom_value_to_node (vbindconst);
  assert (nodbindconst && mom_node_arity (nodbindconst) == 3);
  intptr_t constrk = mom_value_to_int (mom_node_nth (nodbindconst, 1), -1);
  momvalue_t constval = mom_node_nth (nodbindconst, 2);
  assert (constrk >= 0 && constval.typnum != momty_null);
  fprintf (cg->cg_emitfile, "  // constant %s\n",
	   mom_item_cstring (constitm));
  fprintf (cg->cg_emitfile,
	   "  const momvalue_t " CONSTANT_PREFIX_MOM "_%d = ", (int) constrk);
  {
    momitem_t *constitm = NULL;
    if (constval.typnum == momty_item
	&& (constitm = constval.vitem)->itm_space == momspa_predefined)
      {
	if (constitm->itm_anonymous)
	  fprintf (cg->cg_emitfile,
		   " mom_itemv(MOM_PREDEFINED_ANONYMOUS(%s))",
		   mom_item_cstring (constitm));
	else
	  fprintf (cg->cg_emitfile, " mom_itemv(MOM_PREDEFINED_NAMED(%s))",
		   mom_item_cstring (constitm));
      }
    else
      {
	fprintf (cg->cg_emitfile,
		 "\n    mom_raw_item_get_indexed_component (mom_funcitm, %d)",
		 (int) constrk);
      }
  }
  fputs (";\n", cg->cg_emitfile);
}				/* end of cgen_emit_constdecl_mom */


void
cgen_emit_closeddecl_mom (struct codegen_mom_st *cg, unsigned closix,
			  momitem_t *clositm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod,
		   "emit_closeddecl start closix#%u clositm %s",
		   closix, mom_item_cstring (clositm));
  momitem_t *binditm =
    mom_value_to_item (mom_node_nth (cg->cg_funinfonod, funinfo_bindings));
  assert (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association));
  momvalue_t vbindclos = MOM_NONEV;
  if (binditm && binditm->itm_kind == MOM_PREDEFINED_NAMED (association))
    {
      struct momentry_st *ent =
	mom_attributes_find_entry ((struct momattributes_st *)
				   binditm->itm_data1, clositm);
      if (ent)
	vbindclos = ent->ent_val;
    };
  MOM_DEBUGPRINTF (gencod,
		   "emit_closeddecl closix#%u clositm %s vbindclos %s",
		   closix, mom_item_cstring (clositm),
		   mom_output_gcstring (vbindclos));
  MOM_WARNPRINTF
    ("unimplemented cgen_emit_closeddecl_mom closix#%u clositm %s vbindclos %s",
     closix, mom_item_cstring (clositm), mom_output_gcstring (vbindclos));
#warning cgen_emit_closeddecl_mom unimplemented
}				/* end of cgen_emit_closeddecl_mom */




static void
cgen_emit_item_mom (struct codegen_mom_st *cg, momitem_t *itm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itm != NULL);
  momvalue_t vbind = MOM_NONEV;
  {
    struct momentry_st *ent = mom_attributes_find_entry (cg->cg_funbind, itm);
    if (ent)
      vbind = ent->ent_val;
  }
  MOM_DEBUGPRINTF (gencod, "cgen_emit_item itm %s vbind %s",
		   mom_item_cstring (itm), mom_output_gcstring (vbind));
  const momnode_t *bindnod = mom_value_to_node (vbind);
  assert (bindnod);
  momitem_t *natitm = mom_node_conn (bindnod);
  if (natitm == NULL)
    MOM_FATAPRINTF ("corrupted item %s to emit vbind %s",
		    mom_item_cstring (itm), mom_output_gcstring (vbind));
  switch (mom_item_hash (natitm))
    {
    case MOM_PREDEFINED_NAMED_CASE (variable, natitm, otherwisenatlab):
      {
	intptr_t varrk = mom_value_to_int (mom_node_nth (bindnod, 1), -1);
	assert (varrk >= 0);
	fprintf (cg->cg_emitfile, VARIABLE_PREFIX_MOM "%d /*var:%s*/",
		 (int) varrk, mom_item_cstring (itm));
      }
      break;
    case MOM_PREDEFINED_NAMED_CASE (formals, natitm, otherwisenatlab):
      {
	intptr_t varrk = mom_value_to_int (mom_node_nth (bindnod, 1), -1);
	assert (varrk >= 0);
	momitem_t *diritm = mom_value_to_item (mom_node_nth (bindnod, 2));
	if (diritm == MOM_PREDEFINED_NAMED (input_types))
	  {
	    fprintf (cg->cg_emitfile,
		     ARGUMENT_PREFIX_MOM "%d /*formalarg:%s*/", (int) varrk,
		     mom_item_cstring (itm));
	  }
	else if (diritm == MOM_PREDEFINED_NAMED (output_types))
	  {
	    fprintf (cg->cg_emitfile,
		     OUTPUT_PREFIX_MOM "%d /*formalout:%s*/", (int) varrk,
		     mom_item_cstring (itm));
	  }
	else
	  goto otherwisenatlab;
      }
      break;
    otherwisenatlab:
    default:
      MOM_FATAPRINTF ("emitted item %s has unexpected vbind %s",
		      mom_item_cstring (itm), mom_output_gcstring (vbind));

    };				/* end swith natitm */
}				/* end of cgen_emit_item_mom */


static void cgen_emit_node_expr_mom (struct codegen_mom_st *cg,
				     const momvalue_t vnodexpr);

static void
cgen_emit_expr_mom (struct codegen_mom_st *cg, momvalue_t vexpr)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod, "cgen_emit_expr stmt %s vexpr %s",
		   mom_item_cstring (cg->cg_curstmtitm),
		   mom_output_gcstring (vexpr));
  switch (vexpr.typnum)
    {
    case momty_item:
      cgen_emit_item_mom (cg, vexpr.vitem);
      return;
    case momty_int:
      fprintf (cg->cg_emitfile, "%lld", (long long) vexpr.vint);
      return;
    case momty_string:
      fputs (" \"", cg->cg_emitfile);
      mom_output_utf8cstr_cencoded (cg->cg_emitfile, vexpr.vstr->cstr,
				    vexpr.vstr->slen);
      fputs ("\"", cg->cg_emitfile);
      return;
    case momty_double:
      fputs (mom_output_gcstring (vexpr), cg->cg_emitfile);
      return;
    case momty_node:
      {
	cgen_emit_node_expr_mom (cg, vexpr);
	return;
      }
    }
}				/* end of cgen_emit_expr_mom */


static void
cgen_emit_node_expr_mom (struct codegen_mom_st *cg, const momvalue_t vnodexpr)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  const momnode_t *nod = mom_value_to_node (vnodexpr);
  assert (nod != NULL);
  momitem_t *connitm = mom_node_conn (nod);
  assert (connitm);
  assert (mom_hashset_contains (cg->cg_lockeditemset, connitm));
  momvalue_t vcodemit = mom_item_unsync_get_attribute (connitm,
						       MOM_PREDEFINED_NAMED
						       (code_emitter));
  assert (vcodemit.typnum == momty_node);
  MOM_DEBUGPRINTF (gencod,
		   "cgen_emit_node_expr stmt %s vnodexpr %s vcodemit %s",
		   mom_item_cstring (cg->cg_curstmtitm),
		   mom_output_gcstring (vnodexpr),
		   mom_output_gcstring (vcodemit));
  // apply vcodemit to cgitem, vnodexpr
  if (!mom_applval_1itm1val_to_void (vcodemit, cg->cg_codgenitm, vnodexpr)
      || cg->cg_errormsg)
    {
      if (cg->cg_errormsg)
	return;
      CGEN_ERROR_RETURN_MOM (cg,
			     "module item %s : function %s has block %s with expression %s with connective %s failing to emit",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (cg->cg_curblockitm),
			     mom_output_gcstring (vnodexpr),
			     mom_item_cstring (connitm));
    }
  MOM_DEBUGPRINTF (gencod,
		   "cgen_emit_node_expr done stmt %s vnodexpr %s vcodemit %s",
		   mom_item_cstring (cg->cg_curstmtitm),
		   mom_output_gcstring (vnodexpr),
		   mom_output_gcstring (vcodemit));
}				/* end of cgen_emit_node_expr_mom */


static void
cgen_emit_set_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			     momitem_t *insitm);

static void
cgen_emit_chunk_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			       momitem_t *insitm);

static void
cgen_emit_apply_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			       momitem_t *insitm);

static void
cgen_emit_if_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			    momitem_t *insitm);

static void
cgen_emit_jump_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			      momitem_t *insitm);

static void
cgen_emit_int_switch_statement_mom (struct codegen_mom_st *cg, unsigned insix,
				    momitem_t *insitm);

static void
cgen_emit_item_switch_statement_mom (struct codegen_mom_st *cg,
				     unsigned insix, momitem_t *insitm);

static void
cgen_emit_success_statement_mom (struct codegen_mom_st *cg, unsigned insix,
				 momitem_t *insitm);

static void
cgen_emit_fail_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			      momitem_t *insitm);

static void
cgen_emit_other_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			       momitem_t *insitm);

static void
cgen_emit_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			 momitem_t *insitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod,
		   "emit_statement start insix#%u insitm %s",
		   insix, mom_item_cstring (insitm));
  momitem_t *opitm =
    mom_value_to_item (mom_unsync_item_get_nth_component (insitm, 0));
  MOM_DEBUGPRINTF (gencod, "emit_statement insitm %s opitm %s",
		   mom_item_cstring (insitm), mom_item_cstring (opitm));
  assert (opitm != NULL);
  switch (mom_item_hash (opitm))
    {
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (set, opitm, otherwiseoplab):
      {
	cgen_emit_set_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (chunk, opitm, otherwiseoplab):
      {
	cgen_emit_chunk_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (apply, opitm, otherwiseoplab):
      {
	cgen_emit_apply_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
      case MOM_PREDEFINED_NAMED_CASE (if, opitm, otherwiseoplab)
    :
	{
	  cgen_emit_if_statement_mom (cg, insix, insitm);
	}
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (int_switch, opitm, otherwiseoplab):
      {
	cgen_emit_int_switch_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (item_switch, opitm, otherwiseoplab):
      {
	cgen_emit_item_switch_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (jump, opitm, otherwiseoplab):
      {
	cgen_emit_jump_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (success, opitm, otherwiseoplab):
      {
	cgen_emit_success_statement_mom (cg, insix, insitm);
      }
      break;
      ////////////////
    case MOM_PREDEFINED_NAMED_CASE (fail, opitm, otherwiseoplab):
      {
	cgen_emit_fail_statement_mom (cg, insix, insitm);
      }
      break;
    default:
    otherwiseoplab:
      {
	cgen_emit_other_statement_mom (cg, insix, insitm);
      }
    }
}				/* end of cgen_emit_statement_mom */


static void
cgen_emit_set_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			     momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  momitem_t *itmlvar = mom_value_to_item (mom_components_nth (stmtcomps, 1));
  momitem_t *lvarctypitm = cgen_type_of_scanned_item_mom (cg, itmlvar);
  momvalue_t rexprv = mom_components_nth (stmtcomps, 2);
  MOM_DEBUGPRINTF (gencod,
		   "emit set stmt %s itmlvar %s lvarctypitm %s rexprv %s",
		   mom_item_cstring (itmstmt), mom_item_cstring (itmlvar),
		   mom_item_cstring (lvarctypitm),
		   mom_output_gcstring (rexprv));
  if (lvarctypitm == MOM_PREDEFINED_NAMED (locked_item))
    {
      fprintf (cg->cg_emitfile, "  { // locked set into %s\n",
	       mom_item_cstring (itmlvar));
      fprintf (cg->cg_emitfile, "  momlockeditem_t* momoldlocked_%s = ",
	       mom_item_cstring (itmstmt));
      cgen_emit_item_mom (cg, itmlvar);
      fputs (";\n", cg->cg_emitfile);
      fprintf (cg->cg_emitfile, "  momlockeditem_t* momnewlocked_%s = ",
	       mom_item_cstring (itmstmt));
      cgen_emit_expr_mom (cg, rexprv);
      fputs (";\n", cg->cg_emitfile);
      fprintf (cg->cg_emitfile,
	       "  if (momoldlocked_%s != momnewlocked_%s) {\n",
	       mom_item_cstring (itmstmt), mom_item_cstring (itmstmt));
      fprintf (cg->cg_emitfile,
	       "    if (momoldlocked_%s != NULL) mom_item_unlock (momoldlocked_%s);\n",
	       mom_item_cstring (itmstmt), mom_item_cstring (itmstmt));
      fprintf (cg->cg_emitfile,
	       "    if (momnewlocked_%s != NULL) mom_item_lock (momnewlocked_%s);\n",
	       mom_item_cstring (itmstmt), mom_item_cstring (itmstmt));
      fprintf (cg->cg_emitfile, "  } // end lock test %s\n",
	       mom_item_cstring (itmstmt));
      fputs ("    ", cg->cg_emitfile);
      cgen_emit_item_mom (cg, itmlvar);
      fprintf (cg->cg_emitfile, " = momnewlocked_%s;\n",
	       mom_item_cstring (itmstmt));
      fprintf (cg->cg_emitfile, "  momoldlocked_%s = NULL;\n",
	       mom_item_cstring (itmstmt));
      fprintf (cg->cg_emitfile, "  momnewlocked_%s = NULL;\n",
	       mom_item_cstring (itmstmt));
      fprintf (cg->cg_emitfile, "  } // end locked set into %s\n",
	       mom_item_cstring (itmlvar));
    }
  else
    {
      fprintf (cg->cg_emitfile, "// set into %s\n  ",
	       mom_item_cstring (itmlvar));
      cgen_emit_item_mom (cg, itmlvar);
      fputs (" = ", cg->cg_emitfile);
      cgen_emit_expr_mom (cg, rexprv);
      fputs (";\n", cg->cg_emitfile);
    }
}				/* end cgen_emit_set_statement_mom */


static void
cgen_emit_chunk_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			       momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  unsigned stmtlen = mom_components_count (stmtcomps);
  MOM_DEBUGPRINTF (gencod,
		   "emit chunk stmt %s of %u components",
		   mom_item_cstring (itmstmt), stmtlen);
  fprintf (cg->cg_emitfile, "  // chunk of %d components\n", stmtlen);
  for (unsigned ix = 1; ix < stmtlen; ix++)
    {
      momvalue_t vcomp = mom_components_nth (stmtcomps, ix);
      switch (vcomp.typnum)
	{
	case momty_string:
	  fputs (vcomp.vstr->cstr, cg->cg_emitfile);
	  break;
	case momty_int:
	  fprintf (cg->cg_emitfile, "%lld", (long long) vcomp.vint);
	  break;
	case momty_null:
	  continue;
	case momty_item:
	  {
	    momitem_t *itm = vcomp.vitem;
	    if (itm && itm->itm_kind == MOM_PREDEFINED_NAMED (block))
	      {
		fprintf (cg->cg_emitfile,
			 " goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
			 mom_item_cstring (itm));
	      }
	    else
	      cgen_emit_item_mom (cg, itm);
	  }
	  break;
	default:
	  cgen_emit_expr_mom (cg, vcomp);
	  break;
	}
    }
  fputs (" ;\n", cg->cg_emitfile);
}				/* end cgen_emit_chunk_statement_mom */


static void
cgen_emit_apply_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			       momitem_t *itmstmt)
{				/// apply <signature> <results...> <fun> <args...> [<else-block>]
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  unsigned stmtlen = mom_components_count (stmtcomps);
  momitem_t *itmsig = mom_value_to_item (mom_components_nth (stmtcomps, 1));
  assert (itmsig);
  const momvalue_t vinputy =	//
    mom_item_unsync_get_attribute (itmsig,
				   MOM_PREDEFINED_NAMED (input_types));
  momvalue_t voutputy =		//
    mom_item_unsync_get_attribute (itmsig,
				   MOM_PREDEFINED_NAMED (output_types));
  momvalue_t vradix =		//
    mom_item_unsync_get_attribute (itmsig,
				   MOM_PREDEFINED_NAMED (function_radix));

  MOM_DEBUGPRINTF (gencod,
		   "emit apply stmt %s sig %s vinputy %s voutputy %s vradix %s",
		   mom_item_cstring (itmstmt), mom_item_cstring (itmsig),
		   mom_output_gcstring (vinputy),
		   mom_output_gcstring (voutputy),
		   mom_output_gcstring (vradix));
  unsigned nbinargs = mom_seq_length (mom_value_to_tuple (vinputy));
  unsigned nboutres = mom_seq_length (mom_value_to_tuple (voutputy));
  const momstring_t *radixstr = mom_value_to_string (vradix);
  fprintf (cg->cg_emitfile,
	   "// apply with %d input arguments and %d output results, radix %s\n",
	   nbinargs, nboutres, mom_string_cstr (radixstr));
  momvalue_t vfunexpr = mom_components_nth (stmtcomps, 2 + nboutres);
  MOM_DEBUGPRINTF (gencod, "emit apply stmt %s vfunexpr %s",
		   mom_item_cstring (itmstmt),
		   mom_output_gcstring (vfunexpr));
  fprintf (cg->cg_emitfile,
	   "   if (!mom_applval_%s (", mom_string_cstr (radixstr));
  cgen_emit_expr_mom (cg, vfunexpr);
  for (unsigned argix = 0; argix < nbinargs; argix++)
    {
      momvalue_t vargexp =
	mom_components_nth (stmtcomps, 2 + nboutres + argix);
      MOM_DEBUGPRINTF (gencod, "emit apply stmt %s argix#%d vargexp %s",
		       mom_item_cstring (itmstmt), argix,
		       mom_output_gcstring (vargexp));
      fputs (", ", cg->cg_emitfile);
      cgen_emit_expr_mom (cg, vargexp);
    };
  for (unsigned resix = 0; resix < nboutres; resix++)
    {
      momitem_t *curesitm =
	mom_value_to_item (mom_components_nth (stmtcomps, 2 + resix));
      MOM_DEBUGPRINTF (gencod, "emit apply stmt %s resix#%d curesitm %s",
		       mom_item_cstring (itmstmt), resix,
		       mom_item_cstring (curesitm));
      fputs (", &(", cg->cg_emitfile);
      cgen_emit_item_mom (cg, curesitm);
      fputs (")", cg->cg_emitfile);
    };
  fputs (")\n", cg->cg_emitfile);
  momitem_t *elseitm = NULL;
  if (stmtlen > 2 + nbinargs + nboutres)
    elseitm =
      mom_value_to_item (mom_components_nth
			 (stmtcomps, 2 + nbinargs + nboutres));
  MOM_DEBUGPRINTF (gencod, "emit apply stmt %s elseitm %s",
		   mom_item_cstring (itmstmt), mom_item_cstring (elseitm));
  if (elseitm)
    fprintf (cg->cg_emitfile, "    goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
	     mom_item_cstring (elseitm));
  else
    fputs ("     return false;\n", cg->cg_emitfile);
}				/* end cgen_emit_apply_statement_mom */


static void
cgen_emit_if_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			    momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  momvalue_t vcondexpr = mom_components_nth (stmtcomps, 1);
  momitem_t *targetitm =
    mom_value_to_item (mom_components_nth (stmtcomps, 2));
  momitem_t *typconditm = cgen_type_of_scanned_expr_mom (cg, vcondexpr);
  MOM_DEBUGPRINTF (gencod,
		   "emit if stmt %s condexpr %s targetitm %s typcond %s",
		   mom_item_cstring (itmstmt),
		   mom_output_gcstring (vcondexpr),
		   mom_item_cstring (targetitm),
		   mom_item_cstring (typconditm));
  fprintf (cg->cg_emitfile, "// if testing on %s\n",
	   mom_item_cstring (typconditm));
  if (typconditm == MOM_PREDEFINED_NAMED (value))
    {
      fprintf (cg->cg_emitfile, "   if ((");
      cgen_emit_expr_mom (cg, vcondexpr);
      fprintf (cg->cg_emitfile, ").typnum != momty_null)\n");
    }
  else
    {
      fprintf (cg->cg_emitfile, "    if (");
      cgen_emit_expr_mom (cg, vcondexpr);
      fprintf (cg->cg_emitfile, ")\n");
    }
  fprintf (cg->cg_emitfile, "      goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
	   mom_item_cstring (targetitm));
}				/* end cgen_emit_if_statement_mom */


static void
cgen_emit_jump_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			      momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  momitem_t *targetitm =
    mom_value_to_item (mom_components_nth (stmtcomps, 1));
  MOM_DEBUGPRINTF (gencod, "emit jump stmt %s targetitm %s",
		   mom_item_cstring (itmstmt), mom_item_cstring (targetitm));
  fprintf (cg->cg_emitfile, "// jump to %s\n", mom_item_cstring (targetitm));
  fprintf (cg->cg_emitfile, "  goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
	   mom_item_cstring (targetitm));
}				/* end cgen_emit_jump_statement_mom */

static void
cgen_emit_success_statement_mom (struct codegen_mom_st *cg, unsigned insix,
				 momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  fprintf (cg->cg_emitfile, "// success\n");
  fprintf (cg->cg_emitfile, "  " SUCCESS_PREFIX_MOM "_%s = true;\n",
	   mom_item_cstring (cg->cg_curfunitm));
  fprintf (cg->cg_emitfile, "  goto " EPILOGUE_PREFIX_MOM "_%s;\n",
	   mom_item_cstring (cg->cg_curfunitm));
}				/* end of cgen_emit_success_statement_mom */


static void
cgen_emit_fail_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			      momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  fprintf (cg->cg_emitfile, "// fail\n");
  fprintf (cg->cg_emitfile, "  " SUCCESS_PREFIX_MOM "_%s = false;\n",
	   mom_item_cstring (cg->cg_curfunitm));
  fprintf (cg->cg_emitfile, "  goto " EPILOGUE_PREFIX_MOM "_%s;\n",
	   mom_item_cstring (cg->cg_curfunitm));
}				/* end of cgen_emit_fail_statement_mom */



static void
cgen_emit_int_switch_statement_mom (struct codegen_mom_st *cg, unsigned insix,
				    momitem_t *itmstmt)
{				// int_switch <expr> <case....>
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  unsigned stmtlen = mom_components_count (stmtcomps);
  momvalue_t vexpr = mom_components_nth (stmtcomps, 1);
  MOM_DEBUGPRINTF (gencod, "emit int_switch stmt %s with %d cases vexpr %s",
		   mom_item_cstring (itmstmt),
		   stmtlen - 2, mom_output_gcstring (vexpr));
  fprintf (cg->cg_emitfile, "// int_switch with %d cases\n", stmtlen - 2);
  fputs ("   switch (", cg->cg_emitfile);
  cgen_emit_expr_mom (cg, vexpr);
  fputs (") {\n", cg->cg_emitfile);
  for (unsigned caseix = 2; caseix < stmtlen; caseix)
    {
      momvalue_t vcase = mom_components_nth (stmtcomps, caseix);
      MOM_DEBUGPRINTF (gencod, "emit int_switch stmt %s caseix %d vcase %s",
		       mom_item_cstring (itmstmt),
		       caseix, mom_output_gcstring (vcase));
      const momnode_t *casnod = mom_value_to_node (vcase);
      assert (casnod && mom_node_arity (casnod) == 2);
      assert (mom_node_conn (casnod) == MOM_PREDEFINED_NAMED (case));
      const momvalue_t vcasnum = mom_node_nth (casnod, 0);
      const momvalue_t vcasblock = mom_node_nth (casnod, 1);
      assert (vcasnum.typnum == momty_int);
      assert (vcasblock.typnum == momty_item);
      intptr_t num = mom_value_to_int (vcasnum, -1);
      momitem_t *blockitm = mom_value_to_item (vcasblock);
      fprintf (cg->cg_emitfile,
	       "    case %lld: goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
	       (long long) num, mom_item_cstring (blockitm));
    }
  fprintf (cg->cg_emitfile, "    default: break;\n");
  fprintf (cg->cg_emitfile, "  } // end case int_switch %s\n",
	   mom_item_cstring (itmstmt));
}				/* end cgen_emit_int_switch_statement_mom */

struct itemswent_mom_st
{
  const momitem_t *isw_item;
  const momitem_t *isw_block;
};

int
itemswent_cmp_mom (const void *p1, const void *p2, void *data)
{
  unsigned prim = *(unsigned *) data;
  const struct itemswent_mom_st *e1 = (const struct itemswent_mom_st *) p1;
  const struct itemswent_mom_st *e2 = (const struct itemswent_mom_st *) p2;
  const momitem_t *itm1 = e1->isw_item;
  const momitem_t *itm2 = e2->isw_item;
  momhash_t h1 = mom_item_hash (itm1);
  momhash_t h2 = mom_item_hash (itm2);
  unsigned q1 = h1 / prim;
  unsigned q2 = h2 / prim;
  if (q1 == q2)
    return strcmp (itm1->itm_str->cstr, itm2->itm_str->cstr);
  else if (q1 < q2)
    return -1;
  else
    return 1;
}

static void
cgen_emit_item_switch_statement_mom (struct codegen_mom_st *cg,
				     unsigned insix, momitem_t *itmstmt)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (itmstmt);
  struct momcomponents_st *stmtcomps = itmstmt->itm_comps;
  unsigned stmtlen = mom_components_count (stmtcomps);
  unsigned nbcases = stmtlen - 2;
  momvalue_t vexpr = mom_components_nth (stmtcomps, 1);
  assert (stmtlen < MOM_MAX_SEQ_LENGTH / 2);
  struct itemswent_mom_st *swentarr =	//
    MOM_GC_ALLOC ("swentarr",
		  (stmtlen + 1) * sizeof (struct itemswent_mom_st));
  MOM_DEBUGPRINTF (gencod, "emit item_switch stmt %s with %d cases vexpr %s",
		   mom_item_cstring (itmstmt),
		   stmtlen - 2, mom_output_gcstring (vexpr));
  fprintf (cg->cg_emitfile, "// item_switch with %d cases\n", stmtlen - 2);
  for (unsigned caseix = 2; caseix < stmtlen; caseix)
    {
      momvalue_t vcase = mom_components_nth (stmtcomps, caseix);
      MOM_DEBUGPRINTF (gencod, "emit item_switch stmt %s caseix %d vcase %s",
		       mom_item_cstring (itmstmt),
		       caseix, mom_output_gcstring (vcase));
      const momnode_t *casnod = mom_value_to_node (vcase);
      assert (casnod && mom_node_arity (casnod) == 2);
      assert (mom_node_conn (casnod) == MOM_PREDEFINED_NAMED (case));
      const momvalue_t vcasitem = mom_node_nth (casnod, 0);
      const momvalue_t vcasblock = mom_node_nth (casnod, 1);
      assert (vcasitem.typnum == momty_item);
      assert (vcasblock.typnum == momty_item);
      momitem_t *casitm = mom_value_to_item (vcasitem);
      momitem_t *blockitm = mom_value_to_item (vcasblock);
      assert (casitm);
      assert (blockitm);
      swentarr[caseix - 2].isw_item = casitm;
      swentarr[caseix - 2].isw_block = blockitm;
    }
  unsigned prim = (unsigned) mom_prime_above (3 * stmtlen + 5);
  assert (prim > 2);
  qsort_r (swentarr, nbcases, sizeof (struct itemswent_mom_st),
	   itemswent_cmp_mom, &prim);
  fprintf (cg->cg_emitfile, "{ momitem_t* momswitchitem_%s = ",
	   mom_item_cstring (itmstmt));
  cgen_emit_expr_mom (cg, vexpr);
  fputs (";\n", cg->cg_emitfile);
  fprintf (cg->cg_emitfile,
	   "  momhash_t momswitchhash_%s = mom_item_hash(momswitchitem_%s);\n",
	   mom_item_cstring (itmstmt), mom_item_cstring (itmstmt));
  fprintf (cg->cg_emitfile, "  switch (momswitchhash_%s %% %u) {\n",
	   mom_item_cstring (itmstmt), prim);
  int prevhmod = -1;
  for (unsigned ix = 0; ix < nbcases; ix++)
    {
      momitem_t *casitm = (momitem_t *) swentarr[ix].isw_item;
      momitem_t *blockitm = (momitem_t *) swentarr[ix].isw_block;
      assert (casitm != NULL);
      assert (blockitm != NULL);
      int curhmod = mom_item_hash (casitm) % prim;
      assert (curhmod >= 0);
      if (prevhmod != curhmod)
	{
	  if (prevhmod >= 0)
	    fputs ("    break;\n", cg->cg_emitfile);
	  fprintf (cg->cg_emitfile, "   case %d:\n", curhmod);
	  prevhmod = curhmod;
	};
      fprintf (cg->cg_emitfile, "    if (momswitchitem_%s == (",
	       mom_item_cstring (itmstmt));
      cgen_emit_item_mom (cg, casitm);
      fprintf (cg->cg_emitfile, "))  goto " BLOCK_LABEL_PREFIX_MOM "_%s;\n",
	       mom_item_cstring (blockitm));
    }
  fprintf (cg->cg_emitfile, "  default: break;\n"
	   "  } // end switch\n"
	   "  momswitchitem_%s = NULL;\n"
	   "  } // end  item_switch statement %s\n",
	   mom_item_cstring (itmstmt), mom_item_cstring (itmstmt));
}				/* end cgen_emit_item_switch_statement_mom */


static void
cgen_emit_other_statement_mom (struct codegen_mom_st *cg, unsigned insix,
			       momitem_t *insitm)
{
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  assert (insitm);
  MOM_FATAPRINTF ("emit_item_other_statement unimplemented insitm %s",
		  mom_item_cstring (insitm));
#warning cgen_emit_other_statement_mom unimplemented
}				/* end cgen_emit_other_statement_mom */



static void
cgen_third_decorating_pass_mom (momitem_t *itmcgen)
{
  assert (itmcgen
	  && itmcgen->itm_kind == MOM_PREDEFINED_NAMED (code_generation));
  struct codegen_mom_st *cg = (struct codegen_mom_st *) itmcgen->itm_data1;
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  momitem_t *itmmod = cg->cg_moduleitm;
  MOM_DEBUGPRINTF (gencod, "third_decorating_pass  itmcgen %s itmmod %s",
		   mom_item_cstring (itmcgen), mom_item_cstring (itmmod));
  const momseq_t *seqfun = mom_hashset_elements_set (cg->cg_functionhset);
  mom_item_unsync_put_attribute (itmmod,
				 MOM_PREDEFINED_NAMED (emitted_functions),
				 mom_unsafe_setv (seqfun));
  unsigned nbfun = mom_seq_length (seqfun);
  for (unsigned funix = 0; funix < nbfun; funix++)
    {
      const momitem_t *curfunitm = mom_seq_nth (seqfun, funix);
      MOM_DEBUGPRINTF (gencod, "third_decorating_pass funix#%d curfunitm %s",
		       funix, mom_item_cstring (curfunitm));
      struct momentry_st *entfun =
	mom_attributes_find_entry (cg->cg_functionassoc, curfunitm);
      momvalue_t vfuninfo = MOM_NONEV;
      if (entfun)
	vfuninfo = entfun->ent_val;
      MOM_DEBUGPRINTF (gencod,
		       "third_decorating_pass funix#%d curfunitm %s vfuninfo %s",
		       funix, mom_item_cstring (curfunitm),
		       mom_output_gcstring (vfuninfo));
      if (vfuninfo.typnum == momty_null)
	MOM_FATAPRINTF ("unexpected function %s in module %s",
			mom_item_cstring (curfunitm),
			mom_item_cstring (itmmod));
      const momnode_t *nodfuninfo = mom_value_to_node (vfuninfo);
      assert (nodfuninfo != NULL
	      && mom_node_arity (nodfuninfo) == funinfo__last);
      momitem_t *itmsignature =
	mom_value_to_item (mom_node_nth (nodfuninfo, funinfo_signature));
      momitem_t *itmblocks =
	mom_value_to_item (mom_node_nth (nodfuninfo, funinfo_blocks));
      momitem_t *itmbindings =
	mom_value_to_item (mom_node_nth (nodfuninfo, funinfo_bindings));
      momitem_t *itmstart =
	mom_value_to_item (mom_node_nth (nodfuninfo, funinfo_start));
      momvalue_t vsetconst = mom_node_nth (nodfuninfo, funinfo_const);
      momvalue_t vsetclosed = mom_node_nth (nodfuninfo, funinfo_closed);
      momvalue_t vsetvars = mom_node_nth (nodfuninfo, funinfo_vars);
      momvalue_t vformals = mom_node_nth (nodfuninfo, funinfo_formals);
      MOM_DEBUGPRINTF (gencod,
		       "third_decorating_pass funix#%d curfunitm %s; itmsignature %s;"
		       " itmblocks %s; itmbindings %s; itmstart %s; setconst %s setclosed %s setvar %s",
		       funix, mom_item_cstring (curfunitm),
		       mom_item_cstring (itmsignature),
		       mom_item_cstring (itmblocks),
		       mom_item_cstring (itmbindings),
		       mom_item_cstring (itmstart),
		       mom_output_gcstring (vsetconst),
		       mom_output_gcstring (vsetclosed),
		       mom_output_gcstring (vsetvars));
      assert (itmblocks->itm_kind == MOM_PREDEFINED_NAMED (association));
      momvalue_t vsetblocks = MOM_NONEV;
      if (itmblocks->itm_kind == MOM_PREDEFINED_NAMED (association))
	vsetblocks =
	  mom_unsafe_setv (mom_attributes_set
			   ((struct momattributes_st *) itmblocks->itm_data1,
			    MOM_NONEV));
      MOM_DEBUGPRINTF (gencod, "third_decorating_pass funix#%d vsetblocks %s",
		       funix, mom_output_gcstring (vsetblocks));
      mom_item_unsync_put_attribute ((momitem_t *) curfunitm,
				     MOM_PREDEFINED_NAMED (emitted_blocks),
				     vsetblocks);
      mom_item_unsync_put_attribute ((momitem_t *) curfunitm,
				     MOM_PREDEFINED_NAMED (in),
				     mom_itemv (itmmod));
      const struct momseq_st *seqblocks = mom_value_to_set (vsetblocks);
      unsigned nbblocks = mom_seq_length (seqblocks);
      MOM_DEBUGPRINTF (gencod,
		       "third_decorating_pass funix#%d nbblocks %d",
		       funix, nbblocks);
      for (unsigned blix = 0; blix < nbblocks; blix++)
	{
	  momitem_t *itmcurblock =
	    (momitem_t *) mom_seq_nth (seqblocks, (int) blix);
	  MOM_DEBUGPRINTF (gencod,
			   "third_decorating_pass funix#%d function %s: blix#%d curblock %s",
			   funix, mom_item_cstring (curfunitm), blix,
			   mom_item_cstring (itmcurblock));
	  mom_item_unsync_put_attribute (itmcurblock,
					 MOM_PREDEFINED_NAMED (in),
					 mom_itemv (curfunitm));
	  struct momentry_st *ent =
	    mom_attributes_find_entry ((struct momattributes_st *)
				       itmblocks->itm_data1,
				       itmcurblock);
	  momvalue_t valblockinfo = MOM_NONEV;
	  if (ent)
	    valblockinfo = ent->ent_val;
	  MOM_DEBUGPRINTF (gencod,
			   "third_decorating_pass funix#%d function %s: blix#%d curblock %s valblockinfo %s",
			   funix,
			   mom_item_cstring
			   (curfunitm), blix,
			   mom_item_cstring
			   (itmcurblock), mom_output_gcstring (valblockinfo));
	  const momseq_t *tupins =
	    mom_value_to_tuple (mom_node_nth
				(mom_value_to_node (valblockinfo), 1));
	  if (tupins)
	    {
	      unsigned nbins = mom_seq_length (tupins);
	      for (unsigned insix = 0; insix < nbins; insix++)
		{
		  momitem_t *insitm = mom_seq_nth (tupins, insix);
		  mom_item_unsync_put_attribute (insitm,
						 MOM_PREDEFINED_NAMED (in),
						 mom_itemv (itmcurblock));
		}
	    }

	};
      ///
      MOM_DEBUGPRINTF (gencod, "third_decorating_pass funitm %s vsetconst %s",
		       mom_itemv (curfunitm),
		       mom_output_gcstring (vsetconst));

      /*
         mom_item_unsync_put_attribute ((momitem_t *) curfunitm,
         MOM_PREDEFINED_NAMED (emitted_constants),
         tupconst);
       */
      // vsetconst = mom_node_nth (nodfuninfo, funinfo_const);
      // vsetclosed = mom_node_nth (nodfuninfo, funinfo_closed);
      // vsetvars = mom_node_nth (nodfuninfo, funinfo_vars);
      ///
      MOM_DEBUGPRINTF (gencod,
		       "third_decorating_pass funix#%d done curfunitm %s",
		       funix, mom_item_cstring (curfunitm));
    }
  MOM_DEBUGPRINTF (gencod, "third_decorating_pass done itmmod %s",
		   mom_item_cstring (itmmod));
}				/* end cgen_third_decorating_pass_mom */




////////////////
bool
  momfunc_1itm1val_to_item_plain_code_type_scanner
  (const momnode_t *clonode, momitem_t *itmcodgen, const momvalue_t vexpr,
   momitem_t **respitm)
{
  MOM_DEBUGPRINTF (gencod,
		   "plain_code_type_scanner start itmcodgen=%s vexpr=%s",
		   mom_item_cstring (itmcodgen), mom_output_gcstring (vexpr));
  momnode_t *nod = mom_value_to_node (vexpr);
  if (!nod || itmcodgen->itm_kind != MOM_PREDEFINED_NAMED (code_generation))
    return false;
  unsigned arity = mom_node_arity (nod);
  struct codegen_mom_st *cg = (struct codegen_mom_st *) itmcodgen->itm_data1;
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  momitem_t *itmconn = mom_node_conn (nod);
  cgen_lock_item_mom (cg, itmconn);
  momvalue_t vcodexp =		//
    mom_item_unsync_get_attribute (itmconn,
				   MOM_PREDEFINED_NAMED (code_expansion));
  momvalue_t vformals =		//
    mom_item_unsync_get_attribute (itmconn,
				   MOM_PREDEFINED_NAMED (formals));
  momitem_t *conntypitm =
    mom_value_to_item (mom_item_unsync_get_attribute (itmconn,
						      MOM_PREDEFINED_NAMED
						      (type)));
  MOM_DEBUGPRINTF (gencod,
		   "plain_code_type_scanner itmconn %s vcodexp %s vformals %s conntypitm %s",
		   mom_item_cstring (itmconn), mom_output_gcstring (vcodexp),
		   mom_output_gcstring (vformals),
		   mom_item_cstring (conntypitm));
  if (vcodexp.typnum != momty_node || vformals.typnum != momty_tuple
      || !conntypitm)
    {
      CGEN_ERROR_RESULT_MOM (cg, false,
			     "plain_code_type_scanner:: module item %s : function %s has block %s"
			     " with statement %s with bad connective %s in plain vexpr %s",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (cg->cg_curblockitm),
			     mom_item_cstring (cg->cg_curstmtitm),
			     mom_item_cstring (itmconn),
			     mom_output_gcstring (vexpr));
    }
  const momseq_t *tupformals = mom_value_to_tuple (vformals);
  unsigned nbformals = mom_seq_length (tupformals);
  if (nbformals > arity)
    {
      CGEN_ERROR_RESULT_MOM (cg, false,
			     "plain_code_type_scanner:: module item %s : function %s has block %s"
			     " with statement %s with expr %s shorter than %d formals",
			     mom_item_cstring (cg->cg_moduleitm),
			     mom_item_cstring (cg->cg_curfunitm),
			     mom_item_cstring (cg->cg_curblockitm),
			     mom_item_cstring (cg->cg_curstmtitm),
			     mom_output_gcstring (vexpr), nbformals);
    }
  if (cg->cg_errormsg)
    return false;
  for (unsigned formix = 0; formix < nbformals && !cg->cg_errormsg; formix)
    {
      momitem_t *curformitm = mom_seq_nth (tupformals, formix);
      momvalue_t vsubexpr = mom_node_nth (nod, formix);
      cgen_lock_item_mom (cg, curformitm);
      momitem_t *curtypitm =
	mom_value_to_item (mom_item_unsync_get_attribute (curformitm,
							  MOM_PREDEFINED_NAMED
							  (type)));
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_type_scanner formix#%d curformitm %s curtypitm %s vsubexpr %s",
		       formix, mom_item_cstring (curformitm),
		       mom_item_cstring (curtypitm),
		       mom_output_gcstring (vsubexpr));
      if (!curtypitm || curtypitm->itm_kind != MOM_PREDEFINED_NAMED (type))
	CGEN_ERROR_RESULT_MOM (cg, false,
			       "plain_code_type_scanner:: module item %s : function %s has block %s"
			       " with statement %s with expr %s formal (#%d) %s untyped",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm),
			       mom_item_cstring (cg->cg_curblockitm),
			       mom_item_cstring (cg->cg_curstmtitm),
			       mom_output_gcstring (vexpr), formix,
			       mom_item_cstring (curformitm));
      momitem_t *subtypitm = cgen_type_of_scanned_expr_mom (cg, vsubexpr);
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_type_scanner formix#%d vsubexpr %s subtypitm %s curtypitm %s curformitm %s",
		       formix, mom_output_gcstring (vexpr),
		       mom_item_cstring (subtypitm),
		       mom_item_cstring (curtypitm),
		       mom_item_cstring (curformitm));
      if (curtypitm != subtypitm)
	CGEN_ERROR_RESULT_MOM (cg, false,
			       "plain_code_type_scanner:: module item %s : function %s has block %s"
			       " with statement %s with expr %s formal (#%d) %s mistyped, subexpr %s has type %s expecting %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm),
			       mom_item_cstring (cg->cg_curblockitm),
			       mom_item_cstring (cg->cg_curstmtitm),
			       mom_output_gcstring (vexpr), formix,
			       mom_item_cstring (curformitm),
			       mom_output_gcstring (vsubexpr),
			       mom_item_cstring (subtypitm),
			       mom_item_cstring (curtypitm));
    }
  if (cg->cg_errormsg)
    return false;
  if (nbformals == arity)
    {
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_type_scanner vexpr %s gives conntypitm %s",
		       mom_output_gcstring (vexpr),
		       mom_item_cstring (conntypitm));
      if (respitm)
	*respitm = conntypitm;
      return true;
    };
  momitem_t *varestitm =	//
    mom_value_to_item (mom_item_unsync_get_attribute (itmconn,
						      MOM_PREDEFINED_NAMED
						      (variadic_rest)));
  MOM_DEBUGPRINTF (gencod,
		   "plain_code_type_scanner varestitm %s",
		   mom_item_cstring (varestitm));
  if (!varestitm)
    CGEN_ERROR_RESULT_MOM (cg, false,
			   "plain_code_type_scanner:: module item %s : function %s has block %s"
			   " with statement %s with expr %s with non-variadic connective %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (cg->cg_curstmtitm),
			   mom_output_gcstring (vexpr),
			   mom_item_cstring (itmconn));
  cgen_lock_item_mom (cg, varestitm);
  momitem_t *restypitm =
    mom_value_to_item (mom_item_unsync_get_attribute (varestitm,
						      MOM_PREDEFINED_NAMED
						      (type)));
  if (!restypitm || restypitm->itm_kind != MOM_PREDEFINED_NAMED (type))
    CGEN_ERROR_RESULT_MOM (cg, false,
			   "plain_code_type_scanner:: module item %s : function %s has block %s"
			   " with statement %s with expr %s with bad variadic_rest %s",
			   mom_item_cstring (cg->cg_moduleitm),
			   mom_item_cstring (cg->cg_curfunitm),
			   mom_item_cstring (cg->cg_curblockitm),
			   mom_item_cstring (cg->cg_curstmtitm),
			   mom_output_gcstring (vexpr),
			   mom_item_cstring (varestitm));
  for (unsigned rix = nbformals; rix < arity && !cg->cg_errormsg; rix++)
    {
      momvalue_t vrestexpr = mom_node_nth (nod, rix);
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_type_scanner rix#%d vrestexpr %s", rix,
		       mom_output_gcstring (vrestexpr));
      momitem_t *curestypitm = cgen_type_of_scanned_expr_mom (cg, vrestexpr);
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_type_scanner rix#%d restypitm %s",
		       rix, mom_item_cstring (restypitm));
      if (restypitm != curestypitm)
	CGEN_ERROR_RESULT_MOM (cg, false,
			       "plain_code_type_scanner:: module item %s : function %s has block %s"
			       " with statement %s with rest expr #%d %s"
			       " of type %s but variadic_rest %s wants type %s",
			       mom_item_cstring (cg->cg_moduleitm),
			       mom_item_cstring (cg->cg_curfunitm),
			       mom_item_cstring (cg->cg_curblockitm),
			       mom_item_cstring (cg->cg_curstmtitm),
			       rix,
			       mom_output_gcstring (vrestexpr),
			       mom_item_cstring (curestypitm),
			       mom_item_cstring (varestitm),
			       mom_item_cstring (restypitm));
    };
  if (cg->cg_errormsg)
    return false;
  MOM_DEBUGPRINTF (gencod,
		   "plain_code_type_scanner variadic vexpr %s gives conntypitm %s",
		   mom_output_gcstring (vexpr),
		   mom_item_cstring (conntypitm));
  if (respitm)
    *respitm = conntypitm;
  return true;

}				/* end of momfunc_1itm1val_to_item_plain_code_type_scanner */



bool momfunc_1itm1val_to_void_plain_code_emitter
  (const momnode_t *clonode, momitem_t *itmcodgen, const momvalue_t vexpr)
{
  MOM_DEBUGPRINTF (gencod, "plain_code_emitter start itmcodgen=%s vexpr=%s",
		   mom_item_cstring (itmcodgen), mom_output_gcstring (vexpr));
  momnode_t *nod = mom_value_to_node (vexpr);
  if (!nod || itmcodgen->itm_kind != MOM_PREDEFINED_NAMED (code_generation))
    return false;
  struct codegen_mom_st *cg = (struct codegen_mom_st *) itmcodgen->itm_data1;
  momitem_t *connitm = mom_node_conn (nod);
  assert (cg && cg->cg_magic == CODEGEN_MAGIC_MOM);
  unsigned arity = mom_node_arity (nod);
  momvalue_t vcodexp =		//
    mom_item_unsync_get_attribute (connitm,
				   MOM_PREDEFINED_NAMED (code_expansion));
  momnode_t *nodexp = mom_value_to_node (vcodexp);
  if (!nodexp)
    MOM_FATAPRINTF ("code_expansion of %s is not a node",
		    mom_output_gcstring (vcodexp));
  momvalue_t vformals =		//
    mom_item_unsync_get_attribute (connitm,
				   MOM_PREDEFINED_NAMED (formals));
  assert (connitm && mom_hashset_contains (cg->cg_lockeditemset, connitm));
  const momseq_t *tupformals = mom_value_to_tuple (vformals);
  assert (tupformals);
  unsigned nbformals = mom_seq_length (tupformals);
  assert (nbformals <= arity);
  struct momattributes_st *att = mom_attributes_make (6 * arity / 5 + 2);
  for (unsigned formix = 0; formix < nbformals; formix++)
    {
      momitem_t *curformitm = mom_seq_nth (tupformals, formix);
      assert (curformitm);
      if (mom_attributes_find_entry (att, curformitm) != NULL)
	MOM_FATAPRINTF ("duplicate formal itm %s in plain connective %s"
			" for vexpr %s in block %s of function %s in module %s",
			mom_item_cstring (curformitm),
			mom_item_cstring (connitm),
			mom_output_gcstring (vexpr),
			mom_item_cstring (cg->cg_curblockitm),
			mom_item_cstring (cg->cg_curfunitm),
			mom_item_cstring (cg->cg_moduleitm));
      momvalue_t vsubexpr = mom_node_nth (nod, formix);
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_emitter formix#%d curformitm %s vsubexpr %s",
		       formix, mom_item_cstring (curformitm),
		       mom_output_gcstring (vsubexpr));
      att = mom_attributes_put (att, curformitm, &vsubexpr);
    };
  momitem_t *varcountitm =
    mom_value_to_item (mom_item_unsync_get_attribute (connitm,
						      MOM_PREDEFINED_NAMED
						      (variadic_count)));
  momitem_t *varestitm =
    mom_value_to_item (mom_item_unsync_get_attribute (connitm,
						      MOM_PREDEFINED_NAMED
						      (variadic_rest)));
  unsigned nbexp = mom_node_arity (nodexp);
  for (unsigned xix = 0; xix < nbexp && !cg->cg_errormsg; xix++)
    {
      momvalue_t vcurexp = mom_node_nth (nodexp, xix);
      MOM_DEBUGPRINTF (gencod,
		       "plain_code_emitter xix#%d vcurexp %s", xix,
		       mom_output_gcstring (vcurexp));
      switch (vcurexp.typnum)
	{
	case momty_null:
	  continue;
	case momty_string:
	  fputs (mom_string_cstr (vcurexp.vstr), cg->cg_emitfile);
	  continue;
	case momty_int:
	  fprintf (cg->cg_emitfile, "%lld", (long long) vcurexp.vint);
	case momty_item:
	  {
	    momitem_t *curitm = vcurexp.vitem;
	    assert (curitm != NULL);
	    if (curitm == varcountitm)
	      {
		MOM_DEBUGPRINTF (gencod,
				 "plain_code_emitter count %d",
				 (int) (nbexp - nbformals));
		fprintf (cg->cg_emitfile, " /*variadic-count %s:*/%d",
			 mom_item_cstring (varcountitm),
			 (int) (nbexp - nbformals));
		continue;
	      }
	    else if (curitm == varestitm)
	      {
		MOM_DEBUGPRINTF (gencod,
				 "plain_code_emitter got varestitm %s",
				 mom_item_cstring (varestitm));
		unsigned nbrest = nbexp - nbformals;
		fprintf (cg->cg_emitfile, " /*variadic-rest %s:*/",
			 mom_item_cstring (varestitm));
		for (unsigned rix = 0; rix < nbrest && !cg->cg_errormsg;
		     rix++)
		  {
		    if (rix > 0)
		      fputs (", ", cg->cg_emitfile);
		    momvalue_t vcurexp =
		      mom_node_nth (nodexp, nbformals + rix);
		    MOM_DEBUGPRINTF (gencod,
				     "plain_code_emitter rix#%d vcurexp %s",
				     rix, mom_output_gcstring (vcurexp));
		    fprintf (cg->cg_emitfile, " /*rest#%d*/(", rix);
		    cgen_emit_expr_mom (cg, vcurexp);
		    fprintf (cg->cg_emitfile, ")");
		  }
		continue;
	      };
	    momvalue_t valbind = MOM_NONEV;
	    struct momentry_st *ent = mom_attributes_find_entry (att, curitm);
	    if (ent)
	      valbind = ent->ent_val;
	    MOM_DEBUGPRINTF (gencod,
			     "plain_code_emitter curitm %s valbind %s",
			     mom_item_cstring (curitm),
			     mom_output_gcstring (valbind));
	    if (valbind.typnum != momty_null)
	      cgen_emit_expr_mom (cg, valbind);
	    else
	      fprintf (cg->cg_emitfile, "%s", mom_item_cstring (curitm));
	  }
	  break;
	default:
	  cgen_emit_expr_mom (cg, vcurexp);
	}			/* end switch vcurexp.typnum */
    };
  fflush (cg->cg_emitfile);
  MOM_DEBUGPRINTF (gencod,
		   "plain_code_emitter done itmcodgen=%s vexpr=%s fileoff %ld",
		   mom_item_cstring (itmcodgen), mom_output_gcstring (vexpr),
		   ftell (cg->cg_emitfile));
  return true;
}				/* end of momfunc_1itm1val_to_void_plain_code_emitter */



/// eof codgen.c
