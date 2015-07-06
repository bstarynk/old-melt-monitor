// file codjit.c - manage the just-in-time code generation

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
#include "libgccjit.h"
#include <setjmp.h>

#ifndef LIBGCCJIT_HAVE_SWITCH_STATEMENTS
#error  libgccjit is too old since without switch statements
#endif /*LIBGCCJIT_HAVE_SWITCH_STATEMENTS */

#define CODEJIT_MAGIC_MOM 0x23b7bce7    /* codejit magic 599244007 */
struct codejit_mom_st
{
  unsigned cj_magic;            /* always CODEJIT_MAGIC_MOM */
  const momstring_t *cj_errormsg;       /* the error message */
  jmp_buf cj_jmpbuferror;       /* for longjmp on error */
  gcc_jit_context *cj_jitctxt;  /* for GCCJIT */
  momitem_t *cj_codjititm;
  momitem_t *cj_moduleitm;
  struct momhashset_st *cj_lockeditemset;       /* the set of locked items */
  struct momhashset_st *cj_functionhset;        /* the set of functions */
  struct momattributes_st *cj_globalbind;       /* global bindings */
  momitem_t *cj_curfunitm;      /* the current function */
  struct momattributes_st *cj_funbind;  /* the function's bindings */
  struct momqueuevalues_st cj_fundoque; /* the queue of closures to do */
  struct momhashset_st *cj_funconstset; /* the set of constant items */
  struct momhashset_st *cj_funclosedset;        /* the set of closed items */
};


#define CJIT_ERROR_MOM_AT_BIS(Lin,Cj,Fmt,...) do {	\
  struct codejit_mom_st *cj_##Lin = (Cj);		\
  assert (cj_##Lin					\
	  && cj_##Lin->cj_magic == CODEJIT_MAGIC_MOM);	\
  cj_##Lin->cj_errormsg =				\
    mom_make_string_sprintf(Fmt,__VA_ARGS__);		\
  mom_warnprintf_at(__FILE__,Lin,"CODEJIT ERROR: %s",	\
                    cj_##Lin->cj_errormsg->cstr);	\
  longjmp (cj->cj_jmpbuferror, (int)(Lin));		\
 }while(0)

#define CJIT_ERROR_MOM_AT(Lin,Cj,Fmt,...) \
  CJIT_ERROR_MOM_AT_BIS(Lin,Cj,Fmt,__VA_ARGS__)
#define CJIT_ERROR_MOM(Cj,Fmt,...) \
  CJIT_ERROR_MOM_AT(__LINE__,(Cj),(Fmt),__VA_ARGS__)




static void
cjit_lock_item_mom (struct codejit_mom_st *cj, momitem_t *itm)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (itm && !mom_hashset_contains (cj->cj_lockeditemset, itm))
    {
      mom_item_lock (itm);
      cj->cj_lockeditemset = mom_hashset_put (cj->cj_lockeditemset, itm);
    }
}                               /* end of cjit_lock_item_mom */

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
}                               /* end of cjit_unlock_all_items_mom */


static void
cjit_queue_to_do_mom (struct codejit_mom_st *cj, momvalue_t vtodo)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  MOM_DEBUGPRINTF (gencod, "queue_to_do %s", mom_output_gcstring (vtodo));
  if (vtodo.typnum != momty_node)
    CJIT_ERROR_MOM (cj, "queued vtodo %s is not a node",
                    mom_output_gcstring (vtodo));
  mom_queuevalue_push_back (&cj->cj_fundoque, vtodo);
}                               /* end of cjit_queue_to_do_mom */


static void
cjit_do_all_queued_to_do_at_mom (struct codejit_mom_st *cj, int lin)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  MOM_DEBUGPRINTF_AT (__FILE__, lin, gencod,
                      "do_all_queued_to_do start queuesize=%d",
                      (int) mom_queuevalue_size (&cj->cj_fundoque));
  long todocount = 0;
  while (mom_queuevalue_size (&cj->cj_fundoque) > 0)
    {
      todocount++;
      momvalue_t vtodo = mom_queuevalue_pop_front (&cj->cj_fundoque);
      MOM_DEBUGPRINTF_AT (__FILE__, lin, gencod,
                          "do_all_queued_to_do todocount#%ld vtodo=%s",
                          todocount, mom_output_gcstring (vtodo));
      if (!mom_applval_1itm_to_void (vtodo, cj->cj_codjititm))
        CJIT_ERROR_MOM (cj, "failed to do %s (from line#%d)",
                        mom_output_gcstring (vtodo), lin);
      MOM_DEBUGPRINTF_AT (__FILE__, lin, gencod,
                          "do_all_queued_to_do todocount#%ld done",
                          todocount);
    };
  MOM_DEBUGPRINTF_AT (__FILE__, lin, gencod,
                      "do_all_queued_to_do ended todocount#%ld", todocount);
}                               /* end of cjit_do_all_queued_to_do_at_mom */


#define cjit_do_all_queued_to_do_mom(Cj) \
  cjit_do_all_queued_to_do_at_mom((Cj),__LINE__)

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
  cj->cj_jitctxt = gcc_jit_context_acquire ();
  mom_item_lock (itmcjit);
  itmcjit->itm_kind = MOM_PREDEFINED_NAMED (code_generation);
  itmcjit->itm_data1 = (void *) cj;
  int errlin = 0;
  if ((errlin = setjmp (cj->cj_jmpbuferror)))
    {
      assert (cj->cj_errormsg);
      goto end;
    };
  cjit_first_scanning_pass_mom (itmcjit);
  if (cj->cj_errormsg)
    goto end;
end:
  cjit_unlock_all_items_mom (cj);
  if (cj->cj_jitctxt)
    {
      gcc_jit_context_release (cj->cj_jitctxt);
      cj->cj_jitctxt = NULL;
    }
  if (cj->cj_errormsg)
    {
      MOM_WARNPRINTF ("generate_jit_module failed: %s (at line %d)\n",
                      mom_string_cstr (cj->cj_errormsg), errlin);
      *res = mom_stringv (cj->cj_errormsg);
    }
  else
    {
      MOM_INFORMPRINTF ("generate_jit_module sucessful for %s",
                        mom_item_cstring (itm));
      *res = mom_itemv (itm);
    }
  return true;
}                               /* end of momfunc_1itm_to_val__generate_jit_module */



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
    CJIT_ERROR_MOM (cj, "module item %s is not a `code_module`",
                    mom_item_cstring (itmmod));
  ///// compute into cg_functionhset the hashed set of functions
  momvalue_t funsv =            //
    mom_item_unsync_get_attribute (itmmod, MOM_PREDEFINED_NAMED (functions));
  MOM_DEBUGPRINTF (gencod, "in module %s funsv= %s",
                   mom_item_cstring (itmmod), mom_output_gcstring (funsv));
  if (funsv.typnum == momty_node)
    {
      momvalue_t funsetv = MOM_NONEV;
      if (!mom_applval_2itm_to_val (funsv, itmmod, itmcjit, &funsetv)
          || (funsetv.typnum != momty_set || funsetv.typnum != momty_tuple))
        CJIT_ERROR_MOM (cj,
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
    CJIT_ERROR_MOM (cj, "module item %s : functions %s are not a set",
                    mom_item_cstring (itmmod), mom_output_gcstring (funsv));
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
        cj->cj_curfunitm = NULL;
        memset (&cj->cj_fundoque, 0, sizeof (cj->cj_fundoque));
      }
  }
}                               /* end of cjit_first_scanning_pass */


////////////////////////////////////////////////////////////////
static void
cjit_scan_function_for_signature_mom (struct codejit_mom_st *cj,
                                      momitem_t *itmfun,
                                      momitem_t *itmsignature);

static void
cjit_scan_function_constants_mom (struct codejit_mom_st *cj,
                                  momitem_t *itmfun, momvalue_t vconstants);


static void
cjit_scan_function_closed_mom (struct codejit_mom_st *cj,
                               momitem_t *itmfun, momvalue_t vclosed);


static void
cjit_scan_function_variables_mom (struct codejit_mom_st *cj,
                                  momitem_t *itmfun, momvalue_t vvariables);


static void
cjit_scan_function_code_next_mom (struct codejit_mom_st *cj,
                                  momitem_t *itmfun, momvalue_t vcode,
                                  momitem_t *nextitm, int nextpos);


static void
cjit_scan_function_first_mom (struct codejit_mom_st *cj, momitem_t *itmfun)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itmfun != NULL);
  momitem_t *itmsignature = NULL;
  MOM_DEBUGPRINTF (gencod, "cjit_scan_function_first scanning function %s",
                   mom_item_cstring (itmfun));
  cj->cj_curfunitm = itmfun;
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
        vfunctionsig =          //
          mom_item_unsync_get_attribute //
          (itmfun, MOM_PREDEFINED_NAMED (function_signature));
        MOM_DEBUGPRINTF (gencod, "scanning function %s vfunctionsig=%s",
                         mom_item_cstring (itmfun),
                         mom_output_gcstring (vfunctionsig));
        itmsignature = mom_value_to_item (vfunctionsig);
      }
  }
  if (!itmsignature
      || itmsignature->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s without signature",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun));
  cjit_lock_item_mom (cj, itmsignature);
  const unsigned nbfuninitattrs = 15;
  cj->cj_funbind = mom_attributes_make (nbfuninitattrs);
  cj->cj_funconstset = NULL;
  cj->cj_funclosedset = NULL;
  MOM_DEBUGPRINTF (gencod, "scanning function %s itmsignature %s",
                   mom_item_cstring (itmfun),
                   mom_item_cstring (itmsignature));
  cjit_scan_function_for_signature_mom (cj, itmfun, itmsignature);
  //scan the constants
  {
    momvalue_t vconstants = mom_item_unsync_get_attribute       //
      (itmfun,
       MOM_PREDEFINED_NAMED (constants));
    if (vconstants.typnum != momty_null)
      cjit_scan_function_constants_mom (cj, itmfun, vconstants);
  }
  //scan the closed
  {
    momvalue_t vclosed = mom_item_unsync_get_attribute  //
      (itmfun,
       MOM_PREDEFINED_NAMED (closed));
    if (vclosed.typnum != momty_null)
      cjit_scan_function_closed_mom (cj, itmfun, vclosed);
  }

  //scan the variables
  {
    momvalue_t vvariables = mom_item_unsync_get_attribute       //
      (itmfun,
       MOM_PREDEFINED_NAMED (variables));
    if (vvariables.typnum != momty_null)
      cjit_scan_function_variables_mom (cj, itmfun, vvariables);
  }
  //scan the function code
  {
    momvalue_t vcode = mom_item_unsync_get_attribute    //
      (itmfun,
       MOM_PREDEFINED_NAMED (code));
    if (vcode.typnum != momty_null)
      cjit_scan_function_code_next_mom (cj, itmfun, vcode, NULL, -1);
    else
      CJIT_ERROR_MOM (cj,
                      "module item %s : function %s without code",
                      mom_item_cstring (cj->cj_moduleitm),
                      mom_item_cstring (itmfun));
  }
  // do all queued to_do-s
  cjit_do_all_queued_to_do_mom (cj);
  MOM_FATAPRINTF ("cjit_scan_function_first unimplemented itmfun=%s",
                  mom_item_cstring (itmfun));
#warning cjit_scan_function_first_mom unimplemented
}                               /* end of cjit_scan_function_first_mom */


static void
cjit_scan_function_for_signature_mom (struct codejit_mom_st *cj,
                                      momitem_t *itmfun,
                                      momitem_t *itmsignature)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itmfun != NULL);
  assert (itmsignature != NULL);
  assert (itmfun == cj->cj_curfunitm);
  if (cj->cj_errormsg)
    return;
  momvalue_t valformals =       //
    mom_item_unsync_get_attribute (itmfun,
                                   MOM_PREDEFINED_NAMED (formals));
  momvalue_t valresults =       //
    mom_item_unsync_get_attribute (itmfun, MOM_PREDEFINED_NAMED (results));
  momvalue_t valinputypes =     //
    mom_item_unsync_get_attribute (itmsignature,
                                   MOM_PREDEFINED_NAMED (input_types));
  momvalue_t valoutputypes =    //
    mom_item_unsync_get_attribute (itmsignature,
                                   MOM_PREDEFINED_NAMED (output_types));
  const momseq_t *formalseq = mom_value_to_sequ (valformals);
  const momseq_t *resultseq = mom_value_to_sequ (valresults);
  MOM_DEBUGPRINTF (gencod,
                   "scanning function %s signature %s formals %s results %s",
                   mom_item_cstring (itmfun),
                   mom_item_cstring (itmsignature),
                   mom_output_gcstring (valformals),
                   mom_output_gcstring (valresults));
  if (valformals.typnum != momty_null && !formalseq)
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with bad `formals` %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun),
                    mom_output_gcstring (valformals));
  if (valresults.typnum != momty_null && !resultseq)
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with bad `results` %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun),
                    mom_output_gcstring (valresults));
  const momseq_t *inputypeseq = mom_value_to_tuple (valinputypes);
  const momseq_t *outputypeseq = mom_value_to_tuple (valoutputypes);
  unsigned nbins = 0, nbouts = 0;
  if (formalseq
      && (nbins = mom_seq_length (inputypeseq)) != mom_seq_length (formalseq))
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with `formals` %s"
                    " has signature %s with bad `input_types` %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun),
                    mom_output_gcstring (valformals),
                    mom_item_cstring (itmsignature),
                    mom_output_gcstring (valinputypes));
  if (resultseq
      && (nbouts =
          mom_seq_length (outputypeseq)) != mom_seq_length (resultseq))
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with `results` %s"
                    " has signature %s with bad `output_types` %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun),
                    mom_output_gcstring (valresults),
                    mom_item_cstring (itmsignature),
                    mom_output_gcstring (valoutputypes));
  // handle the input formals
  for (unsigned inix = 0; inix < nbins && !cj->cj_errormsg; inix++)
    {
      momitem_t *curformalitm = (momitem_t *) mom_seq_nth (formalseq, inix);
      momitem_t *curintypitm = (momitem_t *) mom_seq_nth (inputypeseq, inix);
      if (mom_attributes_find_entry (cj->cj_funbind, curformalitm))
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with duplicate formal #%d %s bound to %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun),
                        inix,
                        mom_item_cstring (curformalitm),
                        mom_output_gcstring (mom_attributes_find_value
                                             (cj->cj_funbind, curformalitm)));
      cjit_lock_item_mom (cj, curformalitm);
      cjit_lock_item_mom (cj, curintypitm);
      if (mom_value_to_item
          (mom_item_unsync_get_attribute
           (curformalitm, MOM_PREDEFINED_NAMED (type))) != curintypitm)
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with formal #%d %s mismatching type, wants %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun), inix,
                        mom_item_cstring (curformalitm),
                        mom_item_cstring (curintypitm));
      // add the formal bindings
      momvalue_t vnod = mom_nodev_new (MOM_PREDEFINED_NAMED (formals), 3,
                                       mom_itemv (itmfun),
                                       mom_itemv (curintypitm),
                                       mom_intv (inix));
      cj->cj_funbind =
        mom_attributes_put (cj->cj_funbind, curformalitm, &vnod);
      MOM_DEBUGPRINTF (gencod,
                       "scanning function-sig %s bound formal#%d %s to %s",
                       mom_item_cstring (itmfun), inix,
                       mom_item_cstring (curformalitm),
                       mom_output_gcstring (vnod));
    }
  // handle the output results
  for (unsigned outix = 0; outix < nbouts && !cj->cj_errormsg; outix++)
    {
      momitem_t *curesultitm = (momitem_t *) mom_seq_nth (resultseq, outix);
      momitem_t *curoutypitm =
        (momitem_t *) mom_seq_nth (outputypeseq, outix);
      if (mom_attributes_find_entry (cj->cj_funbind, curesultitm))
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with duplicate result #%d %s bound to %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun),
                        outix,
                        mom_item_cstring (curesultitm),
                        mom_output_gcstring (mom_attributes_find_value
                                             (cj->cj_funbind, curesultitm)));
      cjit_lock_item_mom (cj, curesultitm);
      cjit_lock_item_mom (cj, curoutypitm);
      if (mom_value_to_item
          (mom_item_unsync_get_attribute
           (curesultitm, MOM_PREDEFINED_NAMED (type))) != curoutypitm)
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with result #%d %s mismatching type, wants %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun), outix,
                        mom_item_cstring (curesultitm),
                        mom_item_cstring (curoutypitm));
      // add the results bindings
      momvalue_t vnod = mom_nodev_new (MOM_PREDEFINED_NAMED (results), 3,
                                       mom_itemv (itmfun),
                                       mom_itemv (curoutypitm),
                                       mom_intv (outix));
      cj->cj_funbind =
        mom_attributes_put (cj->cj_funbind, curesultitm, &vnod);
      MOM_DEBUGPRINTF (gencod,
                       "scanning function-sig %s bound result#%d %s to %s",
                       mom_item_cstring (itmfun), outix,
                       mom_item_cstring (curesultitm),
                       mom_output_gcstring (vnod));
    }
  MOM_DEBUGPRINTF (gencod,
                   "done scanning function %s signature %s",
                   mom_item_cstring (itmfun),
                   mom_item_cstring (itmsignature));
}                               /* end of cjit_scan_function_for_signature_mom */



static void
cjit_scan_function_constants_mom (struct codejit_mom_st *cj,
                                  momitem_t *itmfun, momvalue_t vconstants)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itmfun != NULL);
  assert (itmfun == cj->cj_curfunitm);
  MOM_DEBUGPRINTF (gencod,
                   "start scanning function %s constants %s",
                   mom_item_cstring (itmfun),
                   mom_output_gcstring (vconstants));
  const momseq_t *constantseq = mom_value_to_sequ (vconstants);
  if (!constantseq)
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with bad constants %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun),
                    mom_output_gcstring (vconstants));
  unsigned nbconsts = mom_seq_length (constantseq);
  for (unsigned kix = 0; kix < nbconsts && !cj->cj_errormsg; kix++)
    {
      momitem_t *curconstitm = (momitem_t *) mom_seq_nth (constantseq, kix);
      if (mom_attributes_find_entry (cj->cj_funbind, curconstitm))
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with duplicate constant #%d %s bound to %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun),
                        kix,
                        mom_item_cstring (curconstitm),
                        mom_output_gcstring (mom_attributes_find_value
                                             (cj->cj_funbind, curconstitm)));
      cjit_lock_item_mom (cj, curconstitm);
      momvalue_t valcurconst =  //
        mom_item_unsync_get_attribute (curconstitm,
                                       MOM_PREDEFINED_NAMED (value));
      if (valcurconst.typnum == momty_null)
        valcurconst = mom_itemv (curconstitm);
      cj->cj_funconstset = mom_hashset_put (cj->cj_funconstset, curconstitm);
      {
        // add the constant bindings
        momvalue_t vnod = mom_nodev_new (MOM_PREDEFINED_NAMED (constant), 3,
                                         mom_itemv (itmfun),
                                         valcurconst,
                                         mom_intv (kix));
        cj->cj_funbind =
          mom_attributes_put (cj->cj_funbind, curconstitm, &vnod);
        MOM_DEBUGPRINTF (gencod,
                         "scanning function-const %s bound constant#%d %s to %s",
                         mom_item_cstring (itmfun), kix,
                         mom_item_cstring (curconstitm),
                         mom_output_gcstring (vnod));
      }
    }
}                               /* end of cjit_scan_function_constants_mom */


static void
cjit_scan_function_closed_mom (struct codejit_mom_st *cj,
                               momitem_t *itmfun, momvalue_t vclosed)
{

  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itmfun != NULL);
  assert (itmfun == cj->cj_curfunitm);
  MOM_DEBUGPRINTF (gencod,
                   "start scanning function %s closed %s",
                   mom_item_cstring (itmfun), mom_output_gcstring (vclosed));
  const momseq_t *closedseq = mom_value_to_sequ (vclosed);
  if (!closedseq)
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with bad closed %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun), mom_output_gcstring (vclosed));
  unsigned nbclosed = mom_seq_length (closedseq);
  for (unsigned clix = 0; clix < nbclosed && !cj->cj_errormsg; clix++)
    {
      momitem_t *curclositm = (momitem_t *) mom_seq_nth (closedseq, clix);
      if (mom_attributes_find_entry (cj->cj_funbind, curclositm))
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with duplicate closed #%d %s bound to %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun),
                        clix,
                        mom_item_cstring (curclositm),
                        mom_output_gcstring (mom_attributes_find_value
                                             (cj->cj_funbind, curclositm)));
      cjit_lock_item_mom (cj, curclositm);
      {
        // add the closed bindings
        momvalue_t vnod = mom_nodev_new (MOM_PREDEFINED_NAMED (closed), 2,
                                         mom_itemv (itmfun),
                                         mom_intv (clix));
        cj->cj_funbind =
          mom_attributes_put (cj->cj_funbind, curclositm, &vnod);
        MOM_DEBUGPRINTF (gencod,
                         "scanning function-clos %s bound closed#%d %s to %s",
                         mom_item_cstring (itmfun), clix,
                         mom_item_cstring (curclositm),
                         mom_output_gcstring (vnod));
      }
    }
}                               /* end cjit_scan_function_closed_mom */



static void
cjit_scan_function_variables_mom (struct codejit_mom_st *cj,
                                  momitem_t *itmfun, momvalue_t vvariables)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itmfun != NULL);
  assert (itmfun == cj->cj_curfunitm);
  MOM_DEBUGPRINTF (gencod,
                   "start scanning function %s variables %s",
                   mom_item_cstring (itmfun),
                   mom_output_gcstring (vvariables));
  const momseq_t *variableseq = mom_value_to_sequ (vvariables);
  if (!variableseq)
    CJIT_ERROR_MOM (cj,
                    "module item %s : function %s with bad variables %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun),
                    mom_output_gcstring (vvariables));
  unsigned nbvars = mom_seq_length (variableseq);
  for (unsigned vix = 0; vix < nbvars && !cj->cj_errormsg; vix++)
    {
      momitem_t *curvaritm = (momitem_t *) mom_seq_nth (variableseq, vix);
      if (mom_attributes_find_entry (cj->cj_funbind, curvaritm))
        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with duplicate variable #%d %s bound to %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun),
                        vix,
                        mom_item_cstring (curvaritm),
                        mom_output_gcstring (mom_attributes_find_value
                                             (cj->cj_funbind, curvaritm)));
      cjit_lock_item_mom (cj, curvaritm);
      momitem_t *vartypitm =    //
        mom_value_to_item (mom_item_unsync_get_attribute (curvaritm,
                                                          MOM_PREDEFINED_NAMED
                                                          (type)));
      if (!vartypitm
          || (cjit_lock_item_mom (cj, vartypitm),
              vartypitm->itm_kind != MOM_PREDEFINED_NAMED (type)))

        CJIT_ERROR_MOM (cj,
                        "module item %s : function %s with variable #%d %s of bad type %s",
                        mom_item_cstring (cj->cj_moduleitm),
                        mom_item_cstring (itmfun),
                        vix,
                        mom_item_cstring (curvaritm),
                        mom_item_cstring (vartypitm));
      {
        // add the variable bindings
        momvalue_t vnod = mom_nodev_new (MOM_PREDEFINED_NAMED (variable), 3,
                                         mom_itemv (itmfun),
                                         mom_intv (vix),
                                         mom_itemv (vartypitm));
        cj->cj_funbind =
          mom_attributes_put (cj->cj_funbind, curvaritm, &vnod);
        MOM_DEBUGPRINTF (gencod,
                         "scanning function-var %s bound variable#%d %s to %s",
                         mom_item_cstring (itmfun), vix,
                         mom_item_cstring (curvaritm),
                         mom_output_gcstring (vnod));
      }
    }
}                               /* end cjit_scan_function_variables_mom */


static void
cjit_scan_node_code_next_mom (struct codejit_mom_st *cj,
                              const momnode_t *nodcod, momitem_t *nextitm,
                              int nextpos);

static void
cjit_scan_block_next_mom (struct codejit_mom_st *cj,
                          momitem_t *blockitm, momitem_t *nextitm,
                          int nextpos);

static void
cjit_scan_function_code_next_mom (struct codejit_mom_st *cj,
                                  momitem_t *itmfun, momvalue_t vcode,
                                  momitem_t *nextitm, int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itmfun != NULL);
  assert (itmfun == cj->cj_curfunitm);
  assert (vcode.typnum != momty_null);
  MOM_DEBUGPRINTF (gencod,
                   "scan_function_code_next start itmfun=%s vcode=%s nextitm=%s nextpos=%d",
                   mom_item_cstring (itmfun), mom_output_gcstring (vcode),
                   mom_item_cstring (nextitm), nextpos);
  switch (vcode.typnum)
    {
    case momty_node:
      {
        const momnode_t *nodcod = mom_value_to_node (vcode);
        momitem_t *opcoditm = mom_node_conn (nodcod);
        cjit_lock_item_mom (cj, opcoditm);
        switch (mom_item_hash (opcoditm))
          {
          case MOM_PREDEFINED_NAMED_CASE (code, opcoditm, bad_code_lab):
            cjit_scan_node_code_next_mom (cj, nodcod, nextitm, nextpos);
            break;
          default:
            break;
          };
      }
      break;
    case momty_item:
      cjit_scan_block_next_mom (cj, vcode.vitem, nextitm, nextpos);
      break;
    default:
      goto bad_code_lab;
    }
  return;
bad_code_lab:
  CJIT_ERROR_MOM (cj, "scan_function_code_next fun %s with bad code %s",
                  mom_item_cstring (itmfun), mom_output_gcstring (vcode));
#warning cjit_scan_function_code_mom unimplemented
}                               /* end of cjit_scan_function_code_mom */


/* function to scan a ^code(...) node */
static void
cjit_scan_node_code_next_mom (struct codejit_mom_st *cj,
                              const momnode_t *nodcod, momitem_t *nextitm,
                              int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (nodcod);
  momitem_t *codblkitm = mom_value_to_item (mom_node_nth (nodcod, 0));
  momvalue_t vcodn = mom_nodev (nodcod);
  if (!codblkitm)
    CJIT_ERROR_MOM (cj,
                    "scan_node_code_next in function %s vcodn=%s should have a code-block-item",
                    mom_item_cstring (cj->cj_curfunitm),
                    mom_output_gcstring (vcodn));
  cjit_lock_item_mom (cj, codblkitm);
  momvalue_t vblknodbind =
    mom_attributes_find_value (cj->cj_funbind, codblkitm);
  MOM_DEBUGPRINTF (gencod,
                   "start scan_node_code_next in function %s vcodn=%s codblkitm=%s vblknodbind=%s nextitm=%s nextpos=%d",
                   mom_item_cstring (cj->cj_curfunitm),
                   mom_output_gcstring (vcodn), mom_item_cstring (codblkitm),
                   mom_output_gcstring (vblknodbind),
                   mom_item_cstring (nextitm), nextpos);
  if (vblknodbind.typnum != momty_null)
    CJIT_ERROR_MOM (cj,
                    "scan_node_code_next in module %s function %s"
                    " got already bound codeblock item %s to %s for node %s",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (cj->cj_curfunitm),
                    mom_item_cstring (codblkitm),
                    mom_output_gcstring (vblknodbind),
                    mom_output_gcstring (vcodn));
  vblknodbind =                 //
    mom_nodev_new (MOM_PREDEFINED_NAMED (code_statement),
                   3, vcodn, mom_itemv (nextitm), mom_intv (nextpos));
  cj->cj_funbind =              //
    mom_attributes_put (cj->cj_funbind, codblkitm, &vblknodbind);
  MOM_DEBUGPRINTF (gencod,
                   "scan_node_code_next function %s vcodn=%s codblkitm=%s new vblknodbind=%s",
                   mom_item_cstring (cj->cj_curfunitm),
                   mom_output_gcstring (vcodn), mom_item_cstring (codblkitm),
                   mom_output_gcstring (vblknodbind));
  /// should probably fill the codblkitm with the statements, or clear it?
  /// should probably add a todo to scan the block
#warning scan_node_code_next unimplemented
  CJIT_ERROR_MOM (cj,
                  "scan_node_code_next unimplemented function %s vcodn=%s",
                  mom_item_cstring (cj->cj_curfunitm),
                  mom_output_gcstring (vcodn));

}                               /* end of cjit_scan_node_code_next_mom */


static void
cjit_scan_block_next_mom (struct codejit_mom_st *cj,
                          momitem_t *blockitm, momitem_t *nextitm,
                          int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  cjit_lock_item_mom (cj, blockitm);
  momvalue_t vblockbind =       //
    mom_attributes_find_value (cj->cj_funbind, blockitm);
  MOM_DEBUGPRINTF
    (gencod,
     "scan_block_next function %s; blockitm=%s vblockbind=%s nextitm=%s nextpos=%d",
     mom_item_cstring (cj->cj_curfunitm), mom_item_cstring (blockitm),
     mom_output_gcstring (vblockbind), mom_item_cstring (nextitm), nextpos);
  momvalue_t vnewblockbind =    //
    mom_nodev_new (MOM_PREDEFINED_NAMED (block),
                   2,
                   mom_itemv (nextitm),
                   mom_intv (nextpos));
  if (vblockbind.typnum == momty_null)
    {
      cj->cj_funbind =          //
        mom_attributes_put (cj->cj_funbind, blockitm, &vnewblockbind);
      MOM_DEBUGPRINTF
        (gencod, "scan_block_next function %s; blockitm=%s bound to %s",
         mom_item_cstring (cj->cj_curfunitm), mom_item_cstring (blockitm),
         mom_output_gcstring (vnewblockbind));
      // add todo scan inside the block
    }
  else if (mom_value_equal (vblockbind, vnewblockbind))
    {
      momnode_t *uselessnewnod = vnewblockbind.vnode;
      vnewblockbind = vblockbind;
      MOM_GC_FREE (uselessnewnod,
                   sizeof (momnode_t) + 2 * sizeof (momvalue_t));
    }
  else
    CJIT_ERROR_MOM (cj,
                    "scan_block_next function %s blockitm %s has invalid blockbind %s expecting %s",
                    mom_item_cstring (cj->cj_curfunitm),
                    mom_item_cstring (blockitm),
                    mom_output_gcstring (vblockbind),
                    mom_output_gcstring (vnewblockbind));

#warning scan_block_next unimplemented
  CJIT_ERROR_MOM (cj,
                  "scan_block_next unimplemented function %s blockitm=%s",
                  mom_item_cstring (cj->cj_curfunitm),
                  mom_item_cstring (blockitm));
}                               /* end of cjit_scan_block_next_mom */
