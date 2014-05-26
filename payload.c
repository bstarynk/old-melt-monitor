// file payload.c

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

////////////////////////////////////////////////////////////////
///// QUEUE PAYLOAD
////////////////////////////////////////////////////////////////

/** the queue payload data is a GC_MALLOC-ed struct mom_valuequeue_st */

void
mom_item_start_queue (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  itm->i_payload =
    MOM_GC_ALLOC ("item queue", sizeof (struct mom_valuequeue_st));
  itm->i_paylkind = mompayk_queue;
}

void
mom_item_queue_add_back (momitem_t *itm, momval_t val)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return;
  assert (itm->i_payload != NULL);
  mom_queue_add_value_back ((struct mom_valuequeue_st *) (itm->i_payload),
			    val);
}

void
mom_item_queue_add_front (momitem_t *itm, momval_t val)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return;
  assert (itm->i_payload != NULL);
  mom_queue_add_value_front ((struct mom_valuequeue_st *) (itm->i_payload),
			     val);
}

bool
mom_item_queue_is_empty (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return true;
  assert (itm->i_payload != NULL);
  return mom_queue_is_empty ((struct mom_valuequeue_st *) (itm->i_payload));
}

unsigned
mom_item_queue_length (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return 0;
  assert (itm->i_payload != NULL);
  return mom_queue_length ((struct mom_valuequeue_st *) (itm->i_payload));
}

momval_t
mom_item_queue_peek_front (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  return
    mom_queue_peek_value_front ((struct mom_valuequeue_st
				 *) (itm->i_payload));
}

momval_t
mom_item_queue_peek_back (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  return
    mom_queue_peek_value_back ((struct mom_valuequeue_st *) (itm->i_payload));
}

momval_t
mom_item_queue_pop_front (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  return
    mom_queue_pop_value_front ((struct mom_valuequeue_st *) (itm->i_payload));
}

static void
payl_queue_load_mom (struct mom_loader_st *ld, momitem_t *itm, momval_t jpayl)
{
  assert (ld != NULL);
  assert (itm && itm->i_typnum == momty_item);
  mom_item_start_queue (itm);
  unsigned len = mom_json_array_size (jpayl);
  for (unsigned ix = 0; ix < len; ix++)
    mom_item_queue_add_back
      (itm, mom_load_value_json (ld, mom_json_array_nth (jpayl, ix)));
}

static void
payl_queue_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_queue);
  assert (itm->i_payload != NULL);
  for (struct mom_vaqelem_st * qel
       = ((struct mom_valuequeue_st *)itm->i_payload)->vaq_first;
       qel != NULL; qel = qel->vqe_next)
    mom_dump_scan_value (du, qel->vqe_val);
}

static momval_t
payl_queue_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jarr = MOM_NULLV;
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_queue);
  assert (itm->i_payload != NULL);
  unsigned qlen =
    mom_queue_length (((struct mom_valuequeue_st *) itm->i_payload));
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrval =
    (qlen < MOM_TINY_MAX) ? tinyarr
    : MOM_GC_ALLOC ("dump queue array", qlen * sizeof (momval_t));
  unsigned cnt = 0;
  for (struct mom_vaqelem_st * qel
       = ((struct mom_valuequeue_st *)itm->i_payload)->vaq_first;
       qel != NULL && cnt < qlen; (qel = qel->vqe_next), cnt++)
    arrval[cnt] = mom_dump_emit_json (du, qel->vqe_val);
  jarr = (momval_t) mom_make_json_array_count (qlen, arrval);
  if (arrval != tinyarr)
    MOM_GC_FREE (arrval);
  return jarr;
}

static const struct mom_payload_descr_st payldescr_queue_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "queue",
  .dpayl_loadfun = payl_queue_load_mom,
  .dpayl_dumpscanfun = payl_queue_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_queue_dump_json_mom,
};


////////////////////////////////////////////////////////////////
///// ROUTINE PAYLOAD
////////////////////////////////////////////////////////////////

void
mom_item_start_routine (momitem_t *itm, const char *routname)
{
  char symbuf[MOM_SYMBNAME_LEN];
  memset (symbuf, 0, sizeof (symbuf));
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  if (!routname || !routname[0])
    routname = mom_string_cstr ((momval_t) mom_item_get_name_or_idstr (itm));
  if (!routname || !routname[0])
    return;
  snprintf (symbuf, sizeof (symbuf), MOM_ROUTINE_NAME_FMT, routname);
  assert (symbuf[MOM_SYMBNAME_LEN] - 1 == '\0');
  for (const char *pc = symbuf; *pc; pc++)
    if (!isalnum (*pc) && *pc != '_')
      return;
  assert (isalpha (symbuf[0]));
  void *routad = dlsym (mom_prog_dlhandle, symbuf);
  if (!routad)
    {
      MOM_WARNPRINTF ("failed to start routine %s: %s", symbuf, dlerror ());
      return;
    };
  const struct momroutinedescr_st *rdescr = routad;
  if (rdescr->rout_magic != MOM_ROUTINE_MAGIC
      || !rdescr->rout_name || !rdescr->rout_module
      || !rdescr->rout_codefun || !rdescr->rout_timestamp)
    MOM_FATAPRINTF ("invalid routine descriptor @%p for %s", routad,
		    routname);
  if (strcmp (routname, rdescr->rout_name))
    MOM_WARNPRINTF ("strange routine descriptor for %s but named %s",
		    routname, rdescr->rout_name);
  itm->i_payload = (void *) rdescr;
  itm->i_paylkind = mompayk_routine;
  MOM_DEBUG (run, MOMOUT_LITERAL ("starting routine item:"),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (", named "),
	     MOMOUT_LITERALV (rdescr->rout_name),
	     MOMOUT_LITERAL (" from module "),
	     MOMOUT_LITERALV (rdescr->rout_module),
	     MOMOUT_LITERAL (" timestamp "),
	     MOMOUT_LITERALV (rdescr->rout_timestamp));
}

static void
payl_routine_load_mom (struct mom_loader_st *ld, momitem_t *itm,
		       momval_t jsonv)
{
  assert (ld != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  mom_item_start_routine (itm, mom_string_cstr (jsonv));
}

static momval_t
payl_routine_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_routine);
  const struct momroutinedescr_st *rdescr = itm->i_payload;
  assert (rdescr != NULL && rdescr->rout_magic == MOM_ROUTINE_MAGIC
	  && rdescr->rout_name != NULL && rdescr->rout_module);
  mom_dump_require_module (du, rdescr->rout_module);
  return (momval_t) mom_make_string (rdescr->rout_name);
}

static const struct mom_payload_descr_st payldescr_routine_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "routine",
  .dpayl_loadfun = payl_routine_load_mom,
  .dpayl_dumpjsonfun = payl_routine_dump_json_mom,
};



////////////////////////////////////////////////////////////////
///// TASKLET PAYLOAD
////////////////////////////////////////////////////////////////

#define TASKLET_THRESHOLD 64
void
mom_item_start_tasklet (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  struct mom_taskletdata_st *itd =
    MOM_GC_ALLOC ("item tasklet", sizeof (struct mom_taskletdata_st));
  const unsigned scalsize = 8;
  const unsigned valsize = 8;
  const unsigned frasize = 6;
  itd->dtk_scalars =
    MOM_GC_SCALAR_ALLOC ("tasklet scalars", scalsize * sizeof (intptr_t));
  itd->dtk_values =
    MOM_GC_ALLOC ("tasklet values", valsize * sizeof (momval_t));
  itd->dtk_closures =
    MOM_GC_ALLOC ("tasklet closures", frasize * sizeof (momnode_t *));
  itd->dtk_frames =
    MOM_GC_SCALAR_ALLOC ("tasklet frames",
			 frasize * sizeof (struct momframe_st));
  itd->dtk_scalsize = scalsize;
  itd->dtk_valsize = valsize;
  itd->dtk_frasize = frasize;
  itm->i_payload = itd;
  itm->i_paylkind = mompayk_tasklet;
}



void
mom_item_tasklet_reserve (momitem_t *itm, unsigned nbint, unsigned nbdbl,
			  unsigned nbval, unsigned nbfram)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return;
  struct mom_taskletdata_st *itd = itm->i_payload;
  /// resize the scalars if needed
  unsigned scalwant =
    itd->dtk_scaltop +
    ((nbint * sizeof (intptr_t) +
      nbdbl * sizeof (double)) / sizeof (intptr_t));
  if (MOM_UNLIKELY (scalwant > itd->dtk_scalsize
		    || (itd->dtk_scalsize > TASKLET_THRESHOLD
			&& 2 * scalwant < itd->dtk_scalsize)))
    {
      unsigned newscalsize =
	((5 * itd->dtk_scaltop / 4 + 3 +
	  ((nbint * sizeof (intptr_t) +
	    nbdbl * sizeof (double)) / sizeof (intptr_t))) | 7) + 1;
      if (newscalsize != itd->dtk_scalsize)
	{
	  intptr_t *newscalars
	    = MOM_GC_SCALAR_ALLOC ("reserved tasklet scalars",
				   newscalsize * sizeof (intptr_t));
	  if (itd->dtk_scaltop > 0)
	    memcpy (newscalars, itd->dtk_scalars,
		    itd->dtk_scaltop * sizeof (intptr_t));
	  MOM_GC_FREE (itd->dtk_scalars);
	  itd->dtk_scalars = newscalars;
	  itd->dtk_scalsize = newscalsize;
	}
    };
  /// resize the values if needed
  unsigned valuwant = itd->dtk_valtop + nbval;
  if (MOM_UNLIKELY
      (valuwant > itd->dtk_valsize
       || (itd->dtk_valsize > TASKLET_THRESHOLD
	   && 2 * valuwant < itd->dtk_valsize)))
    {
      unsigned newvalsize = ((5 * itd->dtk_valtop / 4 + 3 + nbval) | 7) + 1;
      if (newvalsize != itd->dtk_valsize)
	{
	  momval_t *newvalues = MOM_GC_ALLOC ("reserved tasklet values",
					      newvalsize * sizeof (momval_t));
	  memcpy (newvalues, itd->dtk_values,
		  itd->dtk_valtop * sizeof (momval_t));
	  MOM_GC_FREE (itd->dtk_values);
	  itd->dtk_values = newvalues;
	  itd->dtk_valsize = newvalsize;
	}
    }
  /// resize the frames and the closures if needed
  unsigned framwant = itd->dtk_fratop + nbfram;
  if (MOM_UNLIKELY
      (framwant > itd->dtk_frasize
       || (itd->dtk_frasize > TASKLET_THRESHOLD
	   && 2 * framwant < itd->dtk_frasize)))
    {
      unsigned newfrasize = ((5 * itd->dtk_fratop / 4 + 3 + nbfram) | 7) + 1;
      if (newfrasize != itd->dtk_frasize)
	{
	  struct momframe_st *newframes =
	    MOM_GC_SCALAR_ALLOC ("reserved frames of tasklet",
				 sizeof (struct momframe_st) * newfrasize);
	  const momnode_t **newclosures =
	    MOM_GC_ALLOC ("reserved closures of tasklet",
			  sizeof (momnode_t *) * newfrasize);
	  if (itd->dtk_fratop > 0)
	    {
	      memcpy (newframes, itd->dtk_frames,
		      itd->dtk_fratop * sizeof (struct momframe_st));
	      memcpy (newclosures, itd->dtk_closures,
		      itd->dtk_fratop * sizeof (momnode_t *));
	    }
	  MOM_GC_FREE (itd->dtk_frames);
	  MOM_GC_FREE (itd->dtk_closures);
	  itd->dtk_frames = newframes;
	  itd->dtk_closures = newclosures;
	  itd->dtk_frasize = newfrasize;
	}
    }
}


static bool
compute_pushed_data_size_mom (const momnode_t *closn,
			      unsigned *pnbval,
			      unsigned *pnbnum,
			      unsigned *pnbdbl,
			      int *pnewstate,
			      enum mom_pushframedirective_en dir,
			      va_list args)
{
  struct momroutinedescr_st *rdescr = NULL;
  unsigned nbval = 0;
  unsigned nbnum = 0;
  unsigned nbdbl = 0;
  int newstate = 0;
  bool again = true;
  if (pnbval)
    *pnbval = 0;
  if (pnbnum)
    *pnbnum = 0;
  if (pnbdbl)
    *pnbdbl = 0;
  if (!closn || closn->typnum != momty_node)
    return false;
  momitem_t *connitm = (momitem_t *) closn->connitm;
  assert (connitm && connitm->i_typnum == momty_item);
  pthread_mutex_lock (&connitm->i_mtx);
  if (connitm->i_paylkind == mompayk_routine)
    rdescr = connitm->i_payload;
  pthread_mutex_unlock (&connitm->i_mtx);
  assert (!rdescr || rdescr->rout_magic == MOM_ROUTINE_MAGIC);
  while (again && dir != MOMPFRDO__END)
    {
      if ((int) dir < MOMPFRDO__END || (int) dir >= MOMPFRDO__LAST)
	MOM_FATAPRINTF ("invalid push frame directive #%d", (int) dir);
      switch ((enum mom_pushframedirective_en) dir)
	{
	case MOMPFRDO__END:
	  again = false;
	  break;
	  //
	case MOMPFRDO_STATE:
	  newstate = va_arg (args, int);
	  break;
	  // 
	case MOMPFRDO_VALUE /*, momval_t val */ :
	  (void) va_arg (args, momval_t);
	  nbval++;
	  break;
	  //
	case MOMPFRDO_TWO_VALUES /*, momval_t val1, val2 */ :
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 2;
	  break;
	  //
	case MOMPFRDO_THREE_VALUES /*, momval_t val1, val2, val3 */ :
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 3;
	  break;
	  //
	case MOMPFRDO_FOUR_VALUES /*, momval_t val1, val2, val3, val4 */ :
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 4;
	  break;
	  //
	case MOMPFRDO_FIVE_VALUES /*, momval_t val1, val2, val3, val4, val5 */ :
	  (void) va_arg (args,
			 momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 5;
	  break;
	  //
	case MOMPFRDO_SIX_VALUES /*, momval_t val1, val2, val3, val4, val5, val6 */ :
	  (void) va_arg (args,
			 momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 6;
	  break;
	  //
	case MOMPFRDO_ARRAY_VALUES /*, unsigned count, momval_t valarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    momval_t *arr = va_arg (args, momval_t *);
	    if (MOM_UNLIKELY (!arr && count > 0))
	      MOM_FATAPRINTF ("invalid array value to push");
	    nbval += count;
	  }
	  break;
	  //
	case MOMPFRDO_NODE_VALUES /*, momval_t node */ :
	  {
	    momval_t nod = va_arg (args, momval_t);
	    if (nod.ptr && *nod.ptype == momty_node)
	      nbval += nod.pnode->slen;
	  }
	  break;
	  //
	case MOMPFRDO_SEQ_ITEMS /*, momval_t seq */ :
	  {
	    momval_t seqv = va_arg (args, momval_t);
	    if (mom_is_seqitem (seqv))
	      nbval += seqv.pseqitems->slen;
	  }
	  break;
	  //
	  //////// integer numbers
	case MOMPFRDO_INT:
	  (void) va_arg (args, intptr_t);
	  nbnum++;
	  break;
	  //
	case MOMPFRDO_TWO_INTS:
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  nbnum += 2;
	  break;
	  //
	case MOMPFRDO_THREE_INTS:
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  nbnum += 3;
	  break;
	  //
	case MOMPFRDO_FOUR_INTS:
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  nbnum += 4;
	  break;
	  //
	case MOMPFRDO_FIVE_INTS:
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  nbnum += 5;
	  break;
	  //
	case MOMPFRDO_SIX_INTS:
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  (void) va_arg (args, intptr_t);
	  nbnum += 6;
	  break;
	  //
	case MOMPFRDO_ARRAY_INTS /*, unsigned count, intptr_t numarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    intptr_t *arr = va_arg (args, intptr_t *);
	    if (MOM_UNLIKELY (!arr && count > 0))
	      MOM_FATAPRINTF ("invalid array of integers to push");
	    nbnum += count;
	  }
	  break;
	  //
	  //////// doubles
	case MOMPFRDO_DOUBLE:
	  (void) va_arg (args, double);
	  nbdbl++;
	  break;
	  //
	case MOMPFRDO_TWO_DOUBLES:
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  nbdbl += 2;
	  break;
	  //
	case MOMPFRDO_THREE_DOUBLES:
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  nbdbl += 3;
	  break;
	  //
	case MOMPFRDO_FOUR_DOUBLES:
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  nbdbl += 4;
	  break;
	  //
	case MOMPFRDO_FIVE_DOUBLES:
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  nbdbl += 5;
	  break;
	  //
	case MOMPFRDO_SIX_DOUBLES:
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  (void) va_arg (args, double);
	  nbdbl += 6;
	  break;
	  //
	case MOMPFRDO_ARRAY_DOUBLES /*, unsigned count, double numarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    double *arr = va_arg (args, double *);
	    if (MOM_UNLIKELY (!arr && count > 0))
	      MOM_FATAPRINTF ("invalid array of double to push");
	    nbdbl += count;
	  }
	  break;
	  //
	case MOMPFRDO__LAST:
	  MOM_FATAPRINTF ("impossible MOMPFRDO__LAST");
	  break;
	}			/* end switch push frame dir */
    }				/* end while again */

  if (rdescr)
    {
      if (nbval < rdescr->rout_frame_nbval)
	nbval = rdescr->rout_frame_nbval;
      if (nbnum < rdescr->rout_frame_nbnum)
	nbnum = rdescr->rout_frame_nbnum;
      if (nbdbl < rdescr->rout_frame_nbdbl)
	nbdbl = rdescr->rout_frame_nbdbl;
    }
  if (nbnum % 2 != 0)
    nbnum++;
  if (nbval % 2 != 0)
    nbval++;
  if (nbdbl % 2 != 0)
    nbdbl++;
  if (pnbnum)
    *pnbnum = nbnum;
  if (pnbval)
    *pnbval = nbval;
  if (pnbdbl)
    *pnbdbl = nbdbl;
  if (newstate != 0 && pnewstate)
    *pnewstate = newstate;
  return true;
}				// end compute_pushed_data_size_mom



static void
fill_frame_data_mom (intptr_t * numdata, double *dbldata, momval_t *valdata,
		     enum mom_pushframedirective_en dir, va_list args)
{
  bool again = true;
  while (again && dir != MOMPFRDO__END)
    {
      if ((int) dir < MOMPFRDO__END || (int) dir >= MOMPFRDO__LAST)
	MOM_FATAPRINTF ("invalid push frame directive #%d", (int) dir);
      switch ((enum mom_pushframedirective_en) dir)
	{
	case MOMPFRDO__END:
	  again = false;
	  break;
	  //
	case MOMPFRDO_STATE:
	  (void) va_arg (args, int);
	  break;
	  // 
	case MOMPFRDO_VALUE /*, momval_t val */ :
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	  //
	case MOMPFRDO_TWO_VALUES /*, momval_t val1, val2 */ :
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	  //
	case MOMPFRDO_THREE_VALUES /*, momval_t val1, val2, val3 */ :
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	  //
	case MOMPFRDO_FOUR_VALUES /*, momval_t val1, val2, val3, val4 */ :
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	  //
	case MOMPFRDO_FIVE_VALUES /*, momval_t val1, val2, val3, val4, val5 */ :
	  *(valdata++) = va_arg (args,
				 momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	  //
	case MOMPFRDO_SIX_VALUES /*, momval_t val1, val2, val3, val4, val5, val6 */ :
	  *(valdata++) =
	    va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	  //
	case MOMPFRDO_ARRAY_VALUES /*, unsigned count, momval_t valarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    momval_t *arr = va_arg (args, momval_t *);
	    if (MOM_UNLIKELY (!arr && count > 0))
	      MOM_FATAPRINTF ("invalid array value to push");
	    memcpy (valdata, arr, count * sizeof (momval_t));
	    valdata += count;
	  }
	  break;
	  ///////
	  //
	case MOMPFRDO_NODE_VALUES /*, momval_t node */ :
	  {
	    momval_t nod = va_arg (args, momval_t);
	    if (nod.ptr && *nod.ptype == momty_node)
	      {
		memcpy (valdata, nod.pnode->sontab,
			nod.pnode->slen * sizeof (momval_t));
		valdata += nod.pnode->slen;
	      }
	  }
	  break;
	  //
	case MOMPFRDO_SEQ_ITEMS /*, momval_t seq */ :
	  {
	    momval_t seqv = va_arg (args, momval_t);
	    if (mom_is_seqitem (seqv))
	      {
		assert (sizeof (momval_t) == sizeof (momitem_t *));
		memcpy (valdata, seqv.pseqitems->itemseq,
			seqv.pseqitems->slen * sizeof (momval_t));
		valdata += seqv.pseqitems->slen;
	      }
	  }
	  break;
	  //
	  //////// integer numbers
	case MOMPFRDO_INT:
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	  //
	case MOMPFRDO_TWO_INTS:
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	  //
	case MOMPFRDO_THREE_INTS:
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	  //
	case MOMPFRDO_FOUR_INTS:
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	  //
	case MOMPFRDO_FIVE_INTS:
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	  //
	case MOMPFRDO_SIX_INTS:
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	  //
	case MOMPFRDO_ARRAY_INTS /*, unsigned count, intptr_t numarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    intptr_t *arr = va_arg (args, intptr_t *);
	    if (arr)
	      memcpy (numdata, arr, count * sizeof (intptr_t));
	    numdata += count;
	  }
	  break;
	  //
	  //////// doubles
	case MOMPFRDO_DOUBLE:
	  *(dbldata++) = va_arg (args, double);
	  break;
	  //
	case MOMPFRDO_TWO_DOUBLES:
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	  //
	case MOMPFRDO_THREE_DOUBLES:
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	  //
	case MOMPFRDO_FOUR_DOUBLES:
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	  //
	case MOMPFRDO_FIVE_DOUBLES:
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	  //
	case MOMPFRDO_SIX_DOUBLES:
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	  //
	case MOMPFRDO_ARRAY_DOUBLES /*, unsigned count, double numarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    double *arr = va_arg (args, double *);
	    if (arr && count > 0)
	      memcpy (dbldata, arr, count * sizeof (double));
	    dbldata += count;
	  }
	  break;
	  //
	case MOMPFRDO__LAST:
	  MOM_FATAPRINTF ("impossible MOMPFRDO__LAST");
	  break;
	}			/* end switch push frame dir */
    }				/* end while again */
}				//// end fill_frame_data_mom


////////////////

void
mom_item_tasklet_push_frame (momitem_t *itm, momval_t clo,
			     enum mom_pushframedirective_en firstdir, ...)
{
  unsigned nbval = 0, nbnum = 0, nbdbl = 0;
  int state = 0;
  va_list alist;
  assert (itm && itm->i_typnum == momty_item);
  if (!clo.ptr || *clo.ptype != momty_node)
    return;
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return;
  struct mom_taskletdata_st *itd = itm->i_payload;
  va_start (alist, firstdir);
  if (!compute_pushed_data_size_mom (clo.pnode,
				     &nbval, &nbnum, &nbdbl,
				     &state, firstdir, alist))
    return;
  va_end (alist);
  /// grow the scalars if needed
  if (MOM_UNLIKELY (itd->dtk_scaltop +
		    (sizeof (intptr_t) * nbnum +
		     sizeof (double) * nbdbl) / sizeof (intptr_t)
		    >= itd->dtk_scalsize))
    {
      unsigned newscalsize =
	((5 * itd->dtk_scaltop / 4 +
	  (sizeof (intptr_t *) * nbnum +
	   sizeof (double) * nbdbl) / sizeof (intptr_t) + 5) | 7) + 1;
      intptr_t *newscalars = MOM_GC_SCALAR_ALLOC ("tasklet grown scalar zone",
						  newscalsize *
						  sizeof (intptr_t));
      memcpy (newscalars, itd->dtk_scalars,
	      itd->dtk_scaltop * sizeof (intptr_t));
      MOM_GC_FREE (itd->dtk_scalars);
      itd->dtk_scalars = newscalars;
      itd->dtk_scalsize = newscalsize;
    };
  /// grow the values if needed
  if (MOM_UNLIKELY (itd->dtk_valtop + nbval >= itd->dtk_valsize))
    {
      unsigned newvalsize = ((5 * itd->dtk_valtop / 4 + nbval + 6) | 7) + 1;
      momval_t *newvalues =
	MOM_GC_ALLOC ("tasklet grown values", newvalsize * sizeof (momval_t));
      memcpy (newvalues, itd->dtk_values,
	      itd->dtk_valtop * sizeof (momval_t));
      MOM_GC_FREE (itd->dtk_values);
      itd->dtk_values = newvalues;
      itd->dtk_valsize = newvalsize;
    }
  /// grow the frames & closures if needed
  if (MOM_UNLIKELY (itd->dtk_fratop + 1 >= itd->dtk_frasize))
    {
      unsigned newfrasize = ((5 * itd->dtk_fratop / 4 + 6) | 7) + 1;
      struct momframe_st *newframes =
	MOM_GC_SCALAR_ALLOC ("tasklet grown frames",
			     sizeof (struct momframe_st) * newfrasize);
      const momnode_t **newclosures = MOM_GC_ALLOC ("tasklet grown closures",
						    sizeof (momnode_t *) *
						    newfrasize);
      memcpy (newframes, itd->dtk_frames,
	      sizeof (struct momframe_st) * itd->dtk_fratop);
      memcpy (newclosures, itd->dtk_closures,
	      sizeof (momnode_t *) * itd->dtk_fratop);
      MOM_GC_FREE (itd->dtk_frames);
      MOM_GC_FREE (itd->dtk_closures);
      itd->dtk_frames = newframes;
      itd->dtk_closures = newclosures;
      itd->dtk_frasize = newfrasize;
    }
  struct momframe_st *newframe = itd->dtk_frames + itd->dtk_fratop;
  newframe->fr_state = state;
  unsigned froint = newframe->fr_intoff = itd->dtk_scaltop;
  unsigned frodbl = newframe->fr_dbloff = itd->dtk_scaltop + nbnum;
  unsigned froval = newframe->fr_valoff = itd->dtk_valtop;
  unsigned fratop = itd->dtk_fratop;
  memset (itd->dtk_scalars + itd->dtk_scaltop, 0,
	  (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)));
  memset (itd->dtk_values + itd->dtk_valtop, 0, nbval * sizeof (momval_t));
  itd->dtk_scaltop +=
    (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)) / sizeof (intptr_t);
  itd->dtk_valtop += nbval;
  itd->dtk_closures[fratop] = clo.pnode;
  itd->dtk_fratop = fratop + 1;
  va_start (alist, firstdir);
  fill_frame_data_mom ((intptr_t *) (itd->dtk_scalars + froint),
		       (double *) (itd->dtk_scalars + frodbl),
		       (momval_t *) (itd->dtk_values + froval), firstdir,
		       alist);
  va_end (alist);
}


void
mom_item_tasklet_replace_top_frame (momitem_t *itm, momval_t clo,
				    enum mom_pushframedirective_en firstdir,
				    ...)
{
  unsigned nbval = 0, nbnum = 0, nbdbl = 0;
  int state = 0;
  va_list alist;
  assert (itm && itm->i_typnum == momty_item);
  if (!clo.ptr || *clo.ptype != momty_node)
    return;
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return;
  struct mom_taskletdata_st *itd = itm->i_payload;
  if (itd->dtk_fratop == 0)
    return;
  va_start (alist, firstdir);
  if (!compute_pushed_data_size_mom (clo.pnode,
				     &nbval, &nbnum, &nbdbl,
				     &state, firstdir, alist))
    return;
  va_end (alist);
  // pop the old top frame
  struct momframe_st *prevframe = itd->dtk_frames + itd->dtk_fratop - 1;
  unsigned ofpscal = prevframe->fr_intoff;
  unsigned ofpvalu = prevframe->fr_valoff;
  if (itd->dtk_scaltop > ofpscal)
    memset (itd->dtk_scalars + ofpscal, 0,
	    (itd->dtk_scaltop - ofpscal) * sizeof (intptr_t));
  if (itd->dtk_valtop > ofpvalu)
    memset (itd->dtk_values + ofpvalu, 0,
	    (itd->dtk_valtop - ofpvalu) * sizeof (momval_t));
  itd->dtk_scaltop = ofpscal;
  itd->dtk_valtop = ofpvalu;
  memset (prevframe, 0, sizeof (struct momframe_st));
  itd->dtk_closures[itd->dtk_fratop - 1] = NULL;
  itd->dtk_fratop--;
  unsigned fratop = itd->dtk_fratop;
  /// grow the scalars if needed
  if (MOM_UNLIKELY (itd->dtk_scaltop +
		    (sizeof (intptr_t) * nbnum +
		     sizeof (double) * nbdbl) / sizeof (intptr_t)
		    >= itd->dtk_scalsize))
    {
      unsigned newscalsize =
	((5 * itd->dtk_scaltop / 4 +
	  (sizeof (intptr_t *) * nbnum +
	   sizeof (double) * nbdbl) / sizeof (intptr_t) + 5) | 7) + 1;
      intptr_t *newscalars = MOM_GC_SCALAR_ALLOC ("tasklet grown scalar zone",
						  newscalsize *
						  sizeof (intptr_t));
      memcpy (newscalars, itd->dtk_scalars,
	      itd->dtk_scaltop * sizeof (intptr_t));
      MOM_GC_FREE (itd->dtk_scalars);
      itd->dtk_scalars = newscalars;
      itd->dtk_scalsize = newscalsize;
    };
  /// grow the values if needed
  if (MOM_UNLIKELY (itd->dtk_valtop + nbval >= itd->dtk_valsize))
    {
      unsigned newvalsize = ((5 * itd->dtk_valtop / 4 + nbval + 6) | 7) + 1;
      momval_t *newvalues =
	MOM_GC_ALLOC ("tasklet grown values", newvalsize * sizeof (momval_t));
      memcpy (newvalues, itd->dtk_values,
	      itd->dtk_valtop * sizeof (momval_t));
      MOM_GC_FREE (itd->dtk_values);
      itd->dtk_values = newvalues;
      itd->dtk_valsize = newvalsize;
    }
  /// dont need to grow frames or closures, they are sure to fit
  struct momframe_st *newframe = itd->dtk_frames + itd->dtk_fratop;
  unsigned froint = newframe->fr_intoff = itd->dtk_scaltop;
  unsigned frodbl = newframe->fr_dbloff = itd->dtk_scaltop + nbnum;
  unsigned froval = newframe->fr_valoff = itd->dtk_valtop;
  fratop = itd->dtk_fratop;
  memset (itd->dtk_scalars + itd->dtk_scaltop, 0,
	  (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)));
  memset (itd->dtk_values + itd->dtk_valtop, 0, nbval * sizeof (momval_t));
  itd->dtk_scaltop +=
    (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)) / sizeof (intptr_t);
  itd->dtk_valtop += nbval;
  itd->dtk_closures[fratop] = clo.pnode;
  itd->dtk_fratop = fratop + 1;
  va_start (alist, firstdir);
  fill_frame_data_mom ((intptr_t *) (itd->dtk_scalars + froint),
		       (double *) (itd->dtk_scalars + frodbl),
		       (momval_t *) (itd->dtk_values + froval), firstdir,
		       alist);
  va_end (alist);
}



void
mom_item_tasklet_pop_frame (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (fratop == 0)
    return;
  struct momframe_st *prevframe = itd->dtk_frames + fratop - 1;
  unsigned ofpscal = prevframe->fr_intoff;
  unsigned ofpvalu = prevframe->fr_valoff;
  if (itd->dtk_scaltop > ofpscal)
    memset (itd->dtk_scalars + ofpscal, 0,
	    (itd->dtk_scaltop - ofpscal) * sizeof (intptr_t));
  if (itd->dtk_valtop > ofpvalu)
    memset (itd->dtk_values + ofpvalu, 0,
	    (itd->dtk_valtop - ofpvalu) * sizeof (momval_t));
  itd->dtk_scaltop = ofpscal;
  itd->dtk_valtop = ofpvalu;
  memset (prevframe, 0, sizeof (struct momframe_st));
  itd->dtk_closures[fratop - 1] = NULL;
  itd->dtk_fratop = fratop - 1;
  /// shrink perhaps the scalars
  if (MOM_UNLIKELY (itd->dtk_scalsize > TASKLET_THRESHOLD
		    && 2 * ofpscal < itd->dtk_scalsize))
    {
      unsigned newscalsize = ((5 * ofpscal / 4 + 3) | 7) + 1;
      intptr_t *newscalars =
	MOM_GC_SCALAR_ALLOC ("tasklet shrink scalar zone",
			     newscalsize * sizeof (intptr_t));
      memcpy (newscalars, itd->dtk_scalars, ofpscal * sizeof (intptr_t));
      MOM_GC_FREE (itd->dtk_scalars);
      itd->dtk_scalars = newscalars;
      itd->dtk_scalsize = newscalsize;
    }
  /// shrink perhaps the values
  if (MOM_UNLIKELY (itd->dtk_valsize > TASKLET_THRESHOLD
		    && 2 * ofpvalu < itd->dtk_valsize))
    {
      unsigned newvalsize = ((5 * ofpvalu / 4 + 3) | 7) + 1;
      momval_t *newvalues = MOM_GC_ALLOC ("tasklet shrink values",
					  newvalsize * sizeof (momval_t));
      memcpy (newvalues, itd->dtk_values, ofpvalu * sizeof (momval_t));
      MOM_GC_FREE (itd->dtk_values);
      itd->dtk_values = newvalues;
      itd->dtk_valsize = newvalsize;
    };
  /// shrink perhaps the closures and frames
  if (MOM_UNLIKELY (itd->dtk_frasize > TASKLET_THRESHOLD
		    && 2 * fratop < itd->dtk_frasize))
    {
      unsigned newfrasize = ((5 * fratop / 4 + 6) | 7) + 1;
      struct momframe_st *newframes =
	MOM_GC_SCALAR_ALLOC ("tasklet shrink frames",
			     sizeof (struct momframe_st) * newfrasize);
      const momnode_t **newclosures = MOM_GC_ALLOC ("tasklet shrink closures",
						    sizeof (momnode_t *) *
						    newfrasize);
      memcpy (newframes, itd->dtk_frames,
	      sizeof (struct momframe_st) * fratop);
      memcpy (newclosures, itd->dtk_closures, sizeof (momnode_t *) * fratop);
      MOM_GC_FREE (itd->dtk_frames);
      MOM_GC_FREE (itd->dtk_closures);
      itd->dtk_frames = newframes;
      itd->dtk_closures = newclosures;
      itd->dtk_frasize = newfrasize;
    }
}




unsigned
mom_item_tasklet_depth (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  return fratop;
}


int
mom_item_tasklet_frame_state (momitem_t *itm, int frk)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk >= 0 && frk < (int) fratop)
    return itd->dtk_frames[frk].fr_state;
  return 0;
}

momval_t
mom_item_tasklet_frame_closure (momitem_t *itm, int frk)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return MOM_NULLV;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk >= 0 && frk < (int) fratop)
    return (momval_t) itd->dtk_closures[frk];
  return MOM_NULLV;
}


unsigned
mom_item_tasklet_frame_nb_values (momitem_t *itm, int frk)
{
  unsigned nbval = 0;
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return 0;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  if (frk + 1 < (int) fratop)
    {
      struct momframe_st *nxtfram = itd->dtk_frames + frk + 1;
      nbval = nxtfram->fr_valoff - curfram->fr_valoff;
    }
  else
    nbval = itd->dtk_valtop - curfram->fr_valoff;
  return nbval;
}


momval_t
mom_item_tasklet_frame_nth_value (momitem_t *itm, int frk, int vrk)
{
  unsigned nbval = 0;
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return MOM_NULLV;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return MOM_NULLV;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  if (frk + 1 < (int) fratop)
    {
      struct momframe_st *nxtfram = itd->dtk_frames + frk + 1;
      nbval = nxtfram->fr_valoff - curfram->fr_valoff;
    }
  else
    nbval = itd->dtk_valtop - curfram->fr_valoff;
  if (vrk < 0)
    vrk += nbval;
  if (vrk >= 0 && vrk < (int) nbval)
    return itd->dtk_values[curfram->fr_valoff + vrk];
  return MOM_NULLV;
}

unsigned
mom_item_tasklet_frame_nb_numbers (momitem_t *itm, int frk)
{
  unsigned nbnum = 0;
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return 0;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  nbnum = curfram->fr_dbloff - curfram->fr_intoff;
  return nbnum;
}

intptr_t
mom_item_tasklet_frame_nth_number (momitem_t *itm, int frk, int nrk)
{
  unsigned nbnum = 0;
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return 0;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  nbnum = curfram->fr_dbloff - curfram->fr_intoff;
  if (nrk < 0)
    nrk += nbnum;
  if (nrk >= 0 && nrk < (int) nbnum)
    return itd->dtk_scalars[curfram->fr_intoff + nrk];
  return 0;
}


unsigned
mom_item_tasklet_frame_nb_doubles (momitem_t *itm, int frk)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return 0;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  if (frk + 1 < (int) fratop)
    {
      struct momframe_st *nxtfram = itd->dtk_frames + frk + 1;
      return ((nxtfram->fr_intoff -
	       curfram->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
    }
  else
    return ((itd->dtk_scaltop -
	     curfram->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
}

double
mom_item_tasklet_frame_nth_double (momitem_t *itm, int frk, int drk)
{
  unsigned nbdbl = 0;
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return 0.0;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return 0.0;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  if (frk + 1 < (int) fratop)
    {
      struct momframe_st *nxtfram = itd->dtk_frames + frk + 1;
      nbdbl = ((nxtfram->fr_intoff -
		curfram->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
    }
  else
    {
      nbdbl = ((itd->dtk_scaltop -
		curfram->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
    }
  if (drk < 0)
    drk += nbdbl;
  if (drk >= 0 && drk < (int) nbdbl)
    return ((double *) (itd->dtk_scalars + curfram->fr_dbloff))[drk];
  return 0.0;
}


static void
payl_tasklet_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_tasklet && itm->i_payload);
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned valtop = itd->dtk_valtop;
  for (unsigned vix = 0; vix < valtop; vix++)
    mom_dump_scan_value (du, itd->dtk_values[vix]);
  unsigned fratop = itd->dtk_fratop;
  for (unsigned frix = 0; frix < fratop; frix++)
    mom_dump_scan_value (du, (momval_t) itd->dtk_closures[frix]);
}

static momval_t
payl_tasklet_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jres = MOM_NULLV;
  assert (du != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_tasklet && itm->i_payload);
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *jarr =
    (fratop < MOM_TINY_MAX) ? tinyarr : MOM_GC_ALLOC ("jarray frames",
						      fratop *
						      sizeof (momval_t));
  for (unsigned frix = 0; frix < fratop; frix++)
    {
      struct momframe_st *curfram = itd->dtk_frames + frix;
      const momnode_t *curclo = itd->dtk_closures[frix];
      unsigned nbnum = curfram->fr_dbloff - curfram->fr_intoff;
      unsigned nbdbl = 0, nbval = 0;
      if (frix + 1 >= fratop)
	{
	  nbdbl =
	    ((itd->dtk_scaltop -
	      curfram->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
	  nbval = (itd->dtk_valtop - curfram->fr_valoff);
	}
      else
	{
	  struct momframe_st *nxtfram = itd->dtk_frames + frix;
	  nbdbl =
	    ((nxtfram->fr_intoff -
	      curfram->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
	  nbval = nxtfram->fr_valoff - curfram->fr_valoff;
	}
      momval_t valtiny[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t numtiny[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t dbltiny[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t *jvalarr = (nbval < MOM_TINY_MAX) ? valtiny
	: MOM_GC_ALLOC ("tasklet dump values frame",
			nbval * sizeof (momval_t));
      momval_t *jnumarr =
	(nbnum <
	 MOM_TINY_MAX) ? numtiny : MOM_GC_ALLOC ("tasklet dump numbers frame",
						 nbval * sizeof (momval_t));
      momval_t *jdblarr =
	(nbdbl <
	 MOM_TINY_MAX) ? dbltiny : MOM_GC_ALLOC ("tasklet dump doubles frame",
						 nbdbl * sizeof (momval_t));
      for (unsigned vix = 0; vix < nbval; vix++)
	jvalarr[vix]
	  = mom_dump_emit_json (du,
				itd->dtk_values[curfram->fr_valoff + vix]);
      for (unsigned nix = 0; nix < nbnum; nix++)
	jnumarr[nix]
	  = mom_make_integer (itd->dtk_scalars[curfram->fr_intoff + nix]);
      for (unsigned dix = 0; dix < nbdbl; dix++)
	jdblarr[dix]
	  = mom_make_double ((itd->dtk_scalars + curfram->fr_dbloff)[dix]);
      momval_t jnums
	= (nbnum > 0)
	? (momval_t) mom_make_json_array_count (nbnum, jnumarr) : MOM_NULLV;
      momval_t jdbls
	= (nbdbl > 0)
	? (momval_t) mom_make_json_array_count (nbdbl, jdblarr) : MOM_NULLV;
      momval_t jvals
	= (nbval > 0)
	? (momval_t) mom_make_json_array_count (nbval, jvalarr) : MOM_NULLV;
      momval_t jclos = mom_dump_emit_json (du, (momval_t) curclo);
      momval_t jstate = (momval_t) mom_make_integer (curfram->fr_state);
      momval_t jframe
	= (momval_t)
	mom_make_json_object
	(MOMJSOB_ENTRY ((momval_t) mom_named__closure, jclos),
	 MOMJSOB_ENTRY ((momval_t) mom_named__numbers, jnums),
	 MOMJSOB_ENTRY ((momval_t) mom_named__doubles, jdbls),
	 MOMJSOB_ENTRY ((momval_t) mom_named__values, jvals),
	 MOMJSOB_ENTRY ((momval_t) mom_named__state, jstate),
	 MOMJSON_END);
      jarr[frix] = jframe;
      if (jvalarr != valtiny)
	MOM_GC_FREE (jvalarr);
      if (jnumarr != numtiny)
	MOM_GC_FREE (jnumarr);
      if (jdblarr != dbltiny)
	MOM_GC_FREE (jdblarr);
    }
  jres =
    fratop ? (momval_t) mom_make_json_array_count (fratop, jarr) : MOM_NULLV;
  if (jarr != tinyarr)
    MOM_GC_FREE (jarr);
  return jres;
}

static void
payl_tasklet_load_mom (struct mom_loader_st *ld, momitem_t *litm,
		       momval_t jsarr)
{
  assert (ld != NULL);
  assert (litm != NULL && litm->i_typnum == momty_item);
  mom_item_start_tasklet (litm);
  unsigned nbfra = mom_json_array_size (jsarr);
  if (nbfra > 0)
    // a guess about the tasklet needs
    mom_item_tasklet_reserve (litm,
			      /*nbint: */ nbfra * 2,
			      /*nbdbl: */ nbfra / 2,
			      /*nbval: */ nbfra * 3,
			      /*nbfram: */ nbfra + 1);
  for (unsigned frix = 0; frix < nbfra; frix++)
    {
      momval_t jfram = mom_json_array_nth (jsarr, frix);
      momval_t jstate = mom_jsonob_get (jfram, (momval_t) mom_named__state);
      momval_t jclos = mom_jsonob_get (jfram, (momval_t) mom_named__closure);
      momval_t jnums = mom_jsonob_get (jfram, (momval_t) mom_named__numbers);
      momval_t jdbls = mom_jsonob_get (jfram, (momval_t) mom_named__doubles);
      momval_t jvals = mom_jsonob_get (jfram, (momval_t) mom_named__values);
      int curstate = mom_integer_val (jstate);
      momval_t curclos = mom_load_value_json (ld, jclos);
      unsigned nbnums = mom_json_array_size (jnums);
      unsigned nbdbls = mom_json_array_size (jdbls);
      unsigned nbvals = mom_json_array_size (jvals);
      double tinydbl[MOM_TINY_MAX] = { 0.0 };
      double *dblarr = (nbdbls < MOM_TINY_MAX) ? tinydbl
	: MOM_GC_SCALAR_ALLOC ("doubles in frame of tasklet",
			       nbdbls * sizeof (double));
      intptr_t tinynum[MOM_TINY_MAX] = { 0 };
      intptr_t *numarr = (nbnums < MOM_TINY_MAX) ? tinynum
	: MOM_GC_SCALAR_ALLOC ("numbers in frame of tasklet",
			       nbnums * sizeof (intptr_t));
      momval_t tinyval[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t *valarr = (nbvals < MOM_TINY_MAX) ? tinyval
	: MOM_GC_ALLOC ("values in frame of tasklet",
			nbvals * sizeof (momval_t));
      for (unsigned nix = 0; nix < nbnums; nix++)
	numarr[nix] = mom_integer_val (mom_json_array_nth (jnums, nix));
      for (unsigned dix = 0; dix < nbdbls; dix++)
	dblarr[dix] = mom_double_val (mom_json_array_nth (jdbls, dix));
      for (unsigned vix = 0; vix < nbvals; vix++)
	valarr[vix] =
	  mom_load_value_json (ld, mom_json_array_nth (jvals, vix));
      mom_item_tasklet_push_frame (litm, curclos, MOMPFR_STATE (curstate),
				   MOMPFR_ARRAY_VALUES (nbvals, valarr),
				   MOMPFR_ARRAY_INTS (nbnums,
						      (intptr_t *) numarr),
				   MOMPFR_ARRAY_DOUBLES (nbdbls, dblarr),
				   MOMPFR_END ());
    }
}

static const struct mom_payload_descr_st payldescr_tasklet_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "tasklet",
  .dpayl_loadfun = payl_tasklet_load_mom,
  .dpayl_dumpscanfun = payl_tasklet_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_tasklet_dump_json_mom,
};



////////////////////////////////////////////////////////////////
///// BUFFER PAYLOAD
////////////////////////////////////////////////////////////////

#define BUFFER_MAGIC 0x2cdb8c85	/* buffer magic 752585861 */
#define BUFFER_MAXSIZE (32<<20)	/* 33554432, buffer maxsize */
#define BUFFER_THRESHOLD 2048
struct bufferdata_mom_st
{
  char *dbu_zone;		/* atomic, GC-allocated */
  struct momout_st dbu_out;	/* output contains a cookie file */
  unsigned dbu_magic;		/* always BUFFER_MAGIC */
  unsigned dbu_size;		/* size of dbu_zone */
  unsigned dbu_begin;		/* offset of beginning */
  unsigned dbu_end;		/* offset of end */
};

static bool
dbu_reserve_mom (struct bufferdata_mom_st *dbuf, unsigned more)
{
  assert (dbuf && dbuf->dbu_magic == BUFFER_MAGIC);
  assert (dbuf->dbu_end >= dbuf->dbu_begin && dbuf->dbu_end < dbuf->dbu_size);
  unsigned blen = dbuf->dbu_end - dbuf->dbu_begin;
  if (blen + more > BUFFER_MAXSIZE)
    return false;
  if (dbuf->dbu_size > BUFFER_THRESHOLD && blen + more < 2 * dbuf->dbu_size)
    {				// can shrink the buffer
      unsigned newsiz = ((5 * blen / 4 + more + 10) | 0xf) + 1;
      char *newzone = MOM_GC_SCALAR_ALLOC ("shrink buffer zone", newsiz);
      memcpy (newzone, dbuf->dbu_zone + dbuf->dbu_begin, blen);
      MOM_GC_FREE (dbuf->dbu_zone);
      dbuf->dbu_zone = newzone;
      dbuf->dbu_begin = 0;
      dbuf->dbu_end = blen;
      return true;
    }
  if (dbuf->dbu_begin > 0 && blen + more + 1 < dbuf->dbu_size)
    {
      memmove (dbuf->dbu_zone, dbuf->dbu_zone + dbuf->dbu_begin, blen);
      dbuf->dbu_zone[blen] = 0;
      dbuf->dbu_begin = 0;
      dbuf->dbu_end = blen;
      return true;
    }
  // need to grow the buffer
  unsigned newsiz = ((5 * blen / 4 + more + 10) | 0xf) + 1;
  char *newzone = MOM_GC_SCALAR_ALLOC ("grown buffer zone", newsiz);
  memcpy (newzone, dbuf->dbu_zone + dbuf->dbu_begin, blen);
  MOM_GC_FREE (dbuf->dbu_zone);
  dbuf->dbu_zone = newzone;
  dbuf->dbu_begin = 0;
  dbuf->dbu_end = blen;
  return true;
}

static bool
dbu_write_mom (struct bufferdata_mom_st *dbu, const char *buf, size_t siz)
{
  assert (dbu && dbu->dbu_magic == BUFFER_MAGIC);
  assert (dbu->dbu_end >= dbu->dbu_begin && dbu->dbu_end < dbu->dbu_size);
  if (siz > BUFFER_MAXSIZE)
    return false;
  if (dbu->dbu_end - dbu->dbu_begin + siz > BUFFER_MAXSIZE)
    return false;
  if (dbu->dbu_end + siz + 1 > dbu->dbu_size)
    if (!dbu_reserve_mom (dbu, siz + 2))
      return false;
  memcpy (dbu->dbu_zone + dbu->dbu_end, buf, siz);
  dbu->dbu_zone[dbu->dbu_end + siz] = 0;
  dbu->dbu_end += siz;
  assert (dbu->dbu_end >= dbu->dbu_begin && dbu->dbu_end < dbu->dbu_size);
  return true;
}



static ssize_t
dbu_cookie_write_mom (void *cookie, const char *buffer, size_t size)
{
  struct bufferdata_mom_st *dbu = cookie;
  assert (dbu && dbu->dbu_magic == BUFFER_MAGIC);
  if (size > BUFFER_MAXSIZE)
    return -1;
  if (!dbu_write_mom (dbu, buffer, size))
    return -1;
  return size;
}

static const cookie_io_functions_t dbu_cookiefun_mom = {
  .write = dbu_cookie_write_mom
};

void
mom_item_start_buffer (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  struct bufferdata_mom_st *dbuf =
    MOM_GC_ALLOC ("item buffer", sizeof (struct bufferdata_mom_st));
  unsigned inisiz = 256;
  dbuf->dbu_zone = MOM_GC_SCALAR_ALLOC ("buffer zone", inisiz);
  dbuf->dbu_magic = BUFFER_MAGIC;
  dbuf->dbu_size = inisiz;
  dbuf->dbu_begin = 0;
  dbuf->dbu_end = 0;
  FILE *fcookie = fopencookie (dbuf, "w", dbu_cookiefun_mom);
  if (!mom_initialize_output (&dbuf->dbu_out, fcookie, 0))
    return;
  itm->i_payload = dbuf;
  itm->i_paylkind = mompayk_buffer;
}

void
mom_item_buffer_reserve (momitem_t *itm, unsigned more)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_buffer)
    return;
  struct bufferdata_mom_st *dbuf = itm->i_payload;
  assert (dbuf && dbuf->dbu_magic == BUFFER_MAGIC);
  dbu_reserve_mom (dbuf, more + 1);
}

void
mom_item_buffer_out (momitem_t *itm, ...)
{
  va_list alist;
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_buffer)
    return;
  struct bufferdata_mom_st *dbuf = itm->i_payload;
  assert (dbuf && dbuf->dbu_magic == BUFFER_MAGIC);
  va_start (alist, itm);
  MOM_OUTVA (&dbuf->dbu_out, alist);
  va_end (alist);
}

#define BUFFER_CHUNK_MIN 24
static void
payl_buffer_load_mom (struct mom_loader_st *ld, momitem_t *litm,
		      momval_t jsonv)
{
  assert (litm && litm->i_typnum == momty_item);
  assert (ld != NULL);
  unsigned nbchk = mom_json_array_size (jsonv);
  mom_item_start_buffer (litm);
  mom_item_buffer_reserve (litm, (5 * nbchk * BUFFER_CHUNK_MIN) + 10);
  for (unsigned cix = 0; cix < nbchk; cix++)
    mom_item_buffer_out (litm,
			 MOMOUT_LITERALV (mom_string_cstr
					  (mom_json_array_nth (jsonv, cix))),
			 NULL);

}

static momval_t
payl_buffer_dump_json_mom (struct mom_dumper_st *du, momitem_t *ditm)
{
  momval_t jres = MOM_NULLV;
  struct mom_valuequeue_st que = { NULL, NULL };
  assert (du != NULL);
  assert (ditm && ditm->i_typnum == momty_item);
  if (ditm->i_paylkind != mompayk_buffer)
    return MOM_NULLV;
  struct bufferdata_mom_st *dbuf = ditm->i_payload;
  assert (dbuf && dbuf->dbu_magic == BUFFER_MAGIC);
  assert (dbuf->dbu_end >= dbuf->dbu_begin && dbuf->dbu_end < dbuf->dbu_size);
  const char *cbeg = dbuf->dbu_zone + dbuf->dbu_begin;
  const char *cend = dbuf->dbu_zone + dbuf->dbu_end;
  assert (g_utf8_validate (cbeg, cend - cbeg, NULL));
  const char *chk = NULL;
  const char *next = NULL;
  unsigned nbchk = 0;
  for (chk = cbeg; chk != NULL && chk < cend && *chk; chk = next)
    {
      int chklen = 0;
      for (next = chk; chklen < BUFFER_CHUNK_MIN && next < cend && *next;
	   next = g_utf8_next_char (next))
	chklen++;
      for (; chklen < 2 * BUFFER_CHUNK_MIN && next < cend && *next;
	   next = g_utf8_next_char (next))
	{
	  chklen++;
	  gunichar uc = g_utf8_get_char (next);
	  if (g_unichar_isspace (uc) || g_unichar_ispunct (uc))
	    break;
	}
      momval_t chunkv = (momval_t) mom_make_string_len (chk, next - chk);
      mom_queue_add_value_back (&que, chunkv);
      nbchk++;
    }
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrchk =
    (nbchk < MOM_TINY_MAX) ? tinyarr : MOM_GC_ALLOC ("chunks for buffer",
						     nbchk *
						     sizeof (momval_t));
  unsigned cntchk = 0;
  for (struct mom_vaqelem_st * qel = que.vaq_first;
       qel != NULL && cntchk < nbchk; qel = qel->vqe_next)
    arrchk[cntchk++] = qel->vqe_val;
  jres = (momval_t) mom_make_json_array_count (nbchk, arrchk);
  if (arrchk != tinyarr)
    MOM_GC_FREE (arrchk);
  return jres;
}

static void
payl_buffer_finalize_mom (momitem_t *ditm, void *payloadata)
{
  assert (ditm != NULL);
  struct bufferdata_mom_st *dbuf = payloadata;
  assert (dbuf != NULL && dbuf->dbu_magic == BUFFER_MAGIC);
  if (dbuf->dbu_out.mout_file)
    fclose (dbuf->dbu_out.mout_file), dbuf->dbu_out.mout_file = NULL;
  memset (dbuf, 0, sizeof (struct bufferdata_mom_st));
}

static const struct mom_payload_descr_st payldescr_buffer_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "buffer",
  .dpayl_loadfun = payl_buffer_load_mom,
  .dpayl_dumpjsonfun = payl_buffer_dump_json_mom,
  .dpayl_finalizefun = payl_buffer_finalize_mom
};


////////////////////////////////////////////////////////////////
/// process payload. process items are transient, but their finalizer
/// should kill the Unix process


static void
payl_process_finalize_mom (momitem_t *ditm, void *payloadata)
{
  assert (ditm != NULL);
  struct mom_process_data_st *procdata = payloadata;
  assert (procdata && procdata->iproc_magic == MOM_PROCESS_MAGIC);
  MOM_DEBUG (run, MOMOUT_LITERAL ("finalizing process item:"),
	     MOMOUT_ITEM ((const momitem_t *) ditm),
	     MOMOUT_LITERAL (" program:"),
	     MOMOUT_VALUE ((momval_t) procdata->iproc_progname),
	     MOMOUT_LITERAL (" jobnum#"),
	     MOMOUT_DEC_INT ((int) procdata->iproc_jobnum),
	     MOMOUT_LITERAL (" pid#"),
	     MOMOUT_DEC_INT ((int) procdata->iproc_pid),
	     MOMOUT_LITERAL (" outfd#"),
	     MOMOUT_DEC_INT (procdata->iproc_outfd));
  // we kill with SIGTERM the process, and we close its output buffer.
  // I'm not sure what would happen if the process ignores SIGTERM and
  // SIGPIPE; it might perhaps becomes a zombie process..., but
  // ignoring SIGTERM is bad behavior...
  if (procdata->iproc_pid > 0)
    {
      MOM_WARNING (MOMOUT_LITERAL ("finalized process item:"),
		   MOMOUT_ITEM ((const momitem_t *) ditm),
		   MOMOUT_LITERAL (" program:"),
		   MOMOUT_VALUE ((momval_t) procdata->iproc_progname),
		   MOMOUT_LITERAL (" jobnum#"),
		   MOMOUT_DEC_INT ((int) procdata->iproc_jobnum),
		   MOMOUT_LITERAL (" pid#"),
		   MOMOUT_DEC_INT ((int) procdata->iproc_pid),
		   MOMOUT_LITERAL (" is getting a SIGTERM"));
      kill (procdata->iproc_pid, SIGTERM);
    }
  int outfd = -1;
  if ((outfd = procdata->iproc_outfd) > 0)
    {
      procdata->iproc_outfd = -1;
      close (outfd);
    };
}

static const struct mom_payload_descr_st payldescr_process_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "process",
  .dpayl_finalizefun = payl_process_finalize_mom
};

///
static const struct mom_payload_descr_st payldescr_webexchange_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "webexchange",
  .dpayl_finalizefun = mom_paylwebx_finalize
};

////////////////////////////////////////////////////////////////
/******************** payload descriptors *********************/

struct mom_payload_descr_st *mom_payloadescr[mompayk__last + 1] = {
  [mompayk_queue] = (struct mom_payload_descr_st *) &payldescr_queue_mom,
  [mompayk_routine] = (struct mom_payload_descr_st *) &payldescr_routine_mom,
  [mompayk_tasklet] = (struct mom_payload_descr_st *) &payldescr_tasklet_mom,
  [mompayk_buffer] = (struct mom_payload_descr_st *) &payldescr_buffer_mom,
  [mompayk_process] = (struct mom_payload_descr_st *) &payldescr_process_mom,
  [mompayk_webexchange] =
    (struct mom_payload_descr_st *) &payldescr_webexchange_mom,
};
