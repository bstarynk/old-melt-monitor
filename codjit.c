// file codgen.c - manage the just-in-time code generation

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

#define CODEJIT_MAGIC_MOM 0x23b7bce7	/* codejit magic 599244007 */
struct codejit_mom_st
{
  unsigned cj_magic;		/* always CODEJIT_MAGIC_MOM */
  const momstring_t *cj_errormsg;	/* the error message */
  momitem_t *cj_codjititm;
  momitem_t *cj_moduleitm;
  struct momhashset_st *cj_lockeditemset;	/* the set of locked items */
  struct momhashset_st *cj_functionhset;	/* the set of c functions */
};

#define CJIT_ERROR_RETURN_MOM_AT_BIS(Lin,Cj,Fmt,...) do {       \
  struct codejit_mom_st *cj_##Lin = (Cj);                       \
  assert (cj_##Lin && cj_##Lin->cj_magic == CODEJIT_MAGIC_MOM); \
  cj_##Lin->cj_errormsg =                                       \
    mom_make_string_sprintf(Fmt,__VA_ARGS__);                   \
  mom_warnprintf_at(__FILE__,Lin,"CODEJIT ERROR: %s",           \
                    cj_##Lin->cj_errormsg->cstr);               \
  return;                                                       \
 }while(0)
#define CJIT_ERROR_RETURN_MOM_AT(Lin,Cj,Fmt,...) \
  CJIT_ERROR_RETURN_MOM_AT_BIS(Lin,Cj,Fmt,__VA_ARGS__)
#define CJIT_ERROR_RETURN_MOM(Cj,Fmt,...) \
  CJIT_ERROR_RETURN_MOM_AT(__LINE__,(Cj),(Fmt),__VA_ARGS__)


#define CJIT_ERROR_RESULT_AT_BIS_MOM(Lin,Cj,Res,Fmt,...) do {	\
  struct codejit_mom_st *cj_##Lin = (Cj);			\
  assert (cj_##Lin && cj_##Lin->cj_magic == CODEJIT_MAGIC_MOM);	\
  cj_##Lin->cj_errormsg =					\
    mom_make_string_sprintf(Fmt,__VA_ARGS__);			\
  mom_warnprintf_at(__FILE__,Lin,"CODEJIT FAILURE: %s",		\
		    cj_##Lin->cj_errormsg->cstr);		\
  return (Res);							\
 }while(0)
#define CJIT_ERROR_RESULT_AT_MOM(Lin,Cj,Res,Fmt,...)	\
  CJIT_ERROR_RESULT_AT_BIS_MOM(Lin,Cj,Res,Fmt,__VA_ARGS__)
#define CJIT_ERROR_RESULT_MOM(Cj,Res,Fmt,...)			\
  CJIT_ERROR_RESULT_AT_MOM(__LINE__,(Cj),(Res),(Fmt),__VA_ARGS__)



static void
cjit_lock_item_mom (struct codejit_mom_st *cj, momitem_t *itm)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (itm && !mom_hashset_contains (cj->cj_lockeditemset, itm))
    {
      mom_item_lock (itm);
      cj->cj_lockeditemset = mom_hashset_put (cj->cj_lockeditemset, itm);
    }
}

static void
cjit_unlock_all_items_mom (struct codejit_mom_st *cj)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  struct momhashset_st *hset = cj->cj_lockeditemset;
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

static void cjit_first_scanning_pass_mom (momitem_t *itmcjit);

bool
  momfunc_1itm_to_val__generate_jit_module
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (gencod,
		   "generate_jit_module itm=%s closure=%s start",
		   mom_item_cstring (itm),
		   mom_output_gcstring (mom_nodev (clonode)));
  if (!itm || itm == MOM_EMPTY)
    return false;
  *res = MOM_NONEV;
  momitem_t *itmcjit = mom_make_anonymous_item ();
  itmcjit->itm_space = momspa_transient;
  struct codejit_mom_st *cj =
    MOM_GC_ALLOC ("jitgenerator", sizeof (struct codejit_mom_st));
  cj->cj_magic = CODEJIT_MAGIC_MOM;
  cj->cj_moduleitm = itm;
  cj->cj_codjititm = itmcjit;
  mom_item_lock (itmcjit);
  itmcjit->itm_kind = MOM_PREDEFINED_NAMED (code_generation);
  itmcjit->itm_data1 = (void *) cj;
  cjit_first_scanning_pass_mom (itmcjit);
  if (cj->cj_errormsg)
    goto end;
end:
  cjit_unlock_all_items_mom (cj);
  if (cj->cj_errormsg)
    {
      MOM_WARNPRINTF ("generate_jit_module failed: %s\n",
		      mom_string_cstr (cj->cj_errormsg));
      *res = mom_stringv (cj->cj_errormsg);
    }
  else
    {
      MOM_INFORMPRINTF ("generate_jit_module sucessful for %s",
			mom_item_cstring (itm));
      *res = mom_itemv (itm);
    }
  return true;
}				/* end of momfunc_1itm_to_val__generate_jit_module */



static void
cjit_scan_function_first_mom (struct codejit_mom_st *cj, momitem_t *itmfun);


void
cjit_first_scanning_pass_mom (momitem_t *itmcjit)
{
  struct codejit_mom_st *cj = NULL;
  MOM_DEBUGPRINTF (gencod, "cjit_first_scanning_pass start itmcjit=%s",
		   mom_item_cstring (itmcjit));
  if (!itmcjit
      || itmcjit->itm_kind != MOM_PREDEFINED_NAMED (code_generation)
      || !(cj = itmcjit->itm_data1) || cj->cj_magic != CODEJIT_MAGIC_MOM)
    MOM_FATAPRINTF ("cjit_first_scanning_pass: corrupted itmcjit %s",
		    mom_item_cstring (itmcjit));
  momitem_t *itmmod = cj->cj_moduleitm;
  assert (itmmod);
  if (itmmod->itm_kind != MOM_PREDEFINED_NAMED (code_module))
    CJIT_ERROR_RETURN_MOM (cj, "module item %s is not a `code_module`",
			   mom_item_cstring (itmmod));
  ///// prepare the module using its preparation
  momvalue_t resprepv = MOM_NONEV;
  momvalue_t prepv =		//
    mom_item_unsync_get_attribute (itmmod,
				   MOM_PREDEFINED_NAMED (preparation));
  MOM_DEBUGPRINTF (gencod, "preparation of %s is %s",
		   mom_item_cstring (itmmod), mom_output_gcstring (prepv));
  if (prepv.typnum == momty_node
      && (!mom_applval_2itm_to_val (prepv, itmmod, itmcjit, &resprepv)
	  || resprepv.typnum == momty_string))
    {
      MOM_DEBUGPRINTF (gencod, "preparation of %s gave %s",
		       mom_item_cstring (itmmod),
		       mom_output_gcstring (resprepv));
      if (resprepv.typnum == momty_string)
	cj->cj_errormsg = mom_value_to_string (resprepv);
      else
	CJIT_ERROR_RETURN_MOM (cj, "module item %s preparation failed",
			       mom_item_cstring (itmmod));
    }
  else if (prepv.typnum != momty_null)
    CJIT_ERROR_RETURN_MOM (cj, "module item %s has bad preparation %s",
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
      if (!mom_applval_2itm_to_val (prepv, itmmod, itmcjit, &funsetv)
	  || (funsv.typnum != momty_set || funsv.typnum != momty_tuple))
	CJIT_ERROR_RETURN_MOM (cj,
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
    CJIT_ERROR_RETURN_MOM (cj, "module item %s : functions %s are not a set",
			   mom_item_cstring (itmmod),
			   mom_output_gcstring (funsv));
  cj->cj_functionhset =
    mom_hashset_add_sized_items (NULL, funsv.vsequ->slen,
				 (momitem_t *const *) funsv.vsequ->arritm);
  {
    const momseq_t *fseq = mom_hashset_elements_set (cj->cj_functionhset);
    assert (fseq != NULL);
    unsigned nbfun = fseq->slen;
    for (unsigned ix = 0; ix < nbfun; ix++)
      {
	cjit_lock_item_mom (cj, (momitem_t *) fseq->arritm[ix]);
	cjit_scan_function_first_mom (cj, (momitem_t *) fseq->arritm[ix]);
	if (cj->cj_errormsg)
	  return;
      }
  }
}				/* end of cjit_first_scanning_pass */

static void
cjit_scan_function_first_mom (struct codejit_mom_st *cj, momitem_t *itmfun)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  MOM_FATAPRINTF ("cjit_scan_function_first unimplemented itmfun=%s",
		  mom_item_cstring (itmfun));
#warning cjit_scan_function_first_mom unimplemented
}				/* end of cjit_scan_function_first_mom */
