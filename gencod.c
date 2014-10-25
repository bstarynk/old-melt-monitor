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

#define CGEN_MAGIC 0x566801a5	/* cgen magic 1449656741 */
// internal stack allocated structure to generate the C module
struct c_generator_mom_st
{
  unsigned cgen_magic;		// always CGEN_MAGIC
  jmp_buf cgen_jbuf;
  momitem_t *cgen_moditm;
  char *cgen_errmsg;
  unsigned cgen_nbrout;
  momitem_t *cgen_curoutitm;
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
cgen_scan_routine_mom (struct c_generator_mom_st *cgen, momitem_t *itrout);

static void
cgen_scan_block_mom (struct c_generator_mom_st *cgen, momitem_t *itrout);

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
  for (unsigned ix = 0; ix < cgen.cgen_nbrout; ix++)
    {
      momitem_t *curoutitm = mom_set_nth_item (modroutv, ix);
      assert (curoutitm && curoutitm->i_typnum == momty_item);
      cgen.cgen_curoutitm = curoutitm;
      cgen_scan_routine_mom (&cgen, curoutitm);
      cgen.cgen_curoutitm = NULL;
    }
}


static void
cgen_scan_routine_mom (struct c_generator_mom_st *cgen, momitem_t *routitm)
{
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  assert (routitm && routitm->i_typnum == momty_item);
  MOM_DEBUG (gencod, MOMOUT_LITERAL ("cgen_scan_routine routitm="),
	     MOMOUT_ITEM ((const momitem_t *) routitm), NULL);
}

static void
cgen_scan_block_mom (struct c_generator_mom_st *cgen, momitem_t *blockitm)
{
  assert (cgen && cgen->cgen_magic == CGEN_MAGIC);
  assert (blockitm && blockitm->i_typnum == momty_item);
}
