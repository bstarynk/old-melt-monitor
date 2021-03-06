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
   function items. Procedures have an attribute `procedure`, and
   functions have an attribute `tasklet_function` - both associated to
   a starting block


   Routines (procedures or functions) usually have an attribute
   `constant` giving a sequence -set or tuple- of constant items,
   `formals` associated to formal arguments, and perhaps `locals`
   associated to local variables. But any item with `variable`
   associated to a ctype is considered as a variable.

   Tasklet functions have an attribute `constant` giving a sequence -set or
   tuple- of constant items, `formals` associated to formal-arguments,
   `locals` associated to locals variables.


   The code generation, when successful, is updating routines and
   blocks: Every reachable block gets `predecessors` and `successors`
   (sets of related blocks), and `in` the routine containing that
   block.

   Expressions are often nodes, the connective being a procedure or a
   primitive or a constructor.

   A primitive item (useful as connective in expressions) has a
   `primitive_expansion` with a node of connective `chunk` and it has
   a `ctype`

   Constructor items are for expressions like:

     *tuple(<expr>...) to build a tuple using mom_make_tuple_variadic

     *set(<expr>...) to build a set using mom_make_set_variadic

     *json_array(<expr>...) to build a JSON array using mom_make_json_array

     *json_object(<entry>...) to build a JSON object using mom_make_json_object
       where each <entry> is *json_entry(<name>,<value>) or *json_object(<expr>)

     *node(<conn-expr>,<arg-expr>...) to build a node


   A block item should preferably be unique to its procedure or
   function. It has a `block` attribute associated to a `code`
   node. That node should have expression sub-nodes, one of:

     *do (<expr>) for side-effecting expressions
     *if (<cond>,<block>) for conditional jumps
     *assign (<var>,<expr>) for assignments
     *switch (<expr>,<case>...); each <case> is *case(<const-expr>,<block>)
     *dispatch (<expr>,<case>...); each <case> is *case(<const-item>,<block>)
     *chunk (...) for some code chunk with variables

  but the last sub-node in a block code could also be:
     *jump(<block>)
     *call(<return-block>,<fun-expr>,<arg-expr>....) in functions only
     *return(<expr>) or *return()

  A block item might have a `lock` attribute giving a variable
  (holding some item) to lock before entering that block, which will
  be unlocked after exiting from that block.
****/

#define CGEN_MAGIC 0x566802a5	/* cgen magic 1449656997 */

#define CGEN_FORMALARG_PREFIX "momparg_"
#define CGEN_PROC_NUMBER_PREFIX "mompnum_"
#define CGEN_PROC_VALUE_PREFIX "mompval_"
#define CGEN_PROC_DOUBLE_PREFIX "mompdbl_"
#define CGEN_PROC_BLOCK_PREFIX "mompblo_"
#define CGEN_PROC_CONSTANTIDS_PREFIX "mompconstid_"
#define CGEN_PROC_CONSTANTITEMS_PREFIX "mompconstitems_"
#define CGEN_FUN_BLOCK_PREFIX "momfblo_"
#define CGEN_FUN_CONSTANTIDS_PREFIX "momfconstid_"
#define CGEN_FUN_CONSTANTITEMS_PREFIX "momfconstitems_"
#define CGEN_FUN_BLOCKIDS_PREFIX "momfblockid_"
#define CGEN_FUN_LOCVALIDS_PREFIX "momflocvalid_"
#define CGEN_FUN_LOCINTIDS_PREFIX "momflocintid_"
#define CGEN_FUN_LOCDBLIDS_PREFIX "momflocdblid_"
#define CGEN_FUN_CODE_PREFIX "momfuncod_"
#define CGEN_DROUTARR_PREFIX "momdroutarr_"
#define CGEN_MD5MOD_PREFIX "mommd5mod_"

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
  momitem_t *cgen_locksetitm;	// the hset of locked items
  momval_t cgen_modseqv;	// the sequence of routines
  momitem_t *cgen_globassocitm;	// global association item
  FILE *cgen_fil;
  char *cgen_filpath;
  char *cgen_filbase;
  char *cgen_tempath;
  struct
  {
    enum cgenroutkind_mom_en cgrout_kind;
    unsigned cgrout_blockcount;
    momitem_t *cgrout_routitm;	// the current routine.
    momitem_t *cgrout_associtm;	// local association item
    momitem_t *cgrout_blockhsetitm;	// an hashed set of blocks & *jump(fromblock,toblock) nodes
    momitem_t *cgrout_blockqueueitm;	// a queue of block items to be scanned
    momitem_t *cgrout_hsetintitm;	// hashed set of local integer (intptr_t) variables
    momitem_t *cgrout_hsetdblitm;	// hashed set of local floating-point (double) variables
    momitem_t *cgrout_hsetvalitm;	// hashed set of local value (momval_t) variables
  } cgen_rout;
  momtypenc_t cgen_restype;
  struct momout_st cgen_outhead;
  struct momout_st cgen_outbody;
};




const char *
mom_item_generate_jit_tfun_routine (momitem_t *itm, const momval_t jitnode)
{
  MOM_FATAL (MOMOUT_LITERAL
	     ("mom_item_generate_jit_routine unimplemented itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_SPACE (32),
	     MOMOUT_LITERAL ("jitcode="), MOMOUT_VALUE (jitnode), NULL);
#warning mom_item_generate_jit_routine unimplemented
}


static void cgen_error_mom_at (int lin, struct c_generator_mom_st *cgen, ...)
  __attribute__ ((sentinel, noreturn));

static void cgen_update_module_info_mom (struct c_generator_mom_st *cg);

static void
cgen_unlock_all_items_mom (struct c_generator_mom_st *cgen)
{
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  momval_t locksetv = mom_item_hset_items_set (cgen->cgen_locksetitm);
  unsigned nblockeditems = mom_set_cardinal (locksetv);
  for (unsigned ix = 0; ix < nblockeditems; ix++)
    {
      momitem_t *curlockeditm = mom_set_nth_item (locksetv, ix);
      mom_unlock_item (curlockeditm);
    }
  mom_item_clear_payload (cgen->cgen_locksetitm);
}

static void
cgen_lock_item_mom (struct c_generator_mom_st *cgen, momitem_t *curitm)
{
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  if (!curitm)
    return;
  assert (curitm->i_typnum == momty_item);
  momitem_t *locksetitm = cgen->cgen_locksetitm;
  assert (locksetitm && locksetitm->i_typnum == momty_item
	  && locksetitm->i_paylkind == mompayk_hset);
  if (mom_item_hset_contains (locksetitm, (momval_t) curitm))
    return;
  mom_should_lock_item (curitm);
  if (!mom_item_hset_add (locksetitm, (momval_t) curitm))
    MOM_FATAL (MOMOUT_LITERAL
	       ("cgen_lock_item failed to add into locksetitm="),
	       MOMOUT_ITEM ((const momitem_t *) locksetitm),
	       MOMOUT_LITERAL (" curitm="),
	       MOMOUT_ITEM ((const momitem_t *) curitm));
}

static void
cgen_error_mom_at (int lin, struct c_generator_mom_st *cgen, ...)
{
  va_list args;
  char *outbuf = NULL;
  size_t sizbuf = 0;
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  cgen_unlock_all_items_mom (cgen);
  FILE *fout = open_memstream (&outbuf, &sizbuf);
  if (!fout)
    MOM_FATAPRINTF ("failed to open stream for cgenerror %s:%d", __FILE__,
		    lin);
  struct momout_st mout;
  memset (&mout, 0, sizeof (mout));
  mom_initialize_output (&mout, fout, outf_comment);
  va_start (args, cgen);
  mom_outva_at (__FILE__, lin, &mout, args);
  va_end (args);
  fflush (fout);
  cgen->cgen_errmsg = (char *) MOM_GC_STRDUP ("cgen_error", outbuf);
  MOM_DEBUGPRINTF (gencod, "cgen_error_mom #%d: %s", lin, cgen->cgen_errmsg);
  if (MOM_IS_DEBUGGING (gencod))
    {
      MOM_OUT (&cgen->cgen_outhead, MOMOUT_FLUSH ());
      MOM_OUT (&cgen->cgen_outbody, MOMOUT_FLUSH ());
      MOM_DEBUG
	(gencod,
	 MOMOUT_LITERAL ("cgen_error"), MOMOUT_BACKTRACE (20),
	 MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("~*~*~cgen_outhead:"), MOMOUT_NEWLINE (),
	 MOMOUT_LITERALV ((const char *) cgen->cgen_outhead.mout_data),
	 MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("~*~*~cgen_outbody:"), MOMOUT_NEWLINE (),
	 MOMOUT_LITERALV ((const char *) cgen->cgen_outbody.mout_data),
	 MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
    }
  free (outbuf), outbuf = NULL;
  longjmp (cgen->cgen_jbuf, lin);
}

#define CGEN_ERROR_MOM_AT_BIS(Lin,Cgen,...) cgen_error_mom_at(Lin,Cgen,__VA_ARGS__,NULL)
#define CGEN_ERROR_MOM_AT(Lin,Cgen,...) CGEN_ERROR_MOM_AT_BIS(Lin,Cgen,__VA_ARGS__)
#define CGEN_ERROR_MOM(Cgen,...) CGEN_ERROR_MOM_AT(__LINE__,Cgen,__VA_ARGS__)


static void declare_routine_cgen (struct c_generator_mom_st *cgen,
				  unsigned routix);
static void emit_routine_cgen (struct c_generator_mom_st *cgen,
			       unsigned routix, momitem_t *routitm);

static momtypenc_t typenc_cgen (struct c_generator_mom_st *cg, bool check,
				momitem_t *typitm, momval_t fromv);

static void scan_taskletfunction_cgen (struct c_generator_mom_st *cgen,
				       momitem_t *routitm);

static void scan_procedure_cgen (struct c_generator_mom_st *cgen,
				 momitem_t *routitm);

static void scan_instr_cgen (struct c_generator_mom_st *cgen,
			     momitem_t *blkitm, momval_t insv, bool lastins);

static momtypenc_t scan_expr_cgen (struct c_generator_mom_st *cgen,
				   momval_t insv, momval_t expv);

static momtypenc_t scan_node_cgen (struct c_generator_mom_st *cg,
				   momval_t insv, const momnode_t *nod);

static momtypenc_t scan_item_cgen (struct c_generator_mom_st *cgen,
				   momitem_t *varitm);

static void scan_block_cgen (struct c_generator_mom_st *cgen,
			     momitem_t *blockitm, momval_t fromv,
			     momitem_t *fromblockitm);

static void loop_blocks_to_scan_cgen (struct c_generator_mom_st *cgen);

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

static void emit_moduleinit_cgen (struct c_generator_mom_st *cg);



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
    = mom_item_assoc_get ((Cg)->cgen_rout.cgrout_associtm,	\
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
  pthread_mutex_lock (&cgenmtx_mom);
  mycgen.cgen_moditm = moditm;
  if (!dirname || !dirname[0])
    dirname = ".";
  char *mydirname = (char *) MOM_GC_STRDUP ("cgen dirname", dirname);;
  mom_initialize_buffer_output (&mycgen.cgen_outhead, outf_jsonhalfindent);
  mom_initialize_buffer_output (&mycgen.cgen_outbody, outf_jsonhalfindent);
  jr = setjmp (mycgen.cgen_jbuf);
  if (jr)
    {
      MOM_DEBUGPRINTF (gencod, "generate_c_module got errored #%d: %s", jr,
		       mycgen.cgen_errmsg);
      assert (mycgen.cgen_errmsg != NULL);
      errno = 0;
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
  // initialize the set of locked items
  mycgen.cgen_locksetitm = mom_make_item ();
  mom_item_start_hset (mycgen.cgen_locksetitm);
  mom_item_hset_reserve (mycgen.cgen_locksetitm, 250);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("generate_c_module locksetitm:"),
	     MOMOUT_ITEM ((const momitem_t *) mycgen.cgen_locksetitm), NULL);
  cgen_lock_item_mom (&mycgen, mycgen.cgen_locksetitm);
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
    cgen_lock_item_mom (&mycgen, moditm);
    modroutv = mom_item_get_attribute (moditm, mom_named__module_routines);
    if (!mom_is_set (modroutv))
      CGEN_ERROR_MOM (&mycgen, MOMOUT_LITERAL ("generate_c_module module:"),
		      MOMOUT_ITEM ((const momitem_t *) moditm),
		      MOMOUT_SPACE (48),
		      MOMOUT_LITERAL ("has unexpected module_routines:"),
		      MOMOUT_VALUE (modroutv));
  }
  unsigned nbmodrout = mom_set_cardinal (modroutv);
  mycgen.cgen_modseqv = modroutv;
  {
    char basbuf[128];
    snprintf (basbuf, sizeof (basbuf), MOM_SHARED_MODULE_PREFIX "%s.c",
	      mom_ident_cstr_of_item (moditm));
    mycgen.cgen_filbase = (char *) MOM_GC_STRDUP ("base file", basbuf);
  }
  mycgen.cgen_globassocitm = mom_make_item ();
  mom_item_put_attribute
    (mycgen.cgen_globassocitm,
     mom_named__comment,
     MOM_OUTSTRING (0, MOMOUT_LITERAL ("global association for module "),
		    MOMOUT_ITEM ((const momitem_t *) moditm), NULL));
  mom_item_start_assoc (mycgen.cgen_globassocitm);
  cgen_lock_item_mom (&mycgen, mycgen.cgen_globassocitm);
  /// start the head part
  MOM_OUT (&mycgen.cgen_outhead,
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("//// header part"),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("#include \"monimelt.h\""),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("////++++ declaration of "),
	   MOMOUT_DEC_INT ((int) nbmodrout), MOMOUT_LITERAL (" routines:"),
	   MOMOUT_NEWLINE (), NULL);
  /// start the body part
  MOM_OUT (&mycgen.cgen_outbody,
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("//// body part"),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("////++++ implementation of "),
	   MOMOUT_DEC_INT ((int) nbmodrout), MOMOUT_LITERAL (" routines:"),
	   MOMOUT_NEWLINE (), NULL);

  /// iterate on the set of module routines to declare them
  for (unsigned routix = 0; routix < nbmodrout; routix++)
    {
      momitem_t *curoutitm = mom_set_nth_item (modroutv, routix);
      mycgen.cgen_rout.cgrout_routitm = curoutitm;
      declare_routine_cgen (&mycgen, routix);
      mycgen.cgen_rout.cgrout_routitm = NULL;
    }
  /// iterate on the set of module routines to generate them
  nbmodrout = mom_set_cardinal (modroutv);
  for (unsigned routix = 0; routix < nbmodrout; routix++)
    {
      momitem_t *curoutitm = mom_set_nth_item (modroutv, routix);
      memset (&mycgen.cgen_rout, 0, sizeof (mycgen.cgen_rout));
      MOM_OUT (&mycgen.cgen_outbody,
	       MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
      emit_routine_cgen (&mycgen, routix, curoutitm);
      memset (&mycgen.cgen_rout, 0, sizeof (mycgen.cgen_rout));
    }
  MOM_OUT (&mycgen.cgen_outbody, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  // emit module initialization
  emit_moduleinit_cgen (&mycgen);

  MOM_DEBUG			////
    (gencod,
     MOMOUT_LITERAL
     ("finished module generation, need to update the routines info"),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" globassocitm="),
     MOMOUT_ITEM ((const momitem_t *) mycgen.cgen_globassocitm),
     MOMOUT_NEWLINE (),
     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) mycgen.cgen_globassocitm),
     MOMOUT_NEWLINE (),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) mycgen.cgen_globassocitm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" moditm="),
     MOMOUT_ITEM ((const momitem_t *) moditm), MOMOUT_NEWLINE (),
     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) moditm), NULL);

  cgen_update_module_info_mom (&mycgen);


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
	     "// MONIMELT module %s\n// generated file "
	     MOM_SHARED_MODULE_PREFIX "%s.c\n"
	     "///**** DO NOT EDIT! ***",
	     mom_string_cstr ((momval_t) mom_item_get_name_or_idstr (moditm)),
	     mom_ident_cstr_of_item (moditm));
    struct momout_st outf = { 0 };
    mom_initialize_output (&outf, f, 0);
    fflush (mycgen.cgen_outhead.mout_file);
    fflush (mycgen.cgen_outbody.mout_file);
    MOM_OUT (&outf,
	     MOMOUT_NEWLINE (),
	     MOMOUT_GPLV3P_NOTICE ((const char *) mycgen.cgen_filbase),
	     MOMOUT_NEWLINE (), MOMOUT_FLUSH (), MOMOUT_NEWLINE (),
	     MOMOUT_NEWLINE (), NULL);
    MOM_OUT (&outf, MOMOUT_LITERAL ("//// **** head of "),
	     MOMOUT_LITERALV ((const char *) mycgen.cgen_filbase),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	     MOMOUT_LITERALV ((const char *) mycgen.cgen_outhead.mout_data),
	     MOMOUT_NEWLINE (), MOMOUT_FLUSH (), MOMOUT_NEWLINE (),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
    MOM_OUT (&outf, MOMOUT_LITERAL ("//// **** body of "),
	     MOMOUT_LITERALV ((const char *) mycgen.cgen_filbase),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	     MOMOUT_LITERALV ((const char *) mycgen.cgen_outbody.mout_data),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), MOMOUT_FLUSH (),
	     MOMOUT_NEWLINE (), NULL);
    MOM_OUT (&outf, MOMOUT_LITERAL ("//// **** license info of "),
	     MOMOUT_LITERALV ((const char *) mycgen.cgen_filbase),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("const char mom_module_GPL_compatible[]="),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("   \"GPLv3+, generated module "),
	     MOMOUT_ITEM ((const momitem_t *) moditm),
	     MOMOUT_LITERAL ("; commit \" MONIMELT_LAST_COMMITID;"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("///// for Emacs :::"), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("////=== Local Variables:"), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("////=== mode: C"), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("////=== End:"), MOMOUT_NEWLINE (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("//// **** eof " MOM_SHARED_MODULE_PREFIX),
	     MOMOUT_LITERALV ((const char *) mom_ident_cstr_of_item (moditm)),
	     MOMOUT_LITERAL (".c"), MOMOUT_NEWLINE (), MOMOUT_FLUSH (), NULL);
    if (fclose (f))
      MOM_FATAPRINTF ("failed to close module temporary file %s", mtempnam);
    mom_rename_if_content_changed (mtempnam, modnam);
    mom_finalize_buffer_output (&mycgen.cgen_outhead);
    mom_finalize_buffer_output (&mycgen.cgen_outbody);
    MOM_INFORMPRINTF ("generated module file %s", modnam);
  }
  /// at last
  cgen_unlock_all_items_mom (&mycgen);
  pthread_mutex_unlock (&cgenmtx_mom);
  return jr;
}




#define PROCROUTSIG_PREFIX_MOM "momprocsig_"
void
declare_routine_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momval_t formalsv = MOM_NULLV;
  momval_t procv = MOM_NULLV;
  momval_t tfunv = MOM_NULLV;
  momval_t resultv = MOM_NULLV;
  momval_t procrestypev = MOM_NULLV;
  momval_t commv = MOM_NULLV;
  momitem_t *curoutitm = cg->cgen_rout.cgrout_routitm;
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("declare_routine curoutitm="),
	     MOMOUT_ITEM ((const momitem_t *) curoutitm),
	     MOMOUT_LITERAL (" routix#"), MOMOUT_DEC_INT ((int) routix),
	     NULL);
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
    cgen_lock_item_mom (cg, curoutitm);
    procv = mom_item_get_attribute (curoutitm, mom_named__procedure);
    tfunv = mom_item_get_attribute (curoutitm, mom_named__tasklet_function);
    formalsv = mom_item_get_attribute (curoutitm, mom_named__formals);
    resultv = mom_item_get_attribute (curoutitm, mom_named__result);
    commv = mom_item_get_attribute (curoutitm, mom_named__comment);
  }
  if (tfunv.ptr && procv.ptr)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("routine:"),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm),
		    MOMOUT_LITERAL (" rank#"),
		    MOMOUT_DEC_INT ((int) routix),
		    MOMOUT_LITERAL
		    (" cannot have both `procedure` and `tasklet_function` attributes."));
  if (!tfunv.ptr && !procv.ptr)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("routine:"),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm),
		    MOMOUT_LITERAL (" rank#"),
		    MOMOUT_DEC_INT ((int) routix),
		    MOMOUT_LITERAL
		    (" should have one of `procedure` or `tasklet_function` attribute."));
  if (mom_is_string (commv))
    MOM_OUT (&cg->cgen_outhead, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("///~~~declare routine #"),
	     MOMOUT_DEC_INT ((int) routix),
	     MOMOUT_NEWLINE (),
	     MOMOUT_SLASHCOMMENT_STRING (mom_string_cstr (commv)), NULL);
  else
    MOM_OUT (&cg->cgen_outhead, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("///+++declare routine #"),
	     MOMOUT_DEC_INT ((int) routix), MOMOUT_NEWLINE (), NULL);
  if (procv.ptr)
    {
      momval_t argsigv = MOM_NULLV;
      // genuine procedure
      MOM_OUT (&cg->cgen_outhead,
	       MOMOUT_LITERAL ("// declare procedure "),
	       MOMOUT_ITEM ((const momitem_t *) curoutitm),
	       MOMOUT_NEWLINE (), NULL);
      if (!mom_is_tuple (formalsv))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure formals:"),
			MOMOUT_VALUE ((const momval_t) formalsv),
			MOMOUT_LITERAL (" in procedure "),
			MOMOUT_ITEM ((const momitem_t *) curoutitm), NULL);
      unsigned nbargs = mom_tuple_length (formalsv);
      if (mom_is_item (resultv))
	{
	  momitem_t *procresitm = resultv.pitem;
	  cgen_lock_item_mom (cg, procresitm);
	  procrestypev = mom_item_get_attribute (curoutitm, mom_named__ctype);
	}
      else if (!resultv.ptr)
	{
	  procrestypev = (momval_t) mom_named__void;
	}
      else
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid procedure result:"),
			MOMOUT_VALUE ((const momval_t) resultv),
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
	  momitem_t *curargitm = mom_tuple_nth_item (formalsv, aix);
	  momval_t curargtypv = MOM_NULLV;
	  if (!curargitm || curargitm->i_typnum != momty_item)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid procedure formal argument:"),
			    MOMOUT_VALUE ((const momval_t) curargitm),
			    MOMOUT_LITERAL (" in procedure "),
			    MOMOUT_ITEM ((const momitem_t *) curoutitm),
			    MOMOUT_LITERAL (" rank "),
			    MOMOUT_DEC_INT ((int) aix), NULL);
	  cgen_lock_item_mom (cg, curargitm);
	  curargtypv =
	    mom_item_get_attribute (curargitm, mom_named__variable);
	  MOM_DEBUG (gencod, MOMOUT_LITERAL ("declare_routine in procedure "),
		     MOMOUT_VALUE ((const momval_t) curargitm),
		     MOMOUT_LITERAL (" rank "), MOMOUT_DEC_INT ((int) aix),
		     MOMOUT_LITERAL (" curargtypv "),
		     MOMOUT_VALUE ((const momval_t) curargtypv), NULL);
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
      MOM_OUT (&cg->cgen_outhead, MOMOUT_LITERAL (");"), MOMOUT_NEWLINE (),
	       MOMOUT_NEWLINE ());
      momval_t anodv =
	(momval_t) mom_make_node_sized ((momval_t) mom_named__procedure, 3,
					mom_make_integer (routix),
					argsigv, procrestypev);
      mom_item_assoc_put (cg->cgen_globassocitm, curoutitm, anodv);
      MOM_DEBUG (gencod,
		 MOMOUT_LITERAL
		 ("declare_routine associating proc curoutitm="),
		 MOMOUT_ITEM ((const momitem_t *) curoutitm),
		 MOMOUT_LITERAL (" to "),
		 MOMOUT_VALUE ((const momval_t) anodv), NULL);
    }
  else
    {
      // genuine tasklet function
      MOM_OUT (&cg->cgen_outhead,
	       MOMOUT_LITERAL ("// declare tasklet function "),
	       MOMOUT_ITEM ((const momitem_t *) curoutitm),
	       MOMOUT_NEWLINE (), NULL);
      MOM_OUT (&cg->cgen_outhead,
	       MOMOUT_LITERAL ("static int"), MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" " CGEN_FUN_CODE_PREFIX),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (curoutitm)),
	       MOMOUT_LITERAL
	       ("(int, momitem_t*, momval_t, momval_t*, intptr_t*, double*);"),
	       MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
      momval_t anodv = (momval_t)
	mom_make_node_sized ((momval_t) mom_named__tasklet_function, 1,
			     mom_make_integer (routix));
      MOM_DEBUG (gencod,
		 MOMOUT_LITERAL
		 ("declare_routine associating tfun curoutitm="),
		 MOMOUT_ITEM ((const momitem_t *) curoutitm),
		 MOMOUT_LITERAL (" to "),
		 MOMOUT_VALUE ((const momval_t) anodv), NULL);
      mom_item_assoc_put (cg->cgen_globassocitm, curoutitm, anodv);
    }
}

void
emit_routine_cgen (struct c_generator_mom_st *cg, unsigned routix,
		   momitem_t *curoutitm)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (curoutitm && curoutitm->i_typnum == momty_item);
  momval_t curoutval = mom_item_assoc_get (cg->cgen_globassocitm, curoutitm);
  const momitem_t *curoutconnitm = mom_node_conn (curoutval);
  assert (curoutconnitm && curoutconnitm->i_typnum == momty_item);
  cg->cgen_rout.cgrout_routitm = curoutitm;
  /// associtm
  cg->cgen_rout.cgrout_associtm = mom_make_item ();
  mom_item_put_attribute
    (cg->cgen_rout.cgrout_associtm,
     mom_named__comment,
     MOM_OUTSTRING (0, MOMOUT_LITERAL ("assoc-item for "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm)));
  mom_item_start_assoc (cg->cgen_rout.cgrout_associtm);
  cgen_lock_item_mom (cg, cg->cgen_rout.cgrout_associtm);
  /// blockhsetitm
  cg->cgen_rout.cgrout_blockhsetitm = mom_make_item ();
  mom_item_start_hset (cg->cgen_rout.cgrout_blockhsetitm);
  cgen_lock_item_mom (cg, cg->cgen_rout.cgrout_blockhsetitm);
  mom_item_put_attribute
    (cg->cgen_rout.cgrout_blockhsetitm,
     mom_named__comment,
     MOM_OUTSTRING (0, MOMOUT_LITERAL ("block-hset-item for "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm)));
  /// blockqueue
  cg->cgen_rout.cgrout_blockqueueitm = mom_make_item ();
  mom_item_start_queue (cg->cgen_rout.cgrout_blockqueueitm);
  cgen_lock_item_mom (cg, cg->cgen_rout.cgrout_blockqueueitm);
  mom_item_put_attribute
    (cg->cgen_rout.cgrout_blockqueueitm,
     mom_named__comment,
     MOM_OUTSTRING (0, MOMOUT_LITERAL ("block-queue-item for "),
		    MOMOUT_ITEM ((const momitem_t *) curoutitm)));
  ///// ========== the local variable sets
  /// local ints
  {
    momitem_t *hsintitm = cg->cgen_rout.cgrout_hsetintitm = mom_make_item ();
    mom_item_start_hset (hsintitm);
    mom_item_put_attribute
      (hsintitm,
       mom_named__comment,
       MOM_OUTSTRING (0, MOMOUT_LITERAL ("local ints hset for "),
		      MOMOUT_ITEM ((const momitem_t *) curoutitm)));
    MOM_DEBUG			//
      (gencod, MOMOUT_LITERAL ("emit_routine hsetintitm="),
       MOMOUT_ITEM ((const momitem_t *) hsintitm),
       MOMOUT_NEWLINE (),
       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) hsintitm),
       MOMOUT_FLUSH (), NULL);
    cgen_lock_item_mom (cg, hsintitm);
    mom_item_hset_check_integrity ("emit_routine hsintitm", hsintitm);
  }
  /// local dbls
  {
    momitem_t *hsdblitm = cg->cgen_rout.cgrout_hsetdblitm = mom_make_item ();
    mom_item_start_hset (hsdblitm);
    mom_item_put_attribute
      (hsdblitm,
       mom_named__comment,
       MOM_OUTSTRING (0, MOMOUT_LITERAL ("local dbls hset for "),
		      MOMOUT_ITEM ((const momitem_t *) curoutitm)));
    MOM_DEBUG			//
      (gencod, MOMOUT_LITERAL ("emit_routine hsetdblitm="),
       MOMOUT_ITEM ((const momitem_t *) hsdblitm),
       MOMOUT_NEWLINE (),
       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) hsdblitm),
       MOMOUT_FLUSH (), NULL);
    cgen_lock_item_mom (cg, hsdblitm);
    mom_item_hset_check_integrity ("emit_routine hsetdblitm", hsdblitm);
  }
  /// local vals
  {
    momitem_t *hsvalitm = cg->cgen_rout.cgrout_hsetvalitm = mom_make_item ();
    mom_item_start_hset (hsvalitm);
    mom_item_put_attribute
      (hsvalitm,
       mom_named__comment,
       MOM_OUTSTRING (0, MOMOUT_LITERAL ("local vals hset for "),
		      MOMOUT_ITEM ((const momitem_t *) curoutitm)));
    MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_routine hsetvalitm="),
	       MOMOUT_ITEM ((const momitem_t *) hsvalitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) hsvalitm),
	       MOMOUT_FLUSH (), NULL);
    cgen_lock_item_mom (cg, hsvalitm);
    mom_item_hset_check_integrity ("emit_routine hsetvalitm", hsvalitm);
  }
  // starting
  MOM_DEBUG			//
    (gencod, MOMOUT_LITERAL ("emit_routine curoutitm="),
     MOMOUT_ITEM ((const momitem_t *) curoutitm),
     MOMOUT_LITERAL (" routix#"), MOMOUT_DEC_INT ((int) routix),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" locassoc:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" blockhset:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_blockhsetitm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" blockqueue:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_blockqueueitm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" hsetint:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetintitm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" hsetdbl:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetdblitm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" hsetval:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetvalitm),
     MOMOUT_NEWLINE (), NULL);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("//===***===***=== start routine #"),
	   MOMOUT_DEC_INT ((int) routix),
	   MOMOUT_LITERAL (": "),
	   MOMOUT_ITEM ((const momitem_t *) curoutitm),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  if (curoutconnitm == mom_named__procedure)
    {
      cg->cgen_rout.cgrout_kind = cgr_proc;
      scan_procedure_cgen (cg, curoutitm);
      emit_procedure_cgen (cg, routix);
    }
  else if (curoutconnitm == mom_named__tasklet_function)
    {
      cg->cgen_rout.cgrout_kind = cgr_funt;
      scan_taskletfunction_cgen (cg, curoutitm);
      emit_taskletfunction_cgen (cg, routix);
    }
  else
    MOM_FATAL (MOMOUT_LITERAL ("invalid curoutconnitm:"),
	       MOMOUT_ITEM ((const momitem_t *) curoutconnitm));
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_routine done curoutitm="),
	     MOMOUT_ITEM ((const momitem_t *) curoutitm));
  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  cg->cgen_rout.cgrout_associtm = NULL;
  cg->cgen_rout.cgrout_kind = cgr__none;
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
	    (cg->cgen_rout.cgrout_associtm, constitm,
	     (momval_t) mom_make_node_sized ((momval_t) mom_named__constants,
					     2, mom_make_integer (cix),
					     (momval_t) constitm));
	}
    }
  else if (constantsv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid constants:"),
		    MOMOUT_VALUE ((const momval_t) constantsv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->
				 cgen_rout.cgrout_routitm), NULL);
  return nbconsts;
}

static unsigned
bind_closed_values_cgen (struct c_generator_mom_st *cg, momval_t closvalsv)
{
  unsigned nbclosvals = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (mom_is_seqitem (closvalsv))
    {
      nbclosvals = mom_seqitem_length (closvalsv);
      for (unsigned cix = 0; cix < nbclosvals; cix++)
	{
	  momitem_t *clositm = mom_seqitem_nth_item (closvalsv, cix);
	  if (!clositm)
	    continue;
	  CGEN_CHECK_FRESH (cg, "closed value in routine", clositm);
	  mom_item_assoc_put
	    (cg->cgen_rout.cgrout_associtm, clositm,
	     (momval_t) mom_make_node_sized ((momval_t)
					     mom_named__closed_values, 2,
					     mom_make_integer (cix),
					     (momval_t) clositm));
	}
    }
  else if (closvalsv.ptr)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("invalid closed values:"),
		    MOMOUT_VALUE ((const momval_t) closvalsv),
		    MOMOUT_LITERAL (" in routine "),
		    MOMOUT_ITEM ((const momitem_t *) cg->
				 cgen_rout.cgrout_routitm), NULL);
  return nbclosvals;
}




void
scan_procedure_cgen (struct c_generator_mom_st *cg, momitem_t *procitm)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (procitm && procitm->i_typnum == momty_item);
  cgen_lock_item_mom (cg, procitm);
  momval_t procnodev = mom_item_assoc_get (cg->cgen_globassocitm, procitm);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_procedure procitm="),
	     MOMOUT_ITEM ((const momitem_t *) procitm), NULL);
  assert (mom_node_conn (procnodev) == mom_named__procedure);
  momval_t proformalsv = MOM_NULLV;
  momval_t proconstantsv = MOM_NULLV;
  momval_t proprocedurev = MOM_NULLV;
  momval_t proresultv = MOM_NULLV;
  assert (mom_node_conn (procnodev) == mom_named__procedure
	  && mom_node_arity (procnodev) == 3);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_procedure procitm="),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_SPACE (40),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) procitm), NULL);
  proformalsv = mom_item_get_attribute (procitm, mom_named__formals);
  proconstantsv = mom_item_get_attribute (procitm, mom_named__constants);
  proprocedurev = mom_item_get_attribute (procitm, mom_named__procedure);
  proresultv = mom_item_get_attribute (procitm, mom_named__result);
  if (proformalsv.ptr && !mom_is_tuple (proformalsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
		    MOMOUT_ITEM ((const momitem_t *) procitm),
		    MOMOUT_LITERAL (" bad formals:"),
		    MOMOUT_VALUE (proformalsv), NULL);
  unsigned nbformals = mom_tuple_length (proformalsv);
  for (unsigned fix = 0; fix < nbformals; fix++)
    {
      momitem_t *curformitm = mom_tuple_nth_item (proformalsv, fix);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_procedure procitm="),
		 MOMOUT_ITEM ((const momitem_t *) procitm),
		 MOMOUT_LITERAL (" curformitm="),
		 MOMOUT_ITEM ((const momitem_t *) curformitm), NULL);
      if (!curformitm)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("nil formal in procedure:"),
			MOMOUT_ITEM ((const momitem_t *) procitm),
			MOMOUT_LITERAL (" with formals:"),
			MOMOUT_VALUE (proformalsv), NULL);
      CGEN_CHECK_FRESH (cg, "formal of procedure", curformitm);
      cgen_lock_item_mom (cg, curformitm);
      momval_t formtypv =
	mom_item_get_attribute (curformitm, mom_named__variable);
      if (!formtypv.ptr)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("missing `variable` in formal:"),
			MOMOUT_ITEM ((const momitem_t *) curformitm),
			MOMOUT_LITERAL (" of procedure:"),
			MOMOUT_ITEM ((const momitem_t *) procitm), NULL);
      mom_item_assoc_put
	(cg->cgen_rout.cgrout_associtm, curformitm,
	 (momval_t) mom_make_node_sized ((momval_t) mom_named__formals, 3,
					 (momval_t) procitm,
					 mom_make_integer (fix), formtypv));
    }
  /// scan the constants of procedure
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_procedure procitm="),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_LITERAL (" proconstantsv="),
	     MOMOUT_VALUE ((const momval_t) proconstantsv), NULL);
  if (proconstantsv.ptr && !mom_is_seqitem (proconstantsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
		    MOMOUT_ITEM ((const momitem_t *) procitm),
		    MOMOUT_LITERAL (" bad constants:"),
		    MOMOUT_VALUE (proconstantsv), NULL);
  unsigned nbconstants = bind_constants_cgen (cg, proconstantsv);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_procedure procitm="),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_LITERAL (" nbconstants#"),
	     MOMOUT_DEC_INT ((int) nbconstants),
	     MOMOUT_LITERAL (" proprocedurev="),
	     MOMOUT_VALUE ((const momval_t) proprocedurev), NULL);
  if (!mom_is_item (proprocedurev))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
		    MOMOUT_ITEM ((const momitem_t *) procitm),
		    MOMOUT_LITERAL (" non-item starting block `procedure`:"),
		    MOMOUT_VALUE (proprocedurev), NULL);
  scan_block_cgen (cg, proprocedurev.pitem, (momval_t) procitm, NULL);
  loop_blocks_to_scan_cgen (cg);
  momval_t ixv = mom_node_nth (procnodev, 0);
  assert (mom_is_integer (ixv));
  momval_t argsigv = mom_node_nth (procnodev, 1);
  momval_t restypv = mom_node_nth (procnodev, 2);
  momval_t newanodv =		//
    (momval_t) mom_make_node_sized ((momval_t) mom_named__procedure, 4,
				    ixv, argsigv, restypv,
				    (momval_t)
				    cg->cgen_rout.cgrout_blockhsetitm);
  mom_item_assoc_put (cg->cgen_rout.cgrout_associtm, procitm, newanodv);
  assert (mom_is_item (restypv));
  if (proresultv.ptr)
    {
      if (!mom_is_item (proresultv))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
			MOMOUT_ITEM ((const momitem_t *) procitm),
			MOMOUT_LITERAL (" non-item result result:"),
			MOMOUT_VALUE ((const momval_t) proresultv), NULL);

      MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_proc proresultv="),
		 MOMOUT_VALUE ((const momval_t) proresultv),
		 MOMOUT_LITERAL (" for procitm="),
		 MOMOUT_ITEM ((const momitem_t *) procitm), NULL);
      momtypenc_t tres = scan_item_cgen (cg, proresultv.pitem);
      momtypenc_t trestyp =
	typenc_cgen (cg, true, restypv.pitem, (momval_t) procitm);
      if (tres > momtypenc__none && trestyp > momtypenc__none
	  && trestyp != tres)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
			MOMOUT_ITEM ((const momitem_t *) procitm),
			MOMOUT_LITERAL (" type mismatch for result:"),
			MOMOUT_VALUE ((const momval_t) restypv), NULL);
    }
  MOM_DEBUG (gencod,
	     MOMOUT_LITERAL
	     ("scan_procedure associating procitm="),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_LITERAL (" to "),
	     MOMOUT_VALUE ((const momval_t) newanodv), NULL);

}

static void
loop_blocks_to_scan_cgen (struct c_generator_mom_st *cg)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (mom_item_payload_kind (cg->cgen_rout.cgrout_blockqueueitm) ==
	  mompayk_queue);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("loop_blocks remaining block count="),
	     MOMOUT_DEC_INT ((int) mom_item_queue_length
			     (cg->cgen_rout.cgrout_blockqueueitm)), NULL);
  while (!mom_item_queue_is_empty (cg->cgen_rout.cgrout_blockqueueitm))
    {
      momitem_t *curblkitm =
	mom_value_to_item (mom_item_queue_pop_front
			   (cg->cgen_rout.cgrout_blockqueueitm));
      assert (curblkitm && curblkitm->i_typnum == momty_item);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("loop_blocks curblkitm="),
		 MOMOUT_ITEM ((const momitem_t *) curblkitm), NULL);
      momval_t blcodev = mom_item_get_attribute (curblkitm, mom_named__block);
      assert (mom_node_conn (blcodev) == mom_named__code);
      unsigned nbinstr = mom_node_arity (blcodev);
      for (unsigned ix = 0; ix < nbinstr; ix++)
	{
	  momval_t curinsv = mom_node_nth (blcodev, ix);
	  scan_instr_cgen (cg, curblkitm, curinsv, ix + 1 >= nbinstr);
	}
    }
}




static momtypenc_t
typenc_cgen (struct c_generator_mom_st *cg, bool check, momitem_t *typitm,
	     momval_t fromv)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
#define TYPEHASHMAX_MOM 103
#define TYPECASE(Nam) case mom_hashname_##Nam % TYPEHASHMAX_MOM: if (typitm != mom_named_##Nam) break;
  if (typitm)
    {
      switch (mom_item_hash (typitm) % TYPEHASHMAX_MOM)
	{
	  TYPECASE (_intptr_t);
	  return momtypenc_int;
	  TYPECASE (_double);
	  return momtypenc_double;
	  TYPECASE (_momval_t);
	  return momtypenc_val;
	  TYPECASE (_momcstr_t);
	  return momtypenc_string;
	  TYPECASE (_void);
	  return momtypenc__none;
	default:
	  if (!check)
	    return momtypenc__none;
	}
#undef TYPEHASHMAX_MOM
#undef TYPECASE
    }
  if (check)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL ("bad type:"),
		    MOMOUT_ITEM ((const momitem_t *) typitm),
		    MOMOUT_LITERAL (" from:"),
		    MOMOUT_VALUE ((const momval_t) fromv), NULL);
  return momtypenc__none;
}


void
scan_output_cgen (struct c_generator_mom_st *cg, momval_t insv, momval_t outv)
{
  int errline = 0;
  const char *errmsg = NULL;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  char errbuf[96];
  memset (errbuf, 0, sizeof (errbuf));
#define OUTSCANFAIL(Fmt,...) do {				\
    snprintf(errbuf, sizeof(errbuf), Fmt, ##__VA_ARGS__);	\
    errmsg = errbuf;						\
    errline = __LINE__;						\
    goto bad_output;						\
  } while(0)
  ///

  MOM_DEBUG			//
    (gencod,
     MOMOUT_LITERAL ("scan_output start outv="),
     MOMOUT_VALUE ((const momval_t) outv), NULL);
  if (mom_is_node (outv))
    {
      const momitem_t *oconnitm = mom_node_conn (outv);
      cgen_lock_item_mom (cg, (momitem_t *) oconnitm);
      momval_t outformalsv =
	mom_item_get_attribute (oconnitm, mom_named__formals);
      momval_t outexpv =
	mom_item_get_attribute (oconnitm, mom_named__output_expansion);
      momval_t outoutputv =
	mom_item_get_attribute (oconnitm, mom_named__output);
      if (!outexpv.ptr && !outoutputv.ptr)
	{
	  // output node which is a usual expression node
	  scan_node_cgen (cg, insv, outv.pnode);
	  return;
	}
      if (!mom_is_tuple (outformalsv)
	  || mom_node_conn (outexpv) != mom_named__chunk)
	OUTSCANFAIL ("bad output connective %s",
		     mom_item_get_name_or_id_cstr (oconnitm));
      unsigned nbformals = mom_tuple_length (outformalsv);
      unsigned nbargs = mom_node_arity (outv);
      if (nbformals != nbargs)
	OUTSCANFAIL
	  ("output connective %s arity mismatch, want %d got %d args",
	   mom_item_get_name_or_id_cstr (oconnitm), nbformals, nbargs);
      for (unsigned ix = 0; ix < nbargs; ix++)
	{
	  momitem_t *outformitm = mom_tuple_nth_item (outformalsv, ix);
	  momval_t outargv = mom_node_nth (outv, ix);
	  if (!outformitm)
	    OUTSCANFAIL ("output connective %s missing formal#%d",
			 mom_item_get_name_or_id_cstr (oconnitm), ix);
	  cgen_lock_item_mom (cg, (momitem_t *) outformitm);
	  momval_t outypv =
	    mom_item_get_attribute (outformitm, mom_named__variable);
	  if (!mom_is_item (outypv))
	    OUTSCANFAIL
	      ("output connective %s formal #%d %s with bad `variable` :: type",
	       mom_item_get_name_or_id_cstr (oconnitm), ix,
	       mom_item_get_name_or_id_cstr (outformitm));
	  momtypenc_t tparam = typenc_cgen (cg, true, outypv.pitem, outv);
	  momtypenc_t targ = scan_expr_cgen (cg, insv, outargv);
	  if (tparam > momtypenc__none && targ > momtypenc__none
	      && targ != tparam)
	    OUTSCANFAIL ("output connective %s type mismatch for arg#%d",
			 mom_item_get_name_or_id_cstr (oconnitm), ix);
	}
      return;
    }
  else if (mom_is_integer (outv))
    return;
  else if (mom_is_double (outv))
    return;
  else if (mom_is_string (outv))
    return;
  else if (mom_is_item (outv))
    {
      scan_item_cgen (cg, outv.pitem);
      return;
    }
  else
    OUTSCANFAIL ("bad output");
bad_output:
  assert (errline > 0);
  cgen_error_mom_at (errline, cg,
		     MOMOUT_LITERAL ("in scan_output outv="),
		     MOMOUT_VALUE ((const momval_t) outv),
		     MOMOUT_LITERAL (" in instr:"),
		     MOMOUT_VALUE ((const momval_t) insv),
		     MOMOUT_LITERAL (" got error "),
		     MOMOUT_LITERALV ((const char *) errmsg), NULL);
#undef OUTSCANFAIL

}

static momtypenc_t
scan_node_cgen (struct c_generator_mom_st *cg, momval_t insv,
		const momnode_t *nod)
{
  int errline = 0;
  const char *errmsg = NULL;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (nod && nod->typnum == momty_node);
  momval_t nodv = (momval_t) nod;
  const momitem_t *connitm = mom_node_conn (nodv);
  unsigned arity = mom_node_arity (nodv);
  cgen_lock_item_mom (cg, (momitem_t *) connitm);
  char errbuf[96];
  memset (errbuf, 0, sizeof (errbuf));
#define NODESCANFAIL(Fmt,...) do {			\
  snprintf(errbuf, sizeof(errbuf), Fmt, ##__VA_ARGS__);	\
  errmsg = errbuf;					\
  errline = __LINE__;					\
  goto bad_node;					\
} while(0)
#define SCANODHASHMAX_MOM 131
#define SCANCASE(Nam) case mom_hashname_##Nam % SCANODHASHMAX_MOM: if (connitm != mom_named_##Nam) goto other_node;
  momhash_t conh = mom_item_hash (connitm);
  switch (conh % SCANODHASHMAX_MOM)
    {
      //////
      SCANCASE (_tuple);	// *tuple(<expr>...)
      SCANCASE (_set);		// *set(<expr>...)
      SCANCASE (_json_array);	// *json_array(<expr>...)
      {
	for (unsigned ix = 0; ix < arity; ix++)
	  {
	    momval_t sonv = mom_node_nth (nodv, ix);
	    momtypenc_t ts = scan_expr_cgen (cg, insv, sonv);
	    if (ts > momtypenc__none && ts != momtypenc_val)
	      CGEN_ERROR_MOM (cg,
			      MOMOUT_LITERAL
			      ("non-value expression in variadic constructor:"),
			      MOMOUT_VALUE ((const momval_t) nodv),
			      MOMOUT_LITERAL (" at index#"),
			      MOMOUT_DEC_INT ((int) ix),
			      MOMOUT_LITERAL (" in instr:"),
			      MOMOUT_VALUE ((const momval_t) insv), NULL);

	  }
      }
      break;
      //////
      SCANCASE (_json_object);	//  *json_object(<entry>...)
      {
	for (unsigned ix = 0; ix < arity; ix++)
	  {
	    momval_t entrv = mom_node_nth (nodv, ix);
	    const momitem_t *entitm = mom_node_conn (entrv);
	    unsigned entarity = mom_node_arity (entrv);
	    if (entitm == mom_named__json_entry && entarity == 2)
	      {
		// *json_entry(<name>,<value>)
		momtypenc_t tnam =
		  scan_expr_cgen (cg, insv, mom_node_nth (entrv, 0));
		momtypenc_t tval =
		  scan_expr_cgen (cg, insv, mom_node_nth (entrv, 1));
		if (tnam > momtypenc__none && tnam != momtypenc_string
		    && tnam != momtypenc_val && tval > momtypenc__none
		    && tval != momtypenc_val)
		  goto bad_json_entry;
	      }
	    else if (entitm == mom_named__json_object && entarity == 1)
	      {
		// *json_object(<expr>)
		momtypenc_t texp =
		  scan_expr_cgen (cg, insv, mom_node_nth (entrv, 0));
		if (texp > momtypenc__none && texp != momtypenc_val)
		  goto bad_json_entry;
	      }
	    else
	    bad_json_entry:
	      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad entry #"),
			      MOMOUT_DEC_INT ((int) ix),
			      MOMOUT_LITERAL (" in json_object node "),
			      MOMOUT_VALUE ((const momval_t) nodv),
			      MOMOUT_LITERAL (" in instr:"),
			      MOMOUT_VALUE ((const momval_t) insv), NULL);
	  }
      }
      break;
      /////
      SCANCASE (_node);		// *node(<conn-expr>,<arg-expr>...)
      {
	if (arity > 0)
	  {
	    for (unsigned ix = 0; ix < arity; ix++)
	      {
		momval_t argv = mom_node_nth (nodv, ix);
		momtypenc_t targ = scan_expr_cgen (cg, insv, argv);
		if (targ > momtypenc__none && targ != momtypenc_val)
		  NODESCANFAIL ("non-value #%d in *node", (int) ix);
	      }
	  }
	else
	  NODESCANFAIL ("empty *node");
      }
      break;
#undef SCANCASE
#undef SCANODHASHMAX_MOM
      //////////
    other_node:
    default:
      {
	/// both procedure and primitives have `formals` and `ctype` attributes
	momval_t formalsv =
	  mom_item_get_attribute (connitm, mom_named__formals);
	momval_t ctypev = mom_item_get_attribute (connitm, mom_named__ctype);
	momval_t outputv =
	  mom_item_get_attribute (connitm, mom_named__output);
	MOM_DEBUG		//
	  (gencod,
	   MOMOUT_LITERAL ("scan_node quasiprim nodv="),
	   MOMOUT_VALUE ((const momval_t) nodv),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (" connitm:"),
	   MOMOUT_ITEM ((const momitem_t *) connitm),
	   MOMOUT_SPACE (48),
	   MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm), NULL);
	if (!mom_is_tuple (formalsv) || !mom_is_item (ctypev))
	  NODESCANFAIL ("node connective %s without `formals` or `ctype`",
			mom_item_get_name_or_id_cstr (connitm));
	unsigned nbformals = mom_tuple_length (formalsv);
	if (nbformals > arity)
	  NODESCANFAIL ("missing %d arguments", nbformals - arity);
	if (!outputv.ptr && nbformals < arity)
	  NODESCANFAIL ("too much %d arguments", arity - nbformals);
	for (unsigned fix = 0; fix < nbformals; fix++)
	  {
	    const momitem_t *formitm = mom_tuple_nth_item (formalsv, fix);
	    momval_t argv = mom_node_nth (nodv, fix);
	    if (!formitm)
	      NODESCANFAIL ("bad formal #%d", (int) fix);
	    cgen_lock_item_mom (cg, (momitem_t *) formitm);
	    momval_t formctypv =
	      mom_item_get_attribute (formitm, mom_named__variable);
	    if (!mom_is_item (formctypv))
	      NODESCANFAIL ("formal #%d %s without `variable`", (int) fix,
			    mom_item_get_name_or_id_cstr (formitm));
	    momtypenc_t formtyp =
	      typenc_cgen (cg, true, formctypv.pitem, insv);
	    momtypenc_t argtyp = scan_expr_cgen (cg, insv, argv);
	    if (formtyp > momtypenc__none && argtyp > momtypenc__none
		&& formtyp != argtyp)
	      NODESCANFAIL ("formal #%d %s type mismatch", (int) fix,
			    mom_item_get_name_or_id_cstr (formitm));
	  }
	if (outputv.ptr)
	  {
	    if (!mom_is_item (outputv))
	      NODESCANFAIL ("bad `output` in node connective %s",
			    mom_item_get_name_or_id_cstr (connitm));
	    for (int oix = nbformals; oix < (int) arity; oix++)
	      scan_output_cgen (cg, insv, mom_node_nth (nodv, oix));
	  }
	return typenc_cgen (cg, true, ctypev.pitem, insv);
      }
      break;
    }
  NODESCANFAIL ("unexpected");
#undef NODESCANFAIL
bad_node:
  assert (errline > 0);
  cgen_error_mom_at (errline, cg,
		     MOMOUT_LITERAL ("in scan_node bad expression:"),
		     MOMOUT_VALUE ((const momval_t) nodv),
		     MOMOUT_LITERAL (" in instr:"),
		     MOMOUT_VALUE ((const momval_t) insv),
		     MOMOUT_SPACE (60),
		     MOMOUT_LITERALV ((const char *) errmsg), NULL);
}



static momtypenc_t
scan_expr_cgen (struct c_generator_mom_st *cg, momval_t insv, momval_t expv)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_expr start in instr:"),
	     MOMOUT_VALUE ((const momval_t) insv),
	     MOMOUT_LITERAL (" for exp:"),
	     MOMOUT_VALUE ((const momval_t) expv), NULL);
  switch ((momvaltype_t) mom_type (expv))
    {
    case momty_null:
      return momtypenc__none;
    case momty_int:
      return momtypenc_int;
    case momty_string:
      return momtypenc_string;
    case momty_double:
      return momtypenc_double;
    case momty_jsonarray:
    case momty_jsonobject:
    case momty_tuple:
    case momty_set:
      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("scan_expr in instr:"),
		      MOMOUT_VALUE ((const momval_t) insv),
		      MOMOUT_LITERAL (" unexpected exp:"),
		      MOMOUT_VALUE ((const momval_t) expv), NULL);
    case momty_item:
      return scan_item_cgen (cg, expv.pitem);
    case momty_node:
      return scan_node_cgen (cg, insv, expv.pnode);
    }
  CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("scan_expr unimplemented in instr:"),
		  MOMOUT_VALUE ((const momval_t) insv),
		  MOMOUT_LITERAL (" exp:"),
		  MOMOUT_VALUE ((const momval_t) expv), NULL);
}




static momtypenc_t
scan_item_cgen (struct c_generator_mom_st *cg, momitem_t *varitm)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (!varitm)
    return momtypenc__none;
  assert (varitm->i_typnum == momty_item);
  momval_t asexpv = mom_item_assoc_get (cg->cgen_globassocitm, varitm);
  if (MOM_LIKELY (asexpv.ptr == NULL))
    asexpv = mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, varitm);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_item varitm="),
	     MOMOUT_ITEM ((const momitem_t *) varitm),
	     MOMOUT_LITERAL (" asexpv="),
	     MOMOUT_VALUE ((const momval_t) asexpv), NULL);
  if (asexpv.ptr)
    {
      const momitem_t *asitm = mom_node_conn (asexpv);
      assert (asitm != NULL);
#define SCANITEMHASHMAX_MOM 79
#define SCANITCASE(Nam) case mom_hashname_##Nam % SCANITEMHASHMAX_MOM: if (asitm != mom_named_##Nam) break;
      switch (mom_item_hash (asitm) % SCANITEMHASHMAX_MOM)
	{
	  ////
	  SCANITCASE (_procedure);
	  return momtypenc__none;
	  ////
	  SCANITCASE (_formals);
	  {
	    assert (mom_node_arity (asexpv) == 3);
	    momval_t formtypv = mom_node_nth (asexpv, 2);
	    assert (mom_is_item (formtypv));
	    momitem_t *formtypitm = mom_value_to_item (formtypv);
	    return typenc_cgen (cg, true, formtypitm, (momval_t) varitm);
	  }
	  break;
	  ////
	  SCANITCASE (_constants);
	  return momtypenc_val;
	  ////
	  SCANITCASE (_intptr_t);
	  return momtypenc_int;
	  ////
	  SCANITCASE (_momval_t);
	  return momtypenc_val;
	  ////
	  SCANITCASE (_momcstr_t);
	  return momtypenc_string;
	  ////
	  SCANITCASE (_double);
	  return momtypenc_double;
	  ////
	default:
	  ;
	}
#undef SCANITCASE
#undef SCANITEMHASHMAX_MOM
      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("scan_item unexpected asexpv="),
		      MOMOUT_VALUE ((const momval_t) asexpv),
		      MOMOUT_LITERAL (" for item:"),
		      MOMOUT_ITEM ((const momitem_t *) varitm), NULL);
    }
  cgen_lock_item_mom (cg, varitm);
  momval_t varv = mom_item_get_attribute (varitm, mom_named__variable);
  if (mom_is_item (varv))
    {
      momtypenc_t tyvar =
	typenc_cgen (cg, true, varv.pitem, (momval_t) varitm);
      switch (tyvar)
	{
	  /// integer variables
	case momtypenc_int:
	  {
	    assert (mom_item_payload_kind (cg->cgen_rout.cgrout_hsetintitm) ==
		    mompayk_hset);
	    mom_item_hset_check_integrity ("scan_item hsetintitm",
					   cg->cgen_rout.cgrout_hsetintitm);
	    unsigned cnt =
	      mom_item_hset_count (cg->cgen_rout.cgrout_hsetintitm);
	    assert (!mom_item_hset_contains
		    (cg->cgen_rout.cgrout_hsetintitm, (momval_t) varitm));
	    mom_item_assoc_put	////
	      (cg->cgen_rout.cgrout_associtm, varitm,
	       (momval_t) mom_make_node_sized ((momval_t) mom_named__intptr_t,
					       2, mom_make_integer (cnt),
					       cg->cgen_rout.cgrout_routitm));
	    mom_item_hset_add (cg->cgen_rout.cgrout_hsetintitm,
			       (momval_t) varitm);
	    mom_item_hset_check_integrity ("scan_item hsetintitm",
					   cg->cgen_rout.cgrout_hsetintitm);
	    MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_item integer varitm="),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) cnt),
		       NULL);
	    return momtypenc_int;
	  }
	  break;
	  /// double variables
	case momtypenc_double:
	  {
	    assert (mom_item_payload_kind (cg->cgen_rout.cgrout_hsetdblitm) ==
		    mompayk_hset);
	    mom_item_hset_check_integrity ("scan_item hsetdblitm",
					   cg->cgen_rout.cgrout_hsetdblitm);
	    unsigned cnt =
	      mom_item_hset_count (cg->cgen_rout.cgrout_hsetdblitm);
	    assert (!mom_item_hset_contains
		    (cg->cgen_rout.cgrout_hsetdblitm, (momval_t) varitm));
	    mom_item_assoc_put	////
	      (cg->cgen_rout.cgrout_associtm, varitm,
	       (momval_t) mom_make_node_sized ((momval_t) mom_named__double,
					       2, mom_make_integer (cnt),
					       cg->cgen_rout.cgrout_routitm));
	    mom_item_hset_add (cg->cgen_rout.cgrout_hsetdblitm,
			       (momval_t) varitm);
	    mom_item_hset_check_integrity ("scan_item hsetdblitm",
					   cg->cgen_rout.cgrout_hsetdblitm);
	    MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_item double varitm="),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) cnt),
		       NULL);
	    return momtypenc_double;
	  }
	  break;
	  /// value variables
	case momtypenc_val:
	  {
	    assert (mom_item_payload_kind (cg->cgen_rout.cgrout_hsetvalitm) ==
		    mompayk_hset);
	    mom_item_hset_check_integrity ("scan_item hsetvalitm",
					   cg->cgen_rout.cgrout_hsetvalitm);
	    unsigned cnt =
	      mom_item_hset_count (cg->cgen_rout.cgrout_hsetvalitm);
	    assert (!mom_item_hset_contains
		    (cg->cgen_rout.cgrout_hsetvalitm, (momval_t) varitm));
	    mom_item_assoc_put	////
	      (cg->cgen_rout.cgrout_associtm, varitm,
	       (momval_t) mom_make_node_sized ((momval_t) mom_named__momval_t,
					       2, mom_make_integer (cnt),
					       cg->cgen_rout.cgrout_routitm));
	    mom_item_hset_add (cg->cgen_rout.cgrout_hsetvalitm,
			       (momval_t) varitm);
	    mom_item_hset_check_integrity ("scan_item hsetvalitm",
					   cg->cgen_rout.cgrout_hsetvalitm);
	    MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_item value varitm="),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) cnt),
		       NULL);
	    return momtypenc_val;
	  }
	  break;
	case momtypenc__none:
	  CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("scan_item bad void variable:"),
			  MOMOUT_ITEM ((const momitem_t *) varitm), NULL);
	case momtypenc_string:
	  /// perhaps we should accept string variables in procedures only
	  CGEN_ERROR_MOM (cg,
			  MOMOUT_LITERAL ("scan_item bad string variable:"),
			  MOMOUT_ITEM ((const momitem_t *) varitm), NULL);
	default:
	  CGEN_ERROR_MOM (cg,
			  MOMOUT_LITERAL ("scan_item bad variable:"),
			  MOMOUT_ITEM ((const momitem_t *) varitm), NULL);

	}
    }
  momval_t verbatimv = mom_item_get_attribute (varitm, mom_named__verbatim);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_item value varitm="),
	     MOMOUT_ITEM ((const momitem_t *) varitm),
	     MOMOUT_LITERAL (" verbatimv="),
	     MOMOUT_VALUE ((const momval_t) verbatimv), NULL);
  if (mom_is_string (verbatimv))
    {
      momval_t verbtypv = mom_item_get_attribute (varitm, mom_named__ctype);
      if (mom_is_item (verbtypv))
	return typenc_cgen (cg, true, verbtypv.pitem, (momval_t) varitm);
    }
  CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("scan_item failure for varitm:"),
		  MOMOUT_ITEM ((const momitem_t *) varitm),
		  MOMOUT_SPACE (64), NULL);
}


static void
scan_instr_cgen (struct c_generator_mom_st *cg, momitem_t *blockitm,
		 momval_t insv, bool lastinstr)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (blockitm && blockitm->i_typnum == momty_item);
  momitem_t *opitm = (momitem_t *) mom_node_conn (insv);
  unsigned instarity = mom_node_arity (insv);
  if (!opitm)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in block:"),
		    MOMOUT_ITEM ((const momitem_t *) blockitm),
		    MOMOUT_LITERAL (" bad instr:"),
		    MOMOUT_VALUE ((const momval_t) insv), NULL);
  MOM_DEBUG			//
    (gencod,
     MOMOUT_LITERAL ("scan_instr blockitm:"),
     MOMOUT_ITEM ((const momitem_t *) blockitm),
     MOMOUT_LITERAL (" instr:"),
     MOMOUT_VALUE ((const momval_t) insv),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" valhset:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetvalitm),
     MOMOUT_SPACE (48),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetvalitm), NULL);
  mom_item_hset_check_integrity ("scan_instr hsetvalitm",
				 cg->cgen_rout.cgrout_hsetvalitm);
  cgen_lock_item_mom (cg, opitm);
  momhash_t oph = mom_item_hash (opitm);
#define SCANINSOPHASHMAX_MOM 127
#define SCANCASE(Nam) case mom_hashname_##Nam % SCANINSOPHASHMAX_MOM: if (opitm != mom_named_##Nam) break;
  switch (oph % SCANINSOPHASHMAX_MOM)
    {
      SCANCASE (_do);		//   *do (<expr>) for side-effecting expressions
      {
	if (instarity != 1)
	  goto bad_instr;
	(void) scan_expr_cgen (cg, insv, mom_node_nth (insv, 0));
      }
      break;
      ////
      SCANCASE (_if);		//  *if (<cond>,<block>) for conditional jumps
      {
	if (instarity != 2)
	  goto bad_instr;
	scan_expr_cgen (cg, insv, mom_node_nth (insv, 0));
	momitem_t *thenitm = mom_value_to_item (mom_node_nth (insv, 1));
	if (!thenitm)
	  goto bad_instr;
	scan_block_cgen (cg, thenitm, insv, blockitm);
      }
      break;
      ////
      SCANCASE (_assign);	//  *assign (<var>,<expr>) for assignments
      {
	if (instarity != 2)
	  goto bad_instr;
	momitem_t *leftitm = mom_value_to_item (mom_node_nth (insv, 0));
	if (!leftitm)
	  goto bad_instr;
	momtypenc_t tl = scan_item_cgen (cg, leftitm);
	momtypenc_t tr = scan_expr_cgen (cg, insv, mom_node_nth (insv, 1));
	if (tl > momtypenc__none && tr > momtypenc__none && tl != tr)
	  CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("type mismatch in block:"),
			  MOMOUT_ITEM ((const momitem_t *) blockitm),
			  MOMOUT_LITERAL (" invalid assign:"),
			  MOMOUT_VALUE ((const momval_t) insv), NULL);
      }
      break;
      ////
      SCANCASE (_switch);	//   *switch (<expr>,<case>...); each <case> is *case(<const-expr>,<block>)
      {
	if (instarity < 1)
	  goto bad_instr;
	momtypenc_t tx = scan_expr_cgen (cg, insv, mom_node_nth (insv, 0));
	if (tx != momtypenc_int)
	  CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("type mismatch in block:"),
			  MOMOUT_ITEM ((const momitem_t *) blockitm),
			  MOMOUT_LITERAL (" invalid switch:"),
			  MOMOUT_VALUE ((const momval_t) insv), NULL);
	for (unsigned ix = 1; ix < instarity; ix++)
	  {
	    momval_t curcasev = mom_node_nth (insv, ix);
	    if (mom_node_conn (curcasev) != mom_named__case)
	      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("type mismatch in block:"),
			      MOMOUT_ITEM ((const momitem_t *) blockitm),
			      MOMOUT_LITERAL (" invalid case:"),
			      MOMOUT_VALUE ((const momval_t) curcasev),
			      MOMOUT_LITERAL (" #"),
			      MOMOUT_DEC_INT ((int) ix),
			      MOMOUT_LITERAL (" in switch:"),
			      MOMOUT_VALUE ((const momval_t) insv), NULL);
	    momval_t casexprv = (mom_node_nth (curcasev, 0));
	    if (scan_expr_cgen (cg, insv, casexprv) != momtypenc_int)
	      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("non-int case in block:"),
			      MOMOUT_ITEM ((const momitem_t *) blockitm),
			      MOMOUT_LITERAL (" invalid case:"),
			      MOMOUT_VALUE ((const momval_t) curcasev),
			      MOMOUT_LITERAL (" #"),
			      MOMOUT_DEC_INT ((int) ix),
			      MOMOUT_LITERAL (" in switch:"),
			      MOMOUT_VALUE ((const momval_t) insv), NULL);
	    momitem_t *caseblockitm =
	      mom_value_to_item (mom_node_nth (curcasev, 1));
	    if (!caseblockitm)
	      goto bad_instr;
	    scan_block_cgen (cg, caseblockitm, insv, blockitm);
	  }
      }
      break;
      ////
      SCANCASE (_dispatch);	//   *dispatch (<expr>,<case>...); each <case> is *case(<const-item>,<block>)
      {
	if (instarity < 1)
	  goto bad_instr;
	momtypenc_t tx = scan_expr_cgen (cg, insv, mom_node_nth (insv, 0));
	if (tx != momtypenc_val)
	  CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("type mismatch in block:"),
			  MOMOUT_ITEM ((const momitem_t *) blockitm),
			  MOMOUT_LITERAL (" invalid dispatch:"),
			  MOMOUT_VALUE ((const momval_t) insv), NULL);
	for (unsigned ix = 1; ix < instarity; ix++)
	  {
	    momval_t curcasev = mom_node_nth (insv, ix);
	    if (mom_node_conn (curcasev) != mom_named__case)
	      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("type mismatch in block:"),
			      MOMOUT_ITEM ((const momitem_t *) blockitm),
			      MOMOUT_LITERAL (" invalid case:"),
			      MOMOUT_VALUE ((const momval_t) curcasev),
			      MOMOUT_LITERAL (" #"),
			      MOMOUT_DEC_INT ((int) ix),
			      MOMOUT_LITERAL (" in dispatch:"),
			      MOMOUT_VALUE ((const momval_t) insv), NULL);
	    momval_t casexprv = (mom_node_nth (curcasev, 0));
	    if (scan_expr_cgen (cg, insv, casexprv) != momtypenc_val)
	      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("non-int case in block:"),
			      MOMOUT_ITEM ((const momitem_t *) blockitm),
			      MOMOUT_LITERAL (" invalid case:"),
			      MOMOUT_VALUE ((const momval_t) curcasev),
			      MOMOUT_LITERAL (" #"),
			      MOMOUT_DEC_INT ((int) ix),
			      MOMOUT_LITERAL (" in dispatch:"),
			      MOMOUT_VALUE ((const momval_t) insv), NULL);
	    momitem_t *caseblockitm =
	      mom_value_to_item (mom_node_nth (curcasev, 1));
	    if (!caseblockitm)
	      goto bad_instr;
	    scan_block_cgen (cg, caseblockitm, insv, blockitm);
	  }
      }
      break;
      ////
      SCANCASE (_chunk);	//  *chunk (...) for some code chunk with variables
      {
	for (unsigned ix = 1; ix < instarity; ix++)
	  {
	    momval_t curchkv = mom_node_nth (insv, ix);
	    if (!mom_is_string (curchkv) && !mom_is_integer (curchkv)
		&& curchkv.ptr)
	      {
		(void) scan_expr_cgen (cg, insv, curchkv);
	      }
	  }
      }
      break;
      ////
      SCANCASE (_jump);		// *jump(<block>)
      {
	if (instarity != 1 || !lastinstr)
	  goto bad_instr;
	momitem_t *jumpblockitm = mom_value_to_item (mom_node_nth (insv, 0));
	if (!jumpblockitm)
	  goto bad_instr;
	scan_block_cgen (cg, jumpblockitm, insv, blockitm);
      }
      break;
      ////
      SCANCASE (_return);	// *return(<expr>) or *return
      {
	if (instarity > 1 || !lastinstr)
	  goto bad_instr;
	scan_expr_cgen (cg, insv, mom_node_nth (insv, 0));
      }
      break;
    bad_instr:
    default:
      CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("scan_instr; in block:"),
		      MOMOUT_ITEM ((const momitem_t *) blockitm),
		      MOMOUT_LITERAL (" invalid instr:"),
		      MOMOUT_VALUE ((const momval_t) insv), NULL);
      break;
    }
#undef SCANCASE
#undef SCANINSOPHASHMAX_MOM
  mom_item_hset_check_integrity ("scan_instr end hsetvalitm",
				 cg->cgen_rout.cgrout_hsetvalitm);
}



static void
scan_block_cgen (struct c_generator_mom_st *cg, momitem_t *blockitm,
		 momval_t fromv, momitem_t *fromblitm)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  if (!blockitm)
    return;
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_block blockitm="),
	     MOMOUT_ITEM ((const momitem_t *) blockitm),
	     MOMOUT_LITERAL (" fromv="),
	     MOMOUT_VALUE ((const momval_t) fromv),
	     MOMOUT_LITERAL (" fromblitm="),
	     MOMOUT_ITEM ((const momitem_t *) fromblitm), NULL);
  assert (blockitm->i_typnum == momty_item);
  assert (!fromblitm || fromblitm->i_typnum == momty_item);
  assert (mom_item_payload_kind (cg->cgen_rout.cgrout_blockhsetitm) ==
	  mompayk_hset);
  assert (mom_item_payload_kind (cg->cgen_rout.cgrout_blockqueueitm) ==
	  mompayk_queue);
  if (fromblitm)
    {
      (void) mom_item_hset_add
	(cg->cgen_rout.cgrout_blockhsetitm,
	 (momval_t) mom_make_node_sized ((momval_t) mom_named__jump, 2,
					 (momval_t) fromblitm,
					 (momval_t) blockitm));
      (void) mom_item_hset_add
	(cg->cgen_rout.cgrout_blockhsetitm,
	 (momval_t) mom_make_node_sized ((momval_t) mom_named__from, 2,
					 (momval_t) blockitm,
					 (momval_t) fromblitm));
    }
  if (!mom_item_hset_add
      (cg->cgen_rout.cgrout_blockhsetitm, (momval_t) blockitm))
    {
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_block known blockitm="),
		 MOMOUT_ITEM ((const momitem_t *) blockitm), NULL);
      return;
    }
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_block blockitm="),
	     MOMOUT_ITEM ((const momitem_t *) blockitm),
	     MOMOUT_LITERAL (" from="),
	     MOMOUT_VALUE ((const momval_t) fromv),
	     MOMOUT_LITERAL (" fromblitm="),
	     MOMOUT_ITEM ((const momitem_t *) fromblitm), NULL);
  cgen_lock_item_mom (cg, blockitm);
  momval_t blcodev = mom_item_get_attribute (blockitm, mom_named__block);
  if (mom_node_conn (blcodev) != mom_named__code)
    CGEN_ERROR_MOM		///
      (cg, MOMOUT_LITERAL ("in routine:"),
       MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_routitm),
       MOMOUT_LITERAL
       (" bad `block` attribute - not a *code node :"),
       MOMOUT_VALUE ((const momval_t) blcodev),
       MOMOUT_LITERAL ("in block item:"),
       MOMOUT_ITEM ((const momitem_t *) blockitm),
       MOMOUT_LITERAL (" from:"),
       MOMOUT_VALUE ((const momval_t) fromv), NULL);
  momval_t blilockv = mom_item_get_attribute (blockitm, mom_named__lock);
  if (blilockv.ptr)
    {
      if (!mom_is_item (blilockv))
	CGEN_ERROR_MOM		///
	  (cg, MOMOUT_LITERAL ("in routine:"),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_routitm),
	   MOMOUT_LITERAL
	   (" bad `lock` attribute - not an item :"),
	   MOMOUT_VALUE ((const momval_t) blilockv),
	   MOMOUT_LITERAL ("in block item:"),
	   MOMOUT_ITEM ((const momitem_t *) blockitm),
	   MOMOUT_LITERAL (" from:"),
	   MOMOUT_VALUE ((const momval_t) fromv), NULL);
      momitem_t *lockitm = blilockv.pitem;
      if (scan_item_cgen (cg, lockitm) != momtypenc_val)
	CGEN_ERROR_MOM		///
	  (cg, MOMOUT_LITERAL ("in routine:"),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_routitm),
	   MOMOUT_LITERAL
	   (" bad `lock` attribute - not a value variable :"),
	   MOMOUT_VALUE ((const momval_t) blilockv),
	   MOMOUT_LITERAL ("in block item:"),
	   MOMOUT_ITEM ((const momitem_t *) blockitm),
	   MOMOUT_LITERAL (" from:"),
	   MOMOUT_VALUE ((const momval_t) fromv), NULL);
    }
  mom_item_queue_add_back (cg->cgen_rout.cgrout_blockqueueitm,
			   (momval_t) blockitm);
  cg->cgen_rout.cgrout_blockcount++;
  momval_t nodv = (momval_t)	//
    mom_make_node_sized ((momval_t) mom_named__block, 2,
			 mom_make_integer (cg->cgen_rout.cgrout_blockcount),
			 cg->cgen_rout.cgrout_routitm);
  mom_item_assoc_put (cg->cgen_rout.cgrout_associtm, blockitm, nodv);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_block nodv="),
	     MOMOUT_VALUE ((const momval_t) nodv),
	     MOMOUT_LITERAL (" blockitm="),
	     MOMOUT_ITEM ((const momitem_t *) blockitm), NULL);
}				/* end scan_block_cgen */



static momtypenc_t emit_var_item_cgen (struct c_generator_mom_st *cg,
				       momitem_t *varitm);


////////////////
void
emit_procedure_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  momitem_t *procitm = NULL;
  momval_t procnodev = MOM_NULLV;
  momval_t proconstantsv = MOM_NULLV;
  unsigned nbconstants = 0;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  procitm = cg->cgen_rout.cgrout_routitm;
  procnodev = mom_item_assoc_get (cg->cgen_globassocitm, procitm);
  proconstantsv = mom_item_get_attribute (procitm, mom_named__constants);
  nbconstants = mom_seqitem_length (proconstantsv);
  assert (mom_node_conn (procnodev) == mom_named__procedure);
  MOM_DEBUG			//
    (gencod, MOMOUT_LITERAL ("emit_procedure procitm="),
     MOMOUT_ITEM ((const momitem_t *) procitm),
     MOMOUT_SPACE (58),
     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) procitm),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" procnodev="),
     MOMOUT_VALUE ((momval_t) procnodev),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" local-assoc:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
     MOMOUT_SPACE (50),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" blockhset:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_blockhsetitm),
     MOMOUT_SPACE (50),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_blockhsetitm), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" hsetint:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetintitm),
     MOMOUT_SPACE (50),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetintitm), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" hsetdbl:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetdblitm),
     MOMOUT_SPACE (50),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetdblitm), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" hsetval:"),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetvalitm),
     MOMOUT_SPACE (50),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetvalitm), NULL);
  momval_t proctypv = mom_item_get_attribute (procitm, mom_named__ctype);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("// implementation of procedure #"),
	   MOMOUT_DEC_INT ((int) routix),
	   MOMOUT_LITERAL (" = "),
	   MOMOUT_ITEM ((const momitem_t *) procitm), MOMOUT_NEWLINE ());
  cg->cgen_restype = emit_ctype_cgen (cg, &cg->cgen_outbody, proctypv);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
	   MOMOUT_LITERAL (MOM_PROCROUTFUN_PREFIX),
	   MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	   MOMOUT_LITERAL (" ("), NULL);
  momval_t formargsv = mom_item_get_attribute (procitm, mom_named__formals);
  assert (mom_is_tuple (formargsv));
  unsigned nbformals = mom_tuple_length (formargsv);
  for (unsigned aix = 0; aix < nbformals; aix++)
    {
      momitem_t *curargitm = mom_tuple_nth_item (formargsv, aix);
      assert (mom_is_item ((momval_t) curargitm));
      momval_t curargav =
	mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, curargitm);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc curargitm="),
		 MOMOUT_ITEM ((const momitem_t *) curargitm),
		 MOMOUT_LITERAL (" curargv="),
		 MOMOUT_VALUE ((const momval_t) curargav), NULL);
      assert (mom_node_conn (curargav) == mom_named__formals);
      momval_t curargtypv = mom_node_nth (curargav, 2);
      assert (mom_is_item (curargtypv));
      if (aix > 0)
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL (","), MOMOUT_SPACE (40), NULL);
      emit_ctype_cgen (cg, &cg->cgen_outbody, curargtypv);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (" " CGEN_FORMALARG_PREFIX),
	       MOMOUT_DEC_INT ((int) aix),
	       MOMOUT_LITERAL ("/*!formal:"),
	       MOMOUT_ITEM ((const momitem_t *) curargitm),
	       MOMOUT_LITERAL ("*/"), NULL);
    }
  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("{"), MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (),
	   NULL);
  //////////// declare the locals
  ////////////
  /// declare the integers
  {
    const momitem_t *hsetintitm =
      (const momitem_t *) cg->cgen_rout.cgrout_hsetintitm;
    unsigned nbints = mom_item_hset_count ((momitem_t *) hsetintitm);
    mom_item_hset_check_integrity ("emit_proc dcl hsetintitm",
				   cg->cgen_rout.cgrout_hsetintitm);
    momval_t setintsv = mom_item_hset_items_set ((momitem_t *) hsetintitm);
    MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc setints="),
	       MOMOUT_VALUE ((const momval_t) setintsv),
	       MOMOUT_LITERAL (" from inthset:"), MOMOUT_ITEM (hsetintitm),
	       MOMOUT_SPACE (64), MOMOUT_ITEM_PAYLOAD (hsetintitm), NULL);
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/// "),
	     MOMOUT_DEC_INT ((int) nbints),
	     MOMOUT_LITERAL (" integer locals"), MOMOUT_NEWLINE ());
    for (unsigned ix = 0; ix < nbints; ix++)
      {
	const momitem_t *curintitm = mom_set_nth_item (setintsv, ix);
	momval_t curintassv =
	  mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, curintitm);
	MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc curintitm="),
		   MOMOUT_ITEM (curintitm), MOMOUT_LITERAL (" curintassv="),
		   MOMOUT_VALUE (curintassv), NULL);
	int num = mom_integer_val_def (mom_node_nth (curintassv, 0), -1);
	assert (num >= 0);
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("intptr_t " CGEN_PROC_NUMBER_PREFIX),
		 MOMOUT_DEC_INT ((int) num),
		 MOMOUT_LITERAL (" = 0; //!! local int "),
		 MOMOUT_ITEM (curintitm), MOMOUT_NEWLINE ());
      }
  }
  /// declare the doubles
  {
    const momitem_t *hsetdblitm =
      (const momitem_t *) cg->cgen_rout.cgrout_hsetdblitm;
    unsigned nbdbls = mom_item_hset_count ((momitem_t *) hsetdblitm);
    mom_item_hset_check_integrity ("emit_proc dcl hsetdblitm",
				   cg->cgen_rout.cgrout_hsetdblitm);
    momval_t setdblsv = mom_item_hset_items_set ((momitem_t *) hsetdblitm);
    MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc setdbls="),
	       MOMOUT_VALUE ((const momval_t) setdblsv),
	       MOMOUT_LITERAL (" from dblhset:"), MOMOUT_ITEM (hsetdblitm),
	       MOMOUT_SPACE (64), MOMOUT_ITEM_PAYLOAD (hsetdblitm), NULL);
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/// "),
	     MOMOUT_DEC_INT ((int) nbdbls), MOMOUT_LITERAL (" double locals"),
	     MOMOUT_NEWLINE ());
    for (unsigned ix = 0; ix < nbdbls; ix++)
      {
	const momitem_t *curdblitm = mom_set_nth_item (setdblsv, ix);
	momval_t curdblassv =
	  mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, curdblitm);
	MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc curdblitm="),
		   MOMOUT_ITEM (curdblitm), MOMOUT_LITERAL (" curdblassv="),
		   MOMOUT_VALUE (curdblassv), NULL);
	int num = mom_integer_val_def (mom_node_nth (curdblassv, 0), -1);
	assert (num >= 0);
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("double " CGEN_PROC_DOUBLE_PREFIX),
		 MOMOUT_DEC_INT ((int) num),
		 MOMOUT_LITERAL (" = 0.0; //!! local double "),
		 MOMOUT_ITEM (curdblitm), MOMOUT_NEWLINE ());
      }
  }
  /// declare the values
  {
    const momitem_t *hsetvalitm =
      (const momitem_t *) cg->cgen_rout.cgrout_hsetvalitm;
    unsigned nbvals = mom_item_hset_count ((momitem_t *) hsetvalitm);
    mom_item_hset_check_integrity ("emit_proc dcl hsetvalitm",
				   cg->cgen_rout.cgrout_hsetvalitm);
    momval_t setvalsv = mom_item_hset_items_set ((momitem_t *) hsetvalitm);
    MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc setvals="),
	       MOMOUT_VALUE ((const momval_t) setvalsv),
	       MOMOUT_LITERAL (" from valhset:"), MOMOUT_ITEM (hsetvalitm),
	       MOMOUT_SPACE (64), MOMOUT_ITEM_PAYLOAD (hsetvalitm), NULL);
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/// "),
	     MOMOUT_DEC_INT ((int) nbvals), MOMOUT_LITERAL (" values locals"),
	     MOMOUT_NEWLINE ());
    for (unsigned ix = 0; ix < nbvals; ix++)
      {
	const momitem_t *curvalitm = mom_set_nth_item (setvalsv, ix);
	momval_t curvalassv =
	  mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, curvalitm);
	MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc curvalitm="),
		   MOMOUT_ITEM (curvalitm), MOMOUT_LITERAL (" curvalassv="),
		   MOMOUT_VALUE (curvalassv), NULL);
	assert (mom_node_conn (curvalassv) == mom_named__momval_t);
	int num = mom_integer_val_def (mom_node_nth (curvalassv, 0), -1);
	assert (num >= 0);
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("momval_t " CGEN_PROC_VALUE_PREFIX),
		 MOMOUT_DEC_INT (num),
		 MOMOUT_LITERAL (" = MOM_NULLV; //!! local value "),
		 MOMOUT_ITEM (curvalitm), MOMOUT_NEWLINE ());
      }
  }
  //// emit the starting prologue all the blocks
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc procnodev="),
	     MOMOUT_VALUE (procnodev), NULL);
  {
    const momitem_t *hsetblkitm =
      (const momitem_t *) cg->cgen_rout.cgrout_blockhsetitm;
    const momitem_t *startblkitm =
      mom_value_to_item (mom_item_get_attribute
			 (procitm, mom_named__procedure));
    momval_t startacv =
      mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, startblkitm);
    momval_t setblksv = mom_item_hset_items_set ((momitem_t *) hsetblkitm);
    MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc setblksv="),
	       MOMOUT_VALUE ((const momval_t) setblksv),
	       MOMOUT_SPACE (64), MOMOUT_LITERAL (" from blockset:"),
	       MOMOUT_ITEM (hsetblkitm), MOMOUT_SPACE (64),
	       MOMOUT_ITEM_PAYLOAD (hsetblkitm), MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (" with startblkitm:"),
	       MOMOUT_ITEM (startblkitm), MOMOUT_SPACE (64),
	       MOMOUT_LITERAL (" startacv="),
	       MOMOUT_VALUE ((const momval_t) startacv), NULL);
    unsigned nbblocks = mom_set_cardinal (setblksv);
    int startix = -1;
    if (mom_node_conn (startacv) == mom_named__block)
      startix = mom_integer_val_def (mom_node_nth (startacv, 0), -2);
    assert (startix > 0);
    // emit prologue
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL
	     ("static momitem_t* momprocitem; //! prologue of proc. "),
	     MOMOUT_ITEM ((const momitem_t *) procitm), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL
	     ("if (MOM_UNLIKELY(!momprocitem)) momprocitem = mom_procedure_item_of_id(\""),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL ("\");"), MOMOUT_NEWLINE (), NULL);
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/// starting procedure "),
	     MOMOUT_ITEM ((const momitem_t *) procitm), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("goto " CGEN_PROC_BLOCK_PREFIX),
	     MOMOUT_DEC_INT (startix), MOMOUT_LITERAL ("; // start at "),
	     MOMOUT_ITEM ((const momitem_t *) startblkitm), MOMOUT_NEWLINE (),
	     NULL);
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!! "),
	     MOMOUT_DEC_INT ((int) nbblocks),
	     MOMOUT_LITERAL (" blocks in proc. "),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_LITERAL (" */"), MOMOUT_NEWLINE ());
    for (unsigned bix = 1; bix <= nbblocks; bix++)
      {
	momitem_t *curblkitm = mom_set_nth_item (setblksv, bix - 1);
	MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_proc bix#"),
		   MOMOUT_DEC_INT ((int) bix),
		   MOMOUT_LITERAL (" curblkitm="),
		   MOMOUT_ITEM ((const momitem_t *) curblkitm),
		   MOMOUT_SPACE (48),
		   MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) curblkitm),
		   MOMOUT_NEWLINE (), NULL);
	mom_item_hset_check_integrity ("emit_proc bixloop hsetintitm",
				       cg->cgen_rout.cgrout_hsetintitm);
	mom_item_hset_check_integrity ("emit_proc bixloop hsetdblitm",
				       cg->cgen_rout.cgrout_hsetdblitm);
	mom_item_hset_check_integrity ("emit_proc bixloop hsetvalitm",
				       cg->cgen_rout.cgrout_hsetvalitm);
	MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL ("/******** block#"),
		 MOMOUT_DEC_INT ((int) bix), MOMOUT_LITERAL (": "),
		 MOMOUT_ITEM ((const momitem_t *) curblkitm),
		 MOMOUT_LITERAL (" ********/"), MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL (" " CGEN_PROC_BLOCK_PREFIX),
		 MOMOUT_DEC_INT ((int) bix), MOMOUT_LITERAL (":"),
		 MOMOUT_SPACE (64),
		 MOMOUT_LITERAL ("{ //! start procedure block "),
		 MOMOUT_ITEM ((const momitem_t *) curblkitm),
		 MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (), NULL);
	emit_block_cgen (cg, curblkitm);
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_INDENT_LESS (),
		 MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL ("}; //! end procedure block "),
		 MOMOUT_ITEM ((const momitem_t *) curblkitm),
		 MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
      }
  }
  //// should have an epilogue to return a default result
  //// and emit the procedure descriptor
  {
    const momitem_t *procresitm
      =
      mom_value_to_item (mom_item_get_attribute (procitm, mom_named__result));
    momtypenc_t protyp = typenc_cgen (cg, true, mom_value_to_item (proctypv),
				      (momval_t) procitm);
    if (procresitm)
      {
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("//!explicit result "),
		 MOMOUT_ITEM (procresitm),
		 MOMOUT_NEWLINE (), MOMOUT_LITERAL ("return "), NULL);
	emit_var_item_cgen (cg, (momitem_t *) procresitm);
      }
    else
      {
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("/*!defreturn*/return "), NULL);
	switch (protyp)
	  {
	  case momtypenc__none:
	    break;
	  case momtypenc_int:
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("0L"));
	    break;
	  case momtypenc_double:
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("0.0"));
	    break;
	  case momtypenc_val:
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("MOM_NULLV"));
	  case momtypenc_string:
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("NULL"));
	    break;
	  }
      }
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (";"),
	     MOMOUT_INDENT_LESS (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("} // end proc.code "),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
    mom_item_hset_check_integrity ("emitproc end hsetintitm",
				   cg->cgen_rout.cgrout_hsetintitm);
    mom_item_hset_check_integrity ("emitproc end hsetdblitm",
				   cg->cgen_rout.cgrout_hsetdblitm);
    mom_item_hset_check_integrity ("emitproc end hsetvalitm",
				   cg->cgen_rout.cgrout_hsetvalitm);
    // emit the constant ids
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("static const char* const "
			     CGEN_PROC_CONSTANTIDS_PREFIX),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL ("["), MOMOUT_DEC_INT (1 + (int) nbconstants),
	     MOMOUT_LITERAL ("] = { //! for "),
	     MOMOUT_ITEM ((const momitem_t *) procitm), MOMOUT_INDENT_MORE (),
	     MOMOUT_NEWLINE (), NULL);
    MOM_DEBUG			///
      (gencod,
       MOMOUT_LITERAL ("emitproc proconstantsv="),
       MOMOUT_VALUE ((const momval_t) proconstantsv), NULL);
    for (unsigned cix = 0; cix < nbconstants; cix++)
      {
	const momitem_t *curcstitm =
	  mom_seqitem_nth_item (proconstantsv, cix);
	MOM_DEBUG		///
	  (gencod,
	   MOMOUT_LITERAL ("emitproc cix#"),
	   MOMOUT_DEC_INT ((int) cix),
	   MOMOUT_LITERAL (" curcstitm:"),
	   MOMOUT_ITEM ((const momitem_t *) curcstitm), NULL);
	assert (curcstitm != NULL);
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL ("\""),
		 MOMOUT_LITERALV (mom_ident_cstr_of_item (curcstitm)),
		 MOMOUT_LITERAL ("\", //="),
		 MOMOUT_ITEM ((const momitem_t *) curcstitm), NULL);
      }
    MOM_OUT (&cg->cgen_outbody, MOMOUT_INDENT_LESS (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" NULL }; // end constid proc "),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_NEWLINE (), NULL);
    // emit the descriptor
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("const struct momprocrout_st "
			     MOM_PROCROUTDESCR_PREFIX),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL ("// proc.descriptor "),
	     MOMOUT_ITEM ((const momitem_t *) procitm), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" = { .prout_magic = MOM_PROCROUT_MAGIC,"),
	     MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (), NULL);
    switch (protyp)
      {
      case momtypenc__none:
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL (".prout_resty = momtypenc__none,"));
	break;
      case momtypenc_int:
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL (".prout_resty = momtypenc_int,"));
	break;
      case momtypenc_double:
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL (".prout_resty = momtypenc_double,"));
	break;
      case momtypenc_val:
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL (".prout_resty = momtypenc_val,"));
	break;
      case momtypenc_string:
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL (".prout_resty = momtypenc_string,"));
	break;
      };
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_len = "),
	     MOMOUT_DEC_INT ((int) nbconstants), MOMOUT_LITERAL (","),
	     MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_id = \""),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_module = \""),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (cg->cgen_moditm)),
	     MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_constantids = "
			     CGEN_PROC_CONSTANTIDS_PREFIX),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL (","), MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_constantitems = "
			     CGEN_PROC_CONSTANTITEMS_PREFIX),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL (","), MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_addr = (void*)"
			     MOM_PROCROUTFUN_PREFIX),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL (","), MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (".prout_argsig = \""),
	     MOMOUT_LITERALV (mom_string_cstr
			      (mom_node_nth (procnodev, 1))),
	     MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE ());
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (".prout_timestamp= __DATE__ \"@\" __TIME__"),
	     MOMOUT_INDENT_LESS (), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("}; // end proc descriptor"),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
    /// emit the constant items declarations
    MOM_OUT (&cg->cgen_outhead,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("// constant items of procedure "),
	     MOMOUT_ITEM ((const momitem_t *) procitm),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("static const momitem_t* "
			     CGEN_PROC_CONSTANTITEMS_PREFIX),
	     MOMOUT_LITERALV (mom_ident_cstr_of_item (procitm)),
	     MOMOUT_LITERAL ("["), MOMOUT_DEC_INT (1 + (int) nbconstants),
	     MOMOUT_LITERAL ("];"), MOMOUT_NEWLINE (), NULL);
  }
  //// append the block hset to the procedure associated node
  {
    const momitem_t *hsetblkitm =
      (const momitem_t *) cg->cgen_rout.cgrout_blockhsetitm;
    MOM_DEBUG
      (gencod, MOMOUT_LITERAL ("emit_procedure ending old procnodev="),
       MOMOUT_VALUE ((const momval_t) procnodev),
       MOMOUT_NEWLINE (),
       MOMOUT_LITERAL (" hsetblkitm="),
       MOMOUT_ITEM ((const momitem_t *) hsetblkitm), NULL);
    ///
    momval_t newprocnodev =
      (momval_t) mom_make_node_sized (procnodev, 1, (momval_t) hsetblkitm,
				      NULL);
    mom_item_assoc_put (cg->cgen_globassocitm, procitm, newprocnodev);
    MOM_DEBUG
      (gencod, MOMOUT_LITERAL ("emit_procedure ending updated procnodev="),
       MOMOUT_VALUE ((const momval_t) newprocnodev),
       MOMOUT_NEWLINE (),
       MOMOUT_LITERAL ("for procitm:"),
       MOMOUT_ITEM ((const momitem_t *) procitm),
       MOMOUT_LITERAL ("updated globassocitm:"),
       MOMOUT_ITEM ((const momitem_t *) cg->cgen_globassocitm),
       MOMOUT_NEWLINE (),
       MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->cgen_globassocitm),
       MOMOUT_NEWLINE (), NULL);
  }
  return;
}				/* end of emit_procedure_cgen */



////////////////////////////////////////////////////////////////
void
scan_taskletfunction_cgen (struct c_generator_mom_st *cg, momitem_t *tfunitm)
{
  momval_t tfunformalsv = MOM_NULLV;
  momval_t tfunconstantsv = MOM_NULLV;
  momval_t tfunlocalsv = MOM_NULLV;
  momval_t tfunclosedvalsv = MOM_NULLV;
  momval_t tfuntaskletfunctionv = MOM_NULLV;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (tfunitm && tfunitm->i_typnum == momty_item);
  cgen_lock_item_mom (cg, tfunitm);
  momval_t tfunodev = mom_item_assoc_get (cg->cgen_globassocitm, tfunitm);
  tfunformalsv = mom_item_get_attribute (tfunitm, mom_named__formals);
  tfunconstantsv = mom_item_get_attribute (tfunitm, mom_named__constants);
  tfunclosedvalsv =
    mom_item_get_attribute (tfunitm, mom_named__closed_values);
  tfunlocalsv = mom_item_get_attribute (tfunitm, mom_named__locals);
  tfuntaskletfunctionv =
    mom_item_get_attribute (tfunitm, mom_named__tasklet_function);
  MOM_DEBUG (gencod,
	     MOMOUT_LITERAL ("start scan_taskletfunction_cgen tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_SPACE (60),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) tfunitm),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL ("tfunodev="),
	     MOMOUT_VALUE ((const momval_t) tfunodev), MOMOUT_NEWLINE (),
	     NULL);
  // scan the formals
  if (tfunformalsv.ptr && !mom_is_tuple (tfunformalsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in function:"),
		    MOMOUT_ITEM ((const momitem_t *) tfunitm),
		    MOMOUT_LITERAL (" bad formals:"),
		    MOMOUT_VALUE (tfunformalsv), NULL);
  unsigned nbformals = mom_tuple_length (tfunformalsv);
  for (unsigned fix = 0; fix < nbformals; fix++)
    {
      momitem_t *curformitm = mom_tuple_nth_item (tfunformalsv, fix);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("scan_taskletfunction tfunitm="),
		 MOMOUT_ITEM ((const momitem_t *) tfunitm),
		 MOMOUT_LITERAL (" curformitm="),
		 MOMOUT_ITEM ((const momitem_t *) curformitm), NULL);
      if (!curformitm)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("nil formal in function:"),
			MOMOUT_ITEM ((const momitem_t *) tfunitm),
			MOMOUT_LITERAL (" with formals:"),
			MOMOUT_VALUE (tfunformalsv), NULL);
      CGEN_CHECK_FRESH (cg, "formal of tfun", curformitm);
      cgen_lock_item_mom (cg, curformitm);
      momval_t formtypv =
	mom_item_get_attribute (curformitm, mom_named__variable);
      if (!formtypv.ptr)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("missing `variable` in formal:"),
			MOMOUT_ITEM ((const momitem_t *) curformitm),
			MOMOUT_LITERAL (" of tfun:"),
			MOMOUT_ITEM ((const momitem_t *) tfunitm), NULL);
      scan_item_cgen (cg, curformitm);
    }
  /// scan the constants of tfunction
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_tfunc tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_LITERAL (" tfunconstantsv="),
	     MOMOUT_VALUE ((const momval_t) tfunconstantsv), NULL);
  if (tfunconstantsv.ptr && !mom_is_seqitem (tfunconstantsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in tfunc:"),
		    MOMOUT_ITEM ((const momitem_t *) tfunitm),
		    MOMOUT_LITERAL (" bad constants:"),
		    MOMOUT_VALUE (tfunconstantsv), NULL);
  unsigned nbconstants = bind_constants_cgen (cg, tfunconstantsv);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_taskfun tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_LITERAL (" nbconstants#"),
	     MOMOUT_DEC_INT ((int) nbconstants),
	     MOMOUT_LITERAL (" tfuntaskletfunctionv="),
	     MOMOUT_VALUE ((const momval_t) tfuntaskletfunctionv), NULL);
  /// scan the explicitly given locals of tfunction
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_tfunc tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_LITERAL (" tfunlocalsv="),
	     MOMOUT_VALUE ((const momval_t) tfunlocalsv), NULL);
  if (tfunlocalsv.ptr && !mom_is_seqitem (tfunlocalsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in tfunc:"),
		    MOMOUT_ITEM ((const momitem_t *) tfunitm),
		    MOMOUT_LITERAL (" bad locals:"),
		    MOMOUT_VALUE (tfunlocalsv), NULL);
  unsigned nblocals = mom_seqitem_length (tfunlocalsv);
  for (unsigned ix = 0; ix < nblocals; ix++)
    {
      momitem_t *locvaritm = mom_seqitem_nth_item (tfunlocalsv, ix);
      if (!locvaritm)
	continue;
      scan_item_cgen (cg, locvaritm);
    }
  /// scan the closed values of tfunction
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_tfunc tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_LITERAL (" tfunclosedvalsv="),
	     MOMOUT_VALUE ((const momval_t) tfunclosedvalsv), NULL);
  if (tfunclosedvalsv.ptr && !mom_is_seqitem (tfunclosedvalsv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in tfunc:"),
		    MOMOUT_ITEM ((const momitem_t *) tfunitm),
		    MOMOUT_LITERAL (" bad `closed_values`:"),
		    MOMOUT_VALUE (tfunclosedvalsv), NULL);
  unsigned nbclosvals = bind_closed_values_cgen (cg, tfunclosedvalsv);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start scan_taskfun tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_LITERAL (" nbclosvals#"),
	     MOMOUT_DEC_INT ((int) nbclosvals),
	     MOMOUT_LITERAL (" tfuntaskletfunctionv="),
	     MOMOUT_VALUE ((const momval_t) tfuntaskletfunctionv),
	     MOMOUT_LITERAL (" associtm="),
	     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
	     MOMOUT_SPACE (60),
	     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
				  cgen_rout.cgrout_associtm),
	     MOMOUT_NEWLINE (), NULL);
  // scan all the blocks from the start
  if (!mom_is_item (tfuntaskletfunctionv))
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in tfun:"),
		    MOMOUT_ITEM ((const momitem_t *) tfunitm),
		    MOMOUT_LITERAL
		    (" non-item starting block `tasklet_function`:"),
		    MOMOUT_VALUE (tfuntaskletfunctionv), NULL);
  scan_block_cgen (cg, tfuntaskletfunctionv.pitem, (momval_t) tfunitm, NULL);
  loop_blocks_to_scan_cgen (cg);
  MOM_DEBUG			//
    (gencod, MOMOUT_LITERAL ("end scan_taskfun tfunitm="),
     MOMOUT_ITEM ((const momitem_t *) tfunitm),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" associtm="),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
     MOMOUT_SPACE (60),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" blockhsetitm="),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_blockhsetitm),
     MOMOUT_SPACE (60),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_blockhsetitm), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" hsetint="),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetintitm),
     MOMOUT_SPACE (60),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetintitm), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" hsetdbl="),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetdblitm),
     MOMOUT_SPACE (60),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetdblitm), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (" hsetval="),
     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_hsetvalitm),
     MOMOUT_SPACE (60),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetvalitm), MOMOUT_NEWLINE (),
     NULL);
}


// for qsort_r
static int
cmpr_valitems_by_rank_cgen (const void *p1, const void *p2, void *data)
{
  momitem_t *itm1 = *(momitem_t **) p1;
  momitem_t *itm2 = *(momitem_t **) p2;
  momitem_t *asitm = data;
  assert (itm1 && itm1->i_typnum == momty_item);
  assert (itm2 && itm2->i_typnum == momty_item);
  assert (asitm && asitm->i_typnum == momty_item
	  && asitm->i_paylkind == mompayk_assoc);
  momval_t nod1 = mom_item_assoc_get (asitm, itm1);
  momval_t nod2 = mom_item_assoc_get (asitm, itm2);
  assert (mom_node_conn (nod1) == mom_named__momval_t);
  assert (mom_node_conn (nod2) == mom_named__momval_t);
  int rk1 = mom_integer_val_def (mom_node_nth (nod1, 0), -1);
  int rk2 = mom_integer_val_def (mom_node_nth (nod2, 0), -1);
  assert (rk1 >= 0 && rk2 >= 0);
  return rk1 - rk2;
}

// for qsort_r
static int
cmpr_intitems_by_rank_cgen (const void *p1, const void *p2, void *data)
{
  momitem_t *itm1 = *(momitem_t **) p1;
  momitem_t *itm2 = *(momitem_t **) p2;
  momitem_t *asitm = data;
  assert (itm1 && itm1->i_typnum == momty_item);
  assert (itm2 && itm2->i_typnum == momty_item);
  assert (asitm && asitm->i_typnum == momty_item
	  && asitm->i_paylkind == mompayk_assoc);
  momval_t nod1 = mom_item_assoc_get (asitm, itm1);
  momval_t nod2 = mom_item_assoc_get (asitm, itm2);
  assert (mom_node_conn (nod1) == mom_named__intptr_t);
  assert (mom_node_conn (nod2) == mom_named__intptr_t);
  int rk1 = mom_integer_val_def (mom_node_nth (nod1, 0), -1);
  int rk2 = mom_integer_val_def (mom_node_nth (nod2, 0), -1);
  assert (rk1 >= 0 && rk2 >= 0);
  return rk1 - rk2;
}

// for qsort_r
static int
cmpr_dblitems_by_rank_cgen (const void *p1, const void *p2, void *data)
{
  momitem_t *itm1 = *(momitem_t **) p1;
  momitem_t *itm2 = *(momitem_t **) p2;
  momitem_t *asitm = data;
  assert (itm1 && itm1->i_typnum == momty_item);
  assert (itm2 && itm2->i_typnum == momty_item);
  assert (asitm && asitm->i_typnum == momty_item
	  && asitm->i_paylkind == mompayk_assoc);
  momval_t nod1 = mom_item_assoc_get (asitm, itm1);
  momval_t nod2 = mom_item_assoc_get (asitm, itm2);
  assert (mom_node_conn (nod1) == mom_named__double);
  assert (mom_node_conn (nod2) == mom_named__double);
  int rk1 = mom_integer_val_def (mom_node_nth (nod1, 0), -1);
  int rk2 = mom_integer_val_def (mom_node_nth (nod2, 0), -1);
  assert (rk1 >= 0 && rk2 >= 0);
  return rk1 - rk2;
}

void
emit_taskletfunction_cgen (struct c_generator_mom_st *cg, unsigned routix)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momitem_t *tfunitm = cg->cgen_rout.cgrout_routitm;
  assert (tfunitm && tfunitm->i_typnum == momty_item);
  momval_t tfunodev = mom_item_assoc_get (cg->cgen_globassocitm, tfunitm);
  momval_t tfunclosedvalsv =
    mom_item_get_attribute (tfunitm, mom_named__closed_values);
  momval_t intvarsv = MOM_NULLV;
  momval_t dblvarsv = MOM_NULLV;
  momval_t valvarsv = MOM_NULLV;
  MOM_DEBUG (gencod,		//
	     MOMOUT_LITERAL ("emit_tfun tfunitm="),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_SPACE (58),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) tfunitm),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" tfunodev="),
	     MOMOUT_VALUE ((momval_t) tfunodev),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" local-assoc:"),
	     MOMOUT_ITEM ((const momitem_t *) cg->cgen_rout.cgrout_associtm),
	     MOMOUT_SPACE (50),
	     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
				  cgen_rout.cgrout_associtm),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" blockhset:"),
	     MOMOUT_ITEM ((const momitem_t *) cg->
			  cgen_rout.cgrout_blockhsetitm), MOMOUT_SPACE (50),
	     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
				  cgen_rout.cgrout_blockhsetitm),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" hsetint:"),
	     MOMOUT_ITEM ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetintitm), MOMOUT_SPACE (50),
	     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
				  cgen_rout.cgrout_hsetintitm),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" hsetdbl:"),
	     MOMOUT_ITEM ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetdblitm), MOMOUT_SPACE (50),
	     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
				  cgen_rout.cgrout_hsetdblitm),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL (" hsetval:"),
	     MOMOUT_ITEM ((const momitem_t *) cg->
			  cgen_rout.cgrout_hsetvalitm), MOMOUT_SPACE (50),
	     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->
				  cgen_rout.cgrout_hsetvalitm), NULL);
  unsigned nbnumbers =		//
    mom_item_hset_count (cg->cgen_rout.cgrout_hsetintitm);
  unsigned nbdoubles =		//
    mom_item_hset_count (cg->cgen_rout.cgrout_hsetdblitm);
  unsigned nbvalues =		//
    mom_item_hset_count (cg->cgen_rout.cgrout_hsetvalitm);
  momval_t valsetv =
    mom_item_hset_items_set (cg->cgen_rout.cgrout_hsetvalitm);
  momval_t intsetv =
    mom_item_hset_items_set (cg->cgen_rout.cgrout_hsetintitm);
  momval_t dblsetv =
    mom_item_hset_items_set (cg->cgen_rout.cgrout_hsetdblitm);
  momval_t blocksetv =		//
    mom_item_hset_items_set (cg->cgen_rout.cgrout_blockhsetitm);
  MOM_DEBUG (gencod,
	     MOMOUT_LITERAL (" emit_tfun blocksetv="),
	     MOMOUT_VALUE (blocksetv), NULL);
  mom_item_hset_check_integrity ("emitfun start hsetintitm",
				 cg->cgen_rout.cgrout_hsetintitm);
  mom_item_hset_check_integrity ("emitfun start hsetdblitm",
				 cg->cgen_rout.cgrout_hsetdblitm);
  mom_item_hset_check_integrity ("emitfun start hsetvalitm",
				 cg->cgen_rout.cgrout_hsetvalitm);
  unsigned nbblocks = mom_set_cardinal (blocksetv);
  unsigned nbclosedvals = mom_seqitem_length (tfunclosedvalsv);
  momval_t funconstantsv
    = mom_item_get_attribute (tfunitm, mom_named__constants);
  unsigned nbconstants = mom_seqitem_length (funconstantsv);
  MOM_OUT
    (&cg->cgen_outbody,
     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("// implement tasklet function "),
     MOMOUT_ITEM ((const momitem_t *) tfunitm),
     MOMOUT_LITERAL (" rank#"), MOMOUT_DEC_INT ((int) routix),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("static int " CGEN_FUN_CODE_PREFIX),
     MOMOUT_LITERALV (mom_ident_cstr_of_item (tfunitm)),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL
     ("(int momstate,"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL
     ("\t momitem_t* restrict momtasklet, const momval_t momclosure,"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERALV ((const char *)
		      ((nbvalues > 0)
		       ? "\t momval_t* restrict momvals,"
		       :
		       "\t momval_t* restrict momvals_ MOM_UNUSED,")),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERALV ((const char *) ((nbnumbers > 0) ?
				      "\t intptr_t* restrict momnums,"
				      :
				      "\t intptr_t* restrict momnums_ MOM_UNUSED,")),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERALV ((const char *) ((nbdoubles > 0) ?
				      "\t double* restrict momdbls)"
				      :
				      "\t double* restrict momdbls_ MOM_UNUSED)")),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("{ // start of tasklet function "),
     MOMOUT_ITEM ((const momitem_t *) tfunitm),
     MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (), NULL);
  momitem_t *startblockitm =
    mom_value_to_item (mom_item_get_attribute (tfunitm,
					       mom_named__tasklet_function));
  int startix = 1 + mom_set_item_index (blocksetv, startblockitm);
  MOM_DEBUG (gencod,
	     MOMOUT_LITERAL (" emit_tfun startblockitm="),
	     MOMOUT_ITEM ((const momitem_t *) startblockitm),
	     MOMOUT_LITERAL (" startix="), MOMOUT_DEC_INT (startix));
  assert (startix > 0);
  MOM_OUT
    (&cg->cgen_outbody,
     MOMOUT_LITERAL
     ("assert (mom_item_payload_kind (momtasklet) == mompayk_tasklet);"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("if (MOM_UNLIKELY(momstate==0)) {"),
     MOMOUT_INDENT_MORE (),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("momstate = "),
     MOMOUT_DEC_INT (startix), MOMOUT_LITERAL (";"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("mom_item_tasklet_top_frame_set_state (momtasklet, "),
     MOMOUT_DEC_INT (startix), MOMOUT_LITERAL (");"),
     MOMOUT_INDENT_LESS (),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("};"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("assert (mom_is_item (momclosure));"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("momval_t* momclovals ="
		     " mom_item_closure_values (momclosure.pitem);"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("assert (momclovals != NULL);"),
     MOMOUT_NEWLINE (), NULL);
  if (nbvalues > 0)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL
	     ("assert (momvals != NULL); // "),
	     MOMOUT_DEC_INT ((int) nbvalues),
	     MOMOUT_LITERAL (" values."), MOMOUT_NEWLINE (), NULL);
  if (nbnumbers > 0)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL
	     ("assert (momnums != NULL); //"),
	     MOMOUT_DEC_INT ((int)
			     nbnumbers),
	     MOMOUT_LITERAL (" integer numbers."), MOMOUT_NEWLINE (), NULL);
  if (nbdoubles > 0)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL
	     ("assert (momdbls != NULL); //"),
	     MOMOUT_DEC_INT ((int)
			     nbdoubles),
	     MOMOUT_LITERAL (" doubles."), MOMOUT_NEWLINE (), NULL);
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL
	   (" MOM_DEBUG(run,"),
	   MOMOUT_INDENT_MORE (),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   (" MOMOUT_LITERAL(\"start tasklet=\"),"
	    " MOMOUT_ITEM((const momitem_t*)momtasklet),"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (" MOMOUT_LITERAL(\" state#\"),"
			   " MOMOUT_DEC_INT((int)momstate),"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL (" MOMOUT_LITERAL(\" taskfunc "),
	   MOMOUT_ITEM ((const momitem_t *) tfunitm),
	   MOMOUT_LITERAL ("\"));"), MOMOUT_INDENT_LESS (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("switch (momstate) {"), MOMOUT_NEWLINE ());
  for (unsigned six = 1; six <= nbblocks; six++)
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL ("case "),
	       MOMOUT_DEC_INT ((int) six),
	       MOMOUT_LITERAL (": goto " CGEN_FUN_BLOCK_PREFIX),
	       MOMOUT_DEC_INT ((int) six),
	       MOMOUT_LITERAL ("; // block "),
	       MOMOUT_ITEM ((const momitem_t *)
			    mom_seqitem_nth_item (blocksetv, six - 1)),
	       MOMOUT_NEWLINE (), NULL);
    }
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL ("default: return momroutres_pop;"),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("}; // end switch state"),
	   MOMOUT_NEWLINE (), NULL);
  for (unsigned bix = 1; bix <= nbblocks; bix++)
    {
      momitem_t *blkitm = mom_seqitem_nth_item (blocksetv, bix - 1);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// +++++++ function block #"),
	       MOMOUT_DEC_INT ((int) bix), MOMOUT_LITERAL (" "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (CGEN_FUN_BLOCK_PREFIX),
	       MOMOUT_DEC_INT ((int) bix), MOMOUT_LITERAL (":"),
	       MOMOUT_NEWLINE (), NULL);
      mom_item_hset_check_integrity ("emitfun bixloop hsetintitm",
				     cg->cgen_rout.cgrout_hsetintitm);
      mom_item_hset_check_integrity ("emitfun bixloop hsetdblitm",
				     cg->cgen_rout.cgrout_hsetdblitm);
      mom_item_hset_check_integrity ("emitfun bixloop hsetvalitm",
				     cg->cgen_rout.cgrout_hsetvalitm);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("{"), MOMOUT_INDENT_MORE (),
	       MOMOUT_NEWLINE (), NULL);
      emit_block_cgen (cg, blkitm);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_INDENT_LESS (),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("}; // -------- end function block "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("return momroutres_pop;"),
	       MOMOUT_NEWLINE (), NULL);
    }
  mom_item_hset_check_integrity ("emitfun end hsetintitm",
				 cg->cgen_rout.cgrout_hsetintitm);
  mom_item_hset_check_integrity ("emitfun end hsetdblitm",
				 cg->cgen_rout.cgrout_hsetdblitm);
  mom_item_hset_check_integrity ("emitfun end hsetvalitm",
				 cg->cgen_rout.cgrout_hsetvalitm);
  // emit end of function
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_INDENT_LESS (),
	   MOMOUT_LITERAL ("} // end function "),
	   MOMOUT_ITEM ((const momitem_t *) tfunitm),
	   MOMOUT_INDENT_LESS (), MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  // emit the function constant values and constant ids
  {
    MOM_OUT			///
      (&cg->cgen_outhead,
       MOMOUT_NEWLINE (),
       MOMOUT_LITERAL (" // constant items of tasklet function "),
       MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (),
       MOMOUT_LITERAL ("static momitem_t* "
		       CGEN_FUN_CONSTANTITEMS_PREFIX),
       MOMOUT_LITERALV (mom_ident_cstr_of_item (tfunitm)),
       MOMOUT_LITERAL ("["),
       MOMOUT_DEC_INT ((int) nbconstants + 1), MOMOUT_LITERAL ("];"), NULL);
    MOM_OUT			///
      (&cg->cgen_outbody,
       MOMOUT_LITERAL ("static const char* const "
		       CGEN_FUN_CONSTANTIDS_PREFIX),
       MOMOUT_LITERALV (mom_ident_cstr_of_item
			(tfunitm)),
       MOMOUT_LITERAL ("["),
       MOMOUT_DEC_INT ((int) nbconstants + 1),
       MOMOUT_LITERAL
       ("] = { // constant ids of function "),
       MOMOUT_ITEM ((const momitem_t *) tfunitm),
       MOMOUT_INDENT_MORE (), NULL);
    for (unsigned cix = 0; cix < nbconstants; cix++)
      {
	momitem_t *cstitm = mom_seqitem_nth_item (funconstantsv, cix);
	if (!cstitm)
	  continue;
	MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL ("["), MOMOUT_DEC_INT ((int) cix),
		 MOMOUT_LITERAL ("] = \""),
		 MOMOUT_LITERALV (mom_ident_cstr_of_item (cstitm)),
		 MOMOUT_LITERAL ("\", // "),
		 MOMOUT_ITEM ((const momitem_t *) cstitm), NULL);
      }
    MOM_OUT (&cg->cgen_outbody, MOMOUT_INDENT_LESS (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL
	     ("}; // end of function constants of "),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  }
  // emit the block ids
  {
    MOM_OUT			///
      (&cg->cgen_outbody,
       MOMOUT_LITERAL ("static const char* const "
		       CGEN_FUN_BLOCKIDS_PREFIX),
       MOMOUT_LITERALV (mom_ident_cstr_of_item
			(tfunitm)),
       MOMOUT_LITERAL ("["),
       MOMOUT_DEC_INT ((int) nbblocks + 2),
       MOMOUT_LITERAL
       ("] = { // block ids of function "),
       MOMOUT_ITEM ((const momitem_t *) tfunitm),
       MOMOUT_INDENT_MORE (), NULL);
    for (unsigned bix = 0; bix <= nbblocks; bix++)
      {
	momitem_t *blkitm = mom_seqitem_nth_item (blocksetv, bix);
	if (!blkitm)
	  continue;
	MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		 MOMOUT_LITERAL ("["), MOMOUT_DEC_INT ((int) bix),
		 MOMOUT_LITERAL ("] = \""),
		 MOMOUT_LITERALV (mom_ident_cstr_of_item (blkitm)),
		 MOMOUT_LITERAL ("\", //!block "),
		 MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
      }
    MOM_OUT (&cg->cgen_outbody, MOMOUT_INDENT_LESS (),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL
	     ("}; // end of function block-ids of "),
	     MOMOUT_ITEM ((const momitem_t *) tfunitm),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  }
  // emit the value var.ids sorted by rank
  if (nbvalues > 0)
    {
      // collect and sort the valitems
      momitem_t *tinyitemsarr[MOM_TINY_MAX];
      memset (tinyitemsarr, 0, sizeof (tinyitemsarr));
      momitem_t **valitemsarr =	//
	(nbvalues < MOM_TINY_MAX - 1) ? tinyitemsarr	//
	: MOM_GC_ALLOC ("valitemsarr",
			(nbvalues + 1) * sizeof (momitem_t *));
      const momitem_t *asitm = cg->cgen_rout.cgrout_associtm;
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_tfun valsetv="),
		 MOMOUT_VALUE (valsetv),
		 MOMOUT_LITERAL (" asitm="),
		 MOMOUT_ITEM (asitm),
		 MOMOUT_SPACE (60), MOMOUT_ITEM_PAYLOAD (asitm), NULL);
      assert (mom_is_set (valsetv));
      memcpy (valitemsarr, valsetv.pset->itemseq,
	      nbvalues * sizeof (momitem_t *));
      qsort_r (valitemsarr, nbvalues, sizeof (momitem_t *),
	       cmpr_valitems_by_rank_cgen, (void *) asitm);
      valvarsv =
	(momval_t) mom_make_node_from_array ((momval_t) mom_named__values,
					     nbvalues,
					     (momval_t *) valitemsarr);
      /// emit them
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("static const char* const "
			 CGEN_FUN_LOCVALIDS_PREFIX),
	 MOMOUT_LITERALV (mom_ident_cstr_of_item
			  (tfunitm)),
	 MOMOUT_LITERAL ("["),
	 MOMOUT_DEC_INT ((int) nbvalues + 1),
	 MOMOUT_LITERAL
	 ("] = { // value var.ids of function "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm),
	 MOMOUT_INDENT_MORE (), NULL);
      for (unsigned vix = 0; vix < nbvalues; vix++)
	{
	  const momitem_t *curvalitm = (const momitem_t *) valitemsarr[vix];
	  assert (curvalitm != NULL);
	  momval_t asnodval =
	    mom_item_assoc_get ((momitem_t *) asitm, curvalitm);
	  int vnum = mom_integer_val_def (mom_node_nth (asnodval, 0), -1);
	  MOM_DEBUG		//
	    (gencod,
	     MOMOUT_LITERAL ("emit_tfun vix="), MOMOUT_DEC_INT ((int) vix),
	     MOMOUT_LITERAL (" curvalitm:"), MOMOUT_ITEM (curvalitm),
	     MOMOUT_LITERAL (" asnodval:"),
	     MOMOUT_VALUE ((const momval_t) asnodval),
	     MOMOUT_LITERAL ("; vnum#"), MOMOUT_DEC_INT ((int) vnum), NULL);
	  assert (vnum == (int) vix);
	  MOM_OUT		///
	    (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" ["),
	     MOMOUT_DEC_INT ((int) vnum),
	     MOMOUT_LITERAL ("] = \""),
	     MOMOUT_LITERALV ((const char *)
			      mom_ident_cstr_of_item (curvalitm)),
	     MOMOUT_LITERAL ("\", //!! val.var. "),
	     MOMOUT_ITEM ((const momitem_t *) curvalitm), NULL);
	}
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_INDENT_LESS (),
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("}; // end val.var. of "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (), NULL);
      if (valitemsarr != tinyitemsarr)
	MOM_GC_FREE (valitemsarr);
    }
  else
    {
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("// no val.var.ids for "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (), NULL);
    };
  // emit the integers ids sorted by rank
  if (nbnumbers > 0)
    {
      // collect and sort the intitems
      momitem_t *tinyitemsarr[MOM_TINY_MAX];
      memset (tinyitemsarr, 0, sizeof (tinyitemsarr));
      momitem_t **intitemsarr =	//
	(nbnumbers < MOM_TINY_MAX - 1) ? tinyitemsarr	//
	: MOM_GC_ALLOC ("intitemsarr",
			(nbnumbers + 1) * sizeof (momitem_t *));
      const momitem_t *asitm = cg->cgen_rout.cgrout_associtm;
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_tfun intsetv="),
		 MOMOUT_VALUE (intsetv),
		 MOMOUT_LITERAL (" asitm="),
		 MOMOUT_ITEM (asitm),
		 MOMOUT_SPACE (60), MOMOUT_ITEM_PAYLOAD (asitm), NULL);
      assert (mom_is_set (intsetv));
      memcpy (intitemsarr, intsetv.pset->itemseq,
	      nbnumbers * sizeof (momitem_t *));
      qsort_r (intitemsarr, nbnumbers, sizeof (momitem_t *),
	       cmpr_intitems_by_rank_cgen, (void *) asitm);
      intvarsv =
	(momval_t) mom_make_node_sized ((momval_t) mom_named__numbers,
					nbnumbers, (momval_t *) intitemsarr);
      /// emit them
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("static const char* const "
			 CGEN_FUN_LOCINTIDS_PREFIX),
	 MOMOUT_LITERALV (mom_ident_cstr_of_item
			  (tfunitm)),
	 MOMOUT_LITERAL ("["),
	 MOMOUT_DEC_INT ((int) nbnumbers + 1),
	 MOMOUT_LITERAL
	 ("] = { // integer var.ids of function "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm),
	 MOMOUT_INDENT_MORE (), NULL);
      for (unsigned vix = 0; vix < nbnumbers; vix++)
	{
	  const momitem_t *curintitm = (const momitem_t *) intitemsarr[vix];
	  assert (curintitm != NULL);
	  momval_t asnodint =
	    mom_item_assoc_get ((momitem_t *) asitm, curintitm);
	  int vnum = mom_integer_val_def (mom_node_nth (asnodint, 0), -1);
	  MOM_DEBUG		//
	    (gencod,
	     MOMOUT_LITERAL ("emit_tfun vix="), MOMOUT_DEC_INT ((int) vix),
	     MOMOUT_LITERAL (" curintitm:"), MOMOUT_ITEM (curintitm),
	     MOMOUT_LITERAL (" asnodint:"),
	     MOMOUT_VALUE ((const momval_t) asnodint),
	     MOMOUT_LITERAL ("; vnum#"), MOMOUT_DEC_INT ((int) vnum), NULL);
	  assert (vnum == (int) vix);
	  MOM_OUT		///
	    (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" ["),
	     MOMOUT_DEC_INT ((int) vnum),
	     MOMOUT_LITERAL ("] = \""),
	     MOMOUT_LITERALV ((const char *)
			      mom_ident_cstr_of_item (curintitm)),
	     MOMOUT_LITERAL ("\", //!! int.var. "),
	     MOMOUT_ITEM ((const momitem_t *) curintitm), NULL);
	}
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_INDENT_LESS (),
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("}; // end int.var. of "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (), NULL);
      if (intitemsarr != tinyitemsarr)
	MOM_GC_FREE (intitemsarr);
    }
  else
    {
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("// no number.var.ids for "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (), NULL);
    };
  // emit the double var. ids sorted by rank
  if (nbdoubles > 0)
    {
      // collect and sort the dblitems
      momitem_t *tinyitemsarr[MOM_TINY_MAX];
      memset (tinyitemsarr, 0, sizeof (tinyitemsarr));
      momitem_t **dblitemsarr =	//
	(nbdoubles < MOM_TINY_MAX - 1) ? tinyitemsarr	//
	: MOM_GC_ALLOC ("dblitemsarr",
			(nbdoubles + 1) * sizeof (momitem_t *));
      const momitem_t *asitm = cg->cgen_rout.cgrout_associtm;
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_tfun dblsetv="),
		 MOMOUT_VALUE (dblsetv),
		 MOMOUT_LITERAL (" asitm="),
		 MOMOUT_ITEM (asitm),
		 MOMOUT_SPACE (60), MOMOUT_ITEM_PAYLOAD (asitm), NULL);
      assert (mom_is_set (dblsetv));
      memcpy (dblitemsarr, dblsetv.pset->itemseq,
	      nbdoubles * sizeof (momitem_t *));
      qsort_r (dblitemsarr, nbdoubles, sizeof (momitem_t *),
	       cmpr_dblitems_by_rank_cgen, (void *) asitm);
      dblvarsv =
	(momval_t) mom_make_node_sized ((momval_t) mom_named__doubles,
					nbdoubles, (momval_t *) dblitemsarr);
      /// emit them
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("static const char* const "
			 CGEN_FUN_LOCDBLIDS_PREFIX),
	 MOMOUT_LITERALV (mom_ident_cstr_of_item
			  (tfunitm)),
	 MOMOUT_LITERAL ("["),
	 MOMOUT_DEC_INT ((int) nbdoubles + 1),
	 MOMOUT_LITERAL
	 ("] = { // integer var.ids of function "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm),
	 MOMOUT_INDENT_MORE (), NULL);
      for (unsigned vix = 0; vix < nbdoubles; vix++)
	{
	  const momitem_t *curdblitm = (const momitem_t *) dblitemsarr[vix];
	  assert (curdblitm != NULL);
	  momval_t asnoddbl =
	    mom_item_assoc_get ((momitem_t *) asitm, curdblitm);
	  int vnum = mom_integer_val_def (mom_node_nth (asnoddbl, 0), -1);
	  MOM_DEBUG		//
	    (gencod,
	     MOMOUT_LITERAL ("emit_tfun vix="), MOMOUT_DEC_INT ((int) vix),
	     MOMOUT_LITERAL (" curdblitm:"), MOMOUT_ITEM (curdblitm),
	     MOMOUT_LITERAL (" asnoddbl:"),
	     MOMOUT_VALUE ((const momval_t) asnoddbl),
	     MOMOUT_LITERAL ("; vnum#"), MOMOUT_DEC_INT ((int) vnum), NULL);
	  assert (vnum == (int) vix);
	  MOM_OUT		///
	    (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" ["),
	     MOMOUT_DEC_INT ((int) vnum),
	     MOMOUT_LITERAL ("] = \""),
	     MOMOUT_LITERALV ((const char *)
			      mom_ident_cstr_of_item (curdblitm)),
	     MOMOUT_LITERAL ("\", //!! dbl.var. "),
	     MOMOUT_ITEM ((const momitem_t *) curdblitm), NULL);
	}
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_INDENT_LESS (),
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("}; // end dbl.var. of "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (), NULL);
      if (dblitemsarr != tinyitemsarr)
	MOM_GC_FREE (dblitemsarr);
    }
  else
    {
      MOM_OUT			///
	(&cg->cgen_outbody,
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("// no dbl.var.ids for "),
	 MOMOUT_ITEM ((const momitem_t *) tfunitm), MOMOUT_NEWLINE (), NULL);
    };
  // emit the function descriptor
  MOM_OUT			///
    (&cg->cgen_outbody,
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("const struct momtfundescr_st "
		     MOM_TFUN_NAME_PREFIX),
     MOMOUT_LITERALV (mom_ident_cstr_of_item
		      (tfunitm)),
     MOMOUT_LITERAL
     (" = { // tasklet function descriptor "),
     MOMOUT_ITEM ((const momitem_t *) tfunitm),
     MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_magic = MOM_TFUN_MAGIC,"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_minclosize = "),
     MOMOUT_DEC_INT ((int) nbclosedvals),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_nbconstants = "),
     MOMOUT_DEC_INT ((int) nbconstants),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_nbblocks = "),
     MOMOUT_DEC_INT ((int) nbblocks),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_frame_nbval = "),
     MOMOUT_DEC_INT ((int) nbvalues),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_frame_nbnum = "),
     MOMOUT_DEC_INT ((int) nbnumbers),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_frame_nbdbl = "),
     MOMOUT_DEC_INT ((int) nbdoubles),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_constantids = "
		     CGEN_FUN_CONSTANTIDS_PREFIX),
     MOMOUT_LITERALV (mom_ident_cstr_of_item
		      (tfunitm)),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL
     (".tfun_constantitems = (const momitem_t*const*) "
      CGEN_FUN_CONSTANTITEMS_PREFIX),
     MOMOUT_LITERALV (mom_ident_cstr_of_item
		      (tfunitm)),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_ident = \""),
     MOMOUT_LITERALV (mom_ident_cstr_of_item
		      (tfunitm)),
     MOMOUT_LITERAL ("\","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_blockids = "
		     CGEN_FUN_BLOCKIDS_PREFIX),
     MOMOUT_LITERALV (mom_ident_cstr_of_item
		      (tfunitm)),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (), NULL);
  if (nbvalues > 0)
    MOM_OUT			///
      (&cg->cgen_outbody,
       MOMOUT_LITERAL (".tfun_valids = "
		       CGEN_FUN_LOCVALIDS_PREFIX),
       MOMOUT_LITERALV (mom_ident_cstr_of_item
			(tfunitm)),
       MOMOUT_LITERAL (","), MOMOUT_NEWLINE (), NULL);
  if (nbnumbers > 0)
    MOM_OUT			///
      (&cg->cgen_outbody,
       MOMOUT_LITERAL (".tfun_numids = "
		       CGEN_FUN_LOCINTIDS_PREFIX),
       MOMOUT_LITERALV (mom_ident_cstr_of_item
			(tfunitm)),
       MOMOUT_LITERAL (","), MOMOUT_NEWLINE (), NULL);
  if (nbdoubles > 0)
    MOM_OUT			///
      (&cg->cgen_outbody,
       MOMOUT_LITERAL (".tfun_dblids = "
		       CGEN_FUN_LOCDBLIDS_PREFIX),
       MOMOUT_LITERALV (mom_ident_cstr_of_item
			(tfunitm)),
       MOMOUT_LITERAL (","), MOMOUT_NEWLINE (), NULL);
  MOM_OUT			///
    (&cg->cgen_outbody,
     MOMOUT_LITERAL
     (".tfun_module = MONIMELT_CURRENT_MODULE,"),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL (".tfun_codefun = "
		     CGEN_FUN_CODE_PREFIX),
     MOMOUT_LITERALV (mom_ident_cstr_of_item
		      (tfunitm)),
     MOMOUT_LITERAL (","), MOMOUT_NEWLINE (),
     MOMOUT_LITERAL
     (".tfun_timestamp = __DATE__ \"@\" __TIME__"),
     MOMOUT_INDENT_LESS (),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("}; // end function descriptor of "),
     MOMOUT_ITEM ((const momitem_t *) tfunitm),
     MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
  /// update the associated node in the globalassoc
  {
    tfunodev = (momval_t) mom_make_node_sized (tfunodev, 4,
					       intvarsv, dblvarsv, valvarsv,
					       (momval_t) cg->
					       cgen_rout.cgrout_blockhsetitm);
    mom_item_assoc_put (cg->cgen_globassocitm, tfunitm, tfunodev);
    MOM_DEBUG
      (gencod, MOMOUT_LITERAL ("emit_tfun ending updated tfunodev="),
       MOMOUT_VALUE ((const momval_t) tfunodev),
       MOMOUT_NEWLINE (),
       MOMOUT_LITERAL ("for tfunitm:"),
       MOMOUT_ITEM ((const momitem_t *) tfunitm),
       MOMOUT_LITERAL ("updated globassocitm:"),
       MOMOUT_ITEM ((const momitem_t *) cg->cgen_globassocitm),
       MOMOUT_NEWLINE (),
       MOMOUT_ITEM_PAYLOAD ((const momitem_t *) cg->cgen_globassocitm),
       MOMOUT_NEWLINE (), NULL);
  }
  return;
}				/* end emit_taskletfunction_cgen */



momtypenc_t
emit_ctype_cgen (struct c_generator_mom_st *cg,
		 struct momout_st *out, momval_t val)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (!out || out->mout_magic == MOM_MOUT_MAGIC);
  momitem_t *typitm = NULL;
  if (mom_is_item (val))
    typitm = val.pitem;
  else if (mom_is_node (val))
    {
      typitm = (momitem_t *) mom_node_conn (val);
      if (typitm == mom_named__node || typitm == mom_named__set
	  || typitm == mom_named__tuple
	  || typitm == mom_named__json_array
	  || typitm == mom_named__json_object)
	return momtypenc_val;
    }
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
	  momval_t varv = MOM_NULLV;
	  {
	    cgen_lock_item_mom (cg, typitm);
	    ctypv = mom_item_get_attribute (typitm, mom_named__ctype);
	    resv = mom_item_get_attribute (typitm, mom_named__result);
	    varv = mom_item_get_attribute (typitm, mom_named__variable);
	    if (mom_is_item (ctypv))
	      typitm = ctypv.pitem;
	    else if (mom_is_item (resv))
	      typitm = resv.pitem;
	    else if (mom_is_item (varv))
	      typitm = varv.pitem;
	    else
	      typitm = NULL;
	  }
	};
      if (!typitm || count == 0)
	CGEN_ERROR_MOM		//
	  (cg,
	   MOMOUT_LITERAL ("invalid value to type:"),
	   MOMOUT_VALUE ((const momval_t) val),
	   MOMOUT_LITERAL (" typitm="),
	   MOMOUT_ITEM ((const momitem_t *) typitm),
	   MOMOUT_LITERAL (" count#"), MOMOUT_DEC_INT ((int) count), NULL);
    }
  return momtypenc__none;
}


// emit an item as a variable, giving its type or 0 on failure
static momtypenc_t
emit_var_item_cgen (struct c_generator_mom_st *cg, momitem_t *varitm)
{
  momval_t expasv = MOM_NULLV;
  if (varitm == mom_named__result
      && cg->cgen_rout.cgrout_kind == cgr_proc && cg->cgen_restype)
    {
      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64), "momresult");
      return cg->cgen_restype;
    }
  expasv = mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, varitm);
  if (expasv.ptr == NULL)
    expasv = mom_item_assoc_get (cg->cgen_globassocitm, varitm);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_var_item varitm="),
	     MOMOUT_ITEM ((const momitem_t *) varitm),
	     MOMOUT_NEWLINE (), MOMOUT_LITERAL ("expasv="),
	     MOMOUT_VALUE ((const momval_t) expasv), NULL);
  const momitem_t *noditm = mom_node_conn (expasv);
#define EMITITEMASNODHMAX_MOM 61
#define ASNODECASE(Nam) case mom_hashname_##Nam % EMITITEMASNODHMAX_MOM: \
  if (noditm != mom_named_##Nam) goto other_noditm;
  if (noditm)
    switch (mom_item_hash (noditm) % EMITITEMASNODHMAX_MOM)
      {
	//
	ASNODECASE (_constants);
	{
	  int cix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
	  momitem_t *constitm = mom_value_to_item (mom_node_nth (expasv, 1));
	  assert (constitm != NULL && constitm->i_typnum == momty_item);
	  assert (cix >= 0);
	  if (cg->cgen_rout.cgrout_kind == cgr_proc)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("((momval_t) "
				       CGEN_PROC_CONSTANTITEMS_PREFIX),
		       MOMOUT_LITERALV (mom_ident_cstr_of_item
					(cg->cgen_rout.cgrout_routitm)),
		       MOMOUT_LITERAL ("["), MOMOUT_DEC_INT (cix),
		       MOMOUT_LITERAL ("] /*"),
		       MOMOUT_ITEM ((const momitem_t *) constitm),
		       MOMOUT_LITERAL ("*/)"), NULL);
	      return momtypenc_val;
	    }
	  else if (cg->cgen_rout.cgrout_kind == cgr_funt)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("((momval_t) "
				       CGEN_FUN_CONSTANTITEMS_PREFIX),
		       MOMOUT_LITERALV (mom_ident_cstr_of_item
					(cg->cgen_rout.cgrout_routitm)),
		       MOMOUT_LITERAL ("["), MOMOUT_DEC_INT (cix),
		       MOMOUT_LITERAL ("] /*"),
		       MOMOUT_ITEM ((const momitem_t *) constitm),
		       MOMOUT_LITERAL ("*/)"), NULL);
	      return momtypenc_val;
	    }
	  else
	    assert (false && "impossible routkind");
	};
	break;
	////
	ASNODECASE (_formals);
	if (cg->cgen_rout.cgrout_kind == cgr_proc)
	  {
	    int formix =
	      (int) mom_integer_val_def (mom_node_nth (expasv, 1), -1);
	    assert (formix >= 0);
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		     MOMOUT_LITERAL (CGEN_FORMALARG_PREFIX),
		     MOMOUT_DEC_INT (formix), MOMOUT_LITERAL ("/*"),
		     MOMOUT_ITEM ((const momitem_t *) varitm),
		     MOMOUT_LITERAL ("*/ "));
	    return emit_ctype_cgen (cg, NULL, mom_node_nth (expasv, 2));
	  }
	else
	  goto other_noditm;
	////
	ASNODECASE (_closed_values);
	{
	  int cix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
	  momitem_t *clositm = mom_value_to_item (mom_node_nth (expasv, 1));
	  assert (clositm != NULL && clositm->i_typnum == momty_item);
	  assert (cix >= 0);
	  assert (cg->cgen_rout.cgrout_kind == cgr_funt);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL ("(momclovals["),
		   MOMOUT_DEC_INT (cix), MOMOUT_LITERAL ("] /*"),
		   MOMOUT_ITEM ((const momitem_t *) clositm),
		   MOMOUT_LITERAL ("*/)"), NULL);
	  return momtypenc_val;
	}
	goto other_noditm;
	////
	ASNODECASE (_momval_t)
	{
	  int vix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
	  assert (vix >= 0);
	  if (cg->cgen_rout.cgrout_kind == cgr_proc)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("(" CGEN_PROC_VALUE_PREFIX),
		       MOMOUT_DEC_INT ((int) vix),
		       MOMOUT_LITERAL ("/*:"),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL ("*/)"), NULL);
	      return momtypenc_val;
	    }
	  else if (cg->cgen_rout.cgrout_kind == cgr_funt)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("momvals["),
		       MOMOUT_DEC_INT ((int) vix),
		       MOMOUT_LITERAL ("/*:"),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL ("*/]"), NULL);
	      return momtypenc_val;
	    }
	  else
	    assert (false && "impossible routkind");
	}
	goto other_noditm;
	///
	ASNODECASE (_intptr_t);
	{
	  int nix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
	  assert (nix >= 0);
	  if (cg->cgen_rout.cgrout_kind == cgr_proc)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("(" CGEN_PROC_NUMBER_PREFIX),
		       MOMOUT_DEC_INT ((int) nix),
		       MOMOUT_LITERAL ("/*:"),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL ("*/)"), NULL);
	      return momtypenc_int;
	    }
	  else if (cg->cgen_rout.cgrout_kind == cgr_funt)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("momnums["),
		       MOMOUT_DEC_INT ((int) nix),
		       MOMOUT_LITERAL ("/*:"),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL ("*/]"), NULL);
	      return momtypenc_int;
	    }
	  else
	    assert (false && "impossible routkind");
	}
	goto other_noditm;
	////
	ASNODECASE (_double);
	{
	  int dix = mom_integer_val_def (mom_node_nth (expasv, 0), -1);
	  assert (dix >= 0);
	  if (cg->cgen_rout.cgrout_kind == cgr_proc)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("(" CGEN_PROC_DOUBLE_PREFIX),
		       MOMOUT_DEC_INT ((int) dix),
		       MOMOUT_LITERAL ("/*:"),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL ("*/)"), NULL);
	      return momtypenc_double;
	    }
	  else if (cg->cgen_rout.cgrout_kind == cgr_funt)
	    {
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		       MOMOUT_LITERAL ("momdbls["),
		       MOMOUT_DEC_INT ((int) dix),
		       MOMOUT_LITERAL ("/*:"),
		       MOMOUT_ITEM ((const momitem_t *) varitm),
		       MOMOUT_LITERAL ("*/]"), NULL);
	      return momtypenc_double;
	    }
	  else
	    assert (false && "impossible routkind");
	}
	goto other_noditm;
	////
	ASNODECASE (_procedure)
	{
	  // this is for passing procedure addresses
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (64),
		   MOMOUT_LITERAL (MOM_PROCROUTFUN_PREFIX),
		   MOMOUT_LITERALV (mom_ident_cstr_of_item (varitm)));
	  return momtypenc__none;
	}
	goto other_noditm;
	////
      default:
      other_noditm:
	CGEN_ERROR_MOM		//
	  (cg,
	   MOMOUT_LITERAL ("emit_varitem "),
	   MOMOUT_ITEM ((const momitem_t *) varitm),
	   MOMOUT_LITERAL (" has unexpected node expasv "),
	   MOMOUT_VALUE ((const momval_t) expasv), NULL);
#undef EMITITEMASNODHMAX_MOM
#undef ASNODECASE
      }
  /////
  momval_t ctypv = MOM_NULLV;
  momval_t verbatimv = MOM_NULLV;
  {
    cgen_lock_item_mom (cg, varitm);
    verbatimv = mom_item_get_attribute (varitm, mom_named__verbatim);
    ctypv = mom_item_get_attribute (varitm, mom_named__ctype);
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
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emitexpr string:"),
		 MOMOUT_VALUE ((const momval_t) expv));
      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
	       MOMOUT_LITERAL ("/*!litstr:*/ \""),
	       MOMOUT_C_STRING ((const char *) expv.pstring->cstr),
	       MOMOUT_LITERAL ("\""), NULL);
      return momtypenc_string;
    }
  else if (mom_is_integer (expv))
    {
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emitexpr integer:"),
		 MOMOUT_VALUE ((const momval_t) expv));
      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
	       MOMOUT_FMT_LONG_LONG ((const char *) "%lld",
				     (long long) (expv.pint->intval)), NULL);
      return momtypenc_int;
    }
  else if (mom_is_item (expv))
    {
      momitem_t *expitm = expv.pitem;
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emitexpr item var:"),
		 MOMOUT_VALUE ((const momval_t) expv));
      momtypenc_t typva = emit_var_item_cgen (cg, expitm);
      return typva;
    }
  else if (mom_is_node (expv))
    {
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emitexpr node start:"),
		 MOMOUT_VALUE ((const momval_t) expv));
      momtypenc_t rest = emit_node_cgen (cg, expv);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("emitexpr node done:"),
		 MOMOUT_VALUE ((const momval_t) expv));
      return rest;
    }
  else
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid expression"),
		    MOMOUT_VALUE ((const momval_t) expv));
  return 0;
}


// emit an output-like argument
static void
emit_output_arg_cgen (struct c_generator_mom_st *cg,
		      momval_t curoutv, momval_t nodv)
{
  if (mom_is_string (curoutv))
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL
	       (" /*!litoutstr*/MOMOUTDO_LITERAL, \""),
	       MOMOUT_C_STRING ((const char *) curoutv.pstring->cstr),
	       MOMOUT_LITERAL ("\","), MOMOUT_SPACE (50), NULL);
    }
  else if (mom_is_integer (curoutv))
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL
	       (" /*!litoutint*/MOMOUTDO_DEC_INTPTR_T, (intptr_t)"),
	       MOMOUT_DEC_INTPTR_T (mom_integer_val (curoutv)),
	       MOMOUT_LITERAL (","), NULL);
    }
  else if (mom_is_double (curoutv))
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL
	       (" /*!litoutdbl*/MOMOUTDO_DOUBLE_G, (double)"),
	       MOMOUT_DOUBLE_G (mom_double_val (curoutv)),
	       MOMOUT_LITERAL (","), NULL);
    }
  else if (mom_is_item (curoutv))
    {
      momtypenc_t outyp = emit_ctype_cgen (cg, NULL, curoutv);
      switch (outyp)
	{
	case momtypenc_int:
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL
		   (" /*!outintvar*/MOMOUTDO_DEC_INTPTR_T, (intptr_t)"),
		   NULL);
	  emit_var_item_cgen (cg, curoutv.pitem);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","), NULL);
	  break;
	case momtypenc_val:
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL
		   (" /*!outvalvar*/MOMOUTDO_VALUE, (momval_t)"), NULL);
	  emit_var_item_cgen (cg, curoutv.pitem);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","), NULL);
	  break;
	case momtypenc_double:
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL
		   (" /*!outdblvar*/MOMOUTDO_DOUBLE_G, (double)"), NULL);
	  emit_var_item_cgen (cg, curoutv.pitem);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","), NULL);
	  break;
	case momtypenc_string:
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL
		   (" /*!outcstrvar*/MOMOUTDO_LITERAL, (const char*)"), NULL);
	  emit_var_item_cgen (cg, curoutv.pitem);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","), NULL);
	  break;
	default:
	  CGEN_ERROR_MOM
	    (cg,
	     MOMOUT_LITERAL ("invalid output item element:"),
	     MOMOUT_VALUE ((const momval_t) curoutv),
	     MOMOUT_LITERAL (" in node:"),
	     MOMOUT_VALUE ((const momval_t) nodv), NULL);
	}
    }
  else if (mom_is_node (curoutv))
    {
      momitem_t *outconnitm = (momitem_t *) mom_node_conn (curoutv);
      unsigned outarity = mom_node_arity (curoutv);
      momval_t outexpv = MOM_NULLV;
      momval_t outformals = MOM_NULLV;
      {
	cgen_lock_item_mom (cg, outconnitm);
	outexpv =
	  mom_item_get_attribute (outconnitm, mom_named__output_expansion);
	outformals = mom_item_get_attribute (outconnitm, mom_named__formals);
      }
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("output connective:"),
		 MOMOUT_ITEM ((const momitem_t *) outconnitm),
		 MOMOUT_LITERAL (" with output_expansion:"),
		 MOMOUT_VALUE ((const momval_t) outexpv));
      if (mom_node_conn (outexpv) == mom_named__chunk)
	{
	  struct mom_itemattributes_st *argbind = NULL;
	  if (!mom_is_tuple (outformals))
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL ("output item element:"),
	       MOMOUT_VALUE ((const momval_t) curoutv),
	       MOMOUT_LITERAL (" has output connective:"),
	       MOMOUT_ITEM ((const momitem_t *) outconnitm),
	       MOMOUT_LITERAL (" without formals."), NULL);
	  unsigned formarity = mom_tuple_length (outformals);
	  if (formarity != outarity)
	    CGEN_ERROR_MOM
	      (cg,
	       MOMOUT_LITERAL ("output item element:"),
	       MOMOUT_VALUE ((const momval_t) curoutv),
	       MOMOUT_LITERAL (" has arity mismatch"), NULL);
	  argbind = mom_reserve_attribute (argbind, 3 * outarity / 2 + 5);
	  // handle the output expansion, first bind the formals
	  for (unsigned ix = 0; ix < outarity; ix++)
	    {
	      momval_t curargv = mom_node_nth (curoutv, ix);
	      momitem_t *curformitm = mom_tuple_nth_item (outformals, ix);
	      MOM_DEBUG (gencod, MOMOUT_LITERAL ("outputarg ix#"),
			 MOMOUT_DEC_INT ((int) ix),
			 MOMOUT_LITERAL (" curargv:"),
			 MOMOUT_VALUE ((const momval_t) curargv),
			 MOMOUT_LITERAL (" curformitm:"),
			 MOMOUT_ITEM ((const momitem_t *) curformitm), NULL);
	      if (mom_get_attribute (argbind, curformitm).ptr)
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("output formal:"),
				MOMOUT_ITEM ((const momitem_t *)
					     curformitm),
				MOMOUT_LITERAL (" already bound in "),
				MOMOUT_VALUE ((const momval_t) curoutv));
	      if (emit_ctype_cgen (cg, NULL, curargv) !=
		  emit_ctype_cgen (cg, NULL, (momval_t) curformitm))
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("output type mismatch for formal:"),
				MOMOUT_ITEM ((const momitem_t *)
					     curformitm),
				MOMOUT_LITERAL (" with:"),
				MOMOUT_VALUE ((const momval_t) curargv),
				MOMOUT_LITERAL (" in output expr:"),
				MOMOUT_VALUE ((const momval_t) outexpv));
	      argbind = mom_put_attribute (argbind, curformitm, curargv);
	    }
	  // then emit the expanded output_expansion
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL ("/*!outexp "),
		   MOMOUT_ITEM ((const momitem_t *) outconnitm),
		   MOMOUT_LITERAL ("*/"), MOMOUT_SPACE (56));
	  unsigned chkarity = mom_node_arity (outexpv);
	  for (unsigned chkix = 0; chkix < chkarity; chkix++)
	    {
	      momval_t curchkv = mom_node_nth (outexpv, chkix);
	      momitem_t *chkitm = NULL;
	      momval_t chkexpv = MOM_NULLV;
	      if (mom_is_string (curchkv))
		{
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_LITERALV ((const char *)
					    mom_string_cstr (curchkv)));
		}
	      else if (mom_is_integer (curchkv))
		{
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_DEC_INTPTR_T (mom_integer_val (curchkv)));
		}
	      else if ((chkitm = mom_value_to_item (curchkv))
		       &&
		       ((chkexpv = mom_get_attribute (argbind, chkitm)).ptr))
		emit_expr_cgen (cg, chkexpv);
	      else
		CGEN_ERROR_MOM
		  (cg,
		   MOMOUT_LITERAL ("bad output chunk:"),
		   MOMOUT_VALUE ((const momval_t) curchkv),
		   MOMOUT_LITERAL (" index#"),
		   MOMOUT_DEC_INT ((int) chkix),
		   MOMOUT_LITERAL (" in output connective:"),
		   MOMOUT_ITEM ((const momitem_t *) outconnitm),
		   MOMOUT_LITERAL (" for output node:"),
		   MOMOUT_VALUE ((const momval_t) curoutv));
	    }
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL ("/*!endoutexp "),
		   MOMOUT_ITEM ((const momitem_t *) outconnitm),
		   MOMOUT_LITERAL ("*/,"), MOMOUT_SPACE (56));
	}
      else
	{
	  // plain output expression
	  momtypenc_t outyp = emit_ctype_cgen (cg, NULL, curoutv);
	  switch (outyp)
	    {
	    case momtypenc_int:
	      MOM_OUT (&cg->cgen_outbody,
		       MOMOUT_LITERAL
		       (" /*!outintexp*/MOMOUTDO_DEC_INTPTR_T, (intptr_t)"),
		       NULL);
	      break;
	    case momtypenc_val:
	      MOM_OUT (&cg->cgen_outbody,
		       MOMOUT_LITERAL
		       (" /*!outvalexp*/MOMOUTDO_VALUE, (momval_t)"), NULL);
	      break;
	    case momtypenc_double:
	      MOM_OUT (&cg->cgen_outbody,
		       MOMOUT_LITERAL
		       (" /*!outdblexp*/MOMOUTDO_DOUBLE_G, (double)"), NULL);
	      break;
	    case momtypenc_string:
	      MOM_OUT (&cg->cgen_outbody,
		       MOMOUT_LITERAL
		       (" /*!outcstrexp*/MOMOUTDO_LITERAL, (const char*)"),
		       NULL);
	      break;
	    default:
	      CGEN_ERROR_MOM
		(cg,
		 MOMOUT_LITERAL ("invalid output item element:"),
		 MOMOUT_VALUE ((const momval_t) curoutv),
		 MOMOUT_LITERAL (" in node:"),
		 MOMOUT_VALUE ((const momval_t) nodv), NULL);
	    };
	  emit_expr_cgen (cg, curoutv);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
		   MOMOUT_NEWLINE ());
	}
    }
  else
    CGEN_ERROR_MOM
      (cg,
       MOMOUT_LITERAL ("invalid output element:"),
       MOMOUT_VALUE ((const momval_t) curoutv),
       MOMOUT_LITERAL (" in node:"),
       MOMOUT_VALUE ((const momval_t) nodv), NULL);
}


// emit a node and gives its type
static momtypenc_t
emit_node_cgen (struct c_generator_mom_st *cg, momval_t nodv)
{
  momval_t formalsv = MOM_NULLV;
  momval_t resv = MOM_NULLV;
  momval_t ctypev = MOM_NULLV;
  momval_t primexpv = MOM_NULLV;
  momval_t procv = MOM_NULLV;
  momval_t primcountv = MOM_NULLV;
  momitem_t *primoutitm = NULL;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit_node nodv="),
	     MOMOUT_VALUE ((const momval_t) nodv));
  momitem_t *connitm = (momitem_t *) mom_node_conn (nodv);
  int arity = mom_node_arity (nodv);
  if (!connitm)
    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("non-node"),
		    MOMOUT_VALUE ((const momval_t) nodv));
  {
    cgen_lock_item_mom (cg, connitm);
    formalsv = mom_item_get_attribute (connitm, mom_named__formals);
    resv = mom_item_get_attribute (connitm, mom_named__result);
    procv = mom_item_get_attribute (connitm, mom_named__procedure);
    ctypev = mom_item_get_attribute (connitm, mom_named__ctype);
    primexpv =
      mom_item_get_attribute (connitm, mom_named__primitive_expansion);
    primcountv = mom_item_get_attribute (connitm, mom_named__count);
    primoutitm =
      mom_value_to_item (mom_item_get_attribute (connitm, mom_named__output));
  }
  unsigned nbformals = mom_tuple_length (formalsv);
  // handle tuple nodes
  if (connitm == mom_named__tuple)
    {
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!tuple+"),
	       MOMOUT_DEC_INT ((int) arity),
	       MOMOUT_LITERAL (":*/ mom_make_tuple_variadic("),
	       MOMOUT_DEC_INT ((int) arity), NULL);
      for (int ix = 0; ix < arity; ix++)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (", ("), NULL);
	  momval_t curargv = mom_node_nth (nodv, ix);
	  if (emit_expr_cgen (cg, curargv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("non-value argument for `tuple`:"),
			    MOMOUT_VALUE (curargv), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in expr:"), nodv);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), NULL);
	}
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (")/*!endtuple*/"), MOMOUT_SPACE (32));
    }
  // handle set nodes
  else if (connitm == mom_named__set)
    {
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!set+"),
	       MOMOUT_DEC_INT ((int) arity),
	       MOMOUT_LITERAL (":*/ mom_make_set_variadic("),
	       MOMOUT_DEC_INT ((int) arity), NULL);
      for (int ix = 0; ix < arity; ix++)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (", ("), NULL);
	  momval_t curargv = mom_node_nth (nodv, ix);
	  if (emit_expr_cgen (cg, curargv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("non-value argument for `set`:"),
			    MOMOUT_VALUE (curargv), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in expr:"),
			    MOMOUT_VALUE ((const momval_t) nodv));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), NULL);
	}
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")/*!endset*/"),
	       MOMOUT_SPACE (32));
    }
  // handle json_object nodes
  else if (connitm == mom_named__json_object)
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL ("/*!json_object+"),
	       MOMOUT_DEC_INT ((int) arity),
	       MOMOUT_LITERAL (":*/ mom_make_json_object("), NULL);
      for (int jix = 0; jix < arity; jix++)
	{
	  momval_t curargv = mom_node_nth (nodv, jix);
	  const momitem_t *curconnitm = mom_node_conn (curargv);
	  unsigned curarity = mom_node_arity (curargv);
	  if (curconnitm == mom_named__json_entry && curarity == 2)
	    {
	      momval_t expjnamv = mom_node_nth (curargv, 0);
	      momval_t expjvalv = mom_node_nth (curargv, 1);
	      momtypenc_t tnam = emit_ctype_cgen (cg, NULL, expjnamv);
	      momtypenc_t tval = emit_ctype_cgen (cg, NULL, expjvalv);
	      if (tnam == momtypenc_string && tval == momtypenc_val)
		{
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_LITERAL ("MOMJSONDIR__STRING,"),
			   MOMOUT_SPACE (48));
		  emit_expr_cgen (cg, expjnamv);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
			   MOMOUT_SPACE (56));
		  emit_expr_cgen (cg, expjvalv);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
			   MOMOUT_SPACE (32));
		}
	      else if (tnam == momtypenc_val && tval == momtypenc_val)
		{
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_LITERAL ("MOMJSONDIR__ENTRY,"),
			   MOMOUT_SPACE (48));
		  emit_expr_cgen (cg, expjnamv);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
			   MOMOUT_SPACE (56));
		  emit_expr_cgen (cg, expjvalv);
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (","),
			   MOMOUT_SPACE (32));
		}
	      else
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad json_entry:"),
				MOMOUT_VALUE ((const momval_t) curargv),
				MOMOUT_LITERAL
				(" in json_object node "),
				MOMOUT_VALUE ((momval_t) nodv));
	    }
	  else if (curconnitm == mom_named__json_object && curarity == 1)
	    {
	      momval_t expjobv = mom_node_nth (curargv, 0);
	      MOM_OUT (&cg->cgen_outbody,
		       MOMOUT_LITERAL ("MOMJSONDIR__INDIRECT, ("),
		       MOMOUT_SPACE (48));
	      if (emit_expr_cgen (cg, expjobv) != momtypenc_val)
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("bad `json_object` argument:"),
				MOMOUT_VALUE ((const momval_t) expjobv),
				MOMOUT_LITERAL
				(" in `json_object` node:"),
				MOMOUT_VALUE ((const momval_t) nodv));
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("),"), NULL);
	    }
	  else
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL ("bad argument#"),
			    MOMOUT_DEC_INT (jix),
			    MOMOUT_LITERAL (" in `json_object` node:"),
			    MOMOUT_VALUE ((const momval_t) nodv));
	}

      MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
	       MOMOUT_LITERAL ("MOMJSON_END)/*!endjsonobject*/"),
	       MOMOUT_SPACE (32));
    }
  // handle json_array nodes
  else if (connitm == mom_named__json_array)
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL ("/*!json_array+"),
	       MOMOUT_DEC_INT ((int) arity),
	       MOMOUT_LITERAL (":*/ mom_make_json_array("),
	       MOMOUT_DEC_INT ((int) arity), NULL);
      for (int ix = 0; ix < arity; ix++)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (", ("), NULL);
	  momval_t curargv = mom_node_nth (nodv, ix);
	  if (emit_expr_cgen (cg, curargv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("non-value argument for `json_array`:"),
			    MOMOUT_VALUE (curargv), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in expr:"),
			    MOMOUT_VALUE ((const momval_t) nodv));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), NULL);
	}
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (")/*!endjsonarray*/"), MOMOUT_SPACE (32));
    }
  // handle set nodes
  else if (connitm == mom_named__node)
    {
      if (arity == 0)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("missing argument for `node` in expr:"),
			MOMOUT_VALUE ((const momval_t) nodv));
      momval_t connexprv = mom_node_nth (nodv, 0);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!node+"),
	       MOMOUT_DEC_INT ((int) arity - 1),
	       MOMOUT_LITERAL ("*/mom_make_node_sized (("), NULL);
      if (emit_expr_cgen (cg, connexprv) != momtypenc_val)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("non-value connective argument for `node` in expr:"),
			MOMOUT_VALUE ((const momval_t) nodv));
      MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("),"),
	       MOMOUT_SPACE (48), MOMOUT_DEC_INT ((int) arity - 1), NULL);
      for (int ix = 0; ix < arity; ix++)
	{
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (", ("), NULL);
	  momval_t curargv = mom_node_nth (nodv, ix);
	  if (emit_expr_cgen (cg, curargv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("non-value argument for `node`:"),
			    MOMOUT_VALUE (curargv), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in expr:"),
			    MOMOUT_VALUE ((const momval_t) nodv));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (")"), NULL);
	}
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (")/*!endnode*/"), MOMOUT_SPACE (48));
    }
  // handle primitives
  else if (primexpv.ptr)
    {
      struct mom_itemattributes_st *argbind = NULL;
      int formalarity = mom_tuple_length (formalsv);
      if (!mom_is_tuple (formalsv)
	  || !mom_is_node (primexpv) || !mom_is_item (ctypev)
	  || mom_node_conn (primexpv) != mom_named__chunk)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad primitive:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
						connitm),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      if (!primoutitm && formalarity != arity)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("wrong arity for primitive:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm),
			MOMOUT_SPACE (48), MOMOUT_LITERAL ("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      else if (primoutitm && formalarity > arity)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("too small arity for output primitive:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
						connitm),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      if (ctypev.pitem == mom_named__void)
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL
		 ("/*!primitive-void "),
		 MOMOUT_ITEM ((const momitem_t
			       *) connitm), MOMOUT_LITERAL ("*/ "), NULL);
      else
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("(/*!primitive "),
		 MOMOUT_ITEM ((const momitem_t *) connitm),
		 MOMOUT_LITERAL ("*/ "), NULL);
      if (mom_node_conn (primexpv) != mom_named__chunk)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("no `chunk` for `primitive_expansion` in primitive:"),
			MOMOUT_ITEM ((const
				      momitem_t
				      *)
				     connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES
			((const momitem_t *)
			 connitm),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL
			("in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      argbind = mom_reserve_attribute (argbind, 3 * arity / 2 + 5);
      char *argctypestr = MOM_GC_SCALAR_ALLOC ("argctypestr", arity + 3);
      // bind the count if needed
      if (mom_is_item (primcountv))
	argbind =
	  mom_put_attribute (argbind, primcountv.pitem,
			     mom_make_integer (++cg->cgen_count));
      // first loop to bind the formal arguments and get their `ctype`
      for (int ix = 0; ix < formalarity; ix++)
	{
	  momval_t formctypv = MOM_NULLV;
	  momitem_t *formalitm = mom_tuple_nth_item (formalsv, ix);
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
	       MOMOUT_NEWLINE (), MOMOUT_LITERAL ("rank#"),
	       MOMOUT_DEC_INT (ix), MOMOUT_LITERAL (" in node:"),
	       MOMOUT_VALUE ((const momval_t) nodv), NULL);
	  {
	    cgen_lock_item_mom (cg, formalitm);
	    formctypv =
	      mom_item_get_attribute (formalitm, mom_named__variable);
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
	       MOMOUT_LITERAL
	       ("bad `ctype` of primitive formal argument:"),
	       MOMOUT_ITEM ((const momitem_t *) formalitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) formalitm),
	       MOMOUT_LITERAL (" in primitive:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm),
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
	      momval_t bndlocv = MOM_NULLV;
	      if (primoutitm && curchkv.pitem == primoutitm && !bndvalv.ptr)
		{
		  for (unsigned outix = nbformals;
		       outix < (unsigned) arity; outix++)
		    {
		      momval_t curoutv = mom_node_nth (nodv, outix);
		      MOM_DEBUG (gencod,
				 MOMOUT_LITERAL ("output argument:"),
				 MOMOUT_VALUE ((const momval_t)
					       curoutv),
				 MOMOUT_LITERAL (" outix#"),
				 MOMOUT_DEC_INT ((int) outix));
		      emit_output_arg_cgen (cg, curoutv, nodv);
		    };
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_LITERAL ("/*!outputend*/NULL"),
			   MOMOUT_SPACE (48));
		}
	      else if (bndvalv.ptr)
		{
		  MOM_DEBUG (gencod, MOMOUT_LITERAL ("chunk item:"),
			     MOMOUT_ITEM ((const momitem_t *)
					  curchkv.pitem),
			     MOMOUT_LITERAL (" bound to argument:"),
			     MOMOUT_VALUE (bndvalv));
		  momtypenc_t vtyp = emit_expr_cgen (cg, bndvalv);
		  for (unsigned fix = 0; fix < (unsigned) arity; fix++)
		    if (mom_tuple_nth_item (formalsv, fix) ==
			bndvalv.pitem
			&& (unsigned) vtyp != (unsigned) argctypestr[fix])
		      CGEN_ERROR_MOM (cg,
				      MOMOUT_LITERAL
				      ("type mismatch for formal:"),
				      MOMOUT_ITEM ((const momitem_t *)
						   bndvalv.pitem),
				      MOMOUT_LITERAL (" rank#"),
				      MOMOUT_DEC_INT ((int) fix),
				      MOMOUT_LITERAL (" in node:"),
				      MOMOUT_VALUE ((const momval_t)
						    nodv), NULL);
		}
	      else if ((bndlocv =
			mom_item_assoc_get (cg->cgen_rout.cgrout_associtm,
					    curchkv.pitem)).ptr
		       || (bndlocv =
			   mom_item_assoc_get (cg->cgen_globassocitm,
					       curchkv.pitem)).ptr)
		{
		  /* an item in the chunk which is bound is handled as a variable */
		  MOM_DEBUG (gencod, MOMOUT_LITERAL ("chunk item:"),
			     MOMOUT_ITEM ((const momitem_t *)
					  curchkv.pitem),
			     MOMOUT_LITERAL (" bound to outside var:"),
			     MOMOUT_VALUE (bndvalv));
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_LITERAL ("/*!outsidechunk*/"), NULL);
		  emit_var_item_cgen (cg, curchkv.pitem);
		}
	      else
		{
		  /* if an item appears in the chunk and is not bound, we emit it verbatim. */
		  MOM_DEBUG (gencod,
			     MOMOUT_LITERAL
			     ("unbound verbatim chunk item:"),
			     MOMOUT_ITEM ((const momitem_t *)
					  curchkv.pitem), NULL);
		  MOM_OUT (&cg->cgen_outbody,
			   MOMOUT_LITERAL ("/*!verbatimchunk*/"),
			   MOMOUT_ITEM ((const momitem_t *) curchkv.pitem));
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
      if (ctypev.pitem == mom_named__void)
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("/*!endvoidprimitive "),
		 MOMOUT_ITEM ((const momitem_t *) connitm),
		 MOMOUT_LITERAL ("*/ "), NULL);
      else
	MOM_OUT (&cg->cgen_outbody,
		 MOMOUT_LITERAL ("/*!endprimitive "),
		 MOMOUT_ITEM ((const momitem_t *) connitm),
		 MOMOUT_LITERAL ("*/) "), NULL);
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
      if (!mom_is_tuple (formalsv))
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad procedure node:"),
			MOMOUT_VALUE ((const momval_t) nodv),
			MOMOUT_LITERAL
			(" without `formals` in connective:"),
			MOMOUT_ITEM ((const momitem_t *) connitm),
			MOMOUT_SPACE (48),
			MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm));
      if (mom_tuple_length (formalsv) != (unsigned) arity)
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("arity mismatch for procedure:"),
			MOMOUT_ITEM ((const
				      momitem_t
				      *)
				     connitm),
			MOMOUT_LITERAL
			(" in node:"),
			MOMOUT_VALUE ((const momval_t) nodv), NULL);
      if (!ctypev.ptr)
	{
	  if (mom_is_item (resv))
	    {
	      cgen_lock_item_mom (cg, resv.pitem);
	      ctypev = mom_item_get_attribute (resv.pitem, mom_named__ctype);
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
	  int nbargs = mom_tuple_length (formalsv);
	  for (int aix = 0; aix < nbargs; aix++)
	    {
	      if (aix > 0)
		MOM_OUT (&cg->cgen_outhead, MOMOUT_LITERAL (","),
			 MOMOUT_SPACE (48));
	      momitem_t *curformitm = mom_tuple_nth_item (formalsv, aix);
	      momval_t curformctypv = MOM_NULLV;
	      if (!curformitm)
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("bad external procedure:"),
				MOMOUT_ITEM ((const momitem_t *)
					     connitm),
				MOMOUT_LITERAL (" missing formal #"),
				MOMOUT_DEC_INT (aix));
	      {
		cgen_lock_item_mom (cg, curformitm);
		curformctypv =
		  mom_item_get_attribute (curformitm, mom_named__ctype);
	      }
	      if (!mom_is_item (curformctypv))
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("in external procedure:"),
				MOMOUT_ITEM ((const momitem_t *)
					     connitm),
				MOMOUT_LITERAL ("  formal #"),
				MOMOUT_DEC_INT ((int) aix),
				MOMOUT_LITERAL ("="),
				MOMOUT_ITEM ((const momitem_t *)
					     curformitm),
				MOMOUT_LITERAL (" with missing ctype"));
	      emit_ctype_cgen (cg, &cg->cgen_outhead, curformctypv);
	    }
	  MOM_OUT (&cg->cgen_outhead, MOMOUT_INDENT_LESS (),
		   MOMOUT_LITERAL (");"), MOMOUT_NEWLINE ());
	  mom_item_put_attribute (cg->cgen_globassocitm, connitm,
				  (momval_t)
				  mom_make_node_sized
				  ((momval_t) mom_named__procedure, 3,
				   MOM_NULLV, formalsv, ctypev));
	}
      if (mom_item_get_name (connitm))
	MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!"),
		 MOMOUT_ITEM ((const momitem_t *) connitm),
		 MOMOUT_LITERAL ("*/"), NULL);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (" " MOM_PROCROUTFUN_PREFIX),
	       MOMOUT_LITERALV (mom_ident_cstr_of_item (connitm)),
	       MOMOUT_LITERAL (" ("), MOMOUT_INDENT_MORE (), NULL);
      for (int aix = 0; aix < arity; aix++)
	{
	  momval_t curargv = mom_node_nth (nodv, aix);
	  momitem_t *curformitm = mom_tuple_nth_item (formalsv, aix);
	  momval_t curformctypv = MOM_NULLV;
	  if (!curformitm)
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad procedure:"),
			    MOMOUT_ITEM ((const momitem_t *) connitm),
			    MOMOUT_LITERAL (" missing formal #"),
			    MOMOUT_DEC_INT (aix));
	  {
	    cgen_lock_item_mom (cg, curformitm);
	    curformctypv =
	      mom_item_get_attribute (curformitm, mom_named__ctype);
	  }
	  if (!mom_is_item (curformctypv))
	    CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("in procedure:"),
			    MOMOUT_ITEM ((const momitem_t *) connitm),
			    MOMOUT_LITERAL ("  formal #"),
			    MOMOUT_DEC_INT (aix),
			    MOMOUT_LITERAL ("="),
			    MOMOUT_ITEM ((const momitem_t *)
					 curformitm),
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
	       MOMOUT_LITERAL
	       ("bad `ctype` of procedure formal argument:"),
	       MOMOUT_ITEM ((const momitem_t *) curformitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) curformitm),
	       MOMOUT_LITERAL (" in procedure:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_SPACE (48),
	       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) connitm),
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
  momval_t commv = MOM_NULLV;
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (blkitm && blkitm->i_typnum == momty_item);
  {
    cgen_lock_item_mom (cg, blkitm);
    blockv = mom_item_get_attribute (blkitm, mom_named__block);
    lockv = mom_item_get_attribute (blkitm, mom_named__lock);
    commv = mom_item_get_attribute (blkitm, mom_named__comment);
  }
  if (mom_is_string (commv))
    MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	     MOMOUT_SLASHCOMMENT_STRING
	     (mom_string_cstr (commv)), MOMOUT_NEWLINE ());
  else
    MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE ());
  if (mom_node_conn (blockv) != mom_named__code)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL
		    ("invalid block (bad `block`, should be a `code` node):"),
		    MOMOUT_ITEM ((const
				  momitem_t
				  *)
				 blkitm),
		    MOMOUT_SPACE (32),
		    MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) blkitm));
  if (lockv.ptr && !mom_is_item (lockv))
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL
		    ("invalid `lock` in block:"),
		    MOMOUT_ITEM ((const momitem_t *) blkitm),
		    MOMOUT_SPACE (32),
		    MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) blkitm));
  int nbinstr = (int) mom_node_arity (blockv);
  // check the instructions
  for (int ix = 0; ix < nbinstr; ix++)
    {
      momval_t curinsv = mom_node_nth (blockv, ix);
      momitem_t *opitm = (momitem_t *) mom_node_conn (curinsv);
      if (!opitm)
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("bad instruction:"),
			MOMOUT_VALUE ((const momval_t) curinsv),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("at rank#"),
			MOMOUT_DEC_INT (ix),
			MOMOUT_LITERAL ("/"),
			MOMOUT_DEC_INT (nbinstr),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in block item:"),
			MOMOUT_ITEM ((const momitem_t *) blkitm),
			MOMOUT_LITERAL ("having code:"),
			MOMOUT_VALUE ((const momval_t) blockv), NULL);
      assert (opitm != NULL);
      if (opitm != mom_named__do && opitm != mom_named__if
	  && opitm != mom_named__chunk
	  && opitm != mom_named__assign
	  && opitm != mom_named__switch
	  && opitm != mom_named__comment
	  && opitm != mom_named__dispatch
	  && (opitm != mom_named__jump || ix < nbinstr - 1)
	  && (opitm != mom_named__call || ix < nbinstr - 1)
	  && (opitm != mom_named__return || ix < nbinstr - 1))
	CGEN_ERROR_MOM (cg,
			MOMOUT_LITERAL
			("invalid instruction:"),
			MOMOUT_VALUE ((const momval_t) curinsv),
			MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in block:"),
			MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
    }
  // emit the locked item
  if (lockv.pitem != NULL)
    {
      lockix = ++cg->cgen_count;
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("lockv:"),
		 MOMOUT_VALUE (lockv),
		 MOMOUT_LITERAL (" lockix#"),
		 MOMOUT_DEC_INT (lockix),
		 MOMOUT_LITERAL (" in block "),
		 MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("// locked-item "),
	       MOMOUT_ITEM ((const momitem_t *) lockv.pitem),
	       MOMOUT_LITERAL (" in block "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("momitem_t* "
			       CGEN_LOCK_ITEM_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL (" = mom_value_to_item ("));
      if (emit_expr_cgen (cg, lockv) != momtypenc_val)
	CGEN_ERROR_MOM		////
	  (cg,
	   MOMOUT_LITERAL
	   ("bad non-value lock:"),
	   MOMOUT_VALUE ((const momval_t) lockv),
	   MOMOUT_SPACE (48),
	   MOMOUT_LITERAL
	   ("in block:"), MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_LITERAL (") /* locked-item */;"),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("if (!mom_lock_item ("
			       CGEN_LOCK_ITEM_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL (")) goto " CGEN_END_BLOCK_PREFIX),
	       MOMOUT_DEC_INT (lockix), MOMOUT_LITERAL (";"),
	       MOMOUT_NEWLINE (), NULL);
    }
  // emit every instruction
  for (int ix = 0; ix < nbinstr; ix++)
    {
      momval_t curinsv = mom_node_nth (blockv, ix);
      momitem_t *opitm = (momitem_t *) mom_node_conn (curinsv);
      unsigned insarity = mom_node_arity (curinsv);
      MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("//! instr#"),
	       MOMOUT_DEC_INT ((int) (ix + 1)),
	       MOMOUT_LITERAL (" in block "),
	       MOMOUT_ITEM ((const momitem_t *) blkitm),
	       MOMOUT_LITERAL (" ::"), MOMOUT_NEWLINE ());
      assert (opitm != NULL);
      //// DO instruction
      if (opitm == mom_named__do && insarity == 1)
	{
	  /* *do(<expr>) */
	  momval_t doexpv = mom_node_nth (curinsv, 0);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!do*/ "));
	  momtypenc_t xty = emit_ctype_cgen (cg, NULL, doexpv);
	  if (xty != momtypenc__none)
	    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("(void) "));
	  emit_expr_cgen (cg, doexpv);
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL (" /*!done*/;"), MOMOUT_NEWLINE ());
	}
      //// ASSIGN instruction
      else if (opitm == mom_named__assign && insarity == 2)
	{
	  /* *assign(<var>,<expr>) */
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!assign*/ "));
	  momitem_t *lhsitm = mom_value_to_item (mom_node_nth (curinsv, 0));
	  if (!lhsitm)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("bad left-hand-side in assign:"),
			    MOMOUT_VALUE ((const momval_t)
					  curinsv),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix),
			    MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momtypenc_t lhstyp = emit_var_item_cgen (cg, lhsitm);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" = "));
	  momval_t rhsv = mom_node_nth (curinsv, 1);
	  momtypenc_t rtyp = emit_expr_cgen (cg, rhsv);
	  if (lhstyp != rtyp)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("type mismatch in assign:"),
			    MOMOUT_VALUE ((const
					   momval_t)
					  curinsv),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL
			    ("at rank#"),
			    MOMOUT_DEC_INT (ix),
			    MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT
			    (nbinstr),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL
			    ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (";"),
		   MOMOUT_NEWLINE ());
	}
      //// CHUNK instruction
      else if (opitm == mom_named__chunk)
	{
	  /* *chunk(....) */
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL ("/** chunk **/"), MOMOUT_NEWLINE ());
	  for (unsigned cix = 0; cix < insarity; cix++)
	    {
	      momval_t curchkv = mom_node_nth (curinsv, cix);
	      if (mom_is_string (curchkv))
		MOM_OUT (&cg->cgen_outbody,
			 MOMOUT_STRING_VALUE ((const momval_t) curchkv));
	      else
		(void) emit_expr_cgen (cg, curchkv);
	    }
	}
      //// COMMENT instruction
      else if (opitm == mom_named__comment)
	{
	  /* *comment(...) */
	  momval_t com0v = mom_node_nth (curinsv, 0);
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL ("/** comment **/"), MOMOUT_NEWLINE ());
	  if (mom_is_string (com0v))
	    MOM_OUT (&cg->cgen_outbody,
		     MOMOUT_SLASHCOMMENT_STRING
		     (mom_string_cstr (com0v)), MOMOUT_NEWLINE ());
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
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momitem_t *destblkitm = destblockv.pitem;
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL ("/*!if*/ if ("));
	  momtypenc_t ctyp = emit_expr_cgen (cg, testv);
	  if (ctyp == momtypenc__none)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid condition in if:"),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48), MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
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
		  ||
		  !mom_is_integer ((casintv = mom_node_nth (casev, 0)))
		  || !mom_is_item ((casitmv = mom_node_nth (casev, 1))))
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid case:"),
				MOMOUT_VALUE ((const momval_t) casev),
				MOMOUT_LITERAL
				(" in switch instruction:"),
				MOMOUT_VALUE ((const momval_t) curinsv));
	      MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		       MOMOUT_LITERAL ("case "),
		       MOMOUT_DEC_INTPTR_T ((const intptr_t)
					    casintv.pint),
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
		       mom_item_assoc_get (cg->cgen_rout.cgrout_associtm,
					   curcasitm)).ptr
		  || !(assblkv =
		       mom_item_assoc_get (cg->cgen_rout.cgrout_associtm,
					   curblkitm)).ptr
		  || mom_node_conn (asscasv) != mom_named__constants
		  || mom_node_conn (curcasv) != mom_named__block)
		CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid case:"),
				MOMOUT_VALUE ((const momval_t) curcasv),
				MOMOUT_LITERAL
				(" in dispatch instruction:"),
				MOMOUT_VALUE ((const momval_t) curinsv));
	      else
		{
		  casentarr[cix - 1].aten_itm = curcasitm;
		  casentarr[cix - 1].aten_val = (momval_t) curblkitm;
		}
	    };
	  qsort (casentarr, insarity - 1,
		 sizeof (struct mom_attrentry_st), cgen_cmp_case_items_mom);
	  for (int eix = 0; eix < (int) insarity - 1; eix++)
	    if (casentarr[eix].aten_itm == casentarr[eix + 1].aten_itm)
	      CGEN_ERROR_MOM (cg,
			      MOMOUT_LITERAL
			      ("duplicate case:"),
			      MOMOUT_ITEM
			      ((const
				momitem_t
				*)
			       casentarr
			       [eix].aten_itm),
			      MOMOUT_LITERAL
			      (" in dispatch instruction:"),
			      MOMOUT_VALUE ((const momval_t) curinsv));
	  int modh = (4 * insarity / 3 + 2) | 7;
	  qsort_r (casentarr, insarity - 1,
		   sizeof (struct mom_attrentry_st),
		   cgen_cmp_hash_items_mom, (void *) ((intptr_t) modh));
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("// dispatch#"),
		   MOMOUT_DEC_INT (dispix), MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("momitem_t* "
				   CGEN_DISPATCH_PREFIX),
		   MOMOUT_DEC_INT (dispix),
		   MOMOUT_LITERAL ("mom_value_to_item("), NULL);
	  if (emit_expr_cgen (cg, dispv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("invalid dispatcher:"),
			    MOMOUT_VALUE ((const
					   momval_t)
					  dispv),
			    MOMOUT_LITERAL
			    (" in dispatch instruction:"),
			    MOMOUT_VALUE ((const momval_t) curinsv));
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL (") /*dispatcher*/;"),
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
		MOM_OUT (&cg->cgen_outbody,
			 MOMOUT_NEWLINE (),
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
	    CGEN_ERROR_MOM	//
	      (cg,
	       MOMOUT_LITERAL ("invalid destination block in jump"),
	       MOMOUT_VALUE ((const momval_t) curinsv),
	       MOMOUT_SPACE (48),
	       MOMOUT_LITERAL ("at rank#"),
	       MOMOUT_DEC_INT (ix),
	       MOMOUT_LITERAL ("/"),
	       MOMOUT_DEC_INT (nbinstr),
	       MOMOUT_SPACE (48),
	       MOMOUT_LITERAL ("in block:"),
	       MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  momitem_t *destblkitm = destblockv.pitem;
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("/*!jump "),
		   MOMOUT_ITEM ((const momitem_t *) destblkitm),
		   MOMOUT_LITERAL ("*/"), MOMOUT_NEWLINE ());
	  emit_goto_block_cgen (cg, destblkitm, lockix);
	}
      //// CALL instruction
      else if (opitm == mom_named__call && insarity >= 2)
	{
	  /*   *call(<return-block>,<fun-expr>,<arg-expr>....) in functions only */
	  momval_t retblockv = mom_node_nth (curinsv, 0);
	  momval_t funexprv = mom_node_nth (curinsv, 1);
	  if (cg->cgen_rout.cgrout_kind != cgr_funt
	      || !mom_is_item (retblockv))
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERALV ((const char
					      *) ((cg->cgen_rout.cgrout_kind
						   !=
						   cgr_funt) ?
						  "invalid (outside of function) call:"
						  : "invalid call:")),
			    MOMOUT_VALUE ((const momval_t) curinsv),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("at rank#"),
			    MOMOUT_DEC_INT (ix), MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr), MOMOUT_SPACE (48),
			    MOMOUT_LITERAL ("in block:"),
			    MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
	  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("/*!call */"), MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL
		   ("mom_item_tasklet_clear_res(momtasklet);"),
		   MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL
		   ("mom_item_tasklet_push_frame(momtasklet, "), NULL);
	  if (emit_expr_cgen (cg, funexprv) != momtypenc_val)
	    CGEN_ERROR_MOM (cg,
			    MOMOUT_LITERAL
			    ("function:"),
			    MOMOUT_VALUE ((const
					   momval_t)
					  funexprv),
			    MOMOUT_LITERAL
			    (" of non-value type in call:"),
			    MOMOUT_VALUE ((const
					   momval_t)
					  curinsv),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL
			    ("at rank#"),
			    MOMOUT_DEC_INT (ix),
			    MOMOUT_LITERAL ("/"),
			    MOMOUT_DEC_INT (nbinstr),
			    MOMOUT_SPACE (48),
			    MOMOUT_LITERAL
			    ("in block:"),
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
		    char tybuf[4] = {
		      0
		    };
		    tybuf[0] = curargty;
		    tybuf[1] = (char) 0;
		    CGEN_ERROR_MOM (cg,
				    MOMOUT_LITERAL ("invalid type "),
				    MOMOUT_LITERALV ((const char *)
						     tybuf),
				    MOMOUT_LITERAL (" for argument "),
				    MOMOUT_VALUE ((const momval_t)
						  curargexpv),
				    MOMOUT_LITERAL (" in call "),
				    MOMOUT_VALUE (curinsv),
				    MOMOUT_SPACE (48),
				    MOMOUT_LITERAL ("at rank#"),
				    MOMOUT_DEC_INT (ix),
				    MOMOUT_LITERAL ("/"),
				    MOMOUT_DEC_INT (nbinstr),
				    MOMOUT_SPACE (48),
				    MOMOUT_LITERAL (" in block "),
				    MOMOUT_ITEM ((const momitem_t *)
						 blkitm), NULL);;
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
	  if (cg->cgen_rout.cgrout_kind == cgr_funt)
	    {
	      if (insarity > 3)
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL ("too many results in "),
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
	  else if (cg->cgen_rout.cgrout_kind == cgr_proc)
	    {
	      momval_t procv = mom_item_assoc_get (cg->cgen_globassocitm,
						   cg->
						   cgen_rout.cgrout_routitm);
	      assert (mom_node_conn (procv) == mom_named__procedure
		      && mom_node_arity (procv) == 3);
	      momval_t prorestypv = mom_node_nth (procv, 2);
	      if (insarity > 1)
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("too many procedure results in "),
				MOMOUT_VALUE (curinsv),
				MOMOUT_SPACE (48),
				MOMOUT_LITERAL
				("at rank#"),
				MOMOUT_DEC_INT (ix),
				MOMOUT_LITERAL ("/"),
				MOMOUT_DEC_INT (nbinstr),
				MOMOUT_SPACE (48),
				MOMOUT_LITERAL
				(" in block "),
				MOMOUT_ITEM ((const
					      momitem_t *) blkitm), NULL);
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
		  MOM_OUT (&cg->cgen_outbody, MOMOUT_SPACE (48),
			   MOMOUT_LITERAL (";"), MOMOUT_NEWLINE ());
		}
	      else
		CGEN_ERROR_MOM (cg,
				MOMOUT_LITERAL
				("bad procedure result in "),
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
	    }
	  else
	    MOM_FATAPRINTF ("invalid cgen_type #%d for return",
			    (int) cg->cgen_rout.cgrout_kind);
	}
      ///// error case - bad instruction
      else
	CGEN_ERROR_MOM (cg, MOMOUT_LITERAL ("invalid instruction:"),
			MOMOUT_VALUE ((const momval_t) curinsv),
			MOMOUT_SPACE (48),
			MOMOUT_LITERAL ("in block:"),
			MOMOUT_ITEM ((const momitem_t *) blkitm), NULL);
    };
  // emit block epilogue if locking
  if (lockix > 0)
    {
      MOM_OUT (&cg->cgen_outbody,
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("/*! epilogue for lock */"),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("mom_unlock_item ("
			       CGEN_LOCK_ITEM_PREFIX),
	       MOMOUT_DEC_INT (lockix),
	       MOMOUT_LITERAL ("); // unlock "),
	       MOMOUT_ITEM ((const momitem_t *) lockv.pitem),
	       MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL (CGEN_END_BLOCK_PREFIX),
	       MOMOUT_DEC_INT (lockix), MOMOUT_LITERAL (":;"),
	       MOMOUT_NEWLINE (), NULL);
    }
}

static void
emit_goto_block_cgen (struct c_generator_mom_st *cg,
		      momitem_t *blkitm, int lockix)
{
  assert (cg != NULL && cg->cgen_magic == CGEN_MAGIC);
  assert (blkitm != NULL && blkitm->i_typnum == momty_item);
  momval_t blockdatav =
    mom_item_assoc_get (cg->cgen_rout.cgrout_associtm, blkitm);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("emit goto block:"),
	     MOMOUT_ITEM ((const momitem_t *) blkitm),
	     MOMOUT_LITERAL (" lockix#"),
	     MOMOUT_DEC_INT (lockix),
	     MOMOUT_LITERAL (" blockdatav="), MOMOUT_VALUE (blockdatav));
  if (blockdatav.ptr == NULL
      || mom_node_conn (blockdatav) != mom_named__block)
    CGEN_ERROR_MOM (cg,
		    MOMOUT_LITERAL
		    ("bad block to go to:"),
		    MOMOUT_ITEM ((const momitem_t *) blkitm));
  int bix = mom_integer_val_def (mom_node_nth (blockdatav, 0), -2) - 1;
  assert (bix >= 0);
  if (lockix > 0)
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL
	     ("/*!unlock-goto*/ { mom_unlock_item ("
	      CGEN_LOCK_ITEM_PREFIX),
	     MOMOUT_DEC_INT (lockix), MOMOUT_LITERAL ("); "), NULL);
  else
    MOM_OUT (&cg->cgen_outbody, MOMOUT_LITERAL (" "));
  if (cg->cgen_rout.cgrout_kind == cgr_proc)
    // should retrieve the bix
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("goto " CGEN_PROC_BLOCK_PREFIX),
	     MOMOUT_DEC_INT (bix + 1),
	     MOMOUT_LITERAL (" /*!proc.block "),
	     MOMOUT_ITEM ((const momitem_t *) blkitm),
	     MOMOUT_LITERAL ("*/;"), NULL);
  else
    MOM_OUT (&cg->cgen_outbody,
	     MOMOUT_LITERAL (" return "),
	     MOMOUT_DEC_INT (bix + 1),
	     MOMOUT_LITERAL (" /*!func.block "),
	     MOMOUT_ITEM ((const momitem_t *) blkitm),
	     MOMOUT_LITERAL ("*/;"), NULL);
  if (lockix > 0)
    MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("}; //!unlocked "
			     CGEN_LOCK_ITEM_PREFIX),
	     MOMOUT_DEC_INT (lockix), MOMOUT_NEWLINE (), NULL);
}



void
emit_moduleinit_cgen (struct c_generator_mom_st *cg)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  assert (mom_is_item ((momval_t) cg->cgen_moditm));
  assert (mom_is_seqitem (cg->cgen_modseqv));
  unsigned nbrout = mom_seqitem_length (cg->cgen_modseqv);
  MOM_OUT (&cg->cgen_outhead,
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("// declare module md5sum for "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("const char " CGEN_MD5MOD_PREFIX),
	   MOMOUT_LITERALV ((const char *)
			    mom_ident_cstr_of_item
			    (cg->cgen_moditm)),
	   MOMOUT_LITERAL
	   ("[] = MONIMELT_MD5_MODULE; // Makefile generated"),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("// declare module routines descriptor array for "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("static const union momrout_un "
			   CGEN_DROUTARR_PREFIX),
	   MOMOUT_LITERALV ((const char *)
			    mom_ident_cstr_of_item
			    (cg->cgen_moditm)),
	   MOMOUT_LITERAL ("["),
	   MOMOUT_DEC_INT ((int) nbrout + 1),
	   MOMOUT_LITERAL ("];"), MOMOUT_NEWLINE (), NULL);
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL
	   ("// define module routines descriptor array for "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("static const union momrout_un "
			   CGEN_DROUTARR_PREFIX),
	   MOMOUT_LITERALV ((const char *)
			    mom_ident_cstr_of_item
			    (cg->cgen_moditm)),
	   MOMOUT_LITERAL ("["),
	   MOMOUT_DEC_INT ((int) nbrout + 1),
	   MOMOUT_LITERAL ("] = {"), MOMOUT_INDENT_MORE (),
	   MOMOUT_NEWLINE (), NULL);
  for (unsigned rix = 0; rix < nbrout; rix++)
    {
      momitem_t *routitm = mom_seqitem_nth_item (cg->cgen_modseqv, rix);
      if (!routitm)
	continue;
      momval_t routnodv = mom_item_assoc_get (cg->cgen_globassocitm, routitm);
      if (mom_node_conn (routnodv) == mom_named__procedure)
	{
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL ("["), MOMOUT_DEC_INT ((int) rix),
		   MOMOUT_LITERAL ("]= {.rproc= &"
				   MOM_PROCROUTDESCR_PREFIX),
		   MOMOUT_LITERALV (mom_ident_cstr_of_item (routitm)),
		   MOMOUT_LITERAL ("}, // procedure "),
		   MOMOUT_ITEM ((const momitem_t *) routitm),
		   MOMOUT_NEWLINE (), NULL);
	}
      else if (mom_node_conn (routnodv) == mom_named__tasklet_function)
	{
	  MOM_OUT (&cg->cgen_outbody,
		   MOMOUT_LITERAL ("["), MOMOUT_DEC_INT ((int) rix),
		   MOMOUT_LITERAL ("]= {.rtfun= &"
				   MOM_TFUN_NAME_PREFIX),
		   MOMOUT_LITERALV (mom_ident_cstr_of_item (routitm)),
		   MOMOUT_LITERAL ("}, // taskletfun "),
		   MOMOUT_ITEM ((const momitem_t *) routitm),
		   MOMOUT_NEWLINE (), NULL);
	}
      else			/* should never happen */
	MOM_FATAPRINTF ("corrupted routine rix#%d", rix);
    }
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_INDENT_LESS (),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("}; // end of module routines descriptor array for "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   MOMOUT_NEWLINE (), NULL);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("// module initialization for "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("void " MOM_MODULE_INIT_PREFIX),
	   MOMOUT_LITERALV ((const char *)
			    mom_ident_cstr_of_item
			    (cg->cgen_moditm)),
	   MOMOUT_LITERAL (" (void) {"),
	   MOMOUT_INDENT_MORE (), MOMOUT_NEWLINE ());
  MOM_OUT (&cg->cgen_outbody,
	   MOMOUT_LITERAL
	   ("mom_module_internal_initialize (\""),
	   MOMOUT_LITERALV ((const char *)
			    mom_ident_cstr_of_item
			    (cg->cgen_moditm)),
	   MOMOUT_LITERAL ("\" /*!module "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   MOMOUT_LITERAL ("*/,"), MOMOUT_NEWLINE (),
	   // the MONIMELT_MD5_MODULE is computed in the Makefile
	   MOMOUT_LITERAL
	   ("      MONIMELT_MD5_MODULE /*see Makefile*/,  "),
	   MOMOUT_DEC_INT ((int) nbrout),
	   MOMOUT_LITERAL (",  " CGEN_DROUTARR_PREFIX),
	   MOMOUT_LITERALV ((const char *)
			    mom_ident_cstr_of_item
			    (cg->cgen_moditm)), MOMOUT_LITERAL (");"), NULL);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("MOM_INFORMPRINTF(\"module "),
	   MOMOUT_ITEM ((const momitem_t *) cg->cgen_moditm),
	   // the MONIMELT_MD5_MODULE is computed in the Makefile
	   MOMOUT_LITERAL
	   (" of md5 \" MONIMELT_MD5_MODULE \" initialized.\");"), NULL);
  MOM_OUT (&cg->cgen_outbody, MOMOUT_INDENT_LESS (),
	   MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("} // end of module initialization"),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE (), NULL);
}				/* end emit_moduleinit_cgen */

static void
cgen_update_block_info (struct c_generator_mom_st *cg,
			momitem_t *routitm, momitem_t *blhitm,
			const momval_t routval);
void
cgen_update_module_info_mom (struct c_generator_mom_st *cg)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  momitem_t *globitm = cg->cgen_globassocitm;
  momitem_t *moditm = cg->cgen_moditm;
  momval_t setatv = mom_item_assoc_set_attrs (globitm);
  MOM_DEBUG			//
    (gencod,
     MOMOUT_LITERAL ("update_module_info start; globassoc:"),
     MOMOUT_ITEM ((const momitem_t *) globitm),
     MOMOUT_NEWLINE (),
     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) globitm),
     MOMOUT_NEWLINE (),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) globitm),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("setatv="),
     MOMOUT_VALUE ((const momval_t) setatv), NULL);
  unsigned nbelem = mom_set_cardinal (setatv);
  for (unsigned ix = 0; ix < nbelem; ix++)
    {
      const momitem_t *curelitm = mom_set_nth_item (setatv, ix);
      const momval_t curval = mom_item_assoc_get (globitm, curelitm);
      MOM_DEBUG			//
	(gencod,
	 MOMOUT_LITERAL ("update_module_info ix#"),
	 MOMOUT_DEC_INT ((int) ix),
	 MOMOUT_LITERAL (":"),
	 MOMOUT_ITEM (curelitm),
	 MOMOUT_NEWLINE (),
	 MOMOUT_LITERAL ("curval="), MOMOUT_VALUE (curval),
	 MOMOUT_NEWLINE (), NULL);
      const momitem_t *connitm = mom_node_conn (curval);
      const momitem_t *blhitm = NULL;
      if (connitm == mom_named__procedure && mom_node_arity (curval) == 4)
	{
	/** *procedure(<rank>,<sigstring>,<restype>,<blockhset>) */
	  blhitm = mom_value_to_item (mom_node_nth (curval, 3));
	  momval_t rankv = mom_node_nth (curval, 0);
	  // momval_t sigstrv = mom_node_nth (curval, 1);
	  momval_t inodv = (momval_t)	//
	    mom_make_node_sized ((momval_t) mom_named__module,
				 2,
				 (momval_t) moditm,
				 rankv);
	  mom_item_put_attribute ((momitem_t *) curelitm, mom_named__in,
				  inodv);
	  MOM_DEBUG (gencod, MOMOUT_LITERAL ("update_module_info proc="),
		     MOMOUT_ITEM (curelitm), MOMOUT_SPACE (48),
		     MOMOUT_LITERAL ("inodv:"),
		     MOMOUT_VALUE ((const momval_t) inodv), MOMOUT_NEWLINE (),
		     MOMOUT_LITERAL ("blhitm:"), MOMOUT_ITEM (blhitm),
		     MOMOUT_NEWLINE (), MOMOUT_ITEM_ATTRIBUTES (blhitm),
		     MOMOUT_NEWLINE (), MOMOUT_ITEM_PAYLOAD (blhitm), NULL);
	}
      else if (connitm == mom_named__tasklet_function
	       && mom_node_arity (curval) == 5)
	{
	/** *tasklet_function(<rank>,<numbers>,<doubles>,<values>,<blockhset>) */
	  momval_t rankv = mom_node_nth (curval, 0);
	  momval_t numbersv = mom_node_nth (curval, 1);
	  momval_t doublesv = mom_node_nth (curval, 2);
	  momval_t valuesv = mom_node_nth (curval, 3);
	  blhitm = mom_value_to_item (mom_node_nth (curval, 4));
	  momval_t inodv = (momval_t)	//
	    mom_make_node_sized ((momval_t) mom_named__module,
				 2,
				 (momval_t) moditm,
				 rankv);
	  mom_item_put_attribute ((momitem_t *) curelitm, mom_named__in,
				  inodv);
	  MOM_DEBUG (gencod, MOMOUT_LITERAL ("update_module_info tfun="),
		     MOMOUT_ITEM (curelitm), MOMOUT_SPACE (48),
		     MOMOUT_LITERAL ("inodv:"),
		     MOMOUT_VALUE ((const momval_t) inodv), MOMOUT_NEWLINE (),
		     MOMOUT_LITERAL ("numbersv:"),
		     MOMOUT_VALUE ((const momval_t) numbersv),
		     MOMOUT_NEWLINE (), MOMOUT_LITERAL ("doublesv:"),
		     MOMOUT_VALUE ((const momval_t) doublesv),
		     MOMOUT_NEWLINE (), MOMOUT_LITERAL ("valuesv:"),
		     MOMOUT_VALUE ((const momval_t) valuesv),
		     MOMOUT_NEWLINE (), MOMOUT_LITERAL ("blhitm:"),
		     MOMOUT_ITEM (blhitm), MOMOUT_NEWLINE (),
		     MOMOUT_ITEM_ATTRIBUTES (blhitm), MOMOUT_NEWLINE (),
		     MOMOUT_ITEM_PAYLOAD (blhitm), NULL);
	  //// update `numbers`
	  if (numbersv.ptr)
	    {
	      assert (mom_node_conn (numbersv) == mom_named__numbers);
	      unsigned nbnum = mom_node_arity (numbersv);
	      mom_item_put_attribute	//
		((momitem_t *) curelitm, mom_named__numbers,	//
		 (momval_t) mom_make_tuple_from_array	//
		 (nbnum, (const momitem_t **) numbersv.pnode->sontab));
	    }
	  else
	    mom_item_remove_attribute ((momitem_t *) curelitm,
				       mom_named__numbers);
	  //// update `doubles`
	  if (doublesv.ptr)
	    {
	      assert (mom_node_conn (doublesv) == mom_named__doubles);
	      unsigned nbdbl = mom_node_arity (doublesv);
	      mom_item_put_attribute	//
		((momitem_t *) curelitm, mom_named__doubles,	//
		 (momval_t) mom_make_tuple_from_array	//
		 (nbdbl, (const momitem_t **) doublesv.pnode->sontab));
	    }
	  else
	    mom_item_remove_attribute ((momitem_t *) curelitm,
				       mom_named__doubles);
	  //// update `values`
	  if (valuesv.ptr)
	    {
	      assert (mom_node_conn (valuesv) == mom_named__values);
	      unsigned nbval = mom_node_arity (valuesv);
	      mom_item_put_attribute	//
		((momitem_t *) curelitm, mom_named__values,	//
		 (momval_t) mom_make_tuple_from_array	//
		 (nbval, (const momitem_t **) valuesv.pnode->sontab));
	    }
	  else
	    mom_item_remove_attribute ((momitem_t *) curelitm,
				       mom_named__values);
	}
      else
	CGEN_ERROR_MOM
	  (cg, MOMOUT_LITERAL ("update_module_info unexpected curval="),
	   MOMOUT_VALUE (curval),
	   MOMOUT_LITERAL (" for curelitm="), MOMOUT_ITEM (curelitm));
      if (blhitm)
	cgen_update_block_info (cg, (momitem_t *) curelitm,
				(momitem_t *) blhitm, curval);
      MOM_DEBUG (gencod,
		 MOMOUT_LITERAL ("update_module_info updated curelitm="),
		 MOMOUT_ITEM ((const momitem_t *) curelitm),
		 MOMOUT_NEWLINE (),
		 MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) curelitm), NULL);
    }

}				/* end cgen_update_module_info_mom */




static void
cgen_update_block_info (struct c_generator_mom_st *cg,
			momitem_t *routitm, momitem_t *blhitm,
			const momval_t blval)
{
  assert (cg && cg->cgen_magic == CGEN_MAGIC);
  MOM_DEBUG
    (gencod,
     MOMOUT_LITERAL ("update_block_info routitm="),
     MOMOUT_ITEM ((const momitem_t *) routitm),
     MOMOUT_LITERAL (" blhitm="),
     MOMOUT_ITEM ((const momitem_t *) blhitm),
     MOMOUT_NEWLINE (),
     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) blhitm),
     MOMOUT_NEWLINE (),
     MOMOUT_ITEM_PAYLOAD ((const momitem_t *) blhitm),
     MOMOUT_NEWLINE (),
     MOMOUT_LITERAL ("blval="),
     MOMOUT_VALUE (blval), MOMOUT_NEWLINE (), NULL);
  momval_t blsetv = mom_item_hset_items_set (blhitm);
  momval_t blnodv =
    mom_item_hset_sorted_values_node (blhitm, mom_named__blocks);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("update_block_info routitm="),
	     MOMOUT_ITEM ((const momitem_t *) routitm), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" blsetv="),
	     MOMOUT_VALUE ((const momval_t) blsetv), MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL (" blnodv="),
	     MOMOUT_VALUE ((const momval_t) blnodv), NULL);
  mom_item_put_attribute (routitm, mom_named__blocks, blsetv);
  unsigned nbblocks = mom_set_cardinal (blsetv);
  unsigned nodlen = mom_node_arity (blnodv);
  for (unsigned bix = 0; bix < nbblocks; bix++)
    {
      momitem_t *curblitm = mom_set_nth_item (blsetv, bix);
      momval_t inodv = (momval_t)	//
	mom_make_node_sized ((momval_t) mom_node_conn (blval),
			     2,
			     mom_make_integer (bix + 1),
			     (momval_t) routitm);
      mom_item_put_attribute (curblitm, mom_named__in, inodv);
      MOM_DEBUG (gencod,
		 MOMOUT_LITERAL ("updateblock curblitm:"),
		 MOMOUT_ITEM ((const momitem_t *) curblitm),
		 MOMOUT_LITERAL (" in "),
		 MOMOUT_VALUE ((const momval_t) inodv), NULL);
    }
  // we have nbblocks items in blnodv, and the rest is for jumps, with
  // a jump and a from node for each of them.
  unsigned nbjumps = (nodlen - nbblocks) / 2;
  momval_t *fromarr =
    MOM_GC_ALLOC ("fromarr", (nbjumps + 2) * sizeof (momval_t));
  momval_t *jumparr =
    MOM_GC_ALLOC ("jumparr", (nbjumps + 2) * sizeof (momval_t));
  unsigned cntfrom = 0, cntjump = 0;
  for (unsigned nix = 0; nix < nodlen; nix++)
    {
      const momval_t curcompv = mom_node_nth (blnodv, nix);
      if (!mom_is_node (curcompv))
	continue;
      const momitem_t *compconnitm = mom_node_conn (curcompv);
      if (compconnitm == mom_named__jump)
	{
	  assert (cntjump < nbjumps);
	  jumparr[cntjump] = curcompv;
	  MOM_DEBUG (gencod,
		     MOMOUT_LITERAL ("jumparr["),
		     MOMOUT_DEC_INT ((int) cntjump),
		     MOMOUT_LITERAL ("]="), MOMOUT_VALUE (curcompv), NULL);
	  cntjump++;
	}
      else if (compconnitm == mom_named__from)
	{
	  assert (cntfrom < nbjumps);
	  fromarr[cntfrom] = curcompv;
	  MOM_DEBUG (gencod,
		     MOMOUT_LITERAL ("fromarr["),
		     MOMOUT_DEC_INT ((int) cntfrom),
		     MOMOUT_LITERAL ("]="), MOMOUT_VALUE (curcompv), NULL);
	  cntfrom++;
	}
      else
	// this should never happen
	MOM_FATAL
	  (MOMOUT_LITERAL ("invalid component curcompv="),
	   MOMOUT_VALUE (curcompv));
    }
  //// use the fromarr to compute the predecessors relation
  int fromix = 0;
  while (fromix < (int) cntfrom)
    {
      momitem_t *fromitm =
	mom_value_to_item (mom_node_nth (fromarr[fromix], 0));
      assert (fromitm != NULL);
      int nextfromix = fromix;
      do
	{
	  nextfromix++;
	}
      while (nextfromix < (int) cntfrom
	     && mom_value_to_item
	     (mom_node_nth (fromarr[nextfromix], 0)) == fromitm);
      int fromdelta = nextfromix - fromix;
      momitem_t *samefromtinyarr[MOM_TINY_MAX];
      memset (samefromtinyarr, 0, sizeof (samefromtinyarr));
      momitem_t **samefromarr = (fromdelta < MOM_TINY_MAX) ? samefromtinyarr
	: MOM_GC_ALLOC ("samefromarr",
			(fromdelta + 1) * sizeof (momitem_t *));
      for (int frof = 0; frof < fromdelta; frof++)
	{
	  samefromarr[frof] =
	    mom_value_to_item (mom_node_nth (fromarr[fromix + frof], 1));
	}
      momval_t samefromsetv = (momval_t) mom_make_set_from_array (fromdelta,
								  (const
								   momitem_t
								   **)
								  samefromarr);
      if (samefromarr != samefromtinyarr)
	MOM_GC_FREE (samefromarr);
      mom_item_put_attribute (fromitm, mom_named__predecessors, samefromsetv);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("update_block_info fromix#"),
		 MOMOUT_DEC_INT (fromix),
		 MOMOUT_LITERAL (" nextfromix#"),
		 MOMOUT_DEC_INT (nextfromix),
		 MOMOUT_LITERAL (" fromitm="),
		 MOMOUT_ITEM ((const momitem_t *) fromitm),
		 MOMOUT_LITERAL (" samefromsetv="),
		 MOMOUT_VALUE ((const momval_t) samefromsetv), NULL);
      fromix = nextfromix;
    };
  //// use the jumparr to compute the successors relation
  int jumpix = 0;
  while (jumpix < (int) cntjump)
    {
      momitem_t *jumpitm =
	mom_value_to_item (mom_node_nth (jumparr[jumpix], 0));
      assert (jumpitm != NULL);
      int nextjumpix = jumpix;
      do
	{
	  nextjumpix++;
	}
      while (nextjumpix < (int) cntjump
	     && mom_value_to_item
	     (mom_node_nth (jumparr[nextjumpix], 0)) == jumpitm);
      int jumpdelta = nextjumpix - jumpix;
      momitem_t *samejumptinyarr[MOM_TINY_MAX];
      memset (samejumptinyarr, 0, sizeof (samejumptinyarr));
      momitem_t **samejumparr = (jumpdelta < MOM_TINY_MAX) ? samejumptinyarr
	: MOM_GC_ALLOC ("samejumparr",
			(jumpdelta + 1) * sizeof (momitem_t *));
      for (int frof = 0; frof < jumpdelta; frof++)
	{
	  samejumparr[frof] =
	    mom_value_to_item (mom_node_nth (jumparr[jumpix + frof], 1));
	}
      momval_t samejumpsetv = (momval_t) mom_make_set_from_array (jumpdelta,
								  (const
								   momitem_t
								   **)
								  samejumparr);
      if (samejumparr != samejumptinyarr)
	MOM_GC_FREE (samejumparr);
      mom_item_put_attribute (jumpitm, mom_named__successors, samejumpsetv);
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("update_block_info jumpix#"),
		 MOMOUT_DEC_INT (jumpix),
		 MOMOUT_LITERAL (" nextjumpix#"),
		 MOMOUT_DEC_INT (nextjumpix),
		 MOMOUT_LITERAL (" jumpitm="),
		 MOMOUT_ITEM ((const momitem_t *) jumpitm),
		 MOMOUT_LITERAL (" samejumpsetv="),
		 MOMOUT_VALUE ((const momval_t) samejumpsetv), NULL);
      jumpix = nextjumpix;
    };
}				/* end cgen_update_block_info */

//// end of file gencod.c
