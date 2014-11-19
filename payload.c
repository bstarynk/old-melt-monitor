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
  struct mom_valuequeue_st *vq = itm->i_payload;
  mom_queue_add_value_back (vq, val);
}

void
mom_item_queue_add_front (momitem_t *itm, momval_t val)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return;
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  mom_queue_add_value_front (vq, val);
}

bool
mom_item_queue_is_empty (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return true;
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  return mom_queue_is_empty (vq);
}

unsigned
mom_item_queue_length (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return 0;
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  return mom_queue_length (vq);
}

momval_t
mom_item_queue_peek_front (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  return mom_queue_peek_value_front (vq);
}

momval_t
mom_item_queue_peek_back (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  return mom_queue_peek_value_back (vq);
}

momval_t
mom_item_queue_pop_front (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_queue)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  return mom_queue_pop_value_front (vq);
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
  struct mom_valuequeue_st *vq = itm->i_payload;
  for (struct mom_vaqelem_st * qel
       = vq->vaq_first; qel != NULL; qel = qel->vqe_next)
    {
      for (int ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	if (qel->vqe_valtab[ix].ptr)
	  mom_dump_scan_value (du, qel->vqe_valtab[ix]);
    }
}

static momval_t
payl_queue_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jarr = MOM_NULLV;
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_queue);
  assert (itm->i_payload != NULL);
  struct mom_valuequeue_st *vq = itm->i_payload;
  unsigned qlen = mom_queue_length (vq);
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrval =
    (qlen < MOM_TINY_MAX) ? tinyarr
    : MOM_GC_ALLOC ("dump queue array", qlen * sizeof (momval_t));
  unsigned cnt = 0;
  for (struct mom_vaqelem_st * qel = vq->vaq_first;
       qel != NULL && cnt < qlen; qel = qel->vqe_next)
    {
      for (int ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	{
	  momval_t curval = qel->vqe_valtab[ix];
	  if (curval.ptr && cnt < qlen)
	    arrval[cnt++] = mom_dump_emit_json (du, curval);
	}
    }
  jarr = (momval_t) mom_make_json_array_count (cnt, arrval);
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
///// VECTOR PAYLOAD
////////////////////////////////////////////////////////////////


void
mom_item_start_vector (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  itm->i_payload =
    MOM_GC_ALLOC ("item vector", sizeof (struct mom_valuevector_st));
  itm->i_paylkind = mompayk_vector;
}


#define VECTOR_THRESHOLD 64
void
mom_item_vector_reserve (momitem_t *itm, unsigned gap)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_vector)
    return;
  assert (itm->i_payload != NULL);
  struct mom_valuevector_st *vvec = itm->i_payload;
  if (vvec->vvec_count + gap >= vvec->vvec_size)
    {
      momval_t *oldarr = vvec->vvec_array;
      unsigned oldcnt = vvec->vvec_count;
      unsigned newsiz = (((5 * oldcnt / 4 + gap + 3) | 7) + 1);
      momval_t *newarr =
	MOM_GC_ALLOC ("grow vector", newsiz * sizeof (momval_t));
      if (oldcnt > 0)
	memcpy (newarr, oldarr, oldcnt * sizeof (momval_t));
      vvec->vvec_size = newsiz;
      vvec->vvec_array = newarr;
      MOM_GC_FREE (oldarr);
    }
  else if (vvec->vvec_size > VECTOR_THRESHOLD
	   && 2 * (vvec->vvec_count + gap) < vvec->vvec_size)
    {
      momval_t *oldarr = vvec->vvec_array;
      unsigned oldcnt = vvec->vvec_count;
      unsigned newsiz = (((5 * oldcnt / 4 + gap + 3) | 7) + 1);
      momval_t *newarr =
	MOM_GC_ALLOC ("shrink vector", newsiz * sizeof (momval_t));
      if (oldcnt > 0)
	memcpy (newarr, oldarr, oldcnt * sizeof (momval_t));
      vvec->vvec_size = newsiz;
      vvec->vvec_array = newarr;
      MOM_GC_FREE (oldarr);
    }
}

void
mom_item_vector_append1 (momitem_t *itm, momval_t val)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_vector)
    return;
  assert (itm->i_payload != NULL);
  struct mom_valuevector_st *vvec = itm->i_payload;
  if (vvec->vvec_count + 1 >= vvec->vvec_size)
    mom_item_vector_reserve (itm, 2);
  assert (vvec->vvec_count + 1 < vvec->vvec_size);
  vvec->vvec_array[vvec->vvec_count] = val;
  vvec->vvec_count++;
}

void
mom_item_vector_append_sized (momitem_t *itm, unsigned cnt, ...)
{
  va_list alist;
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_vector)
    return;
  assert (itm->i_payload != NULL);
  struct mom_valuevector_st *vvec = itm->i_payload;
  unsigned oldcnt = vvec->vvec_count;
  if (oldcnt + cnt >= vvec->vvec_size)
    mom_item_vector_reserve (itm, cnt + 2);
  va_start (alist, cnt);
  memset (vvec->vvec_array + oldcnt, 0, cnt * sizeof (momval_t));
  vvec->vvec_count = oldcnt + cnt;
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      momval_t val = va_arg (alist, momval_t);
      vvec->vvec_array[oldcnt + ix] = val;
    }
  va_end (alist);
}

void
mom_item_vector_append_til_nil (momitem_t *itm, ...)
{
  va_list alist;
  unsigned cnt = 0;
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_vector)
    return;
  assert (itm->i_payload != NULL);
  struct mom_valuevector_st *vvec = itm->i_payload;
  unsigned oldcnt = vvec->vvec_count;
  va_start (alist, itm);
  while (va_arg (alist, momval_t).ptr != NULL)
      cnt++;
  va_end (alist);
  if (oldcnt + cnt >= vvec->vvec_size)
    mom_item_vector_reserve (itm, cnt + 2);
  va_start (alist, itm);
  memset (vvec->vvec_array + oldcnt, 0, cnt * sizeof (momval_t));
  vvec->vvec_count = oldcnt + cnt;
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      momval_t val = va_arg (alist, momval_t);
      if (val.ptr == MOM_EMPTY)
	val = MOM_NULLV;
      vvec->vvec_array[oldcnt + ix] = val;
    }
  va_end (alist);
}

void
mom_item_vector_append_from_array (momitem_t *itm, unsigned cnt,
				   const momval_t *arr)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_vector)
    return;
  if (!cnt || !arr)
    return;
  assert (itm->i_payload != NULL);
  struct mom_valuevector_st *vvec = itm->i_payload;
  unsigned oldcnt = vvec->vvec_count;
  if (oldcnt + cnt >= vvec->vvec_size)
    mom_item_vector_reserve (itm, cnt + 2);
  vvec->vvec_count = oldcnt + cnt;
  for (unsigned ix = 0; ix < cnt; ix++)
    vvec->vvec_array[oldcnt + ix] = arr[ix];
}

void
mom_item_vector_append_from_node (momitem_t *itm, const momval_t nodv)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_vector)
    return;
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return;
  mom_item_vector_append_from_array (itm, nodv.pnode->slen,
				     nodv.pnode->sontab);
}


momval_t
mom_make_node_from_item_vector (const momitem_t *connitm, momitem_t *vectitm)
{
  if (!connitm || connitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (!vectitm || vectitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (vectitm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = vectitm->i_payload;
  unsigned cnt = vvec->vvec_count;
  return (momval_t) mom_make_node_from_array (connitm, cnt, vvec->vvec_array);
}

momval_t
mom_make_node_from_item_vector_slice (const momitem_t *connitm,
				      momitem_t *vectitm, int firstix,
				      int afterix)
{
  if (!connitm || connitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (!vectitm || vectitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (vectitm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = vectitm->i_payload;
  unsigned cnt = vvec->vvec_count;
  if (firstix < 0)
    firstix += cnt;
  if (afterix < 0)
    afterix += cnt;
  if (firstix >= 0 && firstix < (int) cnt
      && afterix >= firstix && afterix < (int) cnt)
    return (momval_t) mom_make_node_from_array (connitm, afterix - firstix,
						vvec->vvec_array + firstix);
  return MOM_NULLV;
}

momval_t
mom_make_set_from_item_vector (momitem_t *vectitm)
{
  if (!vectitm || vectitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (vectitm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = vectitm->i_payload;
  unsigned cnt = vvec->vvec_count;
  return (momval_t) mom_make_set_from_array (cnt,
					     (const momitem_t
					      **) (vvec->vvec_array));
}

momval_t
mom_make_set_from_item_vector_slice (momitem_t *vectitm, int firstix,
				     int afterix)
{
  if (!vectitm || vectitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (vectitm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = vectitm->i_payload;
  unsigned cnt = vvec->vvec_count;
  if (firstix < 0)
    firstix += cnt;
  if (afterix < 0)
    afterix += cnt;
  if (firstix >= 0 && firstix < (int) cnt
      && afterix >= firstix && afterix < (int) cnt)
    return (momval_t) mom_make_set_from_array (afterix - firstix,
					       (const momitem_t
						**) (vvec->vvec_array +
						     firstix));
  return MOM_NULLV;
}

momval_t
mom_make_tuple_from_item_vector (momitem_t *vectitm)
{
  if (!vectitm || vectitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (vectitm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = vectitm->i_payload;
  unsigned cnt = vvec->vvec_count;
  return (momval_t) mom_make_tuple_from_array (cnt,
					       (const momitem_t
						**) (vvec->vvec_array));
}

momval_t
mom_make_tuple_from_item_vector_slice (momitem_t *vectitm, int firstix,
				       int afterix)
{
  if (!vectitm || vectitm->i_typnum != momty_item)
    return MOM_NULLV;
  if (vectitm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = vectitm->i_payload;
  unsigned cnt = vvec->vvec_count;
  if (firstix < 0)
    firstix += cnt;
  if (afterix < 0)
    afterix += cnt;
  if (firstix >= 0 && firstix < (int) cnt
      && afterix >= firstix && afterix < (int) cnt)
    return (momval_t) mom_make_tuple_from_array (afterix - firstix,
						 (const momitem_t
						  **) (vvec->vvec_array +
						       firstix));
  return MOM_NULLV;
}

static void
payl_vector_load_mom (struct mom_loader_st *ld, momitem_t *itm,
		      momval_t jpayl)
{
  assert (ld != NULL);
  assert (itm && itm->i_typnum == momty_item);
  mom_item_start_vector (itm);
  unsigned len = mom_json_array_size (jpayl);
  if (!len)
    return;
  mom_item_vector_reserve (itm, 9 * len / 8 + 2);
  for (unsigned ix = 0; ix < len; ix++)
    mom_item_vector_append1 (itm,
			     mom_load_value_json (ld,
						  mom_json_array_nth (jpayl,
								      ix)));
}

static void
payl_vector_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_vector);
  assert (itm->i_payload != NULL);
  struct mom_valuevector_st *vvec = itm->i_payload;
  unsigned len = vvec->vvec_count;
  for (unsigned ix = 0; ix < len; ix++)
    mom_dump_scan_value (du, vvec->vvec_array[ix]);
}

static momval_t
payl_vector_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jarr = MOM_NULLV;
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_vector);
  struct mom_valuevector_st *vvec = itm->i_payload;
  unsigned len = vvec->vvec_count;
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrval =
    (len < MOM_TINY_MAX) ? tinyarr
    : MOM_GC_ALLOC ("dump vector array", len * sizeof (momval_t));
  for (unsigned ix = 0; ix < len; ix++)
    arrval[ix] = mom_dump_emit_json (du, vvec->vvec_array[ix]);
  jarr = (momval_t) mom_make_json_array_count (len, arrval);
  if (arrval != tinyarr)
    MOM_GC_FREE (arrval);
  return jarr;
}

static const struct mom_payload_descr_st payldescr_vector_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "vector",
  .dpayl_loadfun = payl_vector_load_mom,
  .dpayl_dumpscanfun = payl_vector_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_vector_dump_json_mom,
};



////////////////////////////////////////////////////////////////
///// ASSOC PAYLOAD
////////////////////////////////////////////////////////////////

/** the assoc payload data is a GC_MALLOC-ed struct mom_itemattributes_st */

void
mom_item_start_assoc (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  const unsigned asiz = 8;
  struct mom_itemattributes_st *assoc	//
    = MOM_GC_ALLOC ("item assoc",
		    sizeof (struct mom_itemattributes_st)
		    + asiz * sizeof (struct mom_attrentry_st));
  assoc->size = asiz;
  itm->i_payload = assoc;
  itm->i_paylkind = mompayk_assoc;
}

void
mom_item_assoc_reserve (momitem_t *itm, unsigned gap)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc)
    return;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  assoc = mom_reserve_attribute (assoc, gap);
  itm->i_payload = assoc;
}

momval_t
mom_item_assoc_get (momitem_t *itm, const momitem_t *atitm)
{
  momval_t res = MOM_NULLV;
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc || !atitm)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  res = mom_get_attribute (assoc, atitm);
  return res;
}

momval_t
mom_item_assoc_set_attrs (momitem_t *itm)
{
  momval_t res = MOM_NULLV;
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  res = (momval_t) mom_set_attributes (assoc);
  return res;
}

void
mom_item_assoc_put (momitem_t *itm, const momitem_t *atitm,
		    const momval_t val)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc || !atitm)
    return;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  assoc = mom_put_attribute (assoc, atitm, val);
  itm->i_payload = assoc;
}

void
mom_item_assoc_remove (momitem_t *itm, const momitem_t *atitm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc || !atitm)
    return;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  assoc = mom_remove_attribute (assoc, atitm);
  itm->i_payload = assoc;
}

static void
payl_assoc_load_mom (struct mom_loader_st *ld, momitem_t *itm, momval_t jpayl)
{
  assert (ld != NULL);
  assert (itm && itm->i_typnum == momty_item);
  mom_item_start_assoc (itm);
  unsigned len = mom_json_array_size (jpayl);
  mom_item_assoc_reserve (itm, 5 * len / 4 + 2);
  for (unsigned aix = 0; aix < len; aix++)
    {
      momval_t jent = mom_json_array_nth (jpayl, (int) aix);
      momval_t jcurattr = mom_jsonob_get (jent, (momval_t) mom_named__attr);
      momval_t jcurval = mom_jsonob_get (jent, (momval_t) mom_named__val);
      momitem_t *curatitm = mom_load_item_json (ld, jcurattr);
      if (!curatitm)
	continue;
      momval_t curval = mom_load_value_json (ld, jcurval);
      mom_item_assoc_put (itm, curatitm, curval);
    }
}

static void
payl_assoc_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc)
    return;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  unsigned asiz = assoc->size;
  for (unsigned aix = 0; aix < asiz; aix++)
    {
      momitem_t *curitm = assoc->itattrtab[aix].aten_itm;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      assert (curitm->i_typnum == momty_item
	      && curitm->i_magic == MOM_ITEM_MAGIC);
      if (curitm->i_space == momspa_none)
	continue;
      momval_t curval = assoc->itattrtab[aix].aten_val;
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      if (mom_has_flags (curval, momflag_transient))
	continue;
      mom_dump_add_scanned_item (du, curitm);
      mom_dump_scan_value (du, curval);
    }
}

static momval_t
payl_assoc_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jres = MOM_NULLV;
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_paylkind != mompayk_assoc)
    return MOM_NULLV;
  assert (itm->i_payload != NULL);
  struct mom_itemattributes_st *assoc = itm->i_payload;
  unsigned asiz = assoc->size;
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrj = (asiz < MOM_TINY_MAX)
    ? tinyarr : MOM_GC_ALLOC ("assoc dump arrj", asiz * sizeof (momval_t));
  unsigned cnt = 0;
  for (unsigned aix = 0; aix < asiz; aix++)
    {
      assert (cnt < asiz);
      momitem_t *curitm = assoc->itattrtab[aix].aten_itm;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      assert (curitm->i_typnum == momty_item
	      && curitm->i_magic == MOM_ITEM_MAGIC);
      if (curitm->i_space == momspa_none)
	continue;
      momval_t curval = assoc->itattrtab[aix].aten_val;
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      if (mom_has_flags (curval, momflag_transient))
	continue;
      momval_t jattr = mom_emit_short_item_json (du, curitm);
      if (!jattr.ptr)
	continue;
      momval_t jval = mom_dump_emit_json (du, curval);
      if (!jval.ptr)
	continue;
      momval_t jent = (momval_t) mom_make_json_object
	(MOMJSOB_ENTRY ((momval_t) mom_named__attr, (momval_t) jattr),
	 MOMJSOB_ENTRY ((momval_t) mom_named__val, (momval_t) jval),
	 MOMJSON_END);
      arrj[cnt++] = jent;
    }
  jres = (momval_t) mom_make_json_array_count (cnt, arrj);
  if (arrj != tinyarr)
    MOM_GC_FREE (arrj);
  return jres;
}

static const struct mom_payload_descr_st payldescr_assoc_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "assoc",
  .dpayl_loadfun = payl_assoc_load_mom,
  .dpayl_dumpscanfun = payl_assoc_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_assoc_dump_json_mom,
};


////////////////////////////////////////////////////////////////
///// HASHSET PAYLOAD
////////////////////////////////////////////////////////////////

void
mom_item_start_hset (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  const unsigned siz = 16;
  struct momhset_st *hset =
    MOM_GC_ALLOC ("hset init", sizeof (struct momhset_st));
  hset->hset_magic = MOM_HSET_MAGIC;
  hset->hset_count = 0;
  hset->hset_size = siz;
  hset->hset_arr = MOM_GC_ALLOC ("hset array", siz * sizeof (momval_t));
  itm->i_paylkind = mompayk_hset;
  itm->i_payload = hset;
}

static inline int
hset_addval_mom (struct momhset_st *hset, momval_t val)
{
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  if (!val.ptr)
    return -1;
  unsigned sz = hset->hset_size;
  assert (hset->hset_count + 3 <= sz);
  momhash_t h = mom_value_hash (val);
  momval_t *arr = hset->hset_arr;
  int pos = -1;
  unsigned startix = h % sz;
  for (unsigned ix = startix; ix < sz; ix++)
    {
      if (arr[ix].ptr == val.ptr)
	return -1;
      else if (arr[ix].ptr == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = ix;
	}
      else if (!arr[ix].ptr)
	{
	  if (pos < 0)
	    pos = ix;
	  goto put;
	}
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      if (arr[ix].ptr == val.ptr)
	return -1;
      else if (arr[ix].ptr == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = ix;
	}
      else if (!arr[ix].ptr)
	{
	  if (pos < 0)
	    pos = ix;
	  goto put;
	}
    }
put:
  assert (pos >= 0);
  arr[pos] = val;
  hset->hset_count++;
  return pos;
}

static inline int
hset_find_mom (const struct momhset_st *hset, momval_t val)
{
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  if (!val.ptr)
    return -1;
  momhash_t h = mom_value_hash (val);
  momval_t *arr = hset->hset_arr;
  unsigned sz = hset->hset_size;
  unsigned startix = h % sz;
  for (unsigned ix = startix; ix < sz; ix++)
    {
      if (arr[ix].ptr == val.ptr)
	return ix;
      else if (arr[ix].ptr == MOM_EMPTY)
	continue;
      else if (!arr[ix].ptr)
	return -1;
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      if (arr[ix].ptr == val.ptr)
	return ix;
      else if (arr[ix].ptr == MOM_EMPTY)
	continue;
      else if (!arr[ix].ptr)
	return -1;
    }
  return -1;
}

static void
hset_reorganize_mom (struct momhset_st *hset, unsigned gap)
{
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  unsigned oldsz = hset->hset_size;
  unsigned oldcnt = hset->hset_count;
  momval_t *oldarr = hset->hset_arr;
  if (7 * oldcnt / 6 + gap + 2 >= oldsz)
    {
      unsigned newsz = ((5 * oldcnt / 4 + 9 + gap) | 0xf) + 1;
      if (newsz == oldsz)
	return;
      momval_t *newarr =
	MOM_GC_ALLOC ("grow hset", newsz * sizeof (momval_t));
      hset->hset_size = newsz;
      hset->hset_count = 0;
      hset->hset_arr = newarr;
    }
  else if (3 * oldcnt + gap < oldsz && oldsz > 30)
    {
      unsigned newsz = ((5 * oldcnt / 4 + 9 + gap) | 0xf) + 1;
      if (newsz == oldsz)
	return;
      momval_t *newarr =
	MOM_GC_ALLOC ("shrink hset", newsz * sizeof (momval_t));
      hset->hset_size = newsz;
      hset->hset_count = 0;
      hset->hset_arr = newarr;
    }
  for (unsigned ix = 0; ix < oldsz; ix++)
    {
      momval_t curval = oldarr[ix];
      if (curval.ptr == MOM_EMPTY || !curval.ptr)
	continue;
      hset_addval_mom (hset, curval);
    }
}

void
mom_item_hset_reserve (momitem_t *itm, unsigned gap)
{
  if (!itm || itm->i_typnum != momty_item || itm->i_paylkind != mompayk_hset)
    return;
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  if (hset->hset_count + gap <= 7 * hset->hset_size / 6)
    hset_reorganize_mom (hset, gap);
  else if (4 * hset->hset_count + 5 < hset->hset_size && hset->hset_size > 16)
    hset_reorganize_mom (hset, gap);
}

bool
mom_item_hset_contains (momitem_t *itm, momval_t elem)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_hset || !elem.ptr)
    return false;
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  return hset_find_mom (hset, elem) >= 0;
}

bool
mom_item_hset_add (momitem_t *itm, momval_t elem)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_hset || !elem.ptr)
    return false;
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  if (6 * hset->hset_count / 5 + 2 >= hset->hset_size)
    hset_reorganize_mom (hset, hset->hset_count / 32 + 4);
  int pos = hset_addval_mom (hset, elem);
  return pos >= 0;
}

bool
mom_item_hset_remove (momitem_t *itm, momval_t elem)
{
  bool found = false;
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_hset || !elem.ptr)
    return false;
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  int pos = hset_find_mom (hset, elem);
  if (pos >= 0)
    {
      found = true;
      hset->hset_arr[pos].ptr = MOM_EMPTY;
      if (hset->hset_size > 30 && hset->hset_count < hset->hset_size / 5)
	hset_reorganize_mom (hset, 3);
    }
  return found;
}

momval_t
mom_item_hset_items_set (momitem_t *itm)
{
  momval_t res = MOM_NULLV;
  momitem_t **itmarr = NULL;
  if (!itm || itm->i_typnum != momty_item || itm->i_paylkind != mompayk_hset)
    return MOM_NULLV;
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  unsigned cnt = hset->hset_count;
  unsigned sz = hset->hset_size;
  momval_t *arr = hset->hset_arr;
  itmarr =
    MOM_GC_ALLOC ("hset itmarr for set", (cnt + 1) * sizeof (momitem_t *));
  unsigned itcnt = 0;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      momval_t curval = arr[ix];
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      assert (itcnt < cnt);
      if (mom_is_item (curval))
	itmarr[itcnt++] = curval.pitem;
    }
  res =
    (momval_t) mom_make_set_from_array (itcnt, (const momitem_t **) itmarr);
  MOM_GC_FREE (itmarr);
  return res;
}




momval_t
mom_item_hset_sorted_values_node (momitem_t *hsetitm, momitem_t *connitm)
{
  momval_t res = MOM_NULLV;
  momval_t *valarr = NULL;
  if (!hsetitm || hsetitm->i_typnum != momty_item
      || hsetitm->i_paylkind != mompayk_hset
      || !connitm || connitm->i_typnum != momty_item)
    return MOM_NULLV;
  struct momhset_st *hset = hsetitm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  unsigned cnt = hset->hset_count;
  unsigned sz = hset->hset_size;
  momval_t *arr = hset->hset_arr;
  valarr =
    MOM_GC_ALLOC ("hset valarr sortedtup", (cnt + 1) * sizeof (momval_t));
  unsigned valcnt = 0;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      momval_t curval = arr[ix];
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      assert (valcnt < cnt);
      valarr[valcnt++] = curval;
    }
  qsort (valarr, valcnt, sizeof (momval_t), mom_valqsort_cmp);
  res = (momval_t) mom_make_node_from_array (connitm, valcnt, valarr);
  MOM_GC_FREE (valarr);
  return res;
}

static void
payl_hset_load_mom (struct mom_loader_st *ld, momitem_t *itm, momval_t jpayl)
{
  unsigned len = mom_json_array_size (jpayl);
  mom_item_start_hset (itm);
  mom_item_hset_reserve (itm, 5 * len / 4 + len / 16 + 10);
  for (unsigned aix = 0; aix < len; aix++)
    {
      momval_t jent = mom_json_array_nth (jpayl, (int) aix);
      momval_t curval = mom_load_value_json (ld, jent);
      mom_item_hset_add (itm, curval);
    }
}

static void
payl_hset_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (itm && itm->i_typnum != momty_item
	  && itm->i_paylkind != mompayk_hset);
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  unsigned sz = hset->hset_size;
  momval_t *arr = hset->hset_arr;
  for (unsigned ix = 0; ix < sz; ix++)
    {
      momval_t curval = arr[ix];
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      mom_dump_scan_value (du, curval);
    }
}

static momval_t
payl_hset_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jres = MOM_NULLV;
  assert (itm && itm->i_typnum != momty_item
	  && itm->i_paylkind != mompayk_hset);
  struct momhset_st *hset = itm->i_payload;
  assert (hset && hset->hset_magic == MOM_HSET_MAGIC);
  momval_t *valarr = NULL;
  unsigned valcnt = 0;
  unsigned cnt = hset->hset_count;
  unsigned sz = hset->hset_size;
  momval_t *arr = hset->hset_arr;
  valarr =
    MOM_GC_ALLOC ("hset valarr sortedtup", (cnt + 1) * sizeof (momval_t));
  for (unsigned ix = 0; ix < sz; ix++)
    {
      momval_t curval = arr[ix];
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      assert (valcnt < cnt);
      valarr[valcnt++] = curval;
    }
  // we sort the array to have a consistent & reproducible JSON
  qsort (valarr, valcnt, sizeof (momval_t), mom_valqsort_cmp);
  for (unsigned ix = 0; ix < valcnt; ix++)
    valarr[ix] = mom_dump_emit_json (du, valarr[ix]);
  jres = (momval_t) mom_make_json_array_count (valcnt, valarr);
  MOM_GC_FREE (valarr);
  return jres;
}

static const struct mom_payload_descr_st payldescr_hset_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "hset",
  .dpayl_loadfun = payl_hset_load_mom,
  .dpayl_dumpscanfun = payl_hset_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_hset_dump_json_mom,
};

////////////////////////////////////////////////////////////////
///// TASKFUN ROUTINE PAYLOAD
////////////////////////////////////////////////////////////////

void
mom_item_start_tfun_routine (momitem_t *itm)
{
  char symbuf[MOM_SYMBNAME_LEN];
  memset (symbuf, 0, sizeof (symbuf));
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  const char *routname =
    mom_string_cstr ((momval_t) mom_item_get_idstr (itm));
  if (!routname || !routname[0])
    return;
  snprintf (symbuf, sizeof (symbuf), MOM_TFUN_NAME_FMT, routname);
  assert (symbuf[MOM_SYMBNAME_LEN - 1] == '\0');
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
  const struct momtfundescr_st *rdescr = routad;
  if (rdescr->tfun_magic != MOM_TFUN_MAGIC
      || !rdescr->tfun_ident || !rdescr->tfun_module
      || !rdescr->tfun_codefun || !rdescr->tfun_timestamp)
    MOM_FATAPRINTF ("invalid routine descriptor @%p for %s", routad,
		    routname);
  if (strcmp (routname, rdescr->tfun_ident))
    MOM_WARNPRINTF ("strange routine descriptor for %s but ident %s",
		    routname, rdescr->tfun_ident);
  itm->i_payload = (void *) rdescr;
  itm->i_paylkind = mompayk_tfunrout;
  MOM_DEBUG (run, MOMOUT_LITERAL ("starting routine item:"),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (", ident "),
	     MOMOUT_LITERALV (rdescr->tfun_ident),
	     MOMOUT_LITERAL (" from module "),
	     MOMOUT_LITERALV (rdescr->tfun_module),
	     MOMOUT_LITERAL (" timestamp "),
	     MOMOUT_LITERALV (rdescr->tfun_timestamp),
	     MOMOUT_LITERAL (" i_paylkind="),
	     MOMOUT_DEC_INT ((int) itm->i_paylkind), NULL);
}

static void
payl_tfunrout_load_mom (struct mom_loader_st *ld, momitem_t *itm,
			momval_t jsonv)
{
  assert (ld != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  MOM_DEBUG (load,
	     MOMOUT_LITERAL ("payl_tfunrout_load_mom itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (" jsonv="),
	     MOMOUT_VALUE ((const momval_t) jsonv), NULL);
  mom_item_start_tfun_routine (itm);
  struct momtfundescr_st *fd = itm->i_payload;
  assert (fd && fd->tfun_magic == MOM_TFUN_MAGIC);
  momval_t jcodejit = mom_jsonob_get (jsonv, (momval_t) mom_named__jit);
  momval_t jconst = mom_jsonob_get (jsonv, (momval_t) mom_named__constants);
  momval_t jfunid =
    mom_jsonob_get (jsonv, (momval_t) mom_named__tasklet_function);
  if (jcodejit.ptr)
    {
      const char *err = mom_item_generate_jit_tfun_routine (itm, jcodejit);
      if (err)
	MOM_FATAL (MOMOUT_LITERAL ("payl_tfunrout_load_mom itm="),
		   MOMOUT_ITEM ((const momitem_t *) itm),
		   MOMOUT_SPACE (32),
		   MOMOUT_LITERAL ("failed:"),
		   MOMOUT_LITERALV ((const char *) err),
		   MOMOUT_NEWLINE (),
		   MOMOUT_LITERAL ("jcodejit="), MOMOUT_VALUE (jcodejit),
		   NULL);
    }
  if (jconst.ptr)
    {
      const momitem_t **oldcstitems = fd->tfun_constantitems;
      unsigned nbconst = mom_json_array_size (jconst);
      if (nbconst > fd->tfun_nbconstants)
	nbconst = fd->tfun_nbconstants;
      assert (nbconst == 0 || oldcstitems);
      for (unsigned ix = 0; ix < nbconst; ix++)
	{
	  momitem_t *olditm = oldcstitems[ix];
	  momitem_t *newitm =
	    mom_load_item_json (ld, mom_json_array_nth (jconst, ix));
	  if (MOM_UNLIKELY (olditm && newitm && olditm != newitm))
	    {
	      MOM_WARNING (MOMOUT_LITERAL ("in loaded task function item:"),
			   MOMOUT_ITEM ((const momitem_t *) itm),
			   MOMOUT_LITERAL (" constant-item#"),
			   MOMOUT_DEC_INT ((int) ix),
			   MOMOUT_LITERAL (" was:"),
			   MOMOUT_ITEM ((const momitem_t *) olditm),
			   MOMOUT_LITERAL (" overriden by:"),
			   MOMOUT_ITEM ((const momitem_t *) newitm), NULL);
	      oldcstitems[ix] = newitm;
	    }
	}
    }
}


static void
payl_tfunrout_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_tfunrout);
  assert (itm->i_payload != NULL);
  const struct momtfundescr_st *rdescr = itm->i_payload;
  assert (rdescr != NULL && rdescr->tfun_magic == MOM_TFUN_MAGIC
	  && rdescr->tfun_ident != NULL);
  if (rdescr->tfun_jitcode.ptr != NULL)
    {
      assert (rdescr->tfun_ident[0] == '.');
      mom_dump_scan_value (du, rdescr->tfun_jitcode);
    }
  else if (rdescr->tfun_constantitems)
    {
      unsigned ln = rdescr->tfun_nbconstants;
      for (unsigned ix = 0; ix < ln; ix++)
	mom_dump_add_scanned_item (du, rdescr->tfun_constantitems[ix]);
    }
  assert (rdescr->tfun_module != NULL);
  if (strcmp (rdescr->tfun_module, MOM_EMPTY_MODULE) != 0)
    mom_dump_scan_need_module (du, rdescr->tfun_module);
}


static momval_t
payl_tfunrout_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_tfunrout);
  const struct momtfundescr_st *rdescr = itm->i_payload;
  momval_t jcstarr = MOM_NULLV;
  assert (rdescr != NULL && rdescr->tfun_magic == MOM_TFUN_MAGIC
	  && rdescr->tfun_ident != NULL);
  if (rdescr->tfun_constantitems)
    {
      unsigned ln = rdescr->tfun_nbconstants;
      momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t *arrj = (ln < MOM_TINY_MAX) ? tinyarr : MOM_GC_ALLOC ("arrj",
								     ln *
								     sizeof
								     (momval_t));
      for (unsigned ix = 0; ix < ln; ix++)
	arrj[ix] =
	  mom_emit_short_item_json (du, rdescr->tfun_constantitems[ix]);
      jcstarr = (momval_t) mom_make_json_array_count (ln, arrj);
    }
  if (rdescr->tfun_jitcode.ptr)
    {
      assert (rdescr->tfun_ident[0] == '.');
      momval_t jcode = mom_dump_emit_json (du, rdescr->tfun_jitcode);
      momval_t jvalr = (momval_t) mom_make_json_object
	(MOMJSOB_ENTRY ((momval_t) mom_named__jit, (momval_t) jcode),
	 MOMJSOB_ENTRY ((momval_t) mom_named__constants, (momval_t) jcstarr),
	 MOMJSON_END);
      return jvalr;
    }
  else
    {
      momval_t jvalr = (momval_t) mom_make_json_object
	(MOMJSOB_ENTRY ((momval_t) mom_named__tasklet_function,
			(momval_t) mom_make_string (rdescr->tfun_ident)),
	 MOMJSOB_ENTRY ((momval_t) mom_named__constants, (momval_t) jcstarr),
	 MOMJSON_END);
      return jvalr;
      return (momval_t) mom_make_string (rdescr->tfun_ident);
    }
}

static const struct mom_payload_descr_st payldescr_tfunrout_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "tfunrout",
  .dpayl_loadfun = payl_tfunrout_load_mom,
  .dpayl_dumpscanfun = payl_tfunrout_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_tfunrout_dump_json_mom,
};



////////////////////////////////////////////////////////////////
///// CLOSURE PAYLOAD
////////////////////////////////////////////////////////////////

void
mom_item_start_closure_of_routine (momitem_t *itm,
				   const struct momtfundescr_st *rdescr,
				   unsigned len)
{
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  if (!rdescr)
    return;
  if (rdescr->tfun_magic != MOM_TFUN_MAGIC
      || !rdescr->tfun_ident || !rdescr->tfun_module
      || !rdescr->tfun_codefun || !rdescr->tfun_timestamp)
    MOM_FATAPRINTF ("invalid routine descriptor @%p for closure",
		    (void *) rdescr);
  if (len < rdescr->tfun_minclosize)
    len = rdescr->tfun_minclosize;
  struct momclosure_st *clos = MOM_GC_ALLOC ("closure payload",
					     sizeof (struct momclosure_st) +
					     len * sizeof (momval_t));
  clos->clos_len = len;
  clos->clos_rout = rdescr;
  clos->clos_magic = MOM_CLOSURE_MAGIC;
  itm->i_payload = clos;
  itm->i_paylkind = mompayk_closure;
}

void
mom_item_start_closure (momitem_t *itm, unsigned len)
{

  char symbuf[MOM_SYMBNAME_LEN];
  memset (symbuf, 0, sizeof (symbuf));
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  const char *routid =
    mom_string_cstr ((momval_t) mom_item_get_name_or_idstr (itm));
  if (!routid || !routid[0])
    return;
  snprintf (symbuf, sizeof (symbuf), MOM_TFUN_NAME_FMT, routid);
  assert (symbuf[MOM_SYMBNAME_LEN - 1] == '\0');
  for (const char *pc = symbuf; *pc; pc++)
    if (!isalnum (*pc) && *pc != '_')
      return;
  assert (isalpha (symbuf[0]));
  void *routad = dlsym (mom_prog_dlhandle, symbuf);
  if (!routad)
    {
      MOM_WARNPRINTF ("failed to start closure %s: %s", symbuf, dlerror ());
      return;
    };
  const struct momtfundescr_st *rdescr = routad;
  if (rdescr->tfun_magic != MOM_TFUN_MAGIC
      || !rdescr->tfun_ident || !rdescr->tfun_module
      || !rdescr->tfun_codefun || !rdescr->tfun_timestamp)
    MOM_FATAPRINTF ("invalid closure routine descriptor @%p for %s", routad,
		    routid);
  if (strcmp (routid, rdescr->tfun_ident))
    MOM_WARNPRINTF ("strange closure routine descriptor for %s but named %s",
		    routid, rdescr->tfun_ident);
  mom_item_start_closure_of_routine (itm, rdescr, len);
}

void
mom_item_closure_set_nth (momitem_t *itm, int rk, momval_t cval)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_closure)
    return;
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  unsigned clen = clos->clos_len;
  if (rk < 0)
    rk += (int) clen;
  if (rk >= 0 && rk < (int) clen)
    clos->clos_valtab[rk] = cval;
}

static void
payl_closure_load_mom (struct mom_loader_st *ld, momitem_t *itm,
		       momval_t jclos)
{
  assert (ld != NULL);
  MOM_DEBUG (load,
	     MOMOUT_LITERAL ("payl_closure_load_mom itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (" jclos="),
	     MOMOUT_VALUE ((const momval_t) jclos), NULL);
  assert (itm && itm->i_typnum == momty_item);
  momval_t jrname =
    mom_jsonob_get (jclos, (momval_t) mom_named__closure_routine);
  momval_t jcval =
    mom_jsonob_get (jclos, (momval_t) mom_named__closed_values);
  assert (mom_is_string (jrname));
  assert (mom_is_jsonable (jcval));
  unsigned nbcloval = mom_json_array_size (jcval);
  mom_item_start_closure (itm, nbcloval);
  if (!nbcloval)
    return;
  for (unsigned ix = 0; ix < nbcloval; ix++)
    mom_item_closure_set_nth (itm, ix,
			      mom_load_value_json (ld,
						   mom_json_array_nth (jcval,
								       ix)));
}

static void
payl_closure_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_closure);
  assert (itm->i_payload != NULL);
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  unsigned len = clos->clos_len;
  for (unsigned ix = 0; ix < len; ix++)
    mom_dump_scan_value (du, clos->clos_valtab[ix]);
  const struct momtfundescr_st *rdescr = clos->clos_rout;
  if (strcmp (rdescr->tfun_module, MOM_EMPTY_MODULE) != 0)
    mom_dump_scan_need_module (du, rdescr->tfun_module);
}

static momval_t
payl_closure_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jclos = MOM_NULLV;
  momval_t jarr = MOM_NULLV;
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_closure);
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  unsigned len = clos->clos_len;
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrval =
    (len < MOM_TINY_MAX) ? tinyarr
    : MOM_GC_ALLOC ("dump closure array", len * sizeof (momval_t));
  for (unsigned ix = 0; ix < len; ix++)
    arrval[ix] = mom_dump_emit_json (du, clos->clos_valtab[ix]);
  jarr = (momval_t) mom_make_json_array_count (len, arrval);
  if (arrval != tinyarr)
    MOM_GC_FREE (arrval);
  const struct momtfundescr_st *rdescr = clos->clos_rout;
  assert (rdescr && rdescr->tfun_magic == MOM_TFUN_MAGIC);
  jclos = (momval_t)
    mom_make_json_object
    (MOMJSOB_ENTRY ((momval_t) mom_named__closure_routine,
		    (momval_t) mom_make_string (rdescr->tfun_ident)),
     MOMJSOB_ENTRY ((momval_t) mom_named__closed_values, jarr), MOMJSON_END);
  return jclos;
}

static const struct mom_payload_descr_st payldescr_closure_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "closure",
  .dpayl_loadfun = payl_closure_load_mom,
  .dpayl_dumpscanfun = payl_closure_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_closure_dump_json_mom,
};



////////////////////////////////////////////////////////////////
///// PROCEDURE PAYLOAD
////////////////////////////////////////////////////////////////
void
mom_item_start_procedure (momitem_t *itm)
{
  char symbuf[72];
  memset (symbuf, 0, sizeof (symbuf));
  assert (itm && itm->i_typnum == momty_item);
  if (itm->i_payload)
    mom_item_clear_payload (itm);
  struct momprocrout_st *prout = NULL;
  snprintf (symbuf, sizeof (symbuf),
	    MOM_PROCROUTDESCR_PREFIX "%s", mom_ident_cstr_of_item (itm));
  prout = dlsym (mom_prog_dlhandle, symbuf);
  if (!prout)
    {
      MOM_WARNPRINTF ("failed to start procedure %s: %s", symbuf, dlerror ());
      return;
    }
  if (prout->prout_magic != MOM_PROCROUT_MAGIC
      || strcmp (prout->prout_id, mom_ident_cstr_of_item (itm))
      || !prout->prout_addr)
    MOM_FATAPRINTF ("corrupted procrout for %s", symbuf);
  itm->i_payload = prout;
  itm->i_paylkind = mompayk_procedure;
}


static void
payl_procedure_load_mom (struct mom_loader_st *ld, momitem_t *itm,
			 momval_t jproc)
{
  assert (ld != NULL);
  MOM_DEBUG (load,
	     MOMOUT_LITERAL ("payl_procedure_load_mom itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (" jproc="),
	     MOMOUT_VALUE ((const momval_t) jproc), NULL);
  assert (itm && itm->i_typnum == momty_item);
  mom_item_start_procedure (itm);
  if (itm->i_paylkind != mompayk_procedure)
    MOM_FATAL (MOMOUT_LITERAL ("failed to load procedure:"),
	       MOMOUT_ITEM ((const momitem_t *) itm));
  struct momprocrout_st *prout = itm->i_payload;
  if (!prout || prout->prout_magic != MOM_PROCROUT_MAGIC)
    MOM_FATAL (MOMOUT_LITERAL ("corrupted loaded procedure:"),
	       MOMOUT_ITEM ((const momitem_t *) itm));
  unsigned plen = prout->prout_len;
  assert (plen == 0 || prout->prout_constantitems != NULL);
  for (unsigned ix = 0; ix < plen; ix++)
    {
      momitem_t *oldcstitm = prout->prout_constantitems[ix];
      momitem_t *newcstitm =
	mom_load_item_json (ld, mom_json_array_nth (jproc, ix));
      if (MOM_UNLIKELY (oldcstitm && newcstitm && oldcstitm != newcstitm))
	{
	  MOM_WARNING (MOMOUT_LITERAL ("in procedure item:"),
		       MOMOUT_ITEM ((const momitem_t *) itm),
		       MOMOUT_LITERAL (" overwriting constant-item#"),
		       MOMOUT_DEC_INT ((int) ix),
		       MOMOUT_LITERAL (" old "),
		       MOMOUT_ITEM ((const momitem_t *) oldcstitm),
		       MOMOUT_LITERAL (" with new "),
		       MOMOUT_ITEM ((const momitem_t *) newcstitm), NULL);
	  ((momitem_t **) prout->prout_constantitems)[ix] = newcstitm;
	}
    }
}


static void
payl_procedure_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_procedure);
  const struct momprocrout_st *prout = itm->i_payload;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  unsigned plen = prout->prout_len;
  assert (plen == 0 || prout->prout_constantitems != NULL);
  for (unsigned ix = 0; ix < plen; ix++)
    {
      momitem_t *cstitm = prout->prout_constantitems[ix];
      mom_dump_add_scanned_item (du, cstitm);
    }
  if (prout->prout_module
      && strcmp (prout->prout_module, MOM_EMPTY_MODULE) != 0)
    mom_dump_scan_need_module (du, prout->prout_module);
}


static momval_t
payl_procedure_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jarr = MOM_NULLV;
  assert (du != NULL);
  assert (itm && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_procedure);
  const struct momprocrout_st *prout = itm->i_payload;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  unsigned plen = prout->prout_len;
  momval_t tinyarr[MOM_TINY_MAX] = { MOM_NULLV };
  momval_t *arrval =
    (plen < MOM_TINY_MAX) ? tinyarr
    : MOM_GC_ALLOC ("dump procedure array", plen * sizeof (momval_t));
  assert (plen == 0 || prout->prout_constantitems != NULL);
  for (unsigned ix = 0; ix < plen; ix++)
    {
      arrval[ix] =
	mom_emit_short_item_json (du, prout->prout_constantitems[ix]);
    }
  jarr = (momval_t) mom_make_json_array_count (plen, arrval);
  if (arrval != tinyarr)
    MOM_GC_FREE (arrval);
  return jarr;
}


static const struct mom_payload_descr_st payldescr_procedure_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "procedure",
  .dpayl_loadfun = payl_procedure_load_mom,
  .dpayl_dumpscanfun = payl_procedure_dump_scan_mom,
  .dpayl_dumpjsonfun = payl_procedure_dump_json_mom,
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
  const unsigned frasize = 6;
  itd->dtk_closurevals =
    MOM_GC_ALLOC ("tasklet closurevals", frasize * sizeof (momval_t));
  itd->dtk_frames =
    MOM_GC_SCALAR_ALLOC ("tasklet frames",
			 frasize * sizeof (struct momframe_st));
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
  /// resize the integers if needed
  unsigned intwant = itd->dtk_inttop + nbint;
  if (MOM_UNLIKELY (intwant > itd->dtk_intsize
		    || (itd->dtk_intsize > TASKLET_THRESHOLD
			&& 2 * intwant < itd->dtk_intsize)))
    {
      unsigned newintsize = ((5 * itd->dtk_inttop / 4 + 3 + nbint) | 7) + 1;
      if (newintsize != itd->dtk_intsize)
	{
	  intptr_t *newints
	    = MOM_GC_SCALAR_ALLOC ("reserved tasklet integers",
				   newintsize * sizeof (intptr_t));
	  if (itd->dtk_inttop > 0)
	    memcpy (newints, itd->dtk_ints,
		    itd->dtk_inttop * sizeof (intptr_t));
	  MOM_GC_FREE (itd->dtk_ints);
	  itd->dtk_ints = newints;
	  itd->dtk_intsize = newintsize;
	}
    };
  /// resize the doubles if needed
  unsigned dblwant = itd->dtk_dbltop + nbdbl;
  if (MOM_UNLIKELY (dblwant > itd->dtk_dblsize
		    || (itd->dtk_dblsize > TASKLET_THRESHOLD
			&& 2 * dblwant < itd->dtk_dblsize)))
    {
      unsigned newdblsize = ((5 * itd->dtk_dbltop / 4 + 3 + nbdbl) | 7) + 1;
      if (newdblsize != itd->dtk_dblsize)
	{
	  double *newdbls = MOM_GC_SCALAR_ALLOC ("reserved tasklet doubles",
						 newdblsize *
						 sizeof (double));
	  if (itd->dtk_dbltop > 0)
	    memcpy (newdbls, itd->dtk_doubles,
		    itd->dtk_dbltop * sizeof (double));
	  MOM_GC_FREE (itd->dtk_doubles);
	  itd->dtk_doubles = newdbls;
	  itd->dtk_dblsize = newdblsize;
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
	  momval_t *newclosurevals =
	    MOM_GC_ALLOC ("reserved closurevals of tasklet",
			  sizeof (momval_t) * newfrasize);
	  if (itd->dtk_fratop > 0)
	    {
	      memcpy (newframes, itd->dtk_frames,
		      itd->dtk_fratop * sizeof (struct momframe_st));
	      memcpy (newclosurevals, itd->dtk_closurevals,
		      itd->dtk_fratop * sizeof (momval_t));
	    }
	  MOM_GC_FREE (itd->dtk_frames);
	  MOM_GC_FREE (itd->dtk_closurevals);
	  itd->dtk_frames = newframes;
	  itd->dtk_closurevals = newclosurevals;
	  itd->dtk_frasize = newfrasize;
	}
    }
}


static bool
compute_pushed_data_size_mom (momval_t clov,
			      unsigned *pnbval,
			      unsigned *pnbnum,
			      unsigned *pnbdbl,
			      int *pnewstate,
			      enum mom_pushframedirective_en dir,
			      va_list args)
{
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
  const struct momtfundescr_st *rdescr = NULL;
  if (!clov.ptr)
    return false;
  switch (*clov.ptype)
    {
    case momty_item:
      {
	mom_should_lock_item (clov.pitem);
	rdescr = mom_item_tfundescr (clov.pitem);
	mom_unlock_item (clov.pitem);
	break;
      }
    case momty_node:
      {
	momitem_t *citm = (momitem_t *) (clov.pnode->connitm);
	assert (citm && citm->i_typnum == momty_item);
	mom_should_lock_item (citm);
	rdescr = mom_item_tfundescr (citm);
	mom_unlock_item (citm);
	break;
      }
    default:
      return false;
    }
  assert (!rdescr || rdescr->tfun_magic == MOM_TFUN_MAGIC);
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
      if (again)
	dir = va_arg (args, enum mom_pushframedirective_en);
    }				/* end while again */

  if (rdescr)
    {
      if (nbval < rdescr->tfun_frame_nbval)
	nbval = rdescr->tfun_frame_nbval;
      if (nbnum < rdescr->tfun_frame_nbnum)
	nbnum = rdescr->tfun_frame_nbnum;
      if (nbdbl < rdescr->tfun_frame_nbdbl)
	nbdbl = rdescr->tfun_frame_nbdbl;
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
      if (again)
	dir = va_arg (args, enum mom_pushframedirective_en);
    }				/* end while again */
}				//// end fill_frame_data_mom


////////////////

void
mom_item_tasklet_push_frame (momitem_t *itm, momval_t clo,
			     enum mom_pushframedirective_en firstdir, ...)
{
  unsigned nbval = 0, nbint = 0, nbdbl = 0;
  int state = 0;
  va_list alist;
  assert (itm && itm->i_typnum == momty_item);
  if (!clo.ptr || (*clo.ptype != momty_item && *clo.ptype != momty_node))
    return;
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return;
  struct mom_taskletdata_st *itd = itm->i_payload;
  va_start (alist, firstdir);
  if (!compute_pushed_data_size_mom (clo,
				     &nbval, &nbint, &nbdbl,
				     &state, firstdir, alist))
    return;
  va_end (alist);
  /// grow the integers if needed
  if (MOM_UNLIKELY (nbint > 0 && itd->dtk_inttop + nbint >= itd->dtk_intsize))
    {
      unsigned newintsize = ((5 * itd->dtk_inttop / 4 + 3 + nbint) | 7) + 1;
      intptr_t *newints = MOM_GC_SCALAR_ALLOC ("tasklet grown integers",
					       newintsize *
					       sizeof (intptr_t));
      if (itd->dtk_inttop > 0)
	memcpy (newints, itd->dtk_ints, itd->dtk_inttop * sizeof (intptr_t));
      MOM_GC_FREE (itd->dtk_ints);
      itd->dtk_ints = newints;
      itd->dtk_intsize = newintsize;
    };
  /// grow the doubles if needed
  if (MOM_UNLIKELY (nbdbl > 0 && itd->dtk_dbltop + nbint > itd->dtk_dblsize))
    {
      unsigned newdblsize = ((5 * itd->dtk_dbltop / 4 + 3 + nbdbl) | 7) + 1;
      double *newdbls = MOM_GC_SCALAR_ALLOC ("tasklet grown doubles",
					     newdblsize * sizeof (double));
      if (itd->dtk_dbltop > 0)
	memcpy (newdbls, itd->dtk_doubles, itd->dtk_dbltop * sizeof (double));
      MOM_GC_FREE (itd->dtk_doubles);
      itd->dtk_doubles = newdbls;
      itd->dtk_dblsize = newdblsize;
    };
  /// grow the values if needed
  if (MOM_UNLIKELY (itd->dtk_valtop + nbval > itd->dtk_valsize))
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
      momval_t *newclosurevals = MOM_GC_ALLOC ("tasklet grown closure vals",
					       sizeof (momval_t) *
					       newfrasize);
      memcpy (newframes, itd->dtk_frames,
	      sizeof (struct momframe_st) * itd->dtk_fratop);
      memcpy (newclosurevals, itd->dtk_closurevals,
	      sizeof (momval_t) * itd->dtk_fratop);
      MOM_GC_FREE (itd->dtk_frames);
      MOM_GC_FREE (itd->dtk_closurevals);
      itd->dtk_frames = newframes;
      itd->dtk_closurevals = newclosurevals;
      itd->dtk_frasize = newfrasize;
    }
  struct momframe_st *newframe = itd->dtk_frames + itd->dtk_fratop;
  newframe->fr_state = state;
  unsigned froint = newframe->fr_intoff = itd->dtk_inttop;
  unsigned frodbl = newframe->fr_dbloff = itd->dtk_dbltop;
  unsigned froval = newframe->fr_valoff = itd->dtk_valtop;
  unsigned fratop = itd->dtk_fratop;
  if (nbint > 0)
    {
      memset (itd->dtk_ints + itd->dtk_inttop, 0, nbint * sizeof (intptr_t));
      itd->dtk_inttop += nbint;
    }
  if (nbdbl > 0)
    {
      memset (itd->dtk_doubles + itd->dtk_dbltop, 0, nbdbl * sizeof (double));
      itd->dtk_dbltop += nbdbl;
    }
  if (nbval > 0)
    {
      memset (itd->dtk_values + itd->dtk_valtop, 0,
	      nbval * sizeof (momval_t));
      itd->dtk_valtop += nbval;
    }
  itd->dtk_closurevals[fratop] = clo;
  itd->dtk_fratop = fratop + 1;
  va_start (alist, firstdir);
  fill_frame_data_mom ((nbint > 0) ? (itd->dtk_ints + froint) : NULL,
		       (nbdbl > 0) ? (itd->dtk_doubles + frodbl) : NULL,
		       (nbval > 0) ? (itd->dtk_values + froval) : NULL,
		       firstdir, alist);
  va_end (alist);
}


void
mom_item_tasklet_replace_top_frame (momitem_t *itm, momval_t clo,
				    enum mom_pushframedirective_en firstdir,
				    ...)
{
  unsigned nbval = 0, nbint = 0, nbdbl = 0;
  int state = 0;
  va_list alist;
  assert (itm && itm->i_typnum == momty_item);
  if (!clo.ptr || (*clo.ptype != momty_item && *clo.ptype != momty_node))
    return;
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return;
  struct mom_taskletdata_st *itd = itm->i_payload;
  if (itd->dtk_fratop == 0)
    return;
  va_start (alist, firstdir);
  if (!compute_pushed_data_size_mom (clo,
				     &nbval, &nbint, &nbdbl,
				     &state, firstdir, alist))
    return;
  va_end (alist);
  // pop the old top frame
  struct momframe_st *prevframe = itd->dtk_frames + itd->dtk_fratop - 1;
  unsigned ofpint = prevframe->fr_intoff;
  unsigned ofpdbl = prevframe->fr_dbloff;
  unsigned ofpvalu = prevframe->fr_valoff;
  if (itd->dtk_inttop > ofpint)
    memset (itd->dtk_ints + ofpint, 0,
	    (itd->dtk_inttop - ofpint) * sizeof (intptr_t));
  if (itd->dtk_dbltop > ofpdbl)
    memset (itd->dtk_doubles + ofpdbl, 0,
	    (itd->dtk_dbltop - ofpdbl) * sizeof (double));
  if (itd->dtk_valtop > ofpvalu)
    memset (itd->dtk_values + ofpvalu, 0,
	    (itd->dtk_valtop - ofpvalu) * sizeof (momval_t));
  itd->dtk_inttop = ofpint;
  itd->dtk_dbltop = ofpdbl;
  itd->dtk_valtop = ofpvalu;
  memset (prevframe, 0, sizeof (struct momframe_st));
  itd->dtk_closurevals[itd->dtk_fratop - 1] = MOM_NULLV;
  itd->dtk_fratop--;
  unsigned fratop = itd->dtk_fratop;
  /// grow the ints if needed
  if (MOM_UNLIKELY (nbint > 0 && itd->dtk_inttop + nbint >= itd->dtk_intsize))
    {
      unsigned newintsize = ((5 * itd->dtk_inttop / 4 + nbint + 3) | 7) + 1;
      intptr_t *newints = MOM_GC_SCALAR_ALLOC ("tasklet grown integers",
					       newintsize *
					       sizeof (intptr_t));
      if (itd->dtk_inttop > 0)
	memcpy (newints, itd->dtk_ints, itd->dtk_inttop * sizeof (intptr_t));
      MOM_GC_FREE (itd->dtk_ints);
      itd->dtk_ints = newints;
      itd->dtk_intsize = newintsize;
    };
  /// grow the doubles if needed
  if (MOM_UNLIKELY (nbdbl > 0 && itd->dtk_dbltop + nbdbl >= itd->dtk_dblsize))
    {
      unsigned newdblsize = ((5 * itd->dtk_dbltop / 4 + nbdbl + 3) | 7) + 1;
      double *newdbls = MOM_GC_SCALAR_ALLOC ("tasklet grown doubles",
					     newdblsize * sizeof (double));
      if (itd->dtk_dbltop > 0)
	memcpy (newdbls, itd->dtk_doubles, itd->dtk_dbltop * sizeof (double));
      MOM_GC_FREE (itd->dtk_doubles);
      itd->dtk_doubles = newdbls;
      itd->dtk_dblsize = newdblsize;
    };
  /// grow the values if needed
  if (MOM_UNLIKELY (nbval > 0 && itd->dtk_valtop + nbval >= itd->dtk_valsize))
    {
      unsigned newvalsize = ((5 * itd->dtk_valtop / 4 + nbval + 6) | 7) + 1;
      momval_t *newvalues =
	MOM_GC_ALLOC ("tasklet grown values", newvalsize * sizeof (momval_t));
      if (itd->dtk_valtop)
	memcpy (newvalues, itd->dtk_values,
		itd->dtk_valtop * sizeof (momval_t));
      MOM_GC_FREE (itd->dtk_values);
      itd->dtk_values = newvalues;
      itd->dtk_valsize = newvalsize;
    }
  /// dont need to grow frames or closures, they are sure to fit
  struct momframe_st *newframe = itd->dtk_frames + itd->dtk_fratop;
  unsigned froint = newframe->fr_intoff = itd->dtk_inttop;
  unsigned frodbl = newframe->fr_dbloff = itd->dtk_dbltop;
  unsigned froval = newframe->fr_valoff = itd->dtk_valtop;
  fratop = itd->dtk_fratop;
  if (nbint > 0)
    {
      memset (itd->dtk_ints + itd->dtk_inttop, 0, nbint * sizeof (intptr_t));
      itd->dtk_inttop += nbint;
    }
  if (nbdbl > 0)
    {
      memset (itd->dtk_doubles + itd->dtk_dbltop, 0, nbdbl * sizeof (double));
      itd->dtk_dbltop += nbdbl;
    }
  if (nbval > 0)
    {
      memset (itd->dtk_values + itd->dtk_valtop, 0,
	      nbval * sizeof (momval_t));
      itd->dtk_valtop += nbval;
    }
  itd->dtk_closurevals[fratop] = clo;
  itd->dtk_fratop = fratop + 1;
  va_start (alist, firstdir);
  fill_frame_data_mom ((nbint > 0) ? (itd->dtk_ints + froint) : NULL,
		       (nbdbl > 0) ? (itd->dtk_doubles + frodbl) : NULL,
		       (nbval > 0) ? (itd->dtk_values + froval) : NULL,
		       firstdir, alist);
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
  unsigned ofpint = prevframe->fr_intoff;
  unsigned ofpdbl = prevframe->fr_dbloff;
  unsigned ofpvalu = prevframe->fr_valoff;
  if (itd->dtk_inttop > ofpint)
    memset (itd->dtk_ints + ofpint, 0,
	    (itd->dtk_inttop - ofpint) * sizeof (intptr_t));
  if (itd->dtk_dbltop > ofpdbl)
    memset (itd->dtk_doubles + ofpdbl, 0,
	    (itd->dtk_dbltop - ofpdbl) * sizeof (double));
  if (itd->dtk_valtop > ofpvalu)
    memset (itd->dtk_values + ofpvalu, 0,
	    (itd->dtk_valtop - ofpvalu) * sizeof (momval_t));
  itd->dtk_inttop = ofpint;
  itd->dtk_dbltop = ofpdbl;
  itd->dtk_valtop = ofpvalu;
  memset (prevframe, 0, sizeof (struct momframe_st));
  itd->dtk_closurevals[fratop - 1] = MOM_NULLV;
  itd->dtk_fratop = fratop - 1;
  /// shrink perhaps the integers
  if (MOM_UNLIKELY (itd->dtk_intsize > TASKLET_THRESHOLD
		    && 2 * ofpint < itd->dtk_intsize))
    {
      unsigned newintsize = ((5 * ofpint / 4 + 3) | 7) + 1;
      intptr_t *newints = MOM_GC_SCALAR_ALLOC ("tasklet shrink integers",
					       newintsize *
					       sizeof (intptr_t));
      if (ofpint > 0)
	memcpy (newints, itd->dtk_ints, ofpint * sizeof (intptr_t));
      MOM_GC_FREE (itd->dtk_ints);
      itd->dtk_ints = newints;
      itd->dtk_intsize = newintsize;
    }
  /// shrink perhaps the doubles
  if (MOM_UNLIKELY (itd->dtk_dblsize > TASKLET_THRESHOLD
		    && 2 * ofpdbl < itd->dtk_dblsize))
    {
      unsigned newdblsize = ((5 * ofpdbl / 4 + 3) | 7) + 1;
      double *newdbls = MOM_GC_SCALAR_ALLOC ("tasklet shrink doubles",
					     newdblsize * sizeof (double));
      if (ofpdbl > 0)
	memcpy (newdbls, itd->dtk_doubles, ofpint * sizeof (double));
      MOM_GC_FREE (itd->dtk_doubles);
      itd->dtk_doubles = newdbls;
      itd->dtk_dblsize = newdblsize;
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
      momval_t *newclosurevals = MOM_GC_ALLOC ("tasklet shrink closureval",
					       sizeof (momval_t) *
					       newfrasize);
      memcpy (newframes, itd->dtk_frames,
	      sizeof (struct momframe_st) * fratop);
      memcpy (newclosurevals, itd->dtk_closurevals,
	      sizeof (momval_t) * fratop);
      MOM_GC_FREE (itd->dtk_frames);
      MOM_GC_FREE (itd->dtk_closurevals);
      itd->dtk_frames = newframes;
      itd->dtk_closurevals = newclosurevals;
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
    return itd->dtk_closurevals[frk];
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


momval_t *
mom_item_tasklet_frame_values_pointer (momitem_t *itm, int frk)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return NULL;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return NULL;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  return itd->dtk_values + curfram->fr_valoff;
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
mom_item_tasklet_frame_nb_ints (momitem_t *itm, int frk)
{
  unsigned nbint = 0;
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
      nbint = nxtfram->fr_intoff - curfram->fr_intoff;
    }
  else
    nbint = itd->dtk_inttop - curfram->fr_intoff;
  return nbint;
}

intptr_t
mom_item_tasklet_frame_nth_int (momitem_t *itm, int frk, int nrk)
{
  unsigned nbint = 0;
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
      nbint = nxtfram->fr_intoff - curfram->fr_intoff;
    }
  else
    nbint = itd->dtk_inttop - curfram->fr_intoff;
  if (nrk < 0)
    nrk += nbint;
  if (nrk >= 0 && nrk < (int) nbint)
    return itd->dtk_ints[curfram->fr_intoff + nrk];
  return 0;
}

intptr_t *
mom_item_tasklet_frame_ints_pointer (momitem_t *itm, int frk)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return NULL;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return NULL;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  return itd->dtk_ints + curfram->fr_intoff;
}

unsigned
mom_item_tasklet_frame_nb_doubles (momitem_t *itm, int frk)
{
  unsigned nbdbl = 0;
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
      nbdbl = nxtfram->fr_dbloff - curfram->fr_dbloff;
    }
  else
    nbdbl = itd->dtk_dbltop - curfram->fr_dbloff;
  return nbdbl;
}



double *
mom_item_tasklet_frame_doubles_pointer (momitem_t *itm, int frk)
{
  assert (itm && itm->i_typnum == momty_item);
  if (!itm->i_payload || itm->i_paylkind != mompayk_tasklet)
    return NULL;
  struct mom_taskletdata_st *itd = itm->i_payload;
  unsigned fratop = itd->dtk_fratop;
  if (frk < 0)
    frk += fratop;
  if (frk < 0 || frk >= (int) fratop)
    return NULL;
  struct momframe_st *curfram = itd->dtk_frames + frk;
  return itd->dtk_doubles + curfram->fr_dbloff;
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
      nbdbl = nxtfram->fr_dbloff - curfram->fr_dbloff;
    }
  else
    nbdbl = itd->dtk_dbltop - curfram->fr_dbloff;
  if (drk < 0)
    drk += nbdbl;
  if (drk >= 0 && drk < (int) nbdbl)
    return itd->dtk_doubles[curfram->fr_dbloff + drk];
  return 0.0;
}


static void
payl_tasklet_dump_scan_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  assert (du != NULL);
  assert (itm != NULL && itm->i_typnum == momty_item);
  assert (itm->i_paylkind == mompayk_tasklet && itm->i_payload);
  struct mom_taskletdata_st *itd = itm->i_payload;
  mom_dump_scan_value (du, itd->dtk_res1);
  mom_dump_scan_value (du, itd->dtk_res2);
  mom_dump_scan_value (du, itd->dtk_res3);
  unsigned valtop = itd->dtk_valtop;
  for (unsigned vix = 0; vix < valtop; vix++)
    mom_dump_scan_value (du, itd->dtk_values[vix]);
  unsigned fratop = itd->dtk_fratop;
  for (unsigned frix = 0; frix < fratop; frix++)
    mom_dump_scan_value (du, (momval_t) itd->dtk_closurevals[frix]);
}


static momval_t
payl_tasklet_dump_json_mom (struct mom_dumper_st *du, momitem_t *itm)
{
  momval_t jres = MOM_NULLV;
  momval_t jframes = MOM_NULLV;
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
      momval_t curclov = itd->dtk_closurevals[frix];
      unsigned nbint = 0, nbdbl = 0, nbval = 0;
      if (frix + 1 < fratop)
	{
	  struct momframe_st *nxtfram = itd->dtk_frames + frix + 1;
	  nbval = nxtfram->fr_valoff - curfram->fr_valoff;
	  nbint = nxtfram->fr_intoff - curfram->fr_intoff;
	  nbdbl = nxtfram->fr_dbloff - curfram->fr_dbloff;
	}
      else
	{
	  nbval = (itd->dtk_valtop - curfram->fr_valoff);
	  nbint = (itd->dtk_inttop - curfram->fr_intoff);
	  nbdbl = (itd->dtk_dbltop - curfram->fr_dbloff);
	}
      momval_t valtiny[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t inttiny[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t dbltiny[MOM_TINY_MAX] = { MOM_NULLV };
      momval_t *jvalarr = (nbval < MOM_TINY_MAX) ? valtiny
	: MOM_GC_ALLOC ("tasklet dump values frame",
			nbval * sizeof (momval_t));
      momval_t *jintarr =
	(nbint <
	 MOM_TINY_MAX) ? inttiny :
	MOM_GC_ALLOC ("tasklet dump integers frame",
		      nbint * sizeof (momval_t));
      momval_t *jdblarr =
	(nbdbl <
	 MOM_TINY_MAX) ? dbltiny : MOM_GC_ALLOC ("tasklet dump doubles frame",
						 nbdbl * sizeof (momval_t));
      for (unsigned vix = 0; vix < nbval; vix++)
	jvalarr[vix]
	  = mom_dump_emit_json (du,
				itd->dtk_values[curfram->fr_valoff + vix]);
      for (unsigned nix = 0; nix < nbint; nix++)
	jintarr[nix]
	  = mom_make_integer (itd->dtk_ints[curfram->fr_intoff + nix]);
      for (unsigned dix = 0; dix < nbdbl; dix++)
	jdblarr[dix]
	  = mom_make_double ((itd->dtk_doubles + curfram->fr_dbloff)[dix]);
      momval_t jints
	= (nbint > 0)
	? (momval_t) mom_make_json_array_count (nbint, jintarr) : MOM_NULLV;
      momval_t jdbls
	= (nbdbl > 0)
	? (momval_t) mom_make_json_array_count (nbdbl, jdblarr) : MOM_NULLV;
      momval_t jvals
	= (nbval > 0)
	? (momval_t) mom_make_json_array_count (nbval, jvalarr) : MOM_NULLV;
      momval_t jclos = mom_dump_emit_json (du, curclov);
      momval_t jstate = (momval_t) mom_make_integer (curfram->fr_state);
      momval_t jframe
	= (momval_t)
	mom_make_json_object
	(MOMJSOB_ENTRY ((momval_t) mom_named__closure, jclos),
	 MOMJSOB_ENTRY ((momval_t) mom_named__numbers, jints),
	 MOMJSOB_ENTRY ((momval_t) mom_named__doubles, jdbls),
	 MOMJSOB_ENTRY ((momval_t) mom_named__values, jvals),
	 MOMJSOB_ENTRY ((momval_t) mom_named__state, jstate),
	 MOMJSON_END);
      jarr[frix] = jframe;
      if (jvalarr != valtiny)
	MOM_GC_FREE (jvalarr);
      if (jintarr != inttiny)
	MOM_GC_FREE (jintarr);
      if (jdblarr != dbltiny)
	MOM_GC_FREE (jdblarr);
    }
  momval_t jres1 = mom_dump_emit_json (du, (momval_t) itd->dtk_res1);
  momval_t jres2 = mom_dump_emit_json (du, (momval_t) itd->dtk_res2);
  momval_t jres3 = mom_dump_emit_json (du, (momval_t) itd->dtk_res3);
  jframes =
    fratop ? (momval_t) mom_make_json_array_count (fratop, jarr) : MOM_NULLV;
  if (jarr != tinyarr)
    MOM_GC_FREE (jarr);
  jres = (momval_t) mom_make_json_object
    (MOMJSOB_ENTRY ((momval_t) mom_named__frames, jframes),
     MOMJSOB_ENTRY ((momval_t) mom_named__res1, jres1),
     MOMJSOB_ENTRY ((momval_t) mom_named__res2, jres2),
     MOMJSOB_ENTRY ((momval_t) mom_named__res3, jres3), MOMJSON_END);
  return jres;
}

static void
payl_tasklet_load_mom (struct mom_loader_st *ld, momitem_t *litm,
		       momval_t jtasklet)
{
  assert (ld != NULL);
  assert (litm != NULL && litm->i_typnum == momty_item);
  mom_item_start_tasklet (litm);
  momval_t jframes = mom_jsonob_get (jtasklet, (momval_t) mom_named__frames);
  momval_t jres1 = mom_jsonob_get (jtasklet, (momval_t) mom_named__res1);
  momval_t jres2 = mom_jsonob_get (jtasklet, (momval_t) mom_named__res2);
  momval_t jres3 = mom_jsonob_get (jtasklet, (momval_t) mom_named__res3);
  unsigned nbfra = mom_json_array_size (jframes);
  if (nbfra > 0)
    // a wild guess about the tasklet needs
    mom_item_tasklet_reserve (litm,
			      /*nbint: */ nbfra * 2,
			      /*nbdbl: */ (nbfra - 1) / 3,
			      /*nbval: */ nbfra * 3,
			      /*nbfram: */ nbfra + 1);
  for (unsigned frix = 0; frix < nbfra; frix++)
    {
      momval_t jfram = mom_json_array_nth (jframes, frix);
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
  {
    momval_t res1 = mom_load_value_json (ld, jres1);
    momval_t res2 = mom_load_value_json (ld, jres2);
    momval_t res3 = mom_load_value_json (ld, jres3);
    mom_item_tasklet_set_3res (litm, res1, res2, res3);
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
    {
      for (int ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	{
	  momval_t curval = qel->vqe_valtab[ix];
	  if (curval.ptr && cntchk < nbchk)
	    arrchk[cntchk++] = curval;
	}
    }
  jres = (momval_t) mom_make_json_array_count (cntchk, arrchk);
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

///
static const struct mom_payload_descr_st payldescr_jsonrpcexchange_mom = {
  .dpayl_magic = MOM_PAYLOAD_MAGIC,
  .dpayl_name = "jsonrpcexchange",
  .dpayl_finalizefun = mom_payljsonrpc_finalize
};

////////////////////////////////////////////////////////////////
/******************** payload descriptors *********************/

struct mom_payload_descr_st *mom_payloadescr[mompayk__last + 1] = {
  [mompayk_queue] = (struct mom_payload_descr_st *) &payldescr_queue_mom,
  [mompayk_tfunrout] =
    (struct mom_payload_descr_st *) &payldescr_tfunrout_mom,
  [mompayk_closure] = (struct mom_payload_descr_st *) &payldescr_closure_mom,
  [mompayk_tasklet] = (struct mom_payload_descr_st *) &payldescr_tasklet_mom,
  [mompayk_buffer] = (struct mom_payload_descr_st *) &payldescr_buffer_mom,
  [mompayk_vector] = (struct mom_payload_descr_st *) &payldescr_vector_mom,
  [mompayk_assoc] = (struct mom_payload_descr_st *) &payldescr_assoc_mom,
  [mompayk_hset] = (struct mom_payload_descr_st *) &payldescr_hset_mom,
  [mompayk_process] = (struct mom_payload_descr_st *) &payldescr_process_mom,
  [mompayk_procedure] =
    (struct mom_payload_descr_st *) &payldescr_procedure_mom,
  [mompayk_webexchange] =
    (struct mom_payload_descr_st *) &payldescr_webexchange_mom,
  [mompayk_jsonrpcexchange] =
    (struct mom_payload_descr_st *) &payldescr_jsonrpcexchange_mom,
};
