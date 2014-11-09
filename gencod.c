// file gencod.c

/**   Copyright (C)  2014 Free Software Foundation, Inc.
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

/****
   A module item is an item with a `module_routines` attribute
   associated to a set or tuple of procedure or routine items.

   A procedure may have an attribute `procedure_result` giving its result
   variable, and a `procedure_arguments` giving its formal
   arguments. But a routine has not them.


****/

#define CGEN_MAGIC 0x566802a5	/* cgen magic 1449656997 */

// internal stack allocated structure to generate the C module
struct c_generator_mom_st
{
  unsigned cgen_magic;		// always CGEN_MAGIC
  jmp_buf cgen_jbuf;
  char *cgen_errmsg;
  momitem_t *cgen_moditm;
  momitem_t *cgen_globassocitm;
  momitem_t *cgen_curoutitm;
  momitem_t *cgen_locassocitm;
  FILE *cgen_fil;
  char *cgen_filpath;
  char *cgen_filbase;
  char *cgen_tempath;
  struct momout_st cgen_outhead;
  struct momout_st cgen_outbody;
};




const char *
mom_item_generate_jit_routine (momitem_t *itm, const momval_t jitnode)
{
  MOM_FATAL (MOMOUT_LITERAL
	     ("mom_item_generate_jit_routine unimplemented itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_SPACE (32),
	     MOMOUT_LITERAL ("jitcode="), MOMOUT_VALUE (jitnode), NULL);
#warning mom_item_generate_jit_routine unimplemented
}


static void cgen_error_mom_at (int lin, struct c_generator_mom_st *cgen, ...)
  __attribute__ ((sentinel));

static void
cgen_error_mom_at (int lin, struct c_generator_mom_st *cgen, ...)
{
  va_list args;
  char *outbuf = NULL;
  size_t sizbuf = 0;
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  FILE *fout = open_memstream (&outbuf, &sizbuf);
  if (!fout)
    MOM_FATAPRINTF ("failed to open stream for cgenerror %s:%d", __FILE__,
		    lin);
  struct momout_st mout;
  memset (&mout, 0, sizeof (mout));
  mom_initialize_output (&mout, fout, 0);
  va_start (args, cgen);
  mom_outva_at (__FILE__, lin, &mout, args);
  va_end (args);
  fflush (fout);
  cgen->cgen_errmsg = (char *) MOM_GC_STRDUP ("cgen_error", outbuf);
  MOM_DEBUGPRINTF (gencod, "cgen_error_mom #%d: %s", lin, cgen->cgen_errmsg);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("cgen_error"), MOMOUT_BACKTRACE (20));
  free (outbuf), outbuf = NULL;
  longjmp (cgen->cgen_jbuf, lin);
}

#define CGEN_ERROR_MOM_AT_BIS(Lin,Cgen,...) cgen_error_mom_at(Lin,Cgen,__VA_ARGS__,NULL)
#define CGEN_ERROR_MOM_AT(Lin,Cgen,...) CGEN_ERROR_MOM_AT_BIS(Lin,Cgen,__VA_ARGS__)
#define CGEN_ERROR_MOM(Cgen,...) CGEN_ERROR_MOM_AT(__LINE__,Cgen,__VA_ARGS__)


static void declare_routine_cgen (struct c_generator_mom_st *cgen,
				  unsigned routix);
static void emit_routine_cgen (struct c_generator_mom_st *cgen,
			       unsigned routix);
static void emit_procedure_cgen (struct c_generator_mom_st *cgen,
				 unsigned routix);
static void emit_taskletroutine_cgen (struct c_generator_mom_st *cgen,
				      unsigned routix);

static void emit_ctype_cgen (struct c_generator_mom_st *cgen,
			     struct momout_st *out, momitem_t *typitm);

#define CGEN_CHECK_FRESH_AT_BIS(Lin,Cg,Msg,Itm) do	\
{ const momitem_t* itm##Lin = (Itm);			\
  if (!itm##Lin || itm##Lin->i_typnum != momty_item)	\
    CGEN_ERROR_MOM_AT					\
      (Lin,Cg,						\
       MOMOUT_LITERAL(Msg " (non-item)"),		\
       MOMOUT_VALUE((const momval_t)itm##Lin));		\
  momval_t globv##Lin					\
    = mom_item_assoc_get ((Cg)->cgen_globassocitm,	\
			  itm##Lin);			\
  if (globv##Lin.ptr)					\
    CGEN_ERROR_MOM_AT					\
      (Lin,Cg,						\
       MOMOUT_LITERAL(Msg " (globally known item):"),	\
       MOMOUT_ITEM(itm##Lin),				\
       MOMOUT_LITERAL(" meet as "),			\
       MOMOUT_VALUE((const momval_t) globv##Lin));	\
  momval_t locv##Lin					\
    = mom_item_assoc_get ((Cg)->cgen_locassocitm,	\
			  itm##Lin);			\
  if (locv##Lin.ptr)					\
    CGEN_ERROR_MOM_AT					\
      (Lin,Cg,						\
       MOMOUT_LITERAL(Msg " (locally known item):"),	\
       MOMOUT_ITEM(itm##Lin),				\
       MOMOUT_LITERAL(" meet as "),			\
       MOMOUT_VALUE((const momval_t) locv##Lin));	\
 } while(0)

#define CGEN_CHECK_FRESH_AT(Lin,Cg,Msg,Itm) CGEN_CHECK_FRESH_AT_BIS(Lin,Cg,Msg,Itm)

#define CGEN_CHECK_FRESH(Cg,Msg,Itm) CGEN_CHECK_FRESH_AT(__LINE__,Cg,Msg,Itm)


static pthread_mutex_t cgenmtx_mom = PTHREAD_MUTEX_INITIALIZER;
int
mom_generate_c_module (momitem_t *moditm, const char *dirname, char **perrmsg)
{
  int jr = 0;
  struct c_generator_mom_st mycgen;
  memset (&mycgen, 0, sizeof mycgen);
  momval_t modroutv = MOM_NULLV;
  mycgen.cgen_magic = CGEN_MAGIC;
  mycgen.cgen_moditm = moditm;
  if (!dirname || !dirname[0])
    dirname = ".";
  mom_initialize_buffer_output (&mycgen.cgen_outhead, outf_jsonhalfindent);
  mom_initialize_buffer_output (&mycgen.cgen_outbody, outf_jsonhalfindent);
  pthread_mutex_lock (&cgenmtx_mom);
  jr = setjmp (mycgen.cgen_jbuf);
  if (jr)
    {
      MOM_DEBUGPRINTF (gencod, "generate_c_module got errored #%d: %s", jr,
		       mycgen.cgen_errmsg);
      assert (mycgen.cgen_errmsg != NULL);
      *perrmsg = mycgen.cgen_errmsg;
      if (mycgen.cgen_outhead.mout_magic)
	{
	  assert (mycgen.cgen_outhead.mout_magic == MOM_MOUT_MAGIC);
	  mom_finalize_buffer_output (&mycgen.cgen_outhead);
	}
      if (mycgen.cgen_outbody.mout_magic)
	{
	  assert (mycgen.cgen_outbody.mout_magic == MOM_MOUT_MAGIC);
	  mom_finalize_buffer_output (&mycgen.cgen_outbody);
	}
      pthread_mutex_unlock (&cgenmtx_mom);
      return jr;
    };
  if (!mom_is_item ((momval_t) moditm))
    CGEN_ERROR_MOM (&mycgen, MOMOUT_LITERAL ("non item module"),
		    MOMOUT_VALUE ((const momval_t) moditm), NULL);
  /// get the set of module routines
  {
    mom_should_lock_item (moditm);
    modroutv = mom_item_get_attribute (moditm, mom_named__module_routines);
    mom_unlock_item (moditm);
    if (!mom_is_set (modroutv))
      CGEN_ERROR_MOM (&mycgen, MOMOUT_LITERAL ("generate_c_module module:"),
		      MOMOUT_ITEM ((const momitem_t *) moditm),
		      MOMOUT_SPACE (48),
		      MOMOUT_LITERAL ("has unexpected module_routines:"),
		      MOMOUT_VALUE (modroutv));
  }
  unsigned nbmodrout = mom_set_cardinal (modroutv);
  {
    char basbuf[128];
    snprintf (basbuf, sizeof (basbuf), MOM_SHARED_MODULE_PREFIX "%s.c",
	      mom_ident_cstr_of_item (moditm));
    mycgen.cgen_filbase = (char *) MOM_GC_STRDUP ("base file", basbuf);
  }
  mycgen.cgen_globassocitm = mom_make_item ();
  mom_item_start_assoc (mycgen.cgen_globassocitm);
  /// start the head part
  MOM_OUT (&mycgen.cgen_outhead,
	   MOMOUT_LITERAL ("// generated monimelt module file "),
	   MOMOUT_LITERALV ((const char *) mycgen.cgen_filbase),
	   MOMOUT_LITERAL (" ** DO NOT EDIT **"), MOMOUT_NEWLINE (),
	   MOMOUT_GPLV3P_NOTICE ((const char *) mycgen.cgen_filbase),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("////++++ declaration of "),
	   MOMOUT_DEC_INT ((int) nbmodrout), MOMOUT_LITERAL (" routines:"),
	   MOMOUT_NEWLINE (), NULL);
  /// start the body part
  MOM_OUT (&mycgen.cgen_outbody,
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("////++++ implementation of "),
	   MOMOUT_DEC_INT ((int) nbmodrout), MOMOUT_LITERAL (" routines:"),
	   MOMOUT_NEWLINE (), NULL);

  /// iterate on the set of module routines to declare them
  for (unsigned routix = 0; routix < nbmodrout; routix++)
    {
      momitem_t *curoutitm = mom_set_nth_item (modroutv, routix);
      mycgen.cgen_curoutitm = curoutitm;
      declare_routine_cgen (&mycgen, routix);
      mycgen.cgen_curoutitm = NULL;
    }
  /// iterate on the set of module routines to generate them
  nbmodrout = mom_set_cardinal (modroutv);
  for (unsigned routix = 0; routix < nbmodrout; routix++)
    {
      momitem_t *curoutitm = mom_set_nth_item (modroutv, routix);
      mycgen.cgen_curoutitm = curoutitm;
      emit_routine_cgen (&mycgen, routix);
      mycgen.cgen_curoutitm = NULL;
    }
  /// at last
  pthread_mutex_unlock (&cgenmtx_mom);
#warning incomplete mom_generate_c_module
  MOM_FATAPRINTF ("incomplete mom_generate_c_module");
  return jr;
}

#define PROCROUTSIG_PREFIX_MOM "momprocsig_"
void
declare_routine_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momval_t procargsv = MOM_NULLV;
  momval_t procresv = MOM_NULLV;
  momval_t procrestypev = MOM_NULLV;
  momitem_t *curoutitm = cg->cgen_curoutitm;
  {
    momval_t oldvalroutv =
      mom_item_assoc_get (cg->cgen_globassocitm, curoutitm);
    if (oldvalroutv.ptr)
      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("already meet routine:"),
		      MOMOUT_ITEM ((const momitem_t *) curoutitm),
		      MOMOUT_LITERAL (" rank#"),
		      MOMOUT_DEC_INT ((int) routix),
		      MOMOUT_LITERAL (" as:"),
		      MOMOUT_VALUE ((const momval_t) oldvalroutv), NULL);
  }
  {
    mom_should_lock_item (curoutitm);
    procargsv =
      mom_item_get_attribute (curoutitm, mom_named__procedure_arguments);
    procresv =
      mom_item_get_attribute (curoutitm, mom_named__procedure_result);
    mom_unlock_item (curoutitm);
  }
  if (procargsv.ptr)
    {
      momval_t argsigv = MOM_NULLV;
      // genuine procedure
      MOM_OUT (&cg->cgen_outhead, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// declare procedure "),
	       MOMOUT_ITEM ((const momitem_t *) curoutitm),
	       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) routix),
	       MOMOUT_NEWLINE (), NULL);
      if (!mom_is_tuple (procargsv))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure arguments:"),
			MOMOUT_VALUE ((const momval_t) procargsv),
			MOMOUT_LITERAL (" in procedure "),
			MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
      unsigned nbargs = mom_tuple_length (procargsv);
      if (mom_is_item (procresv))
	{
	  momitem_t *procresitm = procresv.pitem;
	  mom_should_lock_item (procresitm);
	  procrestypev = mom_item_get_attribute (curoutitm, mom_named__ctype);
	  mom_unlock_item (procresitm);
	}
      else if (!procresv.ptr)
	{
	  procrestypev = (momval_t) mom_named__void;
	}
      else
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure result:"),
			MOMOUT_VALUE ((const momval_t) procresv),
			MOMOUT_LITERAL (" in procedure "),
			MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
      emit_ctype_cgen (cg, &cg->cgen_outhead,
		       mom_value_to_item (procrestypev));
      MOM_OUT (&cg->cgen_outhead, MOMOUT_SPACE (48),
	       MOMOUT_LITERAL (MOM_PROCROUTFUN_PREFIX),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	       MOMOUT_LITERAL (" ("), NULL);
      char argsigtab[16] = { 0 };
      char *argsigbuf =
	(nbargs <
	 sizeof (argsigtab) -
	 1) ? argsigtab : (MOM_GC_SCALAR_ALLOC ("argsigbuf", nbargs + 2));
      for (unsigned aix = 0; aix < nbargs; aix++)
	{
	  momitem_t *curargitm = mom_tuple_nth_item (procargsv, aix);
	  momval_t curargtypv = MOM_NULLV;
	  if (!curargitm || curargitm->i_typnum != momty_item)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL ("invalid procedure argument:"),
			    MOMOUT_VALUE ((const momval_t) curargitm),
			    MOMOUT_LITERAL (" in procedure "),
			    MOMOUT_ITEM ((const momitem_t *) curoutitm),
			    MOMOUT_LITERAL (" rank "),
			    MOMOUT_DEC_INT ((int) aix), NULL);
	  mom_should_lock_item (curargitm);
	  curargtypv = mom_item_get_attribute (curargitm, mom_named__ctype);
	  mom_unlock_item (curargitm);
	  if (aix > 0)
	    MOM_OUT (&cg->cgen_outhead, MOMOUT_LITERAL (","),
		     MOMOUT_SPACE (64));
	  emit_ctype_cgen (cg, &cg->cgen_outhead,
			   mom_value_to_item (curargtypv));
	  if (curargtypv.pitem == mom_named__intptr_t)
	    argsigbuf[aix] = momtypenc_int;
	  else if (curargtypv.pitem == mom_named__momval_t)
	    argsigbuf[aix] = momtypenc_val;
	  else if (curargtypv.pitem == mom_named__double)
	    argsigbuf[aix] = momtypenc_double;
	  else if (curargtypv.pitem == mom_named__momcstr_t)
	    argsigbuf[aix] = momtypenc_string;
	  else if (curargtypv.pitem == mom_named__void)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid void argument in procedure:"),
			    MOMOUT_VALUE ((const momval_t) curoutitm),
			    MOMOUT_LITERAL (" rank "),
			    MOMOUT_DEC_INT ((int) aix), NULL);
	  else
	    MOM_FATAL (MOMOUT_LITERAL ("invalid curargtypv:"),
		       MOMOUT_VALUE (curargtypv), NULL);
	};
      argsigv = (momval_t) mom_make_string (argsigbuf);
      if (argsigbuf != argsigtab)
	MOM_GC_FREE (argsigbuf);
      MOM_OUT (&cg->cgen_outhead, MOMOUT_LITERAL (");"), MOMOUT_NEWLINE ());
      MOM_OUT (&cg->cgen_outhead,
	       MOMOUT_LITERAL ("static const char " PROCROUTSIG_PREFIX_MOM),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	       MOMOUT_LITERAL ("[] = \""),
	       MOMOUT_LITERALV (mom_string_cstr (argsigv)),
	       MOMOUT_LITERAL ("\";"), MOMOUT_NEWLINE ());
      mom_item_assoc_put (cg->cgen_globassocitm, curoutitm,
			  (momval_t)
			  mom_make_node_sized (mom_named__procedure, 1,
					       mom_make_integer (routix)));
    }
  else
    {
      // genuine tasklet routine
      MOM_OUT (&cg->cgen_outhead, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// declare tasklet routine "),
	       MOMOUT_ITEM ((const momitem_t *) curoutitm),
	       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) routix),
	       MOMOUT_NEWLINE (), NULL);
      MOM_OUT (&cg->cgen_outhead,
	       MOMOUT_LITERAL ("int " MOM_ROUTINE_NAME_PREFIX),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	       MOMOUT_LITERAL
	       ("(int, momitem_t*, momval_t, momval_t*, intptr_t*, double*);"),
	       MOMOUT_NEWLINE (), NULL);
      mom_item_assoc_put (cg->cgen_globassocitm, curoutitm,
			  (momval_t)
			  mom_make_node_sized (mom_named__tasklet_routine, 1,
					       mom_make_integer (routix)));
    }
}

void
emit_routine_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momitem_t *curoutitm = cg->cgen_curoutitm;
  assert (curoutitm && curoutitm->i_typnum == momty_item);
  momval_t curoutval = mom_item_assoc_get (cg->cgen_globassocitm, curoutitm);
  const momitem_t *curoutconnitm = mom_node_conn (curoutval);
  assert (curoutconnitm && curoutconnitm->i_typnum == momty_item);
  cg->cgen_locassocitm = mom_make_item ();
  mom_item_start_assoc (cg->cgen_locassocitm);
  if (curoutconnitm == mom_named__procedure)
    emit_procedure_cgen (cg, routix);
  else if (curoutconnitm == mom_named__tasklet_routine)
    emit_taskletroutine_cgen (cg, routix);
  else
    MOM_FATAL (MOMOUT_LITERAL ("invalid curoutconnitm:"),
	       MOMOUT_ITEM ((const momitem_t *) curoutconnitm));
  cg->cgen_locassocitm = NULL;
}

#define CGEN_FORMALARG_PREFIX "momparg_"
void
emit_procedure_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  momitem_t *curoutitm = NULL;
  momval_t procargsv = MOM_NULLV;
  momval_t procresv = MOM_NULLV;
  momval_t procrestypev = MOM_NULLV;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  curoutitm = cg->cgen_curoutitm;
  {
    mom_should_lock_item (curoutitm);
    procargsv =
      mom_item_get_attribute (curoutitm, mom_named__procedure_arguments);
    procresv =
      mom_item_get_attribute (curoutitm, mom_named__procedure_result);
    mom_unlock_item (curoutitm);
  }
  if (!mom_is_tuple (procargsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure arguments:"),
		    MOMOUT_VALUE ((const momval_t) procargsv),
		    MOMOUT_LITERAL (" in procedure "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  unsigned nbargs = mom_tuple_length (procargsv);
  momitem_t *procresitm = NULL;
  if (mom_is_item (procresv))
    {
      procresitm = procresv.pitem;
      CGEN_CHECK_FRESH (cg, "result of procedure", procresitm);
      mom_item_assoc_put
	(cg->cgen_locassocitm, procresitm,
	 (momval_t) mom_make_node_sized (mom_named__procedure_result, 1,
					 mom_make_integer (routix)));
      mom_should_lock_item (procresitm);
      procrestypev = mom_item_get_attribute (curoutitm, mom_named__ctype);
      mom_unlock_item (procresitm);
    }
  else if (!procresv.ptr)
    {
      procrestypev = (momval_t) mom_named__void;
    }
  else
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure result:"),
		    MOMOUT_VALUE ((const momval_t) procresv),
		    MOMOUT_LITERAL (" in procedure "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("// implementation of procedure #"),
	   MOMOUT_DEC_INT ((int) routix), MOMOUT_NEWLINE ());
  emit_ctype_cgen (cg, &cg->cgen_outbody, mom_value_to_item (procrestypev));
  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
	   MOMOUT_LITERAL (MOM_PROCROUTFUN_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL (" ("), NULL);
  for (unsigned aix = 0; aix < nbargs; aix++)
    {
      momitem_t *curargitm = mom_tuple_nth_item (procargsv, aix);
      momval_t curargtypv = MOM_NULLV;
      if (!curargitm || curargitm->i_typnum != momty_item)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL ("invalid procedure argument:"),
			MOMOUT_VALUE ((const momval_t) curargitm),
			MOMOUT_LITERAL (" in procedure "),
			MOMOUT_ITEM ((const momitem_t *) curoutitm),
			MOMOUT_LITERAL (" rank "),
			MOMOUT_DEC_INT ((int) aix), NULL);
      CGEN_CHECK_FRESH (cg, "argument of procedure", curargitm);
      mom_should_lock_item (curargitm);
      curargtypv = mom_item_get_attribute (curargitm, mom_named__ctype);
      mom_unlock_item (curargitm);
      mom_item_assoc_put
	(cg->cgen_locassocitm, procresitm,
	 (momval_t) mom_make_node_sized (mom_named__procedure_arguments, 3,
					 mom_make_integer (routix),
					 mom_make_integer (aix), curargtypv));
      if (aix > 0)
	MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","), MOMOUT_SPACE (64));
      emit_ctype_cgen (cg, &cg->cgen_outbody, mom_value_to_item (curargtypv));
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" " CGEN_FORMALARG_PREFIX),
	       MOMOUT_DEC_INT ((int) aix),
	       MOMOUT_LITERAL (" "),
	       MOMOUT_SLASHCOMMENT_STRING (mom_ident_cstr_of_item
					   (curargitm)), NULL);
    }
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("{"), MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE ());
  unsigned nbprocvals = 0;
#warning should count and bind the constant values in proc
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("static momitem_t* momprocitem;"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("static momval_t* momprocvalues;"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id(\""),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL ("\");"), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("if (MOM_UNLIKELY(!momprocvalues)) momprocvalues = mom_item_procedure_values (momprocitem, "),
	   MOMOUT_DEC_INT ((int) nbprocvals), MOMOUT_LITERAL (");"),
	   MOMOUT_NEWLINE (), NULL);
  if (procrestypev.pitem == mom_named__momval_t)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("momval_t momresult = MOM_NULLV;"),
	     MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == mom_named__intptr_t)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("intptr_t momresult = 0;"), MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == mom_named__double)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("double momresult = 0.0;"), MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == mom_named__momcstr_t)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("momcstr_t momresult = NULL;"),
	     MOMOUT_NEWLINE ());
  if (procrestypev.pitem == NULL || procrestypev.pitem == mom_named__void)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("if (MOM_UNLIKELY(!momprocvalues)) return;"),
	     MOMOUT_NEWLINE ());
  else
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL
	     ("if (MOM_UNLIKELY(!momprocvalues)) return momresult;"),
	     MOMOUT_NEWLINE ());
}


void
emit_taskletroutine_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
}



void
emit_ctype_cgen (struct c_generator_mom_st *cg, struct momout_st *out,
		 momitem_t *typitm)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (out && out->mout_magic == MOM_MOUT_MAGIC);
  if (!typitm || typitm->i_typnum != momty_item)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad ctype:"),
		    MOMOUT_VALUE ((const momval_t) typitm), NULL);
  if (typitm == mom_named__intptr_t)
    MOM_OUT (out, MOMOUT_LITERAL ("intptr_t"), NULL);
  else if (typitm == mom_named__momval_t)
    MOM_OUT (out, MOMOUT_LITERAL ("momval_t"), NULL);
  else if (typitm == mom_named__double)
    MOM_OUT (out, MOMOUT_LITERAL ("double"), NULL);
  else if (typitm == mom_named__void)
    MOM_OUT (out, MOMOUT_LITERAL ("void"), NULL);
  else if (typitm == mom_named__momcstr_t)
    MOM_OUT (out, MOMOUT_LITERAL ("momcstr_t"), NULL);
  else
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid ctype:"),
		    MOMOUT_ITEM ((const momitem_t *) typitm), NULL);
}
