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
///// TASKLET PAYLOAD
////////////////////////////////////////////////////////////////

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
		    || (itd->dtk_scalsize > 64
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
       || (itd->dtk_valsize > 64 && 2 * valuwant < itd->dtk_valsize)))
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
       || (itd->dtk_frasize > 64 && 2 * framwant < itd->dtk_frasize)))
    {
      unsigned newfrasize = ((5 * itd->dtk_fratop / 4 + 3 + nbfram) | 7) + 1;
      if (newfrasize != itd->dtk_frasize)
	{
	  struct momframe_st *newframes =
	    MOM_GC_SCALAR_ALLOC ("reserved frames of tasklet",
				 sizeof (struct momframe_st) * newfrasize);
	  momnode_t **newclosures =
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

#warning unimplemented tasklet
static void
payl_tasklet_load_mom (struct mom_loader_st *ld, momitem_t *litm,
		       momval_t jsob)
{
}

static void
payl_tasklet_dump_scan_mom (struct mom_dumper_st *du, momitem_t *ditm)
{
}

static momval_t
payl_tasklet_dump_json_mom (struct mom_dumper_st *du, momitem_t *ditm)
{
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

#warning unimplemented buffer
static void
payl_buffer_load_mom (struct mom_loader_st *ld, momitem_t *litm,
		      momval_t jsob)
{
}

static void
payl_buffer_dump_scan_mom (struct mom_dumper_st *du, momitem_t *ditm)
{
}

static momval_t
payl_buffer_dump_json_mom (struct mom_dumper_st *du, momitem_t *ditm)
{
}

static const struct mom_payload_descr_st payldescr_buffer_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "buffer",
  .dpayl_loadfun = payl_buffer_load_mom,
  .dpayl_dumpscanfun = payl_buffer_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_buffer_dump_json_mom,
};


////////////////////////////////////////////////////////////////
/******************** payload descriptors *********************/

struct mom_payload_descr_st *mom_payloadescr[mompayk__last + 1] = {
  [mompayk_queue] = (struct mom_payload_descr_st *) &payldescr_queue_mom,
  [mompayk_tasklet] = (struct mom_payload_descr_st *) &payldescr_tasklet_mom,
  [mompayk_buffer] = (struct mom_payload_descr_st *) &payldescr_buffer_mom,
};
