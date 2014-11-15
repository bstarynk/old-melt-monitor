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
   associated to a set or tuple of routines, i.e. procedure or
   function items.

   A procedure should have an attribute `procedure` giving its blocks,
   with `start` giving its starting block.

   Procedures usually have an attribute `constant` giving a sequence
   -set or tuple- of constant items, `formals` associated to formal
   arguments, `values` associated to locals, `numbers` associated to
   numbers, `doubles` associated to doubles.

   Tasklet functions have an attribute `constant` giving a sequence -set or
   tuple- of constant items, `formals` associated to formal-arguments,
   `locals` associated to locals variables.

   Both procedures and functions have a tuple or set of `blocks` and a
   `start` block item.

   Expressions are often nodes, the connective being a procedure or a
   primitive.

   A primitive item (useful as connective in expressions) has a
   `primitive_expansion` with a node of connective `chunk` and it has
   a `ctype`

   A block item should preferably be unique to its procedure or
   function. It has a `block` attribute associated to a `code`
   node. That node should have expression sub-nodes, one of:

     *do (<expr>) for side-effecting expressions
     *if (<cond>,<block>) for conditional jumps
     *assign (<var>,<expr>) for assignments
     *switch (<expr>,<case>...); each <case> is *case(<const-expr>,<block>)
     *dispatch (<expr>,<case>...); each <case> is *case(<const-item>,<block>)

  but the last sub-node in a block code could also be:
     *jump(<block>)
     *call(<return-block>,<fun-expr>,<arg-expr>....) in functions only
     *return(<expr>) or `return`

  A block item might have a `lock` attribute giving a variable
  (holding some item) to lock before entering that block, which will
  be unlocked after exiting from that block.
****/

#define CGEN_MAGIC 0x566802a5	/* cgen magic 1449656997 */

enum cgenroutkind_mom_en
{
  cgr__none,
  cgr_proc,			// procedure
  cgr_funt,			// function for tasklet
};

// internal stack allocated structure to generate the C module
struct c_generator_mom_st
{
  unsigned cgen_magic;		// always CGEN_MAGIC
  unsigned cgen_count;
  jmp_buf cgen_jbuf;		// for error
  char *cgen_errmsg;		// the error message
  momitem_t *cgen_moditm;	// the module item
  momitem_t *cgen_globassocitm;	// global association item
  momitem_t *cgen_curoutitm;	// current routine
  momitem_t *cgen_locassocitm;	// local association item
  /// vectors for functions
  momitem_t *cgen_vecvalitm;	// vector of local value variables
  momitem_t *cgen_vecnumitm;	// vector of local number variables
  momitem_t *cgen_vecdblitm;	// vector of local double variables
  FILE *cgen_fil;
  char *cgen_filpath;
  char *cgen_filbase;
  char *cgen_tempath;
  enum cgenroutkind_mom_en cgen_routkind;
  momtypenc_t cgen_restype;
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
static void emit_taskletfunction_cgen (struct c_generator_mom_st *cgen,
				       unsigned routix);

static void emit_goto_block_cgen (struct c_generator_mom_st *cg,
				  momitem_t *blkitm, int lockix);

// gives the type encoding, and usually emit the ctype of a
// value. Don't emit anything if OUT is null.
static momtypenc_t emit_ctype_cgen (struct c_generator_mom_st *cgen,
				    struct momout_st *out, momval_t val);

static void emit_block_cgen (struct c_generator_mom_st *cgen,
			     momitem_t *blkitm);

static momtypenc_t emit_expr_cgen (struct c_generator_mom_st *cg,
				   momval_t expv);

static momtypenc_t emit_node_cgen (struct c_generator_mom_st *cg,
				   momval_t nodv);

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
  char *mydirname = (char *) dirname;
  mom_initialize_buffer_output (&mycgen.cgen_outhead, outf_jsonhalfindent);
  mom_initialize_buffer_output (&mycgen.cgen_outbody, outf_jsonhalfindent);
  pthread_mutex_lock (&cgenmtx_mom);
  jr = setjmp (mycgen.cgen_jbuf);
  if (jr)
    {
      MOM_DEBUGPRINTF (gencod, "generate_c_module got errored #%d: %s", jr,
		       mycgen.cgen_errmsg);
      assert (mycgen.cgen_errmsg != NULL);
      MOM_WARNING (MOMOUT_LITERAL ("generate_c_module on module:"),
		   MOMOUT_ITEM ((const momitem_t *) moditm),
		   MOMOUT_LITERAL (" failed with error "),
		   MOMOUT_DEC_INT ((int) jr),
		   MOMOUT_LITERAL (" : "),
		   MOMOUT_LITERALV ((const char *) mycgen.cgen_errmsg), NULL);
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
  /// check that the modules/ subdirectory exist, or create it
  {
    DIR *mdir = NULL;
    char dirbuf[MOM_PATH_MAX];
    memset (dirbuf, 0, sizeof (dirbuf));
    if (snprintf (dirbuf, sizeof (dirbuf),
		  "%s/" MOM_SHARED_MODULE_DIRECTORY,
		  mydirname) >= MOM_PATH_MAX - 1)
      CGEN_ERROR_MOM (&mycgen, MOMOUT_LITERAL ("too long modules dir:"),
		      MOMOUT_LITERALV ((const char *) dirbuf));
    if (!(mdir = opendir (dirbuf)))
      {
	if (mkdir (dirbuf, 0700))
	  MOM_FATAPRINTF ("failed to mkdir %s", dirbuf);
      }
    else
      closedir (mdir);
  }
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
  // now we should write the file
  {
    FILE *f = NULL;
    char mtempnam[MOM_PATH_MAX];
    char modnam[MOM_PATH_MAX];
    memset (mtempnam, 0, sizeof (mtempnam));
    memset (modnam, 0, sizeof (mtempnam));
    if (snprintf (modnam, sizeof (modnam),
		  "%s/" MOM_SHARED_MODULE_DIRECTORY "/"
		  MOM_SHARED_MODULE_PREFIX "%s.c",
		  mydirname,
		  mom_ident_cstr_of_item (moditm)) >= MOM_PATH_MAX - 1)
      CGEN_ERROR_MOM (&mycgen,
		      MOMOUT_LITERAL ("failed to name module source file:"),
		      MOMOUT_LITERALV ((const char *) modnam));
    if (snprintf
	(mtempnam, sizeof (mtempnam),
	 "%s/" MOM_SHARED_MODULE_DIRECTORY "/" MOM_SHARED_MODULE_PREFIX
	 "%s.p%d-r%x.tmp", mydirname, mom_ident_cstr_of_item (moditm),
	 (int) getpid (), (unsigned) mom_random_32 ()) >= MOM_PATH_MAX - 1)
      CGEN_ERROR_MOM (&mycgen,
		      MOMOUT_LITERAL ("failed to name temporary file:"),
		      MOMOUT_LITERALV ((const char *) modnam));
    f = fopen (mtempnam, "w");
    if (!f)
      MOM_FATAPRINTF ("failed to open module temporary file %s", mtempnam);
    fprintf (f,
	     "// generated MONIMELT module file " MOM_SHARED_MODULE_PREFIX
	     "%s.c\n", mom_ident_cstr_of_item (moditm));
    fprintf (f, "// DO NOT EDIT\n\n");
    fprintf (f, "\n///// declarations\n");
    fflush (mycgen.cgen_outhead.mout_file);
    fputs (mycgen.cgen_outhead.mout_data, f);
    fprintf (f, "\n\n\n//// implementations\n");
    fflush (mycgen.cgen_outbody.mout_file);
    fputs (mycgen.cgen_outbody.mout_data, f);
    fprintf (f,
	     "\n\n\n//// end of generated file " MOM_SHARED_MODULE_PREFIX
	     "%s.c\n", mom_ident_cstr_of_item (moditm));
    if (fclose (f))
      MOM_FATAPRINTF ("failed to close module temporary file %s", mtempnam);
    mom_rename_if_content_changed (mtempnam, modnam);
    mom_finalize_buffer_output (&mycgen.cgen_outhead);
    mom_finalize_buffer_output (&mycgen.cgen_outbody);
    MOM_INFORMPRINTF ("generated module file %s", modnam);
  }
  /// at last
  pthread_mutex_unlock (&cgenmtx_mom);
  return jr;
}




#define PROCROUTSIG_PREFIX_MOM "momprocsig_"
void
declare_routine_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momval_t procargsv = MOM_NULLV;
  momval_t procv = MOM_NULLV;
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
    procv = mom_item_get_attribute (curoutitm, mom_named__procedure);
    procargsv = mom_item_get_attribute (curoutitm, mom_named__formals);
    procresv = mom_item_get_attribute (curoutitm, mom_named__result);
    mom_unlock_item (curoutitm);
  }
  if (procv.ptr)
    {
      momval_t argsigv = MOM_NULLV;
      // genuine procedure
      MOM_OUT (&cg->cgen_outhead, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// declare procedure "),
	       MOMOUT_ITEM ((const momitem_t *) curoutitm),
	       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) routix),
	       MOMOUT_NEWLINE (), NULL);
      if (!mom_is_tuple (procargsv))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure formals:"),
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
      emit_ctype_cgen (cg, &cg->cgen_outhead, procrestypev);
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
	  emit_ctype_cgen (cg, &cg->cgen_outhead, curargtypv);
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
      mom_item_assoc_put (cg->cgen_globassocitm, curoutitm,
			  (momval_t)
			  mom_make_node_sized (mom_named__procedure, 3,
					       mom_make_integer (routix),
					       argsigv, procrestypev));
    }
  else
    {
      // genuine tasklet function
      MOM_OUT (&cg->cgen_outhead, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// declare tasklet function "),
	       MOMOUT_ITEM ((const momitem_t *) curoutitm),
	       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) routix),
	       MOMOUT_NEWLINE (), NULL);
      MOM_OUT (&cg->cgen_outhead,
	       MOMOUT_LITERAL ("static int " MOM_ROUTINE_NAME_PREFIX),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	       MOMOUT_LITERAL
	       ("(int, momitem_t*, momval_t, momval_t*, intptr_t*, double*);"),
	       MOMOUT_NEWLINE (), NULL);
      mom_item_assoc_put (cg->cgen_globassocitm, curoutitm,
			  (momval_t)
			  mom_make_node_sized (mom_named__tasklet_function, 1,
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
    {
      cg->cgen_routkind = cgr_proc;
      emit_procedure_cgen (cg, routix);
    }
  else if (curoutconnitm == mom_named__tasklet_function)
    {
      cg->cgen_routkind = cgr_funt;
      emit_taskletfunction_cgen (cg, routix);
    }
  else
    MOM_FATAL (MOMOUT_LITERAL ("invalid curoutconnitm:"),
	       MOMOUT_ITEM ((const momitem_t *) curoutconnitm));
  cg->cgen_locassocitm = NULL;
  cg->cgen_routkind = cgr__none;
}


static unsigned
bind_constants_cgen (struct c_generator_mom_st *cg, momval_t constantsv)
{
  unsigned nbconsts = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (constantsv))
    {
      nbconsts = mom_seqitem_length (constantsv);
      for (unsigned cix = 0; cix < nbconsts; cix++)
	{
	  momitem_t *constitm = mom_seqitem_nth_item (constantsv, cix);
	  if (!constitm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "constant in routine", constitm);
	  mom_item_assoc_put
	    (cg->cgen_locassocitm, constitm,
	     (momval_t) mom_make_node_sized (mom_named__constant, 2,
					     mom_make_integer (cix),
					     (momval_t) constitm));
	}
    }
  else if (constantsv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid constants:"),
		    MOMOUT_VALUE ((const momval_t) constantsv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->cgen_curoutitm),
		    NULL);
  return nbconsts;
}

static unsigned
bind_values_cgen (struct c_generator_mom_st *cg, momval_t valuesv)
{
  unsigned nbvalues = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (valuesv))
    {
      nbvalues = mom_seqitem_length (valuesv);
      for (unsigned vix = 0; vix < nbvalues; vix++)
	{
	  momitem_t *valitm = mom_seqitem_nth_item (valuesv, vix);
	  if (!valitm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "value in routine", valitm);
	  mom_item_assoc_put
	    (cg->cgen_locassocitm, valitm,
	     (momval_t) mom_make_node_sized (mom_named__values, 2,
					     mom_make_integer (vix), valitm));
	}
    }
  else if (valuesv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid values:"),
		    MOMOUT_VALUE ((const momval_t) valuesv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->cgen_curoutitm),
		    NULL);
  return nbvalues;
}

static unsigned
bind_doubles_cgen (struct c_generator_mom_st *cg, momval_t doublesv)
{
  unsigned nbdoubles = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (doublesv))
    {
      nbdoubles = mom_seqitem_length (doublesv);
      for (unsigned dix = 0; dix < nbdoubles; dix++)
	{
	  momitem_t *dblitm = mom_seqitem_nth_item (doublesv, dix);
	  if (!dblitm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "double in routine", dblitm);
	  mom_item_assoc_put
	    (cg->cgen_locassocitm, dblitm,
	     (momval_t) mom_make_node_sized (mom_named__doubles, 2,
					     mom_make_integer (dix),
					     (momval_t) dblitm));
	}
    }
  else if (doublesv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid doubles:"),
		    MOMOUT_VALUE ((const momval_t) doublesv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->cgen_curoutitm),
		    NULL);
  return nbdoubles;
}

static unsigned
bind_numbers_cgen (struct c_generator_mom_st *cg, momval_t numbersv)
{
  unsigned nbnumbers = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (numbersv))
    {
      nbnumbers = mom_seqitem_length (numbersv);
      for (unsigned nix = 0; nix < nbnumbers; nix++)
	{
	  momitem_t *numitm = mom_seqitem_nth_item (numbersv, nix);
	  if (!numitm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "number in routine", numitm);
	  mom_item_assoc_put
	    (cg->cgen_locassocitm, numitm,
	     (momval_t) mom_make_node_sized (mom_named__numbers, 2,
					     mom_make_integer (nix), numitm));
	}
    }
  else if (numbersv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid numbers:"),
		    MOMOUT_VALUE ((const momval_t) numbersv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->cgen_curoutitm),
		    NULL);
  return nbnumbers;
}

static unsigned
bind_blocks_cgen (struct c_generator_mom_st *cg, momval_t blocksv)
{
  unsigned nbblocks = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (blocksv))
    {
      nbblocks = mom_seqitem_length (blocksv);
      for (unsigned bix = 0; bix < nbblocks; bix++)
	{
	  momitem_t *blkitm = mom_seqitem_nth_item (blocksv, bix);
	  if (!blkitm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "block in routine", blkitm);
	  mom_item_assoc_put
	    (cg->cgen_locassocitm, blkitm,
	     (momval_t) mom_make_node_sized (mom_named__block, 1,
					     mom_make_integer (bix + 1)));
	}
    }
  else if (blocksv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid blocks:"),
		    MOMOUT_VALUE ((const momval_t) blocksv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->cgen_curoutitm),
		    NULL);
  return nbblocks;
}

static unsigned
bind_functionvars_cgen (struct c_generator_mom_st *cg, momval_t varsv)
{
  unsigned nbvars = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (varsv))
    {
      nbvars = mom_seqitem_length (varsv);
      for (unsigned vix = 0; vix < nbvars; vix++)
	{
	  momitem_t *varitm = mom_seqitem_nth_item (varsv, vix);
	  momval_t vctypv = MOM_NULLV;
	  if (!varitm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "variable in function", varitm);
	  mom_should_lock_item (varitm);
	  vctypv = mom_item_get_attribute (varitm, mom_named__ctype);
	  mom_unlock_item (varitm);
	  if (vctypv.pitem == mom_named__momval_t)
	    {
	      unsigned valcnt = mom_item_vector_count (cg->cgen_vecvalitm);
	      mom_item_vector_append1 (cg->cgen_vecvalitm, (momval_t) varitm);
	      mom_item_assoc_put
		(cg->cgen_locassocitm, varitm,
		 (momval_t) mom_make_node_sized (mom_named__values, 2,
						 mom_make_integer (valcnt),
						 (momval_t) varitm));

	    }
	  else if (vctypv.pitem == mom_named__intptr_t)
	    {
	      unsigned numcnt = mom_item_vector_count (cg->cgen_vecnumitm);
	      mom_item_vector_append1 (cg->cgen_vecnumitm, (momval_t) varitm);
	      mom_item_assoc_put
		(cg->cgen_locassocitm, varitm,
		 (momval_t) mom_make_node_sized (mom_named__numbers, 2,
						 mom_make_integer (numcnt),
						 (momval_t) varitm));
	    }
	  else if (vctypv.pitem == mom_named__double)
	    {
	      unsigned dblcnt = mom_item_vector_count (cg->cgen_vecdblitm);
	      mom_item_vector_append1 (cg->cgen_vecdblitm, (momval_t) varitm);
	      mom_item_assoc_put
		(cg->cgen_locassocitm, varitm,
		 (momval_t) mom_make_node_sized (mom_named__doubles, 2,
						 mom_make_integer (dblcnt),
						 (momval_t) varitm));
	    }
	  else
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL ("bad variable:"),
			    MOMOUT_VALUE ((const momval_t) varitm),
			    MOMOUT_LITERAL (" in function "),
			    MOMOUT_ITEM ((const momitem_t *)
					 cg->cgen_curoutitm), NULL);
	}
    }
  else if (varsv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid vars:"),
		    MOMOUT_VALUE ((const momval_t) varsv),
		    MOMOUT_LITERAL (" in function "),
		    MOMOUT_ITEM ((const momitem_t *) cg->cgen_curoutitm),
		    NULL);
  return nbvars;
}

#define CGEN_FORMALARG_PREFIX "momparg_"
#define CGEN_PROC_NUMBER_PREFIX "mompnum_"
#define CGEN_PROC_VALUE_PREFIX "mompval_"
#define CGEN_PROC_DOUBLE_PREFIX "mompdbl_"
#define CGEN_PROC_BLOCK_PREFIX "mompblo_"

#define CGEN_FUN_BLOCK_PREFIX "momfblo_"
void
emit_procedure_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  momitem_t *curoutitm = NULL;
  momval_t procargsv = MOM_NULLV;
  momval_t procnodev = MOM_NULLV;
  momval_t procresv = MOM_NULLV;
  momval_t procrestypev = MOM_NULLV;
  momval_t proconstantsv = MOM_NULLV;
  momval_t provaluesv = MOM_NULLV;
  momval_t pronumbersv = MOM_NULLV;
  momval_t prodoublesv = MOM_NULLV;
  momval_t problocksv = MOM_NULLV;
  momval_t prostartv = MOM_NULLV;
  unsigned nbproconsts = 0;
  unsigned nbprovalues = 0;
  unsigned nbpronumbers = 0;
  unsigned nbprodoubles = 0;
  unsigned nbproblocks = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  curoutitm = cg->cgen_curoutitm;
  procnodev = mom_item_assoc_get (cg->cgen_globassocitm, curoutitm);
  assert (mom_node_conn (procnodev) == mom_named__procedure);
  {
    mom_should_lock_item (curoutitm);
    procargsv = mom_item_get_attribute (curoutitm, mom_named__formals);
    procresv = mom_item_get_attribute (curoutitm, mom_named__result);
    proconstantsv = mom_item_get_attribute (curoutitm, mom_named__constant);
    provaluesv = mom_item_get_attribute (curoutitm, mom_named__values);
    pronumbersv = mom_item_get_attribute (curoutitm, mom_named__numbers);
    prodoublesv = mom_item_get_attribute (curoutitm, mom_named__doubles);
    problocksv = mom_item_get_attribute (curoutitm, mom_named__blocks);
    prostartv = mom_item_get_attribute (curoutitm, mom_named__start);
    mom_unlock_item (curoutitm);
  }
  cg->cgen_vecvalitm = mom_make_item ();
  mom_item_start_vector (cg->cgen_vecvalitm);
  cg->cgen_vecnumitm = mom_make_item ();
  mom_item_start_vector (cg->cgen_vecnumitm);
  cg->cgen_vecdblitm = mom_make_item ();
  mom_item_start_vector (cg->cgen_vecdblitm);
  if (!mom_is_tuple (procargsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure formals:"),
		    MOMOUT_VALUE ((const momval_t) procargsv),
		    MOMOUT_LITERAL (" in procedure "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  unsigned nbargs = mom_tuple_length (procargsv);
  momitem_t *procresitm = NULL;
  if (procresv.pitem == mom_named__void)
    procrestypev = (momval_t) mom_named__void;
  if (mom_is_item (procresv))
    {
      procresitm = procresv.pitem;
      CGEN_CHECK_FRESH (cg, "result of procedure", procresitm);
      mom_item_assoc_put
	(cg->cgen_locassocitm, procresitm,
	 (momval_t) mom_make_node_sized (mom_named__result, 1,
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
  cg->cgen_restype = emit_ctype_cgen (cg, &cg->cgen_outbody, procrestypev);
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
	(cg->cgen_locassocitm, curargitm,
	 (momval_t) mom_make_node_sized (mom_named__formals, 3,
					 mom_make_integer (routix),
					 mom_make_integer (aix), curargtypv));
      if (aix > 0)
	MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","), MOMOUT_SPACE (64));
      emit_ctype_cgen (cg, &cg->cgen_outbody, curargtypv);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" " CGEN_FORMALARG_PREFIX),
	       MOMOUT_DEC_INT ((int) aix),
	       MOMOUT_LITERAL (" "),
	       MOMOUT_SLASHCOMMENT_STRING (mom_ident_cstr_of_item
					   (curargitm)), NULL);
    }
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("{"), MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE ());
  // count and bind the constants
  nbproconsts = bind_constants_cgen (cg, proconstantsv);
  // count and bind the numbers
  nbpronumbers = bind_numbers_cgen (cg, pronumbersv);
  // count and bind the values
  nbprovalues = bind_values_cgen (cg, provaluesv);
  // count and bind the doubles
  nbprodoubles = bind_doubles_cgen (cg, prodoublesv);
  // count and bind the blocks
  nbproblocks = bind_blocks_cgen (cg, problocksv);
  if (nbproblocks == 0)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("missing blocks in procedure "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  // emit declarations of local numbers
  for (unsigned nix = 0; nix < nbpronumbers; nix++)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("intptr_t " CGEN_PROC_NUMBER_PREFIX),
	     MOMOUT_DEC_INT ((int) nix),
	     MOMOUT_LITERAL (" = 0; "),
	     MOMOUT_SLASHCOMMENT_STRING
	     (mom_string_cstr ((momval_t) mom_item_get_name_or_idstr
			       (mom_seqitem_nth_item (pronumbersv, nix)))),
	     MOMOUT_NEWLINE ());
  // emit declarations of local values
  for (unsigned vix = 0; vix < nbprovalues; vix++)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("momval_t " CGEN_PROC_VALUE_PREFIX),
	     MOMOUT_DEC_INT ((int) vix),
	     MOMOUT_LITERAL (" = MOM_NULLV; "),
	     MOMOUT_SLASHCOMMENT_STRING
	     (mom_string_cstr ((momval_t) mom_item_get_name_or_idstr
			       (mom_seqitem_nth_item (provaluesv, vix)))),
	     MOMOUT_NEWLINE ());
  // emit declarations of local doubles
  for (unsigned dix = 0; dix < nbprodoubles; dix++)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("double " CGEN_PROC_DOUBLE_PREFIX),
	     MOMOUT_DEC_INT ((int) dix),
	     MOMOUT_LITERAL (" = 0.0; "),
	     MOMOUT_SLASHCOMMENT_STRING (mom_string_cstr
					 ((momval_t)
					  mom_item_get_name_or_idstr
					  (mom_seqitem_nth_item
					   (prodoublesv, dix)))),
	     MOMOUT_NEWLINE ());
  // emit prologue
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("static momitem_t* momprocitem;"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("static momval_t* momprocconstants;"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id(\""),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL ("\");"), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("if (MOM_UNLIKELY(!momprocconstants)) momprocconstants = mom_item_procedure_values (momprocitem, "),
	   MOMOUT_DEC_INT ((int) nbproconsts), MOMOUT_LITERAL (");"),
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
	     MOMOUT_LITERAL ("if (MOM_UNLIKELY(!momprocconstants)) return;"),
	     MOMOUT_NEWLINE ());
  else
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL
	     ("if (MOM_UNLIKELY(!momprocconstants)) return momresult;"),
	     MOMOUT_NEWLINE ());
  momitem_t *startitm = mom_value_to_item (prostartv);
  if (!startitm)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("missing start in procedure "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  momval_t startlocv = mom_item_assoc_get (cg->cgen_locassocitm, startitm);
  int startix = -1;
  if (mom_node_conn (startlocv) == mom_named__block)
    startix = mom_integer_val_def (mom_node_nth (startlocv, 0), -2);
  if (startix <= 0 || startix >= (int) nbproblocks + 1)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid start "),
		    MOMOUT_ITEM ((const momitem_t *) startitm),
		    MOMOUT_LITERAL (" bound to "),
		    MOMOUT_VALUE (startlocv),
		    MOMOUT_LITERAL (" in procedure "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("/// starting:"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("goto " CGEN_PROC_BLOCK_PREFIX),
	   MOMOUT_DEC_INT (startix),
	   MOMOUT_LITERAL ("; // start at "),
	   MOMOUT_LITERALV (mom_string_cstr
			    ((momval_t)
			     mom_item_get_name_or_idstr (startitm))),
	   MOMOUT_NEWLINE (), NULL);
  /// emit each block
  for (unsigned bix = 0; bix < nbproblocks; bix++)
    {
      momitem_t *blkitm = mom_seqitem_nth_item (problocksv, bix);
      if (!blkitm)
	continue;
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" " CGEN_PROC_BLOCK_PREFIX),
	       MOMOUT_DEC_INT ((int) bix + 1),
	       MOMOUT_LITERAL (":"),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("{ // procedure block "),
	       MOMOUT_LITERALV (mom_string_cstr
				((momval_t)
				 mom_item_get_name_or_idstr (blkitm))),
	       MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (), NULL);
      emit_block_cgen (cg, blkitm);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_INDENT_LESS (),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("}; // end procedure block "),
	       MOMOUT_LITERALV (mom_string_cstr
				((momval_t)
				 mom_item_get_name_or_idstr (blkitm))),
	       MOMOUT_NEWLINE (), NULL);
      if (procrestypev.pitem == mom_named__void || !procrestypev.ptr)
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("return;"), MOMOUT_NEWLINE (), NULL);
      else
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("return momresult;"),
		 MOMOUT_NEWLINE (), NULL);
    }
  /// emit epilogue
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("} // end of procedure "),
	   MOMOUT_LITERAL (MOM_PROCROUTFUN_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  /// emit the procedure descriptor
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("const struct momprocrout_st "
			   MOM_PROCROUTDESCR_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL (" = { .prout_magic = MOM_PROCROUT_MAGIC,"),
	   MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (), NULL);
  if (procrestypev.pitem == mom_named__momval_t)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_resty = momtypenc_val,"),
	     MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == mom_named__intptr_t)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_resty = momtypenc_int,"),
	     MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == mom_named__double)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_resty = momtypenc_double,"),
	     MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == mom_named__momcstr_t)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_resty = momtypenc_string,"),
	     MOMOUT_NEWLINE ());
  else if (procrestypev.pitem == NULL
	   || procrestypev.pitem == mom_named__void)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_resty = momtypenc__none,"),
	     MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_len = "),
	   MOMOUT_DEC_INT ((int) nbproconsts), MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_id = \""),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_module = \""),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (cg->cgen_moditm)),
	   MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL (".prout_addr = (void*)" MOM_PROCROUTFUN_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_argsig = \""),
	   MOMOUT_LITERALV (mom_string_cstr (mom_node_nth (procnodev, 1))),
	   MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL (".prout_timestamp= __DATE__ \"@\" __TIME__"),
	   MOMOUT_INDENT_LESS (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("}; // end proc descriptor"), MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE (), NULL);
}				/* end emit_procedure_cgen */


void
emit_taskletfunction_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  unsigned nbconstants = 0;
  unsigned nbblocks = 0;
  momval_t funconstantsv = MOM_NULLV;
  momval_t funblocksv = MOM_NULLV;
  momval_t funargsv = MOM_NULLV;
  momval_t funlocalsv = MOM_NULLV;
  momval_t funstartv = MOM_NULLV;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momitem_t *curoutitm = cg->cgen_curoutitm;
  momval_t routnodev = mom_item_assoc_get (cg->cgen_globassocitm, curoutitm);
  assert (mom_node_conn (routnodev) == mom_named__tasklet_function);
  {
    mom_should_lock_item (curoutitm);
    funconstantsv = mom_item_get_attribute (curoutitm, mom_named__constant);
    funargsv = mom_item_get_attribute (curoutitm, mom_named__formals);
    funlocalsv = mom_item_get_attribute (curoutitm, mom_named__locals);
    funblocksv = mom_item_get_attribute (curoutitm, mom_named__blocks);
    funstartv = mom_item_get_attribute (curoutitm, mom_named__start);
    mom_unlock_item (curoutitm);
  }
  // bind the constants for the closure
  nbconstants = bind_constants_cgen (cg, funconstantsv);
  // bind the arguments
  bind_functionvars_cgen (cg, funargsv);
  // bind the locals
  bind_functionvars_cgen (cg, funlocalsv);
  // bind the blocks
  nbblocks = bind_blocks_cgen (cg, funblocksv);
  momitem_t *startitm = mom_value_to_item (funstartv);
  if (!startitm)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("missing start in function "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  momval_t startlocv = mom_item_assoc_get (cg->cgen_locassocitm, startitm);
  int startix = -1;
  if (mom_node_conn (startlocv) == mom_named__block)
    startix = mom_integer_val_def (mom_node_nth (startlocv, 0), -2);
  if (startix <= 0 || startix >= (int) nbblocks + 1)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid start "),
		    MOMOUT_ITEM ((const momitem_t *) startitm),
		    MOMOUT_LITERAL (" bound to "),
		    MOMOUT_VALUE (startlocv),
		    MOMOUT_LITERAL (" in function "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
  /// emit start of function
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("// implement tasklet function "),
	   MOMOUT_ITEM ((const momitem_t *) curoutitm),
	   MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) routix),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("static int " MOM_ROUTINE_NAME_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_LITERAL
	   ("(int momstate, momitem_t* restrict momtasklet, const momval_t momclosure,"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("\t momval_t* restrict momvals, intptr_t* restrict momnums, double* restrict momdbls)"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("{ // start of tasklet function"),
	   MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (), NULL);
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("if (MOM_UNLIKELY(momstate==0)) return "),
	   MOMOUT_DEC_INT (startix), MOMOUT_LITERAL (";"), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("momval_t* momclovals = mom_closed_values (momclosure);"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("switch (momstate) {"),
	   MOMOUT_NEWLINE ());
  for (unsigned six = 1; six <= nbblocks; six++)
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL ("case "),
	       MOMOUT_DEC_INT ((int) six),
	       MOMOUT_LITERAL (": goto " CGEN_FUN_BLOCK_PREFIX),
	       MOMOUT_DEC_INT ((int) six),
	       MOMOUT_LITERAL (";"), MOMOUT_NEWLINE (), NULL);
    }
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("default: return momroutres_pop;"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("}; // end switch state"),
	   MOMOUT_NEWLINE (), NULL);
  for (unsigned six = 1; six <= nbblocks; six++)
    {
      momitem_t *blkitm = mom_seqitem_nth_item (funblocksv, six - 1);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// function block #"),
	       MOMOUT_DEC_INT ((int) six),
	       MOMOUT_LITERAL (" "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (CGEN_FUN_BLOCK_PREFIX),
	       MOMOUT_DEC_INT ((int) six),
	       MOMOUT_LITERAL (":"),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("{"), MOMOUT_INDENT_MORE (), NULL);
      emit_block_cgen (cg, blkitm);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_INDENT_LESS (),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("}; // end function block "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("return momroutres_pop;"),
	       MOMOUT_NEWLINE (), NULL);
    }
  // emit end of function
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("} // end function "),
	   MOMOUT_ITEM ((const momitem_t *) curoutitm),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  // emit the function descriptor
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("const struct momroutinedescr_st "
			   MOM_ROUTINE_NAME_PREFIX),
	   MOMOUT_ITEM ((const momitem_t *) curoutitm),
	   MOMOUT_LITERAL (" = { // function descriptor"),
	   MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_magic = MOM_ROUTINE_MAGIC,"),
	   MOMOUT_NEWLINE (), MOMOUT_LITERAL (".rout_minclosize = "),
	   MOMOUT_DEC_INT ((int) nbconstants), MOMOUT_LITERAL (","),
	   MOMOUT_NEWLINE (), MOMOUT_LITERAL (".rout_frame_nbval = "),
	   MOMOUT_DEC_INT ((int) mom_item_vector_count (cg->cgen_vecvalitm)),
	   MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_frame_nbnum = "),
	   MOMOUT_DEC_INT ((int) mom_item_vector_count (cg->cgen_vecnumitm)),
	   MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_frame_nbdbl = "),
	   MOMOUT_DEC_INT ((int) mom_item_vector_count (cg->cgen_vecdblitm)),
	   MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_name = \""),
	   MOMOUT_ITEM ((const momitem_t *) curoutitm),
	   MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_module = MONIMELT_CURRENT_MODULE,"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_codefun = " MOM_ROUTINE_NAME_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (".rout_timestamp = __DATE__ \"@\" __TIME__"),
	   MOMOUT_NEWLINE (), MOMOUT_INDENT_LESS (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("}; // end function descriptor"),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
}				/* end emit_taskletfunction_cgen */



momtypenc_t
emit_ctype_cgen (struct c_generator_mom_st *cg, struct momout_st *out,
		 momval_t val)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (out && out->mout_magic == MOM_MOUT_MAGIC);
  momitem_t *typitm = NULL;
  if (mom_is_item (val))
    typitm = val.pitem;
  else if (mom_is_node (val))
    typitm = (momitem_t *) mom_node_conn (val);
  else if (mom_is_integer (val))
    typitm = mom_named__intptr_t;
  else if (mom_is_double (val))
    typitm = mom_named__double;
  else if (mom_is_string (val))
    typitm = mom_named__momcstr_t;
  for (int count = 10; count > 0; count--)
    {
      if (!typitm || typitm->i_typnum != momty_item)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad ctype:"),
			MOMOUT_ITEM ((const momitem_t *) typitm),
			MOMOUT_LITERAL (" for value:"),
			MOMOUT_VALUE ((const momval_t) val), NULL);
      if (typitm == mom_named__intptr_t)
	{
	  if (out)
	    MOM_OUT (out, MOMOUT_LITERAL ("intptr_t"), NULL);
	  return momtypenc_int;
	}
      else if (typitm == mom_named__momval_t)
	{
	  if (out)
	    MOM_OUT (out, MOMOUT_LITERAL ("momval_t"), NULL);
	  return momtypenc_val;
	}
      else if (typitm == mom_named__double)
	{
	  if (out)
	    MOM_OUT (out, MOMOUT_LITERAL ("double"), NULL);
	  return momtypenc_double;
	}
      else if (typitm == mom_named__void)
	{
	  if (out)
	    MOM_OUT (out, MOMOUT_LITERAL ("void"), NULL);
	  return momtypenc__none;
	}
      else if (typitm == mom_named__momcstr_t)
	{
	  if (out)
	    MOM_OUT (out, MOMOUT_LITERAL ("momcstr_t"), NULL);
	  return momtypenc_string;
	}
      else if (typitm)
	{
	  momval_t ctypv = MOM_NULLV;
	  momval_t resv = MOM_NULLV;
	  {
	    mom_lock_item (typitm);
	    ctypv = mom_item_get_attribute (typitm, mom_named__ctype);
	    resv = mom_item_get_attribute (typitm, mom_named__result);
	    mom_unlock_item (typitm);
	    if (mom_is_item (ctypv))
	      typitm = ctypv.pitem;
	    else if (mom_is_item (resv))
	      typitm = resv.pitem;
	    else
	      typitm = NULL;
	  }
	};
      if (!typitm || count == 0)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid value to type:"),
			MOMOUT_VALUE ((const momval_t) val), NULL);
    }
  return momtypenc__none;
}


// emit an item as a variable, giving its type or 0 on failure
static momtypenc_t
emit_var_item_cgen (struct c_generator_mom_st *cg, momitem_t *varitm)
{
  momval_t expasv = MOM_NULLV;
  if (varitm == mom_named__result && cg->cgen_routkind == cgr_proc
      && cg->cgen_restype)
    {
      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64), "momresult");
      return cg->cgen_restype;
    }
  expasv = mom_item_assoc_get (cg->cgen_locassocitm, varitm);
  if (expasv.ptr == NULL)
    expasv = mom_item_assoc_get (cg->cgen_globassocitm, varitm);
  const momitem_t *noditm = mom_node_conn (expasv);
  if (noditm == mom_named__constant)
    {
      int cix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
      momitem_t *constitm = mom_value_to_item (mom_node_nth (expasv, 1));
      assert (constitm != NULL && constitm->i_typnum == momty_item);
      assert (cix >= 0);
      if (cg->cgen_routkind == cgr_proc)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL ("(momprocconstants["),
		   MOMOUT_DEC_INT (cix),
		   MOMOUT_LITERAL ("] /*"),
		   MOMOUT_ITEM ((const momitem_t *) constitm),
		   MOMOUT_LITERAL ("*/)"), NULL);
	  return momtypenc_val;
	}
      else if (cg->cgen_routkind == cgr_funt)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL ("(momclovals["),
		   MOMOUT_DEC_INT (cix),
		   MOMOUT_LITERAL ("] /*"),
		   MOMOUT_ITEM ((const momitem_t *) constitm),
		   MOMOUT_LITERAL ("*/)"), NULL);
	  return momtypenc_val;
	}
      else
	assert (false && "impossible routkind");
    }
  else if (noditm == mom_named__values)
    {
      int vix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
      momitem_t *valitm = mom_value_to_item (mom_node_nth (expasv, 1));
      assert (valitm != NULL && valitm->i_typnum == momty_item);
      assert (vix >= 0);
      if (cg->cgen_routkind == cgr_proc)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL (CGEN_PROC_VALUE_PREFIX),
		   MOMOUT_DEC_INT ((int) vix),
		   MOMOUT_LITERAL ("/*"),
		   MOMOUT_ITEM ((const momitem_t *) valitm),
		   MOMOUT_LITERAL ("*/)"), NULL);
	  return momtypenc_val;
	}
      else if (cg->cgen_routkind == cgr_funt)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL ("momvals["),
		   MOMOUT_DEC_INT ((int) vix),
		   MOMOUT_LITERAL ("/*"),
		   MOMOUT_ITEM ((const momitem_t *) valitm),
		   MOMOUT_LITERAL ("*/]"), NULL);
	  return momtypenc_val;
	}
      else
	assert (false && "impossible routkind");
    }
  else if (noditm == mom_named__numbers)
    {
      int nix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
      momitem_t *numitm = mom_value_to_item (mom_node_nth (expasv, 1));
      assert (numitm != NULL && numitm->i_typnum == momty_item);
      assert (nix >= 0);
      if (cg->cgen_routkind == cgr_proc)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL (CGEN_PROC_NUMBER_PREFIX),
		   MOMOUT_DEC_INT ((int) nix),
		   MOMOUT_LITERAL ("/*"),
		   MOMOUT_ITEM ((const momitem_t *) numitm),
		   MOMOUT_LITERAL ("*/"), NULL);
	  return momtypenc_int;
	}
      else if (cg->cgen_routkind == cgr_funt)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL ("momnums["),
		   MOMOUT_DEC_INT ((int) nix),
		   MOMOUT_LITERAL ("/*"),
		   MOMOUT_ITEM ((const momitem_t *) numitm),
		   MOMOUT_LITERAL ("*/]"), NULL);
	  return momtypenc_int;
	}
      else
	assert (false && "impossible routkind");
    }
  else if (noditm == mom_named__doubles)
    {
      int dix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
      momitem_t *dblitm = mom_value_to_item (mom_node_nth (expasv, 1));
      assert (dblitm != NULL && dblitm->i_typnum == momty_item);
      assert (dix >= 0);
      if (cg->cgen_routkind == cgr_proc)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL (CGEN_PROC_DOUBLE_PREFIX),
		   MOMOUT_DEC_INT ((int) dix),
		   MOMOUT_LITERAL ("/*"),
		   MOMOUT_ITEM ((const momitem_t *) dblitm),
		   MOMOUT_LITERAL ("*/"), NULL);
	  return momtypenc_int;
	}
      else if (cg->cgen_routkind == cgr_funt)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL ("momdbls["),
		   MOMOUT_DEC_INT ((int) dix),
		   MOMOUT_LITERAL ("/*"),
		   MOMOUT_ITEM ((const momitem_t *) dblitm),
		   MOMOUT_LITERAL ("*/]"), NULL);
	  return momtypenc_int;
	}
      else
	assert (false && "impossible routkind");
    }
  momval_t ctypv = MOM_NULLV;
  momval_t verbatimv = MOM_NULLV;
  {
    mom_lock_item (varitm);
    verbatimv = mom_item_get_attribute (varitm, mom_named__verbatim);
    ctypv = mom_item_get_attribute (varitm, mom_named__ctype);
    mom_unlock_item (varitm);
  }
  if (mom_is_string (verbatimv) && mom_is_item (ctypv))
    {
      if (ctypv.pitem == mom_named__momval_t)
	{
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERALV (mom_string_cstr (verbatimv)), NULL);
	  return momtypenc_val;
	}
      else if (ctypv.pitem == mom_named__double)
	{
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERALV (mom_string_cstr (verbatimv)), NULL);
	  return momtypenc_double;
	}
      else if (ctypv.pitem == mom_named__momcstr_t)
	{
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERALV (mom_string_cstr (verbatimv)), NULL);
	  return momtypenc_string;
	}
      else if (ctypv.pitem == mom_named__intptr_t)
	{
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERALV (mom_string_cstr (verbatimv)), NULL);
	  return momtypenc_int;
	}
    }
  return momtypenc__none;
}

// emit an expression and gives its type
static momtypenc_t
emit_expr_cgen (struct c_generator_mom_st *cg, momval_t expv)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_string (expv))
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_SPACE (48),
	       MOMOUT_LITERAL ("\""),
	       MOMOUT_JS_STRING ((const char *) expv.pstring->cstr),
	       MOMOUT_LITERAL ("\""), NULL);
      return momtypenc_string;
    }
  else if (mom_is_integer (expv))
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_SPACE (48),
	       MOMOUT_FMT_LONG_LONG ((const char *) "%lld",
				     (long long) (expv.pint->intval)), NULL);
      return momtypenc_int;
    }
  else if (mom_is_item (expv))
    {
      momitem_t *expitm = expv.pitem;
      momtypenc_t typva = emit_var_item_cgen (cg, expitm);
      if (typva != momtypenc__none)
	return typva;
      else
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid expression item:"),
			MOMOUT_ITEM ((const momitem_t *) expitm), NULL);
    }
  else if (mom_is_node (expv))
    return emit_node_cgen (cg, expv);
  else
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid expression"),
		    MOMOUT_VALUE ((const momval_t) expv));
  return 0;
}

// emit a node and gives its type
static momtypenc_t
emit_node_cgen (struct c_generator_mom_st *cg, momval_t nodv)
{
  momval_t argsv = MOM_NULLV;
  momval_t resv = MOM_NULLV;
  momval_t ctypev = MOM_NULLV;
  momval_t primexpv = MOM_NULLV;
  momval_t procv = MOM_NULLV;
  momval_t primcountv = MOM_NULLV;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momitem_t *connitm = (momitem_t *) mom_node_conn (nodv);
  int arity = mom_node_arity (nodv);
  if (!connitm)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("non-node"),
		    MOMOUT_VALUE ((const momval_t) nodv));
  {
    mom_lock_item (connitm);
    argsv = mom_item_get_attribute (connitm, mom_named__formals);
    resv = mom_item_get_attribute (connitm, mom_named__result);
    procv = mom_item_get_attribute (connitm, mom_named__procedure);
    ctypev = mom_item_get_attribute (connitm, mom_named__ctype);
    primexpv =
      mom_item_get_attribute (connitm, mom_named__primitive_expansion);
    primcountv = mom_item_get_attribute (connitm, mom_named__count);
    mom_unlock_item (connitm);
  }
  // handle primitives
  if (primexpv.ptr)
    {
      struct mom_itemattributes_st *argbind = NULL;
      if (!mom_is_tuple (argsv)
	  || !mom_is_node (primexpv) || !mom_is_item (ctypev)
	  || mom_node_conn (primexpv) != mom_named__chunk)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad primitive:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      if ((int) mom_tuple_length (argsv) != arity)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("wrong arity for primitive:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL ("(/*primitive "),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_LITERAL ("*/ "), NULL);
      if (mom_node_conn (primexpv) != mom_named__chunk)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("no `chunk` for `primitive_expansion` in primitive:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm),
			MOMOUT_SPACE (48), MOMOUT_LITERAL ("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      argbind = mom_reserve_attribute (argbind, 3 * arity / 2 + 5);
      char *argctypestr = MOM_GC_SCALAR_ALLOC ("argctypestr", arity + 3);
      // bind the count if needed
      if (mom_is_item (primcountv))
	argbind =
	  mom_put_attribute (argbind, primcountv.pitem,
			     mom_make_integer (++cg->cgen_count));
      // first loop to bind the formal arguments and get their `ctype`
      for (int ix = 0; ix < arity; ix++)
	{
	  momval_t formctypv = MOM_NULLV;
	  momitem_t *formalitm = mom_tuple_nth_item (argsv, ix);
	  momval_t argv = mom_node_nth (nodv, ix);
	  if (!formalitm || !argv.ptr)
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL
	       ("missing argument for primitive:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       connitm),
	       MOMOUT_SPACE (48), MOMOUT_LITERAL ("rank#"),
	       MOMOUT_DEC_INT (ix), MOMOUT_LITERAL (" in node:"),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	  {
	    mom_lock_item (formalitm);
	    formctypv = mom_item_get_attribute (formalitm, mom_named__ctype);
	    mom_unlock_item (formalitm);
	  }
	  momtypenc_t formtyp = 0;
	  if (formctypv.pitem == mom_named__intptr_t)
	    formtyp = momtypenc_int;
	  else if (formctypv.pitem == mom_named__momval_t)
	    formtyp = momtypenc_val;
	  else if (formctypv.pitem == mom_named__double)
	    formtyp = momtypenc_double;
	  else if (formctypv.pitem == mom_named__momcstr_t)
	    formtyp = momtypenc_string;
	  else if (formctypv.pitem == mom_named__void)
	    formtyp = momtypenc__none;
	  else
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL ("bad `ctype` of primitive formal argument:"),
	       MOMOUT_ITEM ((const momitem_t *) formalitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       formalitm),
	       MOMOUT_LITERAL
	       (" in primitive:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       connitm),
	       MOMOUT_SPACE (48), MOMOUT_LITERAL ("rank#"),
	       MOMOUT_DEC_INT (ix), MOMOUT_LITERAL (" in node:"),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	  argctypestr[ix] = formtyp;
	  if (mom_get_attribute (argbind, formalitm).ptr)
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL ("duplicate primitive formal argument:"),
	       MOMOUT_ITEM ((const momitem_t *) formalitm),
	       (" in primitive:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       connitm),
	       MOMOUT_SPACE (48), MOMOUT_LITERAL ("rank#"),
	       MOMOUT_DEC_INT (ix), MOMOUT_LITERAL (" in node:"),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	  argbind = mom_put_attribute (argbind, formalitm, argv);
	}
      // second loop to output the expansion
      int chunklen = mom_node_arity (primexpv);
      for (int chix = 0; chix < chunklen; chix++)
	{
	  momval_t curchkv = mom_node_nth (primexpv, chix);
	  if (mom_is_string (curchkv))
	    MOM_OUT (&cg->cgen_outbody,
		     MOMOUT_LITERALV ((const char *)
				      mom_string_cstr (curchkv)));
	  else if (mom_is_integer (curchkv))
	    MOM_OUT (&cg->cgen_outbody,
		     MOMOUT_DEC_INTPTR_T (mom_integer_val (curchkv)));
	  else if (mom_is_item (curchkv))
	    {
	      momval_t bndvalv = mom_get_attribute (argbind, curchkv.pitem);
	      if (bndvalv.ptr)
		{
		  momtypenc_t vtyp = emit_expr_cgen (cg, bndvalv);
		  for (unsigned fix = 0; fix < (unsigned) arity; fix++)
		    if (mom_tuple_nth_item (argsv, fix) == bndvalv.pitem
			&& (unsigned) vtyp != (unsigned) argctypestr[fix])
		      CGEN_ERROR_MOM
			(cg,
			 MOMOUT_LITERAL ("type mismatch for formal:"),
			 MOMOUT_ITEM ((const momitem_t *) bndvalv.pitem),
			 MOMOUT_LITERAL (" rank#"),
			 MOMOUT_DEC_INT ((int) fix),
			 MOMOUT_LITERAL (" in node:"),
			 MOMOUT_VALUE ((const momval_t) nodv), NULL);
		}
	    }
	  else
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL ("invalid chunk element:"),
	       MOMOUT_VALUE ((const momval_t) curchkv),
	       MOMOUT_DEC_INT (chix), MOMOUT_LITERAL (" in node:"),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	};
      if (ctypev.pitem == mom_named__intptr_t)
	return momtypenc_int;
      else if (ctypev.pitem == mom_named__momval_t)
	return momtypenc_val;
      else if (ctypev.pitem == mom_named__double)
	return momtypenc_double;
      else if (ctypev.pitem == mom_named__momcstr_t)
	return momtypenc_string;
      else
	return momtypenc__none;
    }
  // handle procedures
  else if (procv.ptr)
    {
      if (!mom_is_tuple (argsv))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad procedure node:"),
			MOMOUT_VALUE ((const momval_t) nodv),
			MOMOUT_LITERAL
			(" without `formals` in connective:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm));
      if (mom_tuple_length (argsv) != (unsigned) arity)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("arity mismatch for procedure:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_LITERAL (" in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      if (!ctypev.ptr)
	{
	  if (mom_is_item (resv))
	    {
	      mom_lock_item (resv.pitem);
	      ctypev = mom_item_get_attribute (resv.pitem, mom_named__ctype);
	      mom_unlock_item (resv.pitem);
	    }
	}
      // declare the procedure external if needed
      if (!mom_item_get_attribute (cg->cgen_globassocitm, connitm).ptr)
	{
	  MOM_OUT (&cg->cgen_outhead,
		   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("// external procedure "),
		   MOMOUT_ITEM ((const momitem_t *) connitm),
		   MOMOUT_NEWLINE (), MOMOUT_LITERAL ("extern "), NULL);
	  emit_ctype_cgen (cg, &cg->cgen_outhead, ctypev);
	  MOM_OUT (&cg->cgen_outhead,
		   MOMOUT_LITERAL (" " MOM_PROCROUTFUN_PREFIX),
		   MOMOUT_LITERALV (mom_ident_cstr_of_item (connitm)),
		   MOMOUT_LITERAL (" ("), MOMOUT_INDENT_MORE (), NULL);
	  int nbargs = mom_tuple_length (argsv);
	  for (int aix = 0; aix < nbargs; aix++)
	    {
	      if (aix > 0)
		MOM_OUT (&cg->cgen_outhead, MOMOUT_LITERAL (","),
			 MOMOUT_SPACE (48));
	      momitem_t *curformitm = mom_tuple_nth_item (argsv, aix);
	      momval_t curformctypv = MOM_NULLV;
	      if (!curformitm)
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL ("bad external procedure:"),
				MOMOUT_ITEM ((const momitem_t *) connitm),
				MOMOUT_LITERAL (" missing formal #"),
				MOMOUT_DEC_INT (aix));
	      {
		mom_lock_item (curformitm);
		curformctypv =
		  mom_item_get_attribute (curformitm, mom_named__ctype);
		mom_unlock_item (curformitm);
	      }
	      if (!mom_is_item (curformctypv))
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in external procedure:"),
				MOMOUT_ITEM ((const momitem_t *) connitm),
				MOMOUT_LITERAL ("  formal #"),
				MOMOUT_DEC_INT ((int) aix),
				MOMOUT_LITERAL ("="),
				MOMOUT_ITEM ((const momitem_t *) curformitm),
				MOMOUT_LITERAL (" with missing ctype"));
	      emit_ctype_cgen (cg, &cg->cgen_outhead, curformctypv);
	    }
	  MOM_OUT (&cg->cgen_outhead, MOMOUT_INDENT_LESS (),
		   MOMOUT_LITERAL (");"), MOMOUT_NEWLINE ());
	  mom_item_put_attribute (cg->cgen_globassocitm, connitm,
				  (momval_t)
				  mom_make_node_sized (mom_named__procedure,
						       3, MOM_NULLV, argsv,
						       ctypev));
	}
      if (mom_item_get_name (connitm))
	MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*"),
		 MOMOUT_ITEM ((const momitem_t *) connitm),
		 MOMOUT_LITERAL ("*/"), NULL);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" " MOM_PROCROUTFUN_PREFIX),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (connitm)),
	       MOMOUT_LITERAL (" ("), MOMOUT_INDENT_MORE (), NULL);
      for (int aix = 0; aix < arity; aix++)
	{
	  momval_t curargv = mom_node_nth (nodv, aix);
	  momitem_t *curformitm = mom_tuple_nth_item (argsv, aix);
	  momval_t curformctypv = MOM_NULLV;
	  if (!curformitm)
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad procedure:"),
			    MOMOUT_ITEM ((const momitem_t *) connitm),
			    MOMOUT_LITERAL (" missing formal #"),
			    MOMOUT_DEC_INT (aix));
	  {
	    mom_lock_item (curformitm);
	    curformctypv =
	      mom_item_get_attribute (curformitm, mom_named__ctype);
	    mom_unlock_item (curformitm);
	  }
	  if (!mom_is_item (curformctypv))
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
			    MOMOUT_ITEM ((const momitem_t *) connitm),
			    MOMOUT_LITERAL ("  formal #"),
			    MOMOUT_DEC_INT (aix),
			    MOMOUT_LITERAL ("="),
			    MOMOUT_ITEM ((const momitem_t *) curformitm),
			    MOMOUT_LITERAL (" with missing ctype"));
	  momtypenc_t curformtyp = 0;
	  if (curformctypv.pitem == mom_named__intptr_t)
	    curformtyp = momtypenc_int;
	  else if (curformctypv.pitem == mom_named__momval_t)
	    curformtyp = momtypenc_val;
	  else if (curformctypv.pitem == mom_named__double)
	    curformtyp = momtypenc_double;
	  else if (curformctypv.pitem == mom_named__momcstr_t)
	    curformtyp = momtypenc_string;
	  else if (curformctypv.pitem == mom_named__void)
	    curformtyp = momtypenc__none;
	  else
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL ("bad `ctype` of procedure formal argument:"),
	       MOMOUT_ITEM ((const momitem_t *) curformitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       curformitm),
	       MOMOUT_LITERAL
	       (" in procedure:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
				       connitm),
	       MOMOUT_SPACE (48), MOMOUT_LITERAL ("rank#"),
	       MOMOUT_DEC_INT (aix), MOMOUT_LITERAL (" in node:"),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	  if (curformtyp != emit_expr_cgen (cg, curargv))
	    CGEN_ERROR_MOM
	      (cg, MOMOUT_LITERAL ("procedure argument:"),
	       MOMOUT_VALUE ((const momval_t) curargv),
	       MOMOUT_LITERAL (" mismatching type "),
	       MOMOUT_VALUE ((const momval_t) curformctypv),
	       MOMOUT_LITERAL (" rank#"),
	       MOMOUT_DEC_INT (aix),
	       MOMOUT_LITERAL (" in node "),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL (")"), MOMOUT_INDENT_LESS (),
		   MOMOUT_SPACE (32), NULL);
	}
    }
  return emit_ctype_cgen (cg, NULL, ctypev);
}


static int
cgen_cmp_case_items_mom (const void *p1, const void *p2)
{
  const struct mom_attrentry_st *e1 = p1;
  const struct mom_attrentry_st *e2 = p2;
  return mom_item_cmp (e1->aten_itm, e2->aten_itm);
}

static int
cgen_cmp_hash_items_mom (const void *p1, const void *p2, void *arg)
{
  const struct mom_attrentry_st *e1 = p1;
  const struct mom_attrentry_st *e2 = p2;
  intptr_t h = (intptr_t) arg;
  assert (h > 0);
  return (mom_item_hash (e1->aten_itm) % h) -
    (mom_item_hash (e2->aten_itm) % h);
}


void
emit_block_cgen (struct c_generator_mom_st *cg, momitem_t *blkitm)
{
#define CGEN_LOCK_ITEM_PREFIX "momlockeditem_"
#define CGEN_END_BLOCK_PREFIX "momendblock_"
  int lockix = -1;
  momval_t blockv = MOM_NULLV;
  momval_t lockv = MOM_NULLV;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (blkitm && blkitm->i_typnum == momty_item);
  {
    mom_lock_item (blkitm);
    blockv = mom_item_get_attribute (blkitm, mom_named__block);
    lockv = mom_item_get_attribute (blkitm, mom_named__lock);
    mom_unlock_item (blkitm);
  }
  if (mom_node_conn (blockv) != mom_named__code)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL
		    ("invalid block (bad `block`, should be a `code` node):"),
		    MOMOUT_ITEM ((const momitem_t *) blkitm),
		    MOMOUT_SPACE (32),
		    MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) blkitm));
  if (lockv.ptr && !mom_is_item (lockv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid `lock` in block:"),
		    MOMOUT_ITEM ((const momitem_t *) blkitm),
		    MOMOUT_SPACE (32),
		    MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) blkitm));
  int nbinstr = (int) mom_node_arity (blockv);
  // check the instructions
  for (int ix = 0; ix < nbinstr; ix++)
    {
      momval_t curinsv = mom_node_nth (blockv, ix);
      momitem_t *opitm = (momitem_t *) mom_node_conn (curinsv);
      assert (opitm != NULL);
      if (opitm != mom_named__do
	  && opitm != mom_named__if
	  && opitm != mom_named__assign && opitm != mom_named__switch
	  && opitm != mom_named__assign && opitm != mom_named__dispatch
	  && (opitm != mom_named__jump || ix < nbinstr - 1)
	  && (opitm != mom_named__call || ix < nbinstr - 1)
	  && (opitm != mom_named__return || ix < nbinstr - 1))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid instruction:"),
			MOMOUT_VALUE ((const momval_t) curinsv),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("at rank#"),
			MOMOUT_DEC_INT (ix),
			MOMOUT_LITERAL ("/"),
			MOMOUT_DEC_INT (nbinstr),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in block:"),
			MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
    }
  // emit the locked item
  if (lockv.pitem != NULL)
    {
      lockix = ++cg->cgen_count;
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// locked-item "),
	       MOMOUT_ITEM ((const momitem_t *) lockv.pitem),
	       MOMOUT_LITERAL (" in block "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("momitem_t* " CGEN_LOCK_ITEM_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL (" = mom_value_to_item ("));
      if (emit_expr_cgen (cg, lockv) != momtypenc_val)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad non-value lock:"),
			MOMOUT_VALUE ((const momval_t) lockv),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in block:"),
			MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (") /* locked-item */;"),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("if (!mom_lock_item (" CGEN_LOCK_ITEM_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL (")) goto " CGEN_END_BLOCK_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL (";"), MOMOUT_NEWLINE (), NULL);
    }
  // emit every instruction    
  for (int ix = 0; ix < nbinstr; ix++)
    {
      momval_t curinsv = mom_node_nth (blockv, ix);
      momitem_t *opitm = (momitem_t *) mom_node_conn (curinsv);
      unsigned insarity = mom_node_arity (curinsv);
      assert (opitm != NULL);
      //// DO instruction
      if (opitm == mom_named__do && insarity == 1)
	{
	  /* *do(<expr>) */
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*do*/ (void) "));
	  emit_expr_cgen (cg, mom_node_nth (curinsv, 0));
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL (" /*done*/;"), MOMOUT_NEWLINE ());
	}
      //// ASSIGN instruction
      else if (opitm == mom_named__assign && insarity == 2)
	{
	  /* *assign(<var>,<expr>) */
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*assign*/ "));
	  momitem_t *lhsitm = mom_value_to_item (mom_node_nth (curinsv, 0));
	  if (!lhsitm)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL ("bad left-hand-side in assign:"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momtypenc_t lhstyp = emit_var_item_cgen (cg, lhsitm);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" = "));
	  momval_t rhsv = mom_node_nth (curinsv, 1);
	  momtypenc_t rtyp = emit_expr_cgen (cg, rhsv);
	  if (lhstyp != rtyp)
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("type mismatch in assign:"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix),
			    MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL (";"), MOMOUT_NEWLINE ());
	}
      //// IF instruction
      else if (opitm == mom_named__if && insarity == 2)
	{
	  /* *if(<test-expr>,<dest-block>) */
	  momval_t testv = mom_node_nth (curinsv, 0);
	  momval_t destblockv = mom_node_nth (curinsv, 1);
	  if (!mom_is_item (destblockv))
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid destination block in if:"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momitem_t *destblkitm = destblockv.pitem;
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*if*/ if ("));
	  momtypenc_t ctyp = emit_expr_cgen (cg, testv);
	  if (ctyp == momtypenc__none)
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid condition in if:"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix),
			    MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  if (ctyp == momtypenc_val)
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".ptr"));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"),
		   MOMOUT_INDENT_MORE ());
	  emit_goto_block_cgen (cg, destblkitm, lockix);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_INDENT_LESS (),
		   MOMOUT_SPACE (48));
	}
      //// SWITCH instruction - integer discriminant
      else if (opitm == mom_named__switch && insarity >= 1)
	{
	  /* *switch(<discriminant-expr>,<cases>...) */
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("switch ("));
	  momval_t swixprv = mom_node_nth (curinsv, 0);
	  if (emit_expr_cgen (cg, swixprv) != momtypenc_int)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid non-integral switch expression:"),
			    MOMOUT_VALUE ((const momval_t) swixprv),
			    MOMOUT_LITERAL (" in instruction "),
			    MOMOUT_VALUE ((const momval_t) curinsv));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (") {"),
		   MOMOUT_INDENT_MORE ());
	  for (int casix = 1; casix < (int) insarity; casix++)
	    {
	      momval_t casev = mom_node_nth (curinsv, casix);
	      momval_t casintv = MOM_NULLV;
	      momval_t casitmv = MOM_NULLV;
	      if (!mom_is_node (casev)
		  || mom_node_conn (casev) != mom_named__case
		  || mom_node_arity (casev) != 2
		  || !mom_is_integer ((casintv = mom_node_nth (casev, 0)))
		  || !mom_is_item ((casitmv = mom_node_nth (casev, 1))))
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid case:"),
				MOMOUT_VALUE ((const momval_t) casev),
				MOMOUT_LITERAL (" in switch instruction:"),
				MOMOUT_VALUE ((const momval_t) curinsv));
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		       MOMOUT_LITERAL ("case "),
		       MOMOUT_DEC_INTPTR_T ((const intptr_t) casintv.pint),
		       MOMOUT_LITERAL (": "), NULL);
	      emit_goto_block_cgen (cg, casitmv.pitem, lockix);
	    }

	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("default:;"),
		   MOMOUT_INDENT_LESS (),
		   MOMOUT_SPACE (48), MOMOUT_LITERAL ("}/*end-switch*/"));
	}
      //// DISPATCH instruction - value discriminant
      else if (opitm == mom_named__dispatch && insarity >= 1)
	{
	  /* *dispatch(<discriminant-expr>,<cases>...) */
	  momval_t dispv = mom_node_nth (curinsv, 0);
#define CGEN_DISPATCH_PREFIX "momdispatchitm"
	  int dispix = ++cg->cgen_count;
	  struct mom_attrentry_st *casentarr =
	    MOM_GC_ALLOC ("case item entries",
			  (insarity + 1) * sizeof (struct mom_attrentry_st));
	  for (int cix = 1; cix < (int) insarity; cix++)
	    {
	      momitem_t *curcasitm = NULL;
	      momitem_t *curblkitm = NULL;
	      momval_t asscasv = MOM_NULLV;
	      momval_t assblkv = MOM_NULLV;
	      momval_t curcasv = mom_node_nth (curinsv, cix);
	      if (mom_node_conn (curcasv) != mom_named__case
		  || mom_node_arity (curcasv) != 2
		  || !(curcasitm =
		       mom_value_to_item (mom_node_nth (curcasv, 0)))
		  || !(curblkitm =
		       mom_value_to_item (mom_node_nth (curcasv, 1)))
		  || !(asscasv =
		       mom_item_assoc_get (cg->cgen_locassocitm,
					   curcasitm)).ptr
		  || !(assblkv =
		       mom_item_assoc_get (cg->cgen_locassocitm,
					   curblkitm)).ptr
		  || mom_node_conn (asscasv) != mom_named__constant
		  || mom_node_conn (curcasv) != mom_named__block)
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid case:"),
				MOMOUT_VALUE ((const momval_t) curcasv),
				MOMOUT_LITERAL (" in dispatch instruction:"),
				MOMOUT_VALUE ((const momval_t) curinsv));
	      else
		{
		  casentarr[cix - 1].aten_itm = curcasitm;
		  casentarr[cix - 1].aten_val = (momval_t) curblkitm;
		}
	    };
	  qsort (casentarr, insarity - 1, sizeof (struct mom_attrentry_st),
		 cgen_cmp_case_items_mom);
	  for (int eix = 0; eix < (int) insarity - 1; eix++)
	    if (casentarr[eix].aten_itm == casentarr[eix + 1].aten_itm)
	      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("duplicate case:"),
			      MOMOUT_ITEM ((const momitem_t *)
					   casentarr[eix].aten_itm),
			      MOMOUT_LITERAL (" in dispatch instruction:"),
			      MOMOUT_VALUE ((const momval_t) curinsv));
	  int modh = (4 * insarity / 3 + 2) | 7;
	  qsort_r (casentarr, insarity - 1, sizeof (struct mom_attrentry_st),
		   cgen_cmp_hash_items_mom, (void *) ((intptr_t) modh));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("// dispatch#"), MOMOUT_DEC_INT (dispix),
		   MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("momitem_t* " CGEN_DISPATCH_PREFIX),
		   MOMOUT_DEC_INT (dispix),
		   MOMOUT_LITERAL ("mom_value_to_item("), NULL);
	  if (emit_expr_cgen (cg, dispv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid dispatcher:"),
			    MOMOUT_VALUE ((const momval_t) dispv),
			    MOMOUT_LITERAL (" in dispatch instruction:"),
			    MOMOUT_VALUE ((const momval_t) curinsv));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (") /*dispatcher*/;"),
		   MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("if (" CGEN_DISPATCH_PREFIX),
		   MOMOUT_DEC_INT (dispix),
		   MOMOUT_LITERAL (" != NULL) switch (mom_item_hash ("
				   CGEN_DISPATCH_PREFIX),
		   MOMOUT_DEC_INT (dispix), MOMOUT_LITERAL (") % "),
		   MOMOUT_DEC_INT (modh), MOMOUT_LITERAL (")"),
		   MOMOUT_INDENT_MORE (), NULL);
	  for (int eix = 0; eix < (int) insarity - 1; eix++)
	    {
	      if (eix == 0
		  || casentarr[eix].aten_itm != casentarr[eix - 1].aten_itm)
		MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
			 MOMOUT_LITERALV ((const char *) ((eix > 0) ? "break;"
							  : " ")),
			 MOMOUT_LITERAL ("case "),
			 MOMOUT_DEC_INT ((int)
					 mom_item_hash (casentarr
							[eix].aten_itm) %
					 modh), MOMOUT_LITERAL (":"), NULL);
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		       MOMOUT_LITERAL ("if (" CGEN_DISPATCH_PREFIX),
		       MOMOUT_DEC_INT (dispix), MOMOUT_LITERAL (" == "));
	      emit_expr_cgen (cg, (momval_t) casentarr[eix].aten_itm);
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (") "));
	      emit_goto_block_cgen (cg,
				    mom_value_to_item (casentarr
						       [eix].aten_val),
				    lockix);
	    };
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("break;"), MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("default:;} // end dispatch"),
		   MOMOUT_INDENT_LESS (), MOMOUT_NEWLINE ());
	  MOM_GC_FREE (casentarr);
	}
      //// JUMP instruction
      else if (opitm == mom_named__jump && insarity == 1)
	{
	  /* *jump(<dest-block>) */
	  if (ix != nbinstr - 1)
	    MOM_WARNING (MOMOUT_LITERAL ("jump instruction:"),
			 MOMOUT_VALUE ((const momval_t) curinsv),
			 MOMOUT_SPACE (48),
			 MOMOUT_LITERAL (" not last at rank#"),
			 MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			 MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			 MOMOUT_LITERAL ("in block:"),
			 MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momval_t destblockv = mom_node_nth (curinsv, 0);
	  if (!mom_is_item (destblockv))
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid destination block in jump"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momitem_t *destblkitm = destblockv.pitem;
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("/* jump */"), MOMOUT_NEWLINE ());
	  emit_goto_block_cgen (cg, destblkitm, lockix);
	}
      //// CALL instruction
      else if (opitm == mom_named__call && insarity >= 2)
	{
	  /*   *call(<return-block>,<fun-expr>,<arg-expr>....) in functions only */
	  momval_t retblockv = mom_node_nth (curinsv, 0);
	  momval_t funexprv = mom_node_nth (curinsv, 1);
	  if (cg->cgen_routkind != cgr_funt || !mom_is_item (retblockv))
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERALV ((const char *)
					     ((cg->cgen_routkind !=
					       cgr_funt) ?
					      "invalid (outside of function) call:"
					      : "invalid call:")),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("/* call */"), MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("mom_item_tasklet_clear_res(momtasklet);"),
		   MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL
		   ("mom_item_tasklet_push_frame(momtasklet, "), NULL);
	  if (emit_expr_cgen (cg, funexprv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("function:"),
			    MOMOUT_VALUE ((const momval_t) funexprv),
			    MOMOUT_LITERAL (" of non-value type in call:"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  for (int aix = 2; aix < (int) insarity; aix++)
	    {
	      momval_t curargexpv = mom_node_nth (curinsv, aix);
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
		       MOMOUT_SPACE (48), NULL);
	      momtypenc_t curargty = emit_ctype_cgen (cg, NULL, curargexpv);
	      switch (curargty)
		{
		case momtypenc_int:
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			   MOMOUT_LITERAL ("MOMPFR_INT("), NULL);
		  if (momtypenc_int != emit_expr_cgen (cg, curargexpv))
		    MOM_FATAL (MOMOUT_LITERAL ("corrupted call:"),
			       MOMOUT_VALUE ((const momval_t) curinsv),
			       MOMOUT_LITERAL ("; corrupted non-int arg:"),
			       MOMOUT_VALUE ((const momval_t) curargexpv),
			       NULL);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"));
		  break;
		case momtypenc_val:
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			   MOMOUT_LITERAL ("MOMPFR_VAL("), NULL);
		  if (momtypenc_val != emit_expr_cgen (cg, curargexpv))
		    MOM_FATAL (MOMOUT_LITERAL ("corrupted call:"),
			       MOMOUT_VALUE ((const momval_t) curinsv),
			       MOMOUT_LITERAL ("; corrupted non-val arg:"),
			       MOMOUT_VALUE ((const momval_t) curargexpv),
			       NULL);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"));
		  break;
		case momtypenc_double:
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			   MOMOUT_LITERAL ("MOMPFR_DOUBLE("), NULL);
		  if (momtypenc_double != emit_expr_cgen (cg, curargexpv))
		    MOM_FATAL (MOMOUT_LITERAL ("corrupted call:"),
			       MOMOUT_VALUE ((const momval_t) curinsv),
			       MOMOUT_LITERAL ("; corrupted non-double arg:"),
			       MOMOUT_VALUE ((const momval_t) curargexpv),
			       NULL);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"));
		  break;
		case momtypenc_string:
		default:
		  {
		    char tybuf[4] = { 0 };
		    tybuf[0] = curargty;
		    tybuf[1] = (char) 0;
		    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid type "),
				    MOMOUT_LITERALV ((const char *) tybuf),
				    MOMOUT_LITERAL (" for argument "),
				    MOMOUT_VALUE ((const momval_t)
						  curargexpv),
				    MOMOUT_LITERAL (" in call "),
				    MOMOUT_VALUE (curinsv), MOMOUT_SPACE (48),
				    MOMOUT_LITERAL ("at rank#"),
				    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
				    MOMOUT_DEC_INT (nbinstr),
				    MOMOUT_SPACE (48),
				    MOMOUT_LITERAL (" in block "),
				    MOMOUT_ITEM ((const momitem_t *) blkitm),
				    NULL);;
		  }

		}
	    }
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (", NULL);"),
		   MOMOUT_SPACE (32));
	}
      //// RETURN instruction
      else if ((opitm == mom_named__return && insarity <= 1)
	       || curinsv.pitem == mom_named__return)
	{
	  /* *return(<expr> [, <expr2> [,<expr3>]]) or `return` */
	  if (cg->cgen_routkind == cgr_funt)
	    {
	      if (insarity > 3)
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("too many results in "),
				MOMOUT_VALUE (curinsv), MOMOUT_SPACE (48),
				MOMOUT_LITERAL ("at rank#"),
				MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
				MOMOUT_DEC_INT (nbinstr),
				MOMOUT_SPACE (48),
				MOMOUT_LITERAL (" in block "),
				MOMOUT_ITEM ((const momitem_t *) blkitm),
				NULL);
	      if (insarity > 0)
		{
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
			   MOMOUT_LITERAL ("mom_item_taklet_set_"),
			   MOMOUT_DEC_INT ((int) insarity),
			   MOMOUT_LITERAL ("res (momtasklet"), NULL);
		  for (int rix = 0; rix < (int) insarity; rix++)
		    {
		      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
			       MOMOUT_SPACE (40), MOMOUT_LITERAL ("("));
		      momval_t curresv = mom_node_nth (curinsv, rix);
		      if (momtypenc_val != emit_expr_cgen (cg, curresv))
			CGEN_ERROR_MOM (cg,
					MOMOUT_LITERAL
					("non-value result in "),
					MOMOUT_VALUE (curinsv),
					MOMOUT_SPACE (48),
					MOMOUT_LITERAL ("at rank#"),
					MOMOUT_DEC_INT (ix),
					MOMOUT_LITERAL ("/"),
					MOMOUT_DEC_INT (nbinstr),
					MOMOUT_SPACE (48),
					MOMOUT_LITERAL (" in block "),
					MOMOUT_ITEM ((const momitem_t *)
						     blkitm), NULL);
		      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"),
			       MOMOUT_SPACE (40), NULL);
		    }
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (");"),
			   MOMOUT_NEWLINE ());
		}
	      MOM_OUT (&cg->cgen_outbody,
		       MOMOUT_LITERAL ("return momroutres_pop;"),
		       MOMOUT_NEWLINE ());
	    }
	  else if (cg->cgen_routkind == cgr_proc)
	    {
	      momval_t procv = mom_item_assoc_get (cg->cgen_globassocitm,
						   cg->cgen_curoutitm);
	      assert (mom_node_conn (procv) == mom_named__procedure
		      && mom_node_arity (procv) == 3);
	      momval_t prorestypv = mom_node_nth (procv, 2);
	      if (insarity > 1)
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("too many procedure results in "),
				MOMOUT_VALUE (curinsv), MOMOUT_SPACE (48),
				MOMOUT_LITERAL ("at rank#"),
				MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
				MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
				MOMOUT_LITERAL (" in block "),
				MOMOUT_ITEM ((const momitem_t *) blkitm),
				NULL);
	      if (insarity == 0 || curinsv.pitem == mom_named__return)
		{
		  if (!prorestypv.ptr || prorestypv.pitem == mom_named__void)
		    MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			     MOMOUT_LITERAL ("return;"), MOMOUT_NEWLINE ());
		  else
		    MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			     MOMOUT_LITERAL ("return momresult;"),
			     MOMOUT_NEWLINE ());
		}
	      else if (insarity == 1 && prorestypv.ptr
		       && prorestypv.pitem != mom_named__void)
		{
		  momval_t resexprv = mom_node_nth (curinsv, 0);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			   MOMOUT_LITERAL ("return"), MOMOUT_SPACE (48),
			   NULL);
		  if (emit_ctype_cgen (cg, NULL, prorestypv) !=
		      emit_expr_cgen (cg, resexprv))
		    CGEN_ERROR_MOM (cg,
				    MOMOUT_LITERAL
				    ("incompatible procedure result in "),
				    MOMOUT_VALUE (curinsv), MOMOUT_SPACE (48),
				    MOMOUT_LITERAL ("at rank#"),
				    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
				    MOMOUT_DEC_INT (nbinstr),
				    MOMOUT_SPACE (48),
				    MOMOUT_LITERAL (" in block "),
				    MOMOUT_ITEM ((const momitem_t *) blkitm),
				    NULL);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			   MOMOUT_LITERAL (";"), MOMOUT_NEWLINE ());
		}
	      else
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL ("bad procedure result in "),
				MOMOUT_VALUE (curinsv), MOMOUT_SPACE (48),
				MOMOUT_LITERAL ("at rank#"),
				MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
				MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
				MOMOUT_LITERAL (" in block "),
				MOMOUT_ITEM ((const momitem_t *) blkitm),
				NULL);
	    }
	  else
	    MOM_FATAPRINTF ("invalid cgen_type #%d for return",
			    (int) cg->cgen_routkind);
	}
      ///// error case - bad instruction
      else
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid instruction:"),
			MOMOUT_VALUE ((const momval_t) curinsv),
			MOMOUT_SPACE (48), MOMOUT_LITERAL ("in block:"),
			MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
    };
  // emit block epilogue if locking
  if (lockix > 0)
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("/* epilogue for lock */"),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (CGEN_END_BLOCK_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL (":;"), MOMOUT_NEWLINE (), NULL);
    }
}

static void
emit_goto_block_cgen (struct c_generator_mom_st *cg, momitem_t *blkitm,
		      int lockix)
{
  assert (cg != NULL && cg->cgen_magic == CGEN_MAGIC);
  assert (blkitm != NULL && blkitm->i_typnum == momty_item);
  momval_t blockdatav = mom_item_assoc_get (cg->cgen_locassocitm, blkitm);
  if (blockdatav.ptr == NULL
      || mom_node_conn (blockdatav) != mom_named__block)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad block to go to:"),
		    MOMOUT_ITEM ((const momitem_t *) blkitm));
  int bix = mom_integer_val_def (mom_node_nth (blockdatav, 0), -2) - 1;
  assert (bix >= 0);
  if (lockix > 0)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("/*unlock-goto*/ { mom_unlock_item ("
			     CGEN_LOCK_ITEM_PREFIX), MOMOUT_DEC_INT (lockix),
	     MOMOUT_LITERAL ("); "), NULL);
  else
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" "));
  if (cg->cgen_routkind == cgr_proc)
    // should retrieve the bix
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL ("goto " CGEN_PROC_BLOCK_PREFIX),
	     MOMOUT_DEC_INT (bix),
	     MOMOUT_LITERAL (" /*proc.block "),
	     MOMOUT_ITEM ((const momitem_t *) blkitm),
	     MOMOUT_LITERAL ("*/"), NULL);
  else
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (" return "),
	     MOMOUT_DEC_INT (bix),
	     MOMOUT_LITERAL (" /*func.block "),
	     MOMOUT_ITEM ((const momitem_t *) blkitm),
	     MOMOUT_LITERAL ("*/"), NULL);
  if (lockix > 0)
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("}; //unlocked"),
	     MOMOUT_NEWLINE (), NULL);
}




//// end of file gencod.c
