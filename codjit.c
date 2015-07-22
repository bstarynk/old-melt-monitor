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
#include <libgccjit.h>

#ifndef LIBGCCJIT_HAVE_SWITCH_STATEMENTS
#error  libgccjit is too old since without switch statements
#endif /*LIBGCCJIT_HAVE_SWITCH_STATEMENTS */

#define CODEJIT_MAGIC_MOM 0x23b7bce7    /* codejit magic 599244007 */
struct codejit_mom_st
{
  unsigned cj_magic;            /* always CODEJIT_MAGIC_MOM */
  const momstring_t *cj_errormsg;       /* the error message */
  jmp_buf cj_jmpbuferror;       /* for longjmp on error */
  momitem_t *cj_codjititm;
  momitem_t *cj_moduleitm;
  gcc_jit_context *cj_jitctxt;  /* for GCCJIT */
  FILE *cj_jitlog;              /* for GCCJIT */
  // indexed base of GCCJIT gcc_jit_object pointers
  unsigned cj_objcount;         /* number of GCCJIT gcc_jit_object-s
                                   and index of last used slot in
                                   cj_objptrarr */
  const gcc_jit_object **cj_objptrarr;  /* array of GCCJIT gcc_jit_object-s; slot#0 stays NULL */
  unsigned *cj_objhasharr;      /* hashed array of indexes for GCCJIT */
  unsigned cj_objsize;          /* allocated sizefor GCCJIT  of cj_objptrarr & cj_objhasharr */
  struct momhashset_st *cj_lockeditemset;       /* the set of locked items */
  struct momhashset_st *cj_functionhset;        /* the set of functions */
  struct momattributes_st *cj_globalbind;       /* global bindings */
  momitem_t *cj_curfunitm;      /* the current function */
  struct momattributes_st *cj_funbind;  /* the function's bindings */
  struct momqueuevalues_st cj_fundoque; /* the queue of closures to do */
  struct momhashset_st *cj_funconstset; /* the set of constant items */
  struct momhashset_st *cj_funclosedset;        /* the set of closed items */
  struct momattributes_st *cj_funleadattr;      /* associate for leaders of basic blocks */
  momitem_t *cj_curblockitm;
  momitem_t *cj_curstmtitm;
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


#define cjit_logprintf(Cj,Fmt,...) cjit_logprintf_at(Cj,__LINE__,Fmt "\n",__VA_ARGS__)
static void cjit_logprintf_at (struct codejit_mom_st *cj, int lin,
                               const char *fmt, ...)
  __attribute__ ((format (printf, 3, 4)));

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

static bool
cjit_is_locked_item_mom (struct codejit_mom_st *cj, momitem_t *itm)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (!itm)
    return false;
  return mom_hashset_contains (cj->cj_lockeditemset, itm);
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
}                               /* end of cjit_unlock_all_items_mom */


// given an index, find the GCCJIT object of that index
static inline const gcc_jit_object *
cjit_find_indexed_object_mom (struct codejit_mom_st *cj, unsigned ix)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (ix == 0 || ix > cj->cj_objcount)
    return NULL;
  assert (cj->cj_objptrarr != NULL);
  return cj->cj_objptrarr[ix];
}                               /* end of cjit_find_indexed_object_mom */

// given a GCCJIT object, find its index, or 0 if not known
static unsigned cjit_find_object_index_mom (struct codejit_mom_st *cj,
                                            const gcc_jit_object *obj);

// put a GCCJIT object in the indexed base - or give its index if it was there already
static unsigned cjit_put_object_mom (struct codejit_mom_st *cj,
                                     const gcc_jit_object *obj);

static void
cjit_logprintf_at (struct codejit_mom_st *cj, int lin, const char *fmt, ...)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (!cj->cj_jitlog)
    return;
  va_list args;
  fprintf (cj->cj_jitlog, "%s:%d: ", __FILE__, lin);
  va_start (args, fmt);
  vfprintf (cj->cj_jitlog, fmt, args);
  fflush (cj->cj_jitlog);
  va_end (args);
}                               /* end of cjit_logprintf */


static inline unsigned
cjit_object_hash (const gcc_jit_object *obj)
{
  if (!obj)
    return 0;
  unsigned hob =
    (unsigned) ((((intptr_t) obj * 61007) ^ ((intptr_t) obj / 523 +
                                             13))) >> 1;
  if (hob > 0)
    return hob;
  else
    return ((intptr_t) obj) % 829 + 10;
}                               /* end of cjit_object_hash */


static unsigned
cjit_raw_put_object (struct codejit_mom_st *cj, const gcc_jit_object *obj)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (!obj)
    return 0;
  unsigned obsiz = cj->cj_objsize;
  assert (obsiz > 10 && 5 * cj->cj_objcount + 2 < 4 * obsiz);
  unsigned hob = cjit_object_hash (obj);
  unsigned startix = hob % obsiz;
  for (unsigned ix = startix; ix < obsiz; ix++)
    {
      unsigned curhix = cj->cj_objhasharr[ix];
      if (curhix == 0)
        {
          unsigned newcount = ++cj->cj_objcount;
          assert (newcount < obsiz);
          assert (cj->cj_objptrarr[newcount] == NULL);
          cj->cj_objhasharr[ix] = newcount;
          cj->cj_objptrarr[newcount] = obj;
          return newcount;
        }
      else
        {
          assert (curhix < obsiz);
          assert (curhix <= cj->cj_objcount);
          const gcc_jit_object *curobj = cj->cj_objptrarr[curhix];
          assert (curobj != NULL);
          if (curobj == obj)
            return curhix;
          continue;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      unsigned curhix = cj->cj_objhasharr[ix];
      if (curhix == 0)
        {
          unsigned newcount = ++cj->cj_objcount;
          assert (newcount < obsiz);
          assert (cj->cj_objptrarr[newcount] == NULL);
          cj->cj_objhasharr[ix] = newcount;
          cj->cj_objptrarr[newcount] = obj;
          return newcount;
        }
      else
        {
          assert (curhix < obsiz);
          assert (curhix <= cj->cj_objcount);
          const gcc_jit_object *curobj = cj->cj_objptrarr[curhix];
          assert (curobj != NULL);
          if (curobj == obj)
            return curhix;
          continue;
        }
    }
  // should never happen
  MOM_FATAPRINTF
    ("cjit_raw_put_object corrupted object index tables in cj@%p",
     (void *) cj);
}                               /* end of cjit_raw_put_object */


// given a GCCJIT object, find its index, or 0 if not known
static unsigned
cjit_find_object_index_mom (struct codejit_mom_st *cj,
                            const gcc_jit_object *obj)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (!obj)
    return 0;
  unsigned obsiz = cj->cj_objsize;
  assert (obsiz > 10 && 5 * cj->cj_objcount + 2 < 4 * obsiz);
  unsigned hob = cjit_object_hash (obj);
  unsigned startix = hob % obsiz;
  for (unsigned ix = startix; ix < obsiz; ix++)
    {
      unsigned curhix = cj->cj_objhasharr[ix];
      if (curhix == 0)
        return 0;
      assert (curhix < obsiz);
      assert (curhix <= cj->cj_objcount);
      const gcc_jit_object *curobj = cj->cj_objptrarr[curhix];
      assert (curobj != NULL);
      if (curobj == obj)
        return curhix;
      continue;
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      unsigned curhix = cj->cj_objhasharr[ix];
      if (curhix == 0)
        return 0;
      assert (curhix < obsiz);
      assert (curhix <= cj->cj_objcount);
      const gcc_jit_object *curobj = cj->cj_objptrarr[curhix];
      assert (curobj != NULL);
      if (curobj == obj)
        return curhix;
      continue;
    }
  return 0;
}                               /* end of cjit_find_object_index_mom */


// put a GCCJIT object in the indexed base - or give its index if it was there already
static unsigned
cjit_put_object_mom (struct codejit_mom_st *cj, const gcc_jit_object *obj)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (!obj)
    return 0;
  unsigned obsiz = cj->cj_objsize;
  unsigned obcnt = cj->cj_objcount;
  assert (obsiz > 10 && obcnt < obsiz);
  assert (cj->cj_objptrarr[0] == NULL);
  if (4 * obcnt + 5 >= 3 * obsiz)
    {
      unsigned newsiz = mom_prime_above (3 * obcnt / 2 + 100);
      if (newsiz > obsiz)
        {
          const gcc_jit_object **oldptrarr = cj->cj_objptrarr;
          unsigned *oldhasharr = cj->cj_objhasharr;
          assert (oldptrarr != NULL);
          assert (oldhasharr != NULL);
          cj->cj_objhasharr =
            MOM_GC_SCALAR_ALLOC ("objhasharr",
                                 newsiz * sizeof (cj->cj_objhasharr[0]));
          cj->cj_objptrarr =
            MOM_GC_SCALAR_ALLOC ("objptrarr",
                                 newsiz * sizeof (cj->cj_objptrarr[0]));
          cj->cj_objsize = newsiz;
          cj->cj_objcount = 0;
          for (unsigned ix = 1; ix <= obcnt; ix++)
            {
              const gcc_jit_object *curoldobj = oldptrarr[ix];
              assert (curoldobj != NULL);
              (void) cjit_raw_put_object (cj, curoldobj);
            };
          assert (cj->cj_objcount == obcnt);
          obsiz = newsiz;
          MOM_GC_FREE (oldptrarr, obsiz * sizeof (oldptrarr[0]));
          MOM_GC_FREE (oldhasharr, obsiz * sizeof (oldhasharr[0]));
        }
    }
  return cjit_raw_put_object (cj, obj);
}                               /* end of cjit_put_object_mom */

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
static void cjit_second_emitting_pass_mom (momitem_t *itmcjit);
static void cjit_third_decorating_pass_mom (momitem_t *itmcjit);

static void
cjit_scan_block_next_mom (struct codejit_mom_st *cj,
                          momitem_t *blockitm, momitem_t *nextitm,
                          int nextpos);

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
  {                             // initialize an empty base for GCCJIT
    unsigned initsiz = mom_prime_above (100);
    cj->cj_objptrarr =
      MOM_GC_SCALAR_ALLOC ("GCCJIT objptrarr",
                           initsiz * sizeof (gcc_jit_object *));
    cj->cj_objhasharr =
      MOM_GC_SCALAR_ALLOC ("GCCJIT objhasharr", initsiz * sizeof (unsigned));
    cj->cj_objsize = initsiz;
  }
  if (MOM_IS_DEBUGGING (gencod))
    {
      char logpath[MOM_PATH_MAX];
      memset (logpath, 0, sizeof (logpath));
      snprintf (logpath, sizeof (logpath), "_jit_%s_p%d.log",
                mom_item_cstring (itm), (int) getpid ());
      if ((cj->cj_jitlog = fopen (logpath, "w")) == NULL)
        MOM_WARNPRINTF ("failed to open JIT logfile %s: %m", logpath);
      else
        {
          MOM_INFORMPRINTF ("opened JIT logfile %s for itmcjit=%s",
                            logpath, mom_item_cstring (itmcjit));
          fprintf (cj->cj_jitlog,
                   "##MONIMELT JIT logfile %s for itmcjit=%s\n", logpath,
                   mom_item_cstring (itmcjit));
        }
    };
  itmcjit->itm_kind = MOM_PREDEFINED_NAMED (code_generation);
  itmcjit->itm_data1 = (void *) cj;
  int errlin = 0;
  if ((errlin = setjmp (cj->cj_jmpbuferror)))
    {
      assert (cj->cj_errormsg);
      MOM_DEBUGPRINTF (gencod, "generate_jit_module errlin=%d", errlin);
      goto end;
    };
  MOM_DEBUGPRINTF (gencod,
                   "generate_jit_module itm=%s before first_scanning_pass itmcjit=%s",
                   mom_item_cstring (itm), mom_item_cstring (itmcjit));
  cjit_first_scanning_pass_mom (itmcjit);
  if (cj->cj_errormsg)
    goto end;
  //
  MOM_DEBUGPRINTF (gencod,
                   "generate_jit_module itm=%s before second_emitting_pass itmcjit=%s",
                   mom_item_cstring (itm), mom_item_cstring (itmcjit));
  cjit_second_emitting_pass_mom (itmcjit);
  if (cj->cj_errormsg)
    goto end;
  //
  MOM_DEBUGPRINTF (gencod,
                   "generate_jit_module itm=%s before third_decorating_pass itmcjit=%s",
                   mom_item_cstring (itm), mom_item_cstring (itmcjit));
  cjit_third_decorating_pass_mom (itmcjit);
  if (cj->cj_errormsg)
    goto end;
  MOM_DEBUGPRINTF (gencod,
                   "generate_jit_module itm=%s after third_decorating_pass itmcjit=%s",
                   mom_item_cstring (itm), mom_item_cstring (itmcjit));
end:
  cjit_unlock_all_items_mom (cj);
  if (cj->cj_jitctxt)
    {
      MOM_DEBUGPRINTF (gencod,
                       "generate_jit_module itm=%s releasing jitctxt@%p",
                       mom_item_cstring (itm), (void *) cj->cj_jitctxt);
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
  itmcjit->itm_kind = NULL;
  itmcjit->itm_data1 = NULL;
  mom_item_unlock (itmcjit);
  MOM_DEBUGPRINTF (gencod,
                   "generate_jit_module itm=%s itmcjit=%s done",
                   mom_item_cstring (itm), mom_item_cstring (itmcjit));
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
  cjit_logprintf (cj, "first_scanning_pass itmmod=%s",
                  mom_item_cstring (itmmod));
  ///// compute into cg_functionhset the hashed set of functions
  momvalue_t funsv =            //
    mom_item_unsync_get_attribute (itmmod,
                                   MOM_PREDEFINED_NAMED (functions));
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
                       mom_item_cstring (itmmod),
                       mom_output_gcstring (funsv),
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
  cjit_logprintf (cj, "first_scanning_pass end itmmod=%s",
                  mom_item_cstring (itmmod));
  MOM_DEBUGPRINTF (gencod, "cjit_first_scanning_pass end itmcjit=%s",
                   mom_item_cstring (itmcjit));
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
        if (itmfunkind->itm_kind == MOM_PREDEFINED_NAMED (signature))
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
          (itmfun, MOM_PREDEFINED_NAMED (signature));
        MOM_DEBUGPRINTF (gencod, "scanning function %s vfunctionsig=%s",
                         mom_item_cstring (itmfun),
                         mom_output_gcstring (vfunctionsig));
        itmsignature = mom_value_to_item (vfunctionsig);
      }
  }
  if (!itmsignature
      || itmsignature->itm_kind != MOM_PREDEFINED_NAMED (signature))
    CJIT_ERROR_MOM (cj, "module item %s : function %s without signature",
                    mom_item_cstring (cj->cj_moduleitm),
                    mom_item_cstring (itmfun));
  cjit_lock_item_mom (cj, itmsignature);
  const unsigned nbfuninitattrs = 15;
  cj->cj_funbind = mom_attributes_make (nbfuninitattrs);
  cj->cj_funconstset = NULL;
  cj->cj_funclosedset = NULL;
  cj->cj_funleadattr = mom_attributes_make (32);
  MOM_DEBUGPRINTF (gencod, "scanning function %s itmsignature %s",
                   mom_item_cstring (itmfun),
                   mom_item_cstring (itmsignature));
  cjit_scan_function_for_signature_mom (cj, itmfun, itmsignature);
  //scan the constants
  {
    momvalue_t vconstants = mom_item_unsync_get_attribute       //
      (itmfun,
       MOM_PREDEFINED_NAMED (constants));
    MOM_DEBUGPRINTF (gencod, "scanning function %s vconstants %s",
                     mom_item_cstring (itmfun),
                     mom_output_gcstring (vconstants));
    if (vconstants.typnum != momty_null)
      cjit_scan_function_constants_mom (cj, itmfun, vconstants);
  }
  //scan the closed
  {
    momvalue_t vclosed = mom_item_unsync_get_attribute  //
      (itmfun,
       MOM_PREDEFINED_NAMED (closed));
    MOM_DEBUGPRINTF (gencod, "scanning function %s vclosed %s",
                     mom_item_cstring (itmfun),
                     mom_output_gcstring (vclosed));
    if (vclosed.typnum != momty_null)
      cjit_scan_function_closed_mom (cj, itmfun, vclosed);
  }

  //scan the variables
  {
    momvalue_t vvariables = mom_item_unsync_get_attribute       //
      (itmfun,
       MOM_PREDEFINED_NAMED (variables));
    MOM_DEBUGPRINTF (gencod, "scanning function %s vvariables %s",
                     mom_item_cstring (itmfun),
                     mom_output_gcstring (vvariables));
    if (vvariables.typnum != momty_null)
      cjit_scan_function_variables_mom (cj, itmfun, vvariables);
  }
  //scan the function code
  {
    momvalue_t codv =           //
      mom_item_unsync_get_attribute (itmfun,
                                     MOM_PREDEFINED_NAMED (code));
    momitem_t *coditm =         //
      mom_value_to_item (codv);
    MOM_DEBUGPRINTF (gencod, "scanning function %s coditm %s",
                     mom_item_cstring (itmfun), mom_item_cstring (coditm));
    if (coditm)
      cjit_scan_block_next_mom (cj, coditm, NULL, -1);
    else
      CJIT_ERROR_MOM (cj,
                      "module item %s : function %s with bad code %s",
                      mom_item_cstring (cj->cj_moduleitm),
                      mom_item_cstring (itmfun), mom_output_gcstring (codv));
  }
  // do all queued to_do-s
  MOM_DEBUGPRINTF (gencod, "scanning function %s before doing queued todos",
                   mom_item_cstring (itmfun));
  cjit_do_all_queued_to_do_mom (cj);
  MOM_DEBUGPRINTF (gencod, "scanning function %s after doing queued todos",
                   mom_item_cstring (itmfun));
  MOM_FATAPRINTF ("cjit_scan_function_first unimplemented itmfun=%s",
                  mom_item_cstring (itmfun));
  // perhaps here: cj->cj_funleadattr = NULL;
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
                                         mom_itemv (vartypitm),
                                         mom_intv (vix));
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



static momitem_t *cjit_type_of_scanned_expr_mom (struct codejit_mom_st *cj,
                                                 const momvalue_t vexpr);

static momitem_t *cjit_type_of_scanned_node_mom (struct codejit_mom_st *cj,
                                                 const momnode_t *nod);

static momitem_t *cjit_type_of_scanned_item_mom (struct codejit_mom_st *cj,
                                                 momitem_t *itm);

static momitem_t *cjit_type_of_scanned_variable_mom (struct codejit_mom_st
                                                     *cj, momitem_t *varitm);

static void
cjit_scan_block_next_mom (struct codejit_mom_st *cj,
                          momitem_t *blockitm, momitem_t *nextitm,
                          int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  cjit_lock_item_mom (cj, blockitm);
  if (blockitm->itm_kind != MOM_PREDEFINED_NAMED (block))
    CJIT_ERROR_MOM (cj,
                    "scan_block_next function %s blockitm %s has invalid kind %s",
                    mom_item_cstring (cj->cj_curfunitm),
                    mom_item_cstring (blockitm),
                    mom_item_cstring (blockitm->itm_kind));

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
      // check that every component is a fresh statement item
      unsigned nbstmt = mom_unsync_item_components_count (blockitm);
      for (unsigned ix = 0; ix < nbstmt; ix++)
        {
          momvalue_t curstmtv =
            mom_raw_item_get_indexed_component (blockitm, ix);
          momitem_t *curstmtitm =       //
            mom_value_to_item (curstmtv);
          if (curstmtitm)
            cjit_lock_item_mom (cj, curstmtitm);
          if (!curstmtitm
              || curstmtitm->itm_kind !=
              MOM_PREDEFINED_NAMED (code_statement))
            CJIT_ERROR_MOM (cj,
                            "scan_block_next function %s blockitm %s has bad statment #%u %s",
                            mom_item_cstring (cj->cj_curfunitm),
                            mom_item_cstring (blockitm), ix,
                            mom_output_gcstring (curstmtv));
          momvalue_t stmtbindv =
            mom_attributes_find_value (cj->cj_funbind, curstmtitm);
          if (stmtbindv.typnum != momty_null)
            CJIT_ERROR_MOM (cj,
                            "scan_block_next function %s blockitm %s"
                            " has statement #%u %s already bound to %s",
                            mom_item_cstring (cj->cj_curfunitm),
                            mom_item_cstring (blockitm), ix,
                            mom_item_cstring (curstmtitm),
                            mom_output_gcstring (stmtbindv));
          stmtbindv = mom_nodev_new (MOM_PREDEFINED_NAMED (code_statement), 2,
                                     mom_itemv (blockitm), mom_intv (ix));
          cj->cj_funbind =      //
            mom_attributes_put (cj->cj_funbind, curstmtitm, &stmtbindv);

        }
      // add todo scan inside the block
      cjit_queue_to_do_mom (cj, //
                            mom_nodev_new (MOM_PREDEFINED_NAMED
                                           (jitdo_scan_block), 3,
                                           mom_itemv (blockitm),
                                           mom_itemv (nextitm),
                                           mom_intv (nextpos)));
      return;
    }
  else if (mom_value_equal (vblockbind, vnewblockbind))
    {
      momnode_t *uselessnewnod = (momnode_t *) vnewblockbind.vnode;
      vnewblockbind = vblockbind;
      MOM_GC_FREE (uselessnewnod,
                   sizeof (momnode_t) + 2 * sizeof (momvalue_t));
      MOM_DEBUGPRINTF (gencod,
                       "scan_block_next function %s; blockitm=%s already found, vblockbind=%s",
                       mom_item_cstring
                       (cj->cj_curfunitm),
                       mom_item_cstring (blockitm),
                       mom_output_gcstring (vblockbind));
      return;
    }
  else
    CJIT_ERROR_MOM (cj,
                    "scan_block_next function %s blockitm %s has invalid blockbind %s expecting %s",
                    mom_item_cstring (cj->cj_curfunitm),
                    mom_item_cstring (blockitm),
                    mom_output_gcstring (vblockbind),
                    mom_output_gcstring (vnewblockbind));
}                               /* end of cjit_scan_block_next_mom */


static void
cjit_scan_stmt_if_next_mom (struct codejit_mom_st *cj,
                            momitem_t *stmtitm, momitem_t *nextitm,
                            int nextpos);

static void
cjit_scan_stmt_int_switch_next_mom (struct codejit_mom_st *cj,
                                    momitem_t *stmtitm, momitem_t *nextitm,
                                    int nextpos);


static void
cjit_scan_stmt_item_switch_next_mom (struct codejit_mom_st *cj,
                                     momitem_t *stmtitm, momitem_t *nextitm,
                                     int nextpos);



static void
cjit_scan_stmt_jump_next_mom (struct codejit_mom_st *cj,
                              momitem_t *stmtitm, momitem_t *nextitm,
                              int nextpos);


static void
cjit_scan_stmt_code_next_mom (struct codejit_mom_st *cj,
                              momitem_t *stmtitm, momitem_t *nextitm,
                              int nextpos);

static void
cjit_scan_stmt_block_next_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momitem_t *nextitm,
                               int nextpos);

static void
cjit_scan_stmt_loop_next_mom (struct codejit_mom_st *cj,
                              momitem_t *stmtitm, momitem_t *nextitm,
                              int nextpos);


static void
cjit_scan_stmt_break_next_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momitem_t *nextitm,
                               int nextpos);


static void
cjit_scan_stmt_apply_next_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momitem_t *nextitm,
                               int nextpos);

static void
cjit_scan_stmt_apply_else_next_mom (struct codejit_mom_st *cj,
                                    momitem_t *stmtitm, momitem_t *nextitm,
                                    int nextpos);


static void
cjit_scan_stmt_primitive_next_mom (struct codejit_mom_st *cj,
                                   momitem_t *primopitm, momitem_t *stmtitm,
                                   momitem_t *nextitm, int nextpos);


static momitem_t *
cjit_get_statement_mom (struct codejit_mom_st *cj, momitem_t *blockitm,
                        int pos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (!blockitm || pos < 0)
    {
      MOM_DEBUGPRINTF (gencod, "get_statement: blockitm %s pos %d => NULL",
                       mom_item_cstring (blockitm), pos);
      return NULL;
    }
  if (!cjit_is_locked_item_mom (cj, blockitm)
      || blockitm->itm_kind != MOM_PREDEFINED_NAMED (block))
    CJIT_ERROR_MOM (cj, "get_statement: invalid blockitm %s",
                    mom_item_cstring (blockitm));
  unsigned nbstmt = mom_unsync_item_components_count (blockitm);
  if (pos < 0 || pos >= (int) nbstmt)
    CJIT_ERROR_MOM (cj,
                    "get_statement: blockitm %s with invalid pos %d (nbstmt=%u)",
                    mom_item_cstring (blockitm), pos, nbstmt);
  momvalue_t curstmtv = mom_raw_item_get_indexed_component (blockitm, pos);
  momitem_t *curstmtitm =       //
    mom_value_to_item (curstmtv);
  if (!curstmtitm)
    CJIT_ERROR_MOM (cj,
                    "get_statement: blockitm %s with bad curstmt %s at pos %d",
                    mom_item_cstring (blockitm),
                    mom_output_gcstring (curstmtv), pos);
  cjit_lock_item_mom (cj, curstmtitm);
  if (curstmtitm->itm_kind != MOM_PREDEFINED_NAMED (code_statement))
    CJIT_ERROR_MOM (cj,
                    "get_statement: blockitm %s with bad curstmt %s of kind %s at pos %d",
                    mom_item_cstring (blockitm),
                    mom_item_cstring (curstmtitm),
                    mom_item_cstring (curstmtitm->itm_kind), pos);
  MOM_DEBUGPRINTF (gencod, "get_statement: blockitm %s pos %d == %s",
                   mom_item_cstring (blockitm), pos,
                   mom_item_cstring (curstmtitm));
  return curstmtitm;
}                               /* end of cjit_get_statement_mom */


/* the blockitm & pos gives the reference where the stmtitm is
   sitting; it is not the "next" instruction, since we don't care
   about it ... */
static void
cjit_add_basic_block_stmt_leader_mom (struct codejit_mom_st *cj,
                                      momitem_t *stmtitm,
                                      /*inside: */ momitem_t *blockitm,
                                      int pos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (stmtitm);
  cjit_lock_item_mom (cj, stmtitm);
  if (stmtitm->itm_kind != MOM_PREDEFINED_NAMED (code_statement))
    CJIT_ERROR_MOM (cj, "invalid statement %s leading in block %s pos#%d",
                    mom_item_cstring (stmtitm), mom_item_cstring (blockitm),
                    pos);
  if (blockitm && pos >= 0)
    {
      momvalue_t curstmtv =
        mom_raw_item_get_indexed_component (blockitm, pos);
      momitem_t *curstmtitm =   //
        mom_value_to_item (curstmtv);
      if (curstmtitm != stmtitm)
        CJIT_ERROR_MOM (cj, "invalid statement %s leading in block %s pos#%d;"
                        " expecting %s",
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (blockitm), pos,
                        mom_output_gcstring (curstmtv));
    }
  momvalue_t leadv = mom_attributes_find_value (cj->cj_funleadattr, stmtitm);
  const momnode_t *leadnod = NULL;
  if (leadv.typnum == momty_null)
    {
      leadv = mom_nodev_new (MOM_PREDEFINED_NAMED (block), 2,
                             mom_itemv (blockitm), mom_intv (pos));
      cj->cj_funleadattr =
        mom_attributes_put (cj->cj_funleadattr, stmtitm, &leadv);
      MOM_DEBUGPRINTF (gencod,
                       "adding basic block leader stmtitm=%s leadv=%s",
                       mom_item_cstring (stmtitm),
                       mom_output_gcstring (leadv));
      return;
    }
  else if (!(leadnod = mom_value_to_node (leadv))
           || mom_node_conn (leadnod) != MOM_PREDEFINED_NAMED (block)
           || mom_node_arity (leadnod) != 2
           || mom_value_to_item (mom_node_nth (leadnod, 0)) != blockitm
           || mom_value_to_int (mom_node_nth (leadnod, 1), -2) != pos)
    CJIT_ERROR_MOM (cj, "invalid statement %s leading in block %s pos#%d;"
                    " already leading %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (blockitm),
                    pos, mom_output_gcstring (leadv));
  MOM_DEBUGPRINTF (gencod, "got basic block leader stmtitm=%s leadv=%s",
                   mom_item_cstring (stmtitm), mom_output_gcstring (leadv));
}                               /* end cjit_add_basic_block_stmt_leader_mom */


bool
momfunc_1itm_to_void__jitdo_scan_block (const
                                        momnode_t *clonod, momitem_t *cjitm)
{
  momitem_t *blockitm = NULL;
  momitem_t *nextitm = NULL;
  struct codejit_mom_st *cj = NULL;
  /// should never happen...
  if (mom_node_arity (clonod) != 3      //
      || !(blockitm =           //
           mom_value_to_item (mom_node_nth (clonod, 0)))        //
      || !cjitm || cjitm->itm_kind != MOM_PREDEFINED_NAMED (code_generation)
      || !(cj = cjitm->itm_data1) || cj->cj_magic != CODEJIT_MAGIC_MOM)
    MOM_FATAPRINTF ("jitdo_scan_block: bad clonode %s or cjitm %s",
                    mom_output_gcstring (mom_nodev (clonod)),
                    mom_item_cstring (cjitm));
  nextitm = mom_value_to_item (mom_node_nth (clonod, 1));
  int nextpos = mom_value_to_int (mom_node_nth (clonod, 1), -2);
  MOM_DEBUGPRINTF (gencod,
                   "jitdo_scan_block start blockitm=%s nextitm=%s nextpos=%d cjitm=%s",
                   mom_item_cstring (blockitm),
                   mom_item_cstring (nextitm),
                   nextpos, mom_item_cstring (cjitm));
  // cjit_scan_block_next_mom has already locked the statement
  if (!cjit_is_locked_item_mom (cj, blockitm)
      || blockitm->itm_kind != MOM_PREDEFINED_NAMED (block))
    CJIT_ERROR_MOM (cj,
                    "jitdo_scan_block: invalid blockitm %s",
                    mom_item_cstring (blockitm));
  cj->cj_curblockitm = blockitm;
  cj->cj_curstmtitm = NULL;
  unsigned nbstmt = mom_unsync_item_components_count (blockitm);
  momitem_t **stmtarr = MOM_GC_ALLOC ("stmtarr",
                                      (nbstmt + 1) * sizeof (momitem_t *));
  for (unsigned six = 0; six < nbstmt; six++)
    {
      momvalue_t curstmtv =
        mom_raw_item_get_indexed_component (blockitm, six);
      momitem_t *curstmtitm =   //
        mom_value_to_item (curstmtv);
      unsigned stmtlen = 0;
      if (!cjit_is_locked_item_mom (cj, curstmtitm)
          || curstmtitm->itm_kind != MOM_PREDEFINED_NAMED (code_statement)
          || !(stmtlen = mom_unsync_item_components_count (curstmtitm)))
        CJIT_ERROR_MOM (cj,
                        "jitdo_scan_block: in blockitm %s invalid statement #%d %s"
                        " of length %u", mom_item_cstring (blockitm), six,
                        mom_output_gcstring (curstmtv), stmtlen);
      cj->cj_curstmtitm = curstmtitm;
      stmtarr[six] = curstmtitm;
      momvalue_t curopv = mom_raw_item_get_indexed_component (curstmtitm, 0);
      momitem_t *curopitm =     //
        mom_value_to_item (curopv);
      if (!curopitm)
        goto bad_statement_lab;
      momitem_t *curnextitm = ((six + 1) < nbstmt) ? blockitm : nextitm;
      int curnextpos = ((six + 1) < nbstmt) ? ((int) six + 1) : nextpos;
      MOM_DEBUGPRINTF
        (gencod, "jitdo_scan_block blockitm %s curstmtitm %s six#%d"
         " curopitm %s curnextitm %s curnextpos=%d",
         mom_item_cstring (blockitm),
         mom_item_cstring (curstmtitm), six,
         mom_item_cstring (curopitm),
         mom_item_cstring (curnextitm), curnextpos);
      cjit_lock_item_mom (cj, curopitm);
      switch (mom_item_hash (curopitm))
        {
        case MOM_CASE_PREDEFINED_NAMED (int_switch, curopitm, otherwiseoplab):
          cjit_scan_stmt_int_switch_next_mom (cj, curstmtitm, curnextitm,
                                              curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (item_switch, curopitm, otherwiseoplab):
          cjit_scan_stmt_item_switch_next_mom (cj, curstmtitm, curnextitm,
                                               curnextpos);
          break;
          case MOM_CASE_PREDEFINED_NAMED (if, curopitm, otherwiseoplab)
        :
            cjit_scan_stmt_if_next_mom (cj, curstmtitm, curnextitm,
                                        curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (jump, curopitm, otherwiseoplab):
          cjit_scan_stmt_jump_next_mom (cj, curstmtitm, curnextitm,
                                        curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (block, curopitm, otherwiseoplab):
          cjit_scan_stmt_block_next_mom (cj, curstmtitm, curnextitm,
                                         curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (loop, curopitm, otherwiseoplab):
          cjit_scan_stmt_loop_next_mom (cj, curstmtitm, curnextitm,
                                        curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (break, curopitm, otherwiseoplab):
          cjit_scan_stmt_break_next_mom (cj, curstmtitm, curnextitm,
                                         curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (apply, curopitm, otherwiseoplab):
          cjit_scan_stmt_apply_next_mom (cj, curstmtitm, curnextitm,
                                         curnextpos);
          break;
        case MOM_CASE_PREDEFINED_NAMED (apply_else, curopitm, otherwiseoplab):
          cjit_scan_stmt_apply_else_next_mom (cj, curstmtitm, curnextitm,
                                              curnextpos);
          break;
        default:
        otherwiseoplab:
          if (curopitm->itm_kind == MOM_PREDEFINED_NAMED (primitive))
            cjit_scan_stmt_primitive_next_mom (cj, curopitm, curstmtitm,
                                               curnextitm, curnextpos);
          else
            goto bad_statement_lab;
        }
#warning jitdo_scan_block should do something with stmtarr
      cj->cj_curstmtitm = NULL;
      continue;
    bad_statement_lab:
      CJIT_ERROR_MOM (cj,
                      "jitdo_scan_block: in blockitm %s invalid statement #%d %s"
                      " with bad stmtop %s",
                      mom_item_cstring (blockitm), six,
                      mom_output_gcstring (curstmtv),
                      mom_output_gcstring (curopv));
    }
  cj->cj_curblockitm = NULL;
  cj->cj_curstmtitm = NULL;
  MOM_DEBUGPRINTF (gencod, "jitdo_scan_block end blockitm=%s",
                   mom_item_cstring (blockitm));
  return true;
}                               /* end of momfunc_1itm_to_void__jitdo_scan_block */


static intptr_t
cjit_get_integer_constant_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momvalue_t vexpr)
{
  momitem_t *expitm = NULL;
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (vexpr.typnum == momty_int)
    return vexpr.vint;
  else if ((expitm = mom_value_to_item (vexpr)) != NULL)
    {
      cjit_lock_item_mom (cj, expitm);
      momvalue_t constbindv =
        mom_attributes_find_value (cj->cj_funbind, expitm);
      momvalue_t constv = MOM_NONEV;
      const momnode_t *constbindnod = mom_value_to_node (constbindv);
      if (mom_node_conn (constbindnod) == MOM_PREDEFINED_NAMED (constant))
        {
          assert (mom_node_arity (constbindnod) == 3);
          constv = mom_node_nth (constbindnod, 1);
        }
      if (expitm->itm_kind != MOM_PREDEFINED_NAMED (constant)
          || constv.typnum != momty_int
          || cjit_type_of_scanned_expr_mom (cj,
                                            vexpr) !=
          MOM_PREDEFINED_NAMED (integer))
        goto badconstantlab;
      if (constv.typnum != momty_int)
        goto badconstantlab;
      return constv.vint;
    }
  else
  badconstantlab:
    CJIT_ERROR_MOM (cj, "invalid integer constant %s in statement %s",
                    mom_output_gcstring (vexpr), mom_item_cstring (stmtitm));
}                               /* end of cjit_get_integer_constant_mom */



static momitem_t *
cjit_get_item_constant_mom (struct codejit_mom_st *cj,
                            momitem_t *stmtitm, momvalue_t vexpr)
{
  momitem_t *expitm = NULL;
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if ((expitm = mom_value_to_item (vexpr)) != NULL)
    {
      cjit_lock_item_mom (cj, expitm);
      momvalue_t constbindv =
        mom_attributes_find_value (cj->cj_funbind, expitm);
      momvalue_t constv = MOM_NONEV;
      const momnode_t *constbindnod = mom_value_to_node (constbindv);
      if (mom_node_conn (constbindnod) == MOM_PREDEFINED_NAMED (constant))
        {
          assert (mom_node_arity (constbindnod) == 3);
          constv = mom_node_nth (constbindnod, 1);
        }
      if (expitm->itm_kind != MOM_PREDEFINED_NAMED (constant)
          || constv.typnum != momty_item
          || cjit_type_of_scanned_expr_mom (cj,
                                            vexpr) !=
          MOM_PREDEFINED_NAMED (item))
        goto badconstantlab;
      if (constv.typnum != momty_item)
        goto badconstantlab;
      return constv.vitem;
    }
  else
  badconstantlab:
    CJIT_ERROR_MOM (cj, "invalid item constant %s in statement %s",
                    mom_output_gcstring (vexpr), mom_item_cstring (stmtitm));
}                               /* end of cjit_get_item_constant_mom */

static void
cjit_add_block_statement_leader_mom (struct codejit_mom_st *cj,
                                     momitem_t *blockitm, int pos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  if (blockitm)
    {
      cjit_lock_item_mom (cj, blockitm);
      if (blockitm->itm_kind == MOM_PREDEFINED_NAMED (block))
        {
          momitem_t *stmtitm = cjit_get_statement_mom (cj, blockitm, pos);
          if (stmtitm)
            {
              cjit_add_basic_block_stmt_leader_mom (cj, stmtitm,
                                                    blockitm, pos);
            }
        }
    }
}                               /* end of cjit_add_block_statement_leader_mom */


static void
cjit_scan_stmt_if_next_mom (struct codejit_mom_st *cj,
                            momitem_t *stmtitm, momitem_t *nextitm,
                            int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  /* `if` *test* *then-block* [ *else-block* ]; the next statement is
     a leader. */
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  if (stmtlen < 3 || stmtlen > 4)
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_if: invalid if stmtitm %s of length %u",
                    mom_item_cstring (stmtitm), stmtlen);
  momvalue_t testexprv =        //
    mom_raw_item_get_indexed_component (stmtitm, 1);
  {
    momitem_t *typtestitm = cjit_type_of_scanned_expr_mom (cj, testexprv);
    if (!typtestitm || typtestitm == MOM_PREDEFINED_NAMED (void))
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_if: invalid if stmtitm %s with bad test %s",
                        mom_item_cstring (stmtitm),
                        mom_output_gcstring (testexprv));
  }
  momitem_t *thenitm =
    mom_value_to_item (mom_raw_item_get_indexed_component (stmtitm, 2));
  if (!thenitm)
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_if: invalid if stmtitm %s with bad then part",
                    mom_item_cstring (stmtitm));
  cjit_lock_item_mom (cj, thenitm);
  if (thenitm->itm_kind != MOM_PREDEFINED_NAMED (block))
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_if: invalid if stmtitm %s"
                    " with non-block then item %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (thenitm));
  cjit_scan_block_next_mom (cj, thenitm, nextitm, nextpos);
  if (stmtlen > 3)
    {
      momitem_t *elseitm =
        mom_value_to_item (mom_raw_item_get_indexed_component (stmtitm, 3));
      if (!elseitm)
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_if: invalid if stmtitm %s with bad else part",
                        mom_item_cstring (stmtitm));
      cjit_lock_item_mom (cj, elseitm);
      if (elseitm->itm_kind != MOM_PREDEFINED_NAMED (block))
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_if: invalid if stmtitm %s"
                        " with non-block else item %s",
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (elseitm));
      cjit_scan_block_next_mom (cj, elseitm, nextitm, nextpos);
    };
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_if_next_mom



static void
cjit_scan_stmt_int_switch_next_mom (struct codejit_mom_st *cj,
                                    momitem_t *stmtitm,
                                    momitem_t *nextitm, int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  momvalue_t selexprv = MOM_NONEV;
  momitem_t *seltypitm = NULL;
  if (stmtlen < 2
      || (selexprv =
          mom_raw_item_get_indexed_component (stmtitm,
                                              1)).typnum == momty_null
      || (seltypitm =
          cjit_type_of_scanned_expr_mom (cj, selexprv))
      != MOM_PREDEFINED_NAMED (integer))
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_int_switch: invalid int_switch statement %s",
                    mom_item_cstring (stmtitm));
  struct momhashassoc_st *ha = mom_hassoc_reserve (NULL, 4 * stmtlen / 3 + 5);
  for (unsigned casix = 2; casix < stmtlen; casix++)
    {
      momvalue_t casev = mom_raw_item_get_indexed_component (stmtitm, casix);
      const momnode_t *casenod = mom_value_to_node (casev);
      momitem_t *casconnitm = mom_node_conn (casenod);
      unsigned caselen = mom_node_arity (casenod);
      momitem_t *casblockitm = NULL;
      if (casconnitm == MOM_PREDEFINED_NAMED (case) && caselen == 2)
        {
          momvalue_t constv = mom_node_nth (casenod, 0);
          intptr_t casnum =
            cjit_get_integer_constant_mom (cj, stmtitm, constv);
          momvalue_t numv = mom_intv (casnum);
          // for convenience, we catch duplicate single-number cases.
          if (mom_hassoc_get (ha, numv).typnum != momty_null)
            CJIT_ERROR_MOM (cj,
                            "scan_stmt_int_switch: in int_switch statement %s"
                            " duplicate case #%d %s",
                            mom_item_cstring (stmtitm), casix,
                            mom_output_gcstring (casev));
          ha = mom_hassoc_put (ha, numv, casev);
          casblockitm = mom_value_to_item (mom_node_nth (casenod, 1));
          if (!casblockitm)
            goto badcase_lab;
        }
      else if (casconnitm == MOM_PREDEFINED_NAMED (case_range)
               && caselen == 4)
        {
          // we don't catch duplicate cases when using case_range,
          // since GCCJIT will error if something is wrong..
          momvalue_t loconstv = mom_node_nth (casenod, 0);
          intptr_t caslonum =
            cjit_get_integer_constant_mom (cj, stmtitm, loconstv);
          momvalue_t hiconstv = mom_node_nth (casenod, 0);
          intptr_t cashinum =
            cjit_get_integer_constant_mom (cj, stmtitm, hiconstv);
          if (caslonum > cashinum)
            goto badcase_lab;
          casblockitm = mom_value_to_item (mom_node_nth (casenod, 2));
          if (!casblockitm)
            goto badcase_lab;
        }
      else
      badcase_lab:
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_int_switch: invalid int_switch statement %s"
                        " bad case#%d %s",
                        mom_item_cstring (stmtitm), casix,
                        mom_output_gcstring (casev));
      cjit_lock_item_mom (cj, casblockitm);
      if (casblockitm->itm_kind != MOM_PREDEFINED_NAMED (block))
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_int_switch: invalid int_switch statement %s"
                        " bad case#%d %s with bad block %s",
                        mom_item_cstring (stmtitm), casix,
                        mom_output_gcstring (casev),
                        mom_item_cstring (casblockitm));
      cjit_scan_block_next_mom (cj, casblockitm, nextitm, nextpos);
    }                           /* end for casix */
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_int_switch_next_mom




static void
cjit_scan_stmt_item_switch_next_mom (struct codejit_mom_st *cj,
                                     momitem_t *stmtitm,
                                     momitem_t *nextitm, int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  momvalue_t selexprv = MOM_NONEV;
  momitem_t *seltypitm = NULL;
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  if (stmtlen < 2
      || (selexprv =
          mom_raw_item_get_indexed_component (stmtitm,
                                              1)).typnum == momty_null
      || !(seltypitm = cjit_type_of_scanned_expr_mom (cj, selexprv))
      || !mom_code_compatible_types (MOM_PREDEFINED_NAMED (item), seltypitm))
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_item_switch: invalid item_switch statement %s",
                    mom_item_cstring (stmtitm));
  struct momattributes_st *tb = mom_attributes_make (4 * stmtlen / 3 + 5);
  for (unsigned casix = 2; casix < stmtlen; casix++)
    {
      momvalue_t casev = mom_raw_item_get_indexed_component (stmtitm, casix);
      const momnode_t *casenod = mom_value_to_node (casev);
      momitem_t *casconnitm = mom_node_conn (casenod);
      unsigned caselen = mom_node_arity (casenod);
      momitem_t *casblockitm = NULL;
      if (casconnitm == MOM_PREDEFINED_NAMED (case) && caselen == 2)
        {
          momvalue_t casitmv = mom_node_nth (casenod, 0);
          momitem_t *casitm =
            cjit_get_item_constant_mom (cj, stmtitm, casitmv);
          if (mom_attributes_find_entry (tb, casitm))
            CJIT_ERROR_MOM (cj,
                            "scan_stmt_item_switch: item_switch statement %s"
                            " has case #%d: %s with duplicate item %s",
                            mom_item_cstring (stmtitm),
                            casix,
                            mom_output_gcstring (casev),
                            mom_item_cstring (casitm));
          tb = mom_attributes_put (tb, casitm, &casev);
          casblockitm = mom_value_to_item (mom_node_nth (casenod, 1));
          if (!casblockitm)
            goto badcase_lab;
        }
      else
      badcase_lab:
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_item_switch: item_switch statement %s"
                        " has invalid case #%d: %s",
                        mom_item_cstring (stmtitm),
                        casix, mom_output_gcstring (casev));
      cjit_lock_item_mom (cj, casblockitm);
      if (casblockitm->itm_kind != MOM_PREDEFINED_NAMED (block))
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_item_switch: invalid item_switch statement %s"
                        " bad case#%d %s with bad block %s",
                        mom_item_cstring (stmtitm), casix,
                        mom_output_gcstring (casev),
                        mom_item_cstring (casblockitm));
      cjit_scan_block_next_mom (cj, casblockitm, nextitm, nextpos);
    }
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_item_switch_next_mom




static void
cjit_scan_stmt_jump_next_mom (struct codejit_mom_st *cj,
                              momitem_t *stmtitm, momitem_t *nextitm,
                              int nextpos)
{/**
  * `jump` *block-item* ; is an unconditional jump (so could play the
  role of a `continue` to restart the block); 
 **/
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  momvalue_t targetv = MOM_NONEV;
  momitem_t *targetitm = NULL;
  if (stmtlen != 2
      || ((targetv = mom_raw_item_get_indexed_component (stmtitm, 1)).typnum
          != momty_item)
      || !(targetitm = targetv.vitem)
      || (cjit_lock_item_mom (cj, targetitm),
          targetitm->itm_kind != MOM_PREDEFINED_NAMED (block)))
    CJIT_ERROR_MOM (cj, "scan_stmt_jump: invalid jump statement %s",
                    mom_item_cstring (stmtitm));
  cjit_scan_block_next_mom (cj, targetitm, nextitm, nextpos);
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_jump_next_mom


static void
cjit_scan_stmt_break_next_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momitem_t *nextitm,
                               int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  momvalue_t targetv = MOM_NONEV;
  momitem_t *targetitm = NULL;
  if (stmtlen != 2
      || ((targetv = mom_raw_item_get_indexed_component (stmtitm, 1)).typnum
          != momty_item)
      || !(targetitm = targetv.vitem)
      || (cjit_lock_item_mom (cj, targetitm),
          targetitm->itm_kind != MOM_PREDEFINED_NAMED (block)))
    CJIT_ERROR_MOM (cj, "scan_stmt_break: invalid break statement %s",
                    mom_item_cstring (stmtitm));
  momvalue_t targbindv =
    mom_attributes_find_value (cj->cj_funbind, targetitm);
  const momnode_t *targbindnod = NULL;
  momvalue_t targbind0v = MOM_NONEV;
  momvalue_t targbind1v = MOM_NONEV;
  momitem_t *targnextitm = NULL;
  int targnextpos = -2;
  if (targbindv.typnum == momty_null)
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_break: break statement %s with unbound target block %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (targetitm));
  if (!(targbindnod = mom_value_to_node (targbindv))
      || mom_node_conn (targbindnod) != MOM_PREDEFINED_NAMED (block)
      || mom_node_arity (targbindnod) != 2
      || !(targnextitm =
           mom_value_to_item ((targbind0v = mom_node_nth (targbindnod, 0))))
      || (targnextpos =
          mom_value_to_int ((targbind1v =
                             mom_node_nth (targbindnod, 1)), -2)) < 0)
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_break: break statement %s"
                    " with strange target %s bound to %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (targetitm),
                    mom_output_gcstring (targbindv));
  momvalue_t vnod = mom_nodev_new (MOM_PREDEFINED_NAMED (break), 3,
                                   mom_itemv (targetitm),
                                   targbind0v,
                                   targbind1v);
  cj->cj_funbind = mom_attributes_put (cj->cj_funbind, stmtitm, &vnod);
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_break_next_mom



static void
cjit_scan_stmt_block_next_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momitem_t *nextitm,
                               int nextpos)
{
  /**
* `block` *blockitem* *sub-statement* ... like `code` statements, but
  the block is explicitly given.
  **/
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  if (stmtlen < 2)
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_block: invalid block statement %s",
                    mom_item_cstring (stmtitm));
  for (unsigned six = 2; six < stmtlen; six++)
    {
      momvalue_t substmtv = mom_raw_item_get_indexed_component (stmtitm, six);
      momitem_t *substmtitm = mom_value_to_item (substmtv);
      if (!substmtitm
          || (cjit_lock_item_mom (cj, substmtitm),
              substmtitm->itm_kind != MOM_PREDEFINED_NAMED (code_statement)))
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_block: invalid block statement %s with bad substmt#%d %s",
                        mom_item_cstring (stmtitm), six,
                        mom_output_gcstring (substmtv));
    };
  momitem_t *blockitm =         //
    mom_value_to_item (mom_raw_item_get_indexed_component (stmtitm, 1));
  if (!blockitm)
    {
      blockitm = mom_make_anonymous_item ();
      blockitm->itm_kind = MOM_PREDEFINED_NAMED (block);
      blockitm->itm_space =     //
        (stmtitm->itm_space > momspa_transient
         && stmtitm->itm_space <
         momspa_predefined) ? stmtitm->itm_space : momspa_transient;
      mom_unsync_item_put_nth_component (stmtitm, 1, mom_itemv (blockitm));
      cjit_lock_item_mom (cj, blockitm);
    }
  momvalue_t vblockbind =       //
    mom_attributes_find_value (cj->cj_funbind, blockitm);
  if (vblockbind.typnum != momty_null)
    CJIT_ERROR_MOM (cj, "scan_stmt_block: invalid block statement %s"
                    " with block %s already bound to %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (blockitm),
                    mom_output_gcstring (vblockbind));
  blockitm->itm_comps = mom_components_reserve (NULL, stmtlen - 2);
  for (unsigned six = 2; six < stmtlen; six++)
    {
      momvalue_t substmtv = mom_raw_item_get_indexed_component (stmtitm, six);
      momitem_t *substmtitm = mom_value_to_item (substmtv);
      assert (substmtitm && cjit_is_locked_item_mom (cj, substmtitm)
              && substmtitm->itm_kind ==
              MOM_PREDEFINED_NAMED (code_statement));
      blockitm->itm_comps =
        mom_components_append1 (blockitm->itm_comps, mom_itemv (substmtitm));
    }
  cjit_scan_block_next_mom (cj, blockitm, nextitm, nextpos);
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_block_next_mom



static void
cjit_scan_stmt_loop_next_mom (struct codejit_mom_st *cj,
                              momitem_t *stmtitm, momitem_t *nextitm,
                              int nextpos)
{
  /**
* `loop` *blockitem* *sub-statement* ... like a `block` statement, for
an infinite loop of given block item, but the block is explicitly
given - if it is nil, it becomes created and stored as the component
#1.
   **/
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  if (stmtlen < 2)
    CJIT_ERROR_MOM (cj,
                    "scan_stmt_loop: invalid loop statement %s",
                    mom_item_cstring (stmtitm));
  for (unsigned six = 2; six < stmtlen; six++)
    {
      momvalue_t substmtv = mom_raw_item_get_indexed_component (stmtitm, six);
      momitem_t *substmtitm = mom_value_to_item (substmtv);
      if (!substmtitm
          || (cjit_lock_item_mom (cj, substmtitm),
              substmtitm->itm_kind != MOM_PREDEFINED_NAMED (code_statement)))
        CJIT_ERROR_MOM (cj,
                        "scan_stmt_loop: invalid loop statement %s with bad substmt#%d %s",
                        mom_item_cstring (stmtitm), six,
                        mom_output_gcstring (substmtv));
    };
  momitem_t *blockitm =         //
    mom_value_to_item (mom_raw_item_get_indexed_component (stmtitm, 1));
  if (!blockitm)
    {
      blockitm = mom_make_anonymous_item ();
      blockitm->itm_kind = MOM_PREDEFINED_NAMED (block);
      blockitm->itm_space =     //
        (stmtitm->itm_space > momspa_transient
         && stmtitm->itm_space <
         momspa_predefined) ? stmtitm->itm_space : momspa_transient;
      mom_unsync_item_put_nth_component (stmtitm, 1, mom_itemv (blockitm));
      cjit_lock_item_mom (cj, blockitm);
    }
  momvalue_t vblockbind =       //
    mom_attributes_find_value (cj->cj_funbind, blockitm);
  if (vblockbind.typnum != momty_null)
    CJIT_ERROR_MOM (cj, "scan_stmt_loop: invalid loop statement %s"
                    " with block %s already bound to %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (blockitm),
                    mom_output_gcstring (vblockbind));
  momitem_t *lastjumpitm = NULL;
  {
    unsigned blocklen = mom_unsync_item_components_count (blockitm);
    unsigned laststmtlen = 0;
    momitem_t *laststmtitm = (blocklen > 0)     //
      ?
      mom_value_to_item (mom_raw_item_get_indexed_component
                         (blockitm, blocklen - 1)) : NULL;
    if (laststmtitm
        && (cjit_lock_item_mom (cj, laststmtitm),
            laststmtitm->itm_kind == MOM_PREDEFINED_NAMED (code_statement))
        && (laststmtlen = mom_unsync_item_components_count (laststmtitm)) == 2
        &&
        (mom_value_to_item
         (mom_raw_item_get_indexed_component (laststmtitm, 0)) ==
         MOM_PREDEFINED_NAMED (jump))
        &&
        (mom_value_to_item
         (mom_raw_item_get_indexed_component (laststmtitm, 1)) == blockitm))
      lastjumpitm = laststmtitm;
  }
  if (!lastjumpitm)
    {
      lastjumpitm = mom_make_anonymous_item ();
      lastjumpitm->itm_kind = MOM_PREDEFINED_NAMED (code_statement);
      lastjumpitm->itm_space =  //
        (blockitm->itm_space > momspa_transient
         && blockitm->itm_space <
         momspa_predefined) ? blockitm->itm_space : momspa_transient;
    }
  cjit_lock_item_mom (cj, lastjumpitm);
  lastjumpitm->itm_comps = mom_components_reserve (NULL, 2);
  lastjumpitm->itm_comps =      //
    mom_components_append1 (lastjumpitm->itm_comps,
                            mom_itemv (MOM_PREDEFINED_NAMED (jump)));
  lastjumpitm->itm_comps =      //
    mom_components_append1 (lastjumpitm->itm_comps, mom_itemv (blockitm));
  blockitm->itm_comps = mom_components_reserve (NULL, stmtlen - 1);
  for (unsigned six = 2; six < stmtlen; six++)
    {
      momvalue_t substmtv = mom_raw_item_get_indexed_component (stmtitm, six);
      momitem_t *substmtitm = mom_value_to_item (substmtv);
      assert (substmtitm && cjit_is_locked_item_mom (cj, substmtitm)
              && substmtitm->itm_kind ==
              MOM_PREDEFINED_NAMED (code_statement));
      blockitm->itm_comps =
        mom_components_append1 (blockitm->itm_comps, mom_itemv (substmtitm));
    }
  blockitm->itm_comps =
    mom_components_append1 (blockitm->itm_comps, mom_itemv (lastjumpitm));
  cjit_scan_block_next_mom (cj, blockitm, nextitm, nextpos);
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
}                               // end of cjit_scan_stmt_loop_next_mom


static void
cjit_scan_apply_next_mom (struct codejit_mom_st *cj,
                          momitem_t *stmtitm, momitem_t *nextitm,
                          int nextpos, bool haselse)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  if (stmtlen < 3)
    goto badapply_lab;
  momitem_t *sigitm             //
    = mom_value_to_item (mom_raw_item_get_indexed_component (stmtitm, 1));
  if (!sigitm || ((cjit_lock_item_mom (cj, sigitm), sigitm->itm_kind)   //
                  != MOM_PREDEFINED_NAMED (signature)))
    goto badapply_lab;
  momvalue_t inputypv =         //
    mom_item_unsync_get_attribute (sigitm,
                                   MOM_PREDEFINED_NAMED (input_types));
  const momseq_t *inputyptup = mom_value_to_tuple (inputypv);
  momvalue_t outputypv =        //
    mom_item_unsync_get_attribute (sigitm,
                                   MOM_PREDEFINED_NAMED (output_types));
  const momseq_t *outputyptup = mom_value_to_tuple (outputypv);
  if (!inputyptup || !outputyptup)
    {
      // issue a warning, the signature is corrupted
      MOM_WARNPRINTF ("apply statement %s with corrupted signature %s"
                      " has input_types %s, output_types %s",
                      mom_item_cstring (stmtitm),
                      mom_item_cstring (sigitm),
                      mom_output_gcstring (inputypv),
                      mom_output_gcstring (outputypv));
      goto badapply_lab;
    };
  unsigned nbins = mom_seq_length (inputyptup);
  unsigned nbouts = mom_seq_length (outputyptup);
  if (haselse)
    {
      if (stmtlen != nbins + nbouts + 3)
        CJIT_ERROR_MOM (cj,
                        "scan_apply_next: invalid apply_else statement %s"
                        " in block %s in function %s"
                        " of size %d"
                        " but needs %d results and %d arguments",
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (cj->cj_curblockitm),
                        mom_item_cstring (cj->cj_curfunitm),
                        stmtlen, nbouts, nbins);
    }
  else
    {
      if (stmtlen != nbins + nbouts + 2)
        CJIT_ERROR_MOM (cj,
                        "scan_apply_next: invalid apply statement %s"
                        " in block %s in function %s"
                        " of size %d"
                        " but needs %d results and %d arguments",
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (cj->cj_curblockitm),
                        mom_item_cstring (cj->cj_curfunitm),
                        stmtlen, nbouts, nbins);
    }
  for (unsigned resix = 0; resix < nbouts; resix++)
    {
      momitem_t *curesoutitm =  //
        mom_value_to_item (mom_raw_item_get_indexed_component
                           (stmtitm, 2 + resix));
      momitem_t *curestypitm = NULL;
      momitem_t *curoutformaltypitm = mom_seq_nth (outputyptup, resix);
      if (!curesoutitm || !curoutformaltypitm
          || !(curestypitm =
               cjit_type_of_scanned_variable_mom (cj, curesoutitm))
          || curestypitm != curoutformaltypitm)
        CJIT_ERROR_MOM (cj,
                        "scan_apply_next: apply statement %s"
                        " in block %s in function %s"
                        " has bad result#%d %s expected type %s actual type %s",
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (cj->cj_curblockitm),
                        mom_item_cstring (cj->cj_curfunitm), resix,
                        mom_item_cstring (curesoutitm),
                        mom_item_cstring (curoutformaltypitm),
                        mom_item_cstring (curestypitm));
    };                          /* end for resix */
  for (unsigned argix = 0; argix < nbins; argix++)
    {
      momvalue_t curargexprv =  //
        mom_raw_item_get_indexed_component (stmtitm, 2 + nbouts + argix);
      momitem_t *curargtypitm = NULL;
      momitem_t *curinformaltypitm = mom_seq_nth (inputyptup, argix);
      if (!curinformaltypitm
          || !(curargtypitm =
               cjit_type_of_scanned_expr_mom (cj, curargexprv))
          || !mom_code_compatible_types (curinformaltypitm, curargtypitm))
        CJIT_ERROR_MOM (cj,
                        "scan_apply_next: apply statement %s"
                        " in block %s in function %s"
                        " has bad argument#%d %s expected type %s actual type %s",
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (cj->cj_curblockitm),
                        mom_item_cstring (cj->cj_curfunitm), argix,
                        mom_output_gcstring (curargexprv),
                        mom_item_cstring (curinformaltypitm),
                        mom_item_cstring (curargtypitm));
    };                          /* end for argix */
  if (nextitm)
    cjit_add_block_statement_leader_mom (cj, nextitm, nextpos);
#warning cjit_scan_apply_next_mom incomplete
  MOM_FATAPRINTF ("cjit_scan_apply_next_mom incomplete stmtitm %s",
                  mom_item_cstring (stmtitm));
badapply_lab:
  CJIT_ERROR_MOM (cj,
                  "scan_apply_next: invalid apply statement %s in block %s in function %s",
                  mom_item_cstring (stmtitm),
                  mom_item_cstring (cj->cj_curblockitm),
                  mom_item_cstring (cj->cj_curfunitm));
}                               /* end of cjit_scan_apply_next_mom */



static void
cjit_scan_stmt_apply_next_mom (struct codejit_mom_st *cj,
                               momitem_t *stmtitm, momitem_t *nextitm,
                               int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  cjit_scan_apply_next_mom (cj, stmtitm, nextitm, nextpos, false);
}                               // end of cjit_scan_stmt_apply_next_mom

static void
cjit_scan_stmt_apply_else_next_mom (struct codejit_mom_st *cj,
                                    momitem_t *stmtitm,
                                    momitem_t *nextitm, int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  cjit_scan_apply_next_mom (cj, stmtitm, nextitm, nextpos, true);
}                               // end of cjit_scan_stmt_apply_else_next_mom


static void
cjit_scan_stmt_primitive_next_mom (struct codejit_mom_st *cj,
                                   momitem_t *primopitm, momitem_t *stmtitm,
                                   momitem_t *nextitm, int nextpos)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  unsigned stmtlen = mom_unsync_item_components_count (stmtitm);
  assert (primopitm
          && primopitm->itm_kind == MOM_PREDEFINED_NAMED (primitive));
  assert (stmtitm
          && stmtitm->itm_kind == MOM_PREDEFINED_NAMED (code_statement));
  momitem_t *sigitm =           //
    mom_value_to_item           //
    (mom_item_unsync_get_attribute (primopitm,
                                    MOM_PREDEFINED_NAMED (signature)));
  if (!sigitm
      || (cjit_lock_item_mom (cj, sigitm),
          sigitm->itm_kind) != MOM_PREDEFINED_NAMED (signature))
    goto bad_signature_lab;
  momvalue_t inputypv =         //
    mom_item_unsync_get_attribute (sigitm,
                                   MOM_PREDEFINED_NAMED (input_types));
  const momseq_t *inputyptup = mom_value_to_tuple (inputypv);
  if (!inputyptup)
    {
      // this really should not happen, we issue a specific warning
      MOM_WARNPRINTF ("corrupted signature %s of primitive %s"
                      " has bad input_types %s",
                      mom_item_cstring (sigitm),
                      mom_item_cstring (primopitm),
                      mom_output_gcstring (inputypv));
      goto bad_signature_lab;
    };
  unsigned nbins = mom_seq_length (inputyptup);
  if (stmtlen != nbins + 1)
    CJIT_ERROR_MOM (cj,
                    "bad primitive statement %s with operator %s"
                    " had mismatching arity (length %d, expecting %d)"
                    " for signature %s"
                    " in block %s in function %s",
                    mom_item_cstring (stmtitm), mom_item_cstring (primopitm),
                    stmtlen, nbins + 1,
                    mom_item_cstring (sigitm),
                    mom_item_cstring (cj->cj_curblockitm),
                    mom_item_cstring (cj->cj_curfunitm));
  for (unsigned ix = 0; ix < nbins; ix++)
    {
      momitem_t *curtypitm = mom_seq_nth (inputyptup, ix);
      assert (curtypitm != NULL);
      momvalue_t curarg = mom_raw_item_get_indexed_component (stmtitm, ix);
      momitem_t *argtypitm = cjit_type_of_scanned_expr_mom (cj, curarg);
      if (curtypitm != argtypitm)
        CJIT_ERROR_MOM (cj,
                        "in primitive %s statement %s in block %s in function %s"
                        " formal #%d expecting type %s but argument %s has type %s",
                        mom_item_cstring (primopitm),
                        mom_item_cstring (stmtitm),
                        mom_item_cstring (cj->cj_curblockitm),
                        mom_item_cstring (cj->cj_curfunitm), ix,
                        mom_item_cstring (argtypitm),
                        mom_output_gcstring (curarg),
                        mom_item_cstring (curtypitm));
    }
  return;
bad_signature_lab:
  CJIT_ERROR_MOM (cj,
                  "bad primitive statement %s with operator %s in block %s in function %s",
                  mom_item_cstring (stmtitm), mom_item_cstring (primopitm),
                  mom_item_cstring (cj->cj_curblockitm),
                  mom_item_cstring (cj->cj_curfunitm));
}                               /* end of cjit_scan_stmt_primitive_next_mom */



static momitem_t *
cjit_type_of_scanned_expr_mom (struct codejit_mom_st *cj,
                               const momvalue_t vexpr)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  switch (vexpr.typnum)
    {
    case momty_null:
      return NULL;
    case momty_int:
      return MOM_PREDEFINED_NAMED (integer);
    case momty_double:
      return MOM_PREDEFINED_NAMED (double);
    case momty_node:
      return cjit_type_of_scanned_node_mom (cj, vexpr.vnode);
    case momty_item:
      return cjit_type_of_scanned_item_mom (cj, vexpr.vitem);
    case momty_set:
    case momty_tuple:
    default:
      CJIT_ERROR_MOM (cj,
                      "type_of_scanned_expr: bad expression %s in statement %s in block %s in function %s",
                      mom_output_gcstring (vexpr),
                      mom_item_cstring (cj->cj_curstmtitm),
                      mom_item_cstring (cj->cj_curblockitm),
                      mom_item_cstring (cj->cj_curfunitm));
    }
}                               /* end of cjit_type_of_scanned_expr_mom */


static momitem_t *
cjit_type_of_scanned_node_mom (struct codejit_mom_st *cj,
                               const momnode_t *nod)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (nod != NULL);
  momvalue_t nodev = mom_nodev (nod);
  unsigned arity = mom_node_arity (nod);
  momitem_t *connitm = mom_node_conn (nod);
  momitem_t *typarg0itm = NULL;
  momitem_t *typarg1itm = NULL;
  switch (mom_item_hash (connitm))
    {
      //////////////// unary operations
    case MOM_CASE_PREDEFINED_NAMED (jit_abs, connitm, otherwiseoplab):
      if (arity == 1            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj,
                                                 mom_node_nth (nod,
                                                               0)))) != NULL)
          && (typarg0itm == MOM_PREDEFINED_NAMED (integer)
              || typarg0itm == MOM_PREDEFINED_NAMED (double)))
          return typarg0itm;
      else
        goto badnode_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_bitnot, connitm, otherwiseoplab):
      if (arity == 1            //
          && ((typarg0itm       //
               = cjit_type_of_scanned_expr_mom (cj,
                                                mom_node_nth (nod,
                                                              0))) != NULL)
          && typarg0itm == MOM_PREDEFINED_NAMED (integer))
        return MOM_PREDEFINED_NAMED (integer);
      else
        goto badnode_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_minus, connitm, otherwiseoplab):
      if (arity == 1            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj,
                                                 mom_node_nth (nod,
                                                               0)))) != NULL)
          && (typarg0itm == MOM_PREDEFINED_NAMED (integer)
              || typarg0itm == MOM_PREDEFINED_NAMED (double)))
          return typarg0itm;
      else
        goto badnode_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_negate, connitm, otherwiseoplab):
      if (arity == 1            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj,
                                                 mom_node_nth (nod,
                                                               0)))) != NULL)
          && (typarg0itm == MOM_PREDEFINED_NAMED (integer)
              || typarg0itm == MOM_PREDEFINED_NAMED (double)
              || mom_code_compatible_types (typarg0itm,
                                            MOM_PREDEFINED_NAMED (item))))
          return typarg0itm;
      else
        goto badnode_lab;
      //////////////// binary operations
    case MOM_CASE_PREDEFINED_NAMED (jit_plus, connitm, otherwiseoplab):
      goto binaryarithmeticop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_sub, connitm, otherwiseoplab):
      goto binaryarithmeticop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_mult, connitm, otherwiseoplab):
      goto binaryarithmeticop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_div, connitm, otherwiseoplab):
      goto binaryarithmeticop_lab;
    binaryarithmeticop_lab:
      if (arity == 2            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 0)))) != NULL)  //
          && (((typarg1itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 1)))) != NULL)  //
          && (typarg0itm == typarg1itm) //
          &&
          ((typarg0itm == MOM_PREDEFINED_NAMED (integer)
            || typarg0itm == MOM_PREDEFINED_NAMED (double))))
          return typarg0itm;
      else
        goto badnode_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_mod, connitm, otherwiseoplab):
      goto binaryintegerop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_bitand, connitm, otherwiseoplab):
      goto binaryintegerop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_bitor, connitm, otherwiseoplab):
      goto binaryintegerop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_bitxor, connitm, otherwiseoplab):
      goto binaryintegerop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_leftshift, connitm, otherwiseoplab):
      goto binaryintegerop_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_rightshift, connitm, otherwiseoplab):
      goto binaryintegerop_lab;
    binaryintegerop_lab:
      if (arity == 2            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 0)))) != NULL)  //
          && (((typarg1itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 1)))) != NULL)  //
          && (typarg0itm == typarg1itm) //
          && (typarg0itm == MOM_PREDEFINED_NAMED (integer)))
        return typarg0itm;
      else
        goto badnode_lab;
      //////////////// binary comparisons
    case MOM_CASE_PREDEFINED_NAMED (jit_eq, connitm, otherwiseoplab):
      goto binarycomparison_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_ne, connitm, otherwiseoplab):
      goto binarycomparison_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_lt, connitm, otherwiseoplab):
      goto binarycomparison_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_le, connitm, otherwiseoplab):
      goto binarycomparison_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_gt, connitm, otherwiseoplab):
      goto binarycomparison_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_ge, connitm, otherwiseoplab):
      goto binarycomparison_lab;
    binarycomparison_lab:
      if (arity == 2            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 0)))) != NULL)  //
          && (((typarg1itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 1)))) != NULL)  //
          && (typarg0itm == typarg1itm))
        return MOM_PREDEFINED_NAMED (integer);
      else
        goto badnode_lab;
      //////////////// logical lazy andthen & orelse operators

    case MOM_CASE_PREDEFINED_NAMED (jit_andthen, connitm, otherwiseoplab):
      goto binarylazy_lab;
    case MOM_CASE_PREDEFINED_NAMED (jit_orelse, connitm, otherwiseoplab):
      goto binarylazy_lab;
    binarylazy_lab:
      if (arity == 2            //
          && (((typarg0itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 0)))) != NULL)  //
          && (((typarg1itm      //
                = cjit_type_of_scanned_expr_mom (cj, mom_node_nth (nod, 1)))) != NULL)  //
          && (typarg0itm == typarg1itm))
        return (typarg0itm);
      else
        goto badnode_lab;
      ////////////////
    otherwiseoplab:
    default:

#warning cjit_type_of_scanned_node_mom unimplemented
      MOM_FATAPRINTF
        ("cjit_type_of_scanned_node_mom %s unimplemented",
         mom_output_gcstring (nodev));
      break;
    }
badnode_lab:
  CJIT_ERROR_MOM (cj,
                  "type of scanned node: invalid nodev %s in statement %s of block %s of function %s",
                  mom_output_gcstring (nodev),
                  mom_item_cstring (cj->cj_curstmtitm),
                  mom_item_cstring (cj->cj_curblockitm),
                  mom_item_cstring (cj->cj_curfunitm));
}                               /* end of cjit_type_of_scanned_node_mom */



static momitem_t *
cjit_type_of_scanned_item_mom (struct codejit_mom_st *cj, momitem_t *itm)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (itm != NULL);
  cjit_lock_item_mom (cj, itm);
  if (itm->itm_kind == MOM_PREDEFINED_NAMED (variable))
    return cjit_type_of_scanned_variable_mom (cj, itm);
#warning cjit_type_of_scanned_item_mom unimplemented
  MOM_FATAPRINTF
    ("cjit_type_of_scanned_item_mom %s unimplemented",
     mom_item_cstring (itm));
}                               /* end of cjit_type_of_scanned_item_mom */

static momitem_t *
cjit_type_of_scanned_variable_mom (struct codejit_mom_st *cj,
                                   momitem_t *varitm)
{
  assert (cj && cj->cj_magic == CODEJIT_MAGIC_MOM);
  assert (varitm != NULL);
  cjit_lock_item_mom (cj, varitm);
  if (varitm->itm_kind != MOM_PREDEFINED_NAMED (variable))
    CJIT_ERROR_MOM (cj,
                    "type of scanned variable: invalid variable %s"
                    " in statement %s in block %s in function %s",
                    mom_item_cstring (varitm),
                    mom_item_cstring (cj->cj_curstmtitm),
                    mom_item_cstring (cj->cj_curblockitm),
                    mom_item_cstring (cj->cj_curfunitm));
  momvalue_t bindv =            //
    mom_attributes_find_value (cj->cj_funbind, varitm);
  const momnode_t *bindnod = NULL;
  if (bindv.typnum == momty_null || !(bindnod = mom_value_to_node (bindv)))
    CJIT_ERROR_MOM (cj,
                    "type of scanned variable: unbound variable %s"
                    " in statement %s in block %s in function %s",
                    mom_item_cstring (varitm),
                    mom_item_cstring (cj->cj_curstmtitm),
                    mom_item_cstring (cj->cj_curblockitm),
                    mom_item_cstring (cj->cj_curfunitm));
  momitem_t *bindconnitm = mom_node_conn (bindnod);
  unsigned bindarity = mom_node_arity (bindnod);
  if (bindconnitm == MOM_PREDEFINED_NAMED (formals) && bindarity == 3)
    {
      return mom_value_to_item (mom_node_nth (bindnod, 1));
    };
  if (bindconnitm == MOM_PREDEFINED_NAMED (results) && bindarity == 3)
    {
      return mom_value_to_item (mom_node_nth (bindnod, 1));
    }
  if (bindconnitm == MOM_PREDEFINED_NAMED (variable) && bindarity == 3)
    {
      return mom_value_to_item (mom_node_nth (bindnod, 1));
    }
  CJIT_ERROR_MOM (cj,
                  "type of scanned variable: variable %s wrongly bound to %s "
                  " in statement %s in block %s in function %s",
                  mom_item_cstring (varitm),
                  mom_output_gcstring (bindv),
                  mom_item_cstring (cj->cj_curstmtitm),
                  mom_item_cstring (cj->cj_curblockitm),
                  mom_item_cstring (cj->cj_curfunitm));
}                               /* end of cjit_type_of_scanned_variable_mom */



static void
cjit_second_emitting_pass_mom (momitem_t *itmcjit)
{
  struct codejit_mom_st *cj = NULL;
  MOM_DEBUGPRINTF (gencod, "cjit_second_emitting_pass start itmcjit=%s",
                   mom_item_cstring (itmcjit));
  if (!itmcjit
      || itmcjit->itm_kind != MOM_PREDEFINED_NAMED (code_generation)
      || !(cj = itmcjit->itm_data1) || cj->cj_magic != CODEJIT_MAGIC_MOM)
    MOM_FATAPRINTF ("cjit_second_emitting_pass: corrupted itmcjit %s",
                    mom_item_cstring (itmcjit));
}                               /* end of cjit_second_emitting_pass_mom */



static void
cjit_third_decorating_pass_mom (momitem_t *itmcjit)
{
  struct codejit_mom_st *cj = NULL;
  MOM_DEBUGPRINTF (gencod, "cjit_third_decorating_pass start itmcjit=%s",
                   mom_item_cstring (itmcjit));
  if (!itmcjit
      || itmcjit->itm_kind != MOM_PREDEFINED_NAMED (code_generation)
      || !(cj = itmcjit->itm_data1) || cj->cj_magic != CODEJIT_MAGIC_MOM)
    MOM_FATAPRINTF ("cjit_third_decorating_pass: corrupted itmcjit %s",
                    mom_item_cstring (itmcjit));
}                               /* end of cjit_third_decorating_pass_mom */
