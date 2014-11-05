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
   associated to a set of routine items.

   A routine item is an item with a `body` attribute associated to a
   block item (for the starting state).

   A block item is an item with a `block` attribute associated to a `block` node.
   The sons of that node are instruction nodes.
 ****/


#define CGEN_MAGIC 0x566801a5	/* cgen magic 1449656741 */

// routine specific structure
struct cgen_routine_mom_st
{
  momitem_t *cgrout_item;	/* the routine item */
  unsigned cgrout_nbblock;
  unsigned cgrout_sizeblock;
  momitem_t **cgrout_blocktab;	/* of cgrout_sizeblock size */
};

// internal stack allocated structure to generate the C module
struct c_generator_mom_st
{
  unsigned cgen_magic;		// always CGEN_MAGIC
  jmp_buf cgen_jbuf;
  momitem_t *cgen_moditm;
  char *cgen_errmsg;
  unsigned cgen_nbrout;
  momitem_t *cgen_curoutitm;
  struct mom_itemattributes_st *cgen_attrs;
  struct cgen_routine_mom_st *cgen_routab;
  struct mom_valuequeue_st cgen_blockqueue;
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
  free (outbuf), outbuf = NULL;
  longjmp (cgen->cgen_jbuf, lin);
}

#define CGEN_ERROR_MOM_AT_BIS(Lin,Cgen,...) cgen_error_mom_at(Lin,Cgen,__VA_ARGS__,NULL)
#define CGEN_ERROR_MOM_AT(Lin,Cgen,...) CGEN_ERROR_MOM_AT_BIS(Lin,Cgen,__VA_ARGS__)
#define CGEN_ERROR_MOM(Cgen,...) CGEN_ERROR_MOM_AT(__LINE__,Cgen,__VA_ARGS__)


static void
cgen_scan_routine_mom (struct c_generator_mom_st *cgen, momitem_t *itrout,
		       int index);

static void
cgen_scan_block_mom (struct c_generator_mom_st *cgen, int routix,
		     momitem_t *itblock);


static void
cgen_scan_instr_mom (struct c_generator_mom_st *cgen, int routix,
		     const momitem_t *blockitm, int insix, momval_t insv);

static inline void
cgen_should_scan_block_mom (struct c_generator_mom_st *cgen, int routix,
			    momitem_t *blockitm)
{
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  assert (routix >= 0 && routix < (int) cgen->cgen_nbrout);
  assert (blockitm && blockitm->i_typnum == momty_item);
  momval_t blockat = mom_get_attribute (cgen->cgen_attrs, blockitm);
  momval_t ixv = MOM_NULLV;
  if (blockat.ptr)
    {
      if (mom_node_conn (blockat) == mom_named__block
	  && (ixv = mom_node_nth (blockat, 0)).ptr
	  && mom_is_integer (ixv) && mom_integer_val_def (ixv, -1) == routix)
	return;
      else
	CGEN_ERROR_MOM (cgen, MOMOUT_LITERAL ("already meet block:"),
			MOMOUT_ITEM ((const momitem_t *) blockitm),
			MOMOUT_LITERAL (" in routine:"),
			MOMOUT_ITEM ((const momitem_t *)
				     cgen->cgen_curoutitm));
    }
  int blockix = (int) cgen->cgen_routab[routix].cgrout_nbblock++;
  cgen->cgen_routab[routix].cgrout_blocktab[blockix] = blockitm;
  cgen->cgen_attrs = mom_put_attribute
    (cgen->cgen_attrs,
     blockitm,
     (momval_t) mom_make_node_sized (mom_named__block,
				     2,
				     mom_make_integer (routix),
				     mom_make_integer (blockix)));
  mom_queue_add_value_back (&cgen->cgen_blockqueue, (momval_t) blockitm);
}

int
mom_generate_c_module (momitem_t *moditm, char **perrmsg)
{
  int jr = 0;
  struct c_generator_mom_st cgen;
  memset (&cgen, 0, sizeof cgen);
  momval_t modroutv = MOM_NULLV;
  assert (perrmsg != NULL);
  if (!moditm || moditm->i_typnum != momty_item)
    {
      *perrmsg = "non-item module";
      return __LINE__;
    }
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("start of generate_c_module moditm:"),
	     MOMOUT_ITEM ((const momitem_t *) moditm), NULL);
  cgen.cgen_magic = CGEN_MAGIC;
  cgen.cgen_moditm = moditm;
  cgen.cgen_attrs = mom_reserve_attribute (NULL, 32);
  jr = setjmp (cgen.cgen_jbuf);
  if (jr)
    {
      MOM_DEBUGPRINTF (gencod, "generate_c_module got errored #%d: %s", jr,
		       cgen.cgen_errmsg);
      assert (cgen.cgen_errmsg != NULL);
      *perrmsg = cgen.cgen_errmsg;
      return jr;
    }
  {
    mom_should_lock_item (moditm);
    modroutv = mom_item_get_attribute (moditm, mom_named__module_routines);
    mom_unlock_item (moditm);
    if (!mom_is_set (modroutv))
      CGEN_ERROR_MOM (&cgen, MOMOUT_LITERAL ("generate_c_module module:"),
		      MOMOUT_ITEM ((const momitem_t *) moditm),
		      MOMOUT_SPACE (48),
		      MOMOUT_LITERAL ("has unexpected module_routines:"),
		      MOMOUT_VALUE (modroutv));
  }
  cgen.cgen_nbrout = mom_set_cardinal (modroutv);
  cgen.cgen_routab = MOM_GC_ALLOC ("cgen routine",
				   cgen.cgen_nbrout *
				   sizeof (struct cgen_routine_mom_st));
  for (unsigned ix = 0; ix < cgen.cgen_nbrout; ix++)
    {
      momitem_t *curoutitm = mom_set_nth_item (modroutv, ix);
      assert (curoutitm && curoutitm->i_typnum == momty_item);
      cgen.cgen_curoutitm = curoutitm;
      cgen_scan_routine_mom (&cgen, curoutitm, ix);
      cgen.cgen_curoutitm = NULL;
    }
}


static void
cgen_scan_routine_mom (struct c_generator_mom_st *cgen, momitem_t *routitm,
		       int ix)
{
  momitem_t *bodyitm = NULL;
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  assert (routitm && routitm->i_typnum == momty_item);
  assert (ix >= 0 && ix < (int) cgen->cgen_nbrout);
  struct cgen_routine_mom_st *cgr = cgen->cgen_routab + ix;
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("cgen_scan_routine routitm="),
	     MOMOUT_ITEM ((const momitem_t *) routitm), NULL);
  momval_t routat = mom_get_attribute (cgen->cgen_attrs, routitm);
  if (routat.ptr)
    {
      MOM_DEBUG (gencod, MOMOUT_LITERAL ("cgen_scan_routine routat="),
		 MOMOUT_VALUE ((const momval_t) routat));
      CGEN_ERROR_MOM (cgen,
		      MOMOUT_LITERAL ("scan_routine already meet routitm:"),
		      MOMOUT_ITEM ((const momitem_t *) routitm),
		      MOMOUT_LITERAL (" #"), MOMOUT_DEC_INT (ix), NULL);
    }
  {
    mom_should_lock_item (routitm);
    bodyitm = mom_value_to_item (mom_item_get_attribute (routitm,
							 mom_named__body));
    mom_unlock_item (routitm);
  }
  if (!bodyitm)
    CGEN_ERROR_MOM (cgen,
		    MOMOUT_LITERAL ("scan_routine missing body in routitm:"),
		    MOMOUT_ITEM ((const momitem_t *) routitm));
  cgr->cgrout_item = routitm;
  cgr->cgrout_nbblock = 0;
  cgr->cgrout_sizeblock = 16;
  cgr->cgrout_blocktab =
    MOM_GC_ALLOC ("blocktab", cgr->cgrout_sizeblock * sizeof (momitem_t *));
  cgen->cgen_attrs = mom_put_attribute
    (cgen->cgen_attrs,
     routitm,
     (momval_t) mom_make_node_sized (mom_named__module_routines, 1,
				     mom_make_integer (ix)));
  cgen_should_scan_block_mom (cgen, ix, bodyitm);
  while (!mom_queue_is_empty (&cgen->cgen_blockqueue))
    {
      momitem_t *blockitm =
	mom_value_to_item (mom_queue_pop_value_front
			   (&cgen->cgen_blockqueue));
      assert (blockitm && blockitm->i_typnum == momty_item);
      cgen_scan_block_mom (cgen, ix, blockitm);
    }
}

static void
cgen_scan_block_mom (struct c_generator_mom_st *cgen, int routix,
		     momitem_t *blockitm)
{
  momval_t blockv = MOM_NULLV;
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  assert (routix >= 0 && routix < (int) cgen->cgen_nbrout);
  assert (blockitm && blockitm->i_typnum == momty_item);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("cgen_scan_block blockitm="),
	     MOMOUT_ITEM ((const momitem_t *) blockitm),
	     MOMOUT_SPACE (48),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) blockitm), NULL);
  {
    mom_should_lock_item (blockitm);
    blockv = mom_item_get_attribute (blockitm, mom_named__block);
    mom_unlock_item (blockitm);
  }
  if (mom_node_conn (blockv) != mom_named__block)
    CGEN_ERROR_MOM (cgen,
		    MOMOUT_LITERAL ("bad block value:"),
		    MOMOUT_VALUE ((const momval_t) blockv),
		    MOMOUT_LITERAL ("in blockitm:"),
		    MOMOUT_ITEM ((const momitem_t *) blockitm),
		    MOMOUT_LITERAL (" routix="),
		    MOMOUT_DEC_INT (routix), NULL);
  unsigned aritblock = mom_node_arity (blockv);
  for (unsigned insix = 0; insix < aritblock; insix++)
    {
      momval_t curinsv = mom_node_nth (blockv, insix);
      cgen_scan_instr_mom (cgen, routix, blockitm, insix, curinsv);
    }
}



static void
cgen_scan_instr_mom (struct c_generator_mom_st *cgen, int routix,
		     const momitem_t *blockitm, int insix, momval_t insv)
{
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  const momitem_t *opitm = mom_node_conn (insv);
  if (!opitm)
    CGEN_ERROR_MOM (cgen, MOMOUT_LITERAL ("bad instruction#"),
		    MOMOUT_DEC_INT (insix),
		    MOMOUT_SPACE (48),
		    MOMOUT_VALUE ((const momval_t) insv),
		    MOMOUT_SPACE (48),
		    MOMOUT_LITERAL ("in block:"),
		    MOMOUT_ITEM (blockitm),
		    MOMOUT_SPACE (48),
		    MOMOUT_LITERAL ("routine#"),
		    MOMOUT_DEC_INT (routix),
		    MOMOUT_SPACE (48),
		    MOMOUT_ITEM ((const momitem_t *) cgen->cgen_curoutitm),
		    NULL);
#define OPHASHMOD 2381
  switch (opitm->i_hash % OPHASHMOD)
    {
    case 1548541904 % OPHASHMOD:
      if (opitm == mom_named__assign)
	{
	  momval_t leftv = mom_node_nth (insv, 0);
	  momval_t rightv = mom_node_nth (insv, 1);

	};
      break;
    case 4023442404 % OPHASHMOD:
      if (opitm == mom_named__perform)
	{
	  momval_t exprv = mom_node_nth (insv, 0);
	};
      break;
    }
#undef OPHASHMOD

}

//// eof gencod.c
