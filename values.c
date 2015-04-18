// file values.c

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

momhash_t
mom_valueptr_hash (momvalue_t *pval)
{
  if (!pval)
    return 0;
  momhash_t h = 0;
  switch ((momvaltype_t) pval->typnum)
    {
    case momty_null:
      return 0;
    case momty_int:
      {
	intptr_t i = pval->vint;
	h = ((momhash_t) i ^ ((momhash_t) (i >> 27)));
	if (!h)
	  {
	    h = (momhash_t) i;
	    if (!h)
	      h = 12753;
	  }
	return h;
      }
    case momty_double:
      {
	double d = pval->vdbl;
	if (isnan (d))
	  return 3000229;
	int e = 0;
	double x = frexp (d, &e);
	h = ((momhash_t) (x / (M_PI * M_LN2 * DBL_EPSILON))) ^ e;
	if (!h)
	  {
	    h = e;
	    if (!h)
	      h = (x > 0.0) ? 1689767 : (x < 0.0) ? 2000281 : 13;
	  }
	return h;
      }
    case momty_string:
      {
	const momstring_t *ps = pval->vstr;
	assert (ps);
	return ps->shash;
      }
    case momty_set:
      {
	const momseq_t *ps = pval->vset;
	assert (ps);
	h = (11 * ps->shash) ^ (31 * ps->slen);
	if (MOM_UNLIKELY (!h))
	  h = ((17 * ps->slen) & 0xfffff) + 3;
	return h;
      }
    case momty_tuple:
      {
	const momseq_t *pt = pval->vtuple;
	assert (pt);
	h = (19 * pt->shash) ^ (59 * pt->slen);
	if (MOM_UNLIKELY (!h))
	  h = ((37 * pt->slen) & 0xfffff) + 10;
	return h;
      }
    case momty_node:
      {
	const momnode_t *pn = pval->vnode;
	assert (pn);
	return pn->shash;
      }
    case momty_item:
      {
	const momitem_t *pitm = pval->vitem;
	assert (pitm);
	assert (pitm->itm_str);
	return pitm->itm_str->shash;
      }
    }
  return 0;
}

/********************* strings ********************/
momhash_t
mom_cstring_hash_len (const char *str, int len)
{
  if (!str)
    return 0;
  if (len < 0)
    len = strlen (str);
  int l = len;
  momhash_t h1 = 0, h2 = len, h;
  while (l > 4)
    {
      h1 =
	(509 * h2 +
	 307 * ((signed char *) str)[0]) ^ (1319 * ((signed char *) str)[1]);
      h2 =
	(17 * l + 5 + 5309 * h2) ^ ((3313 * ((signed char *) str)[2]) +
				    9337 * ((signed char *) str)[3] + 517);
      l -= 4;
      str += 4;
    }
  if (l > 0)
    {
      h1 = (h1 * 7703) ^ (503 * ((signed char *) str)[0]);
      if (l > 1)
	{
	  h2 = (h2 * 7717) ^ (509 * ((signed char *) str)[1]);
	  if (l > 2)
	    {
	      h1 = (h1 * 9323) ^ (11 + 523 * ((signed char *) str)[2]);
	      if (l > 3)
		{
		  h2 =
		    (h2 * 7727 + 127) ^ (313 +
					 547 * ((signed char *) str)[3]);
		}
	    }
	}
    }
  h = (h1 * 29311 + 59) ^ (h2 * 7321 + 120501);
  if (!h)
    {
      h = h1;
      if (!h)
	{
	  h = h2;
	  if (!h)
	    h = (len & 0xffffff) + 11;
	}
    }
  return h;
}


const momstring_t *
mom_make_string (const char *str)
{
  if (!str)
    return NULL;
  unsigned slen = strlen (str);
  if (MOM_UNLIKELY (slen > MOM_MAX_STRING_LENGTH))
    MOM_FATAPRINTF ("too long %d string to make %.50s", slen, str);
  if (MOM_UNLIKELY (u8_check ((const uint8_t *) str, (size_t) slen) != NULL))
    MOM_FATAPRINTF ("invalid UTF8 in %d-sized string %.50s", slen, str);
  momstring_t *res = GC_MALLOC_ATOMIC (sizeof (momstring_t) + slen + 1);
  if (MOM_UNLIKELY (!res))
    MOM_FATAPRINTF ("failed to allocate string of %d bytes", slen);
  memset (res, 0, sizeof (momstring_t) + slen + 1);
  res->slen = slen;
  res->shash = mom_cstring_hash_len (str, slen);
  memcpy (res->cstr, str, slen);
  return res;
}

//// common for tuples & sets
static void
update_seq_hash_mom (momseq_t *seq)
{
  assert (seq && seq->shash == 0);
  momhash_t h1 = 13, h2 = 1019;
  momhash_t h = 0;
  unsigned tlen = seq->slen;
  for (unsigned ix = 0; ix + 1 < tlen; ix += 2)
    {
      momhash_t hitm1 = mom_item_hash (seq->arritm[ix]);
      assert (hitm1 != 0);
      h1 = ((13 * h1) ^ (2027 * hitm1)) + ix;
      momhash_t hitm2 = mom_item_hash (seq->arritm[ix + 1]);
      assert (hitm2 != 0);
      h2 = ((31 * h2) ^ (1049 * hitm2)) - (unsigned) ix;
    }
  if (tlen % 2)
    h1 = (211 * h1) ^ (17 * mom_item_hash (seq->arritm[tlen - 1]));
  h = h1 ^ h2;
  if (MOM_UNLIKELY (!h))
    h = h1;
  if (MOM_UNLIKELY (!h))
    h = h2;
  if (MOM_UNLIKELY (!h))
    h = ((tlen * 11) & 0xffffff) + 13;
  seq->shash = h;
}

////////////////////////////////////////////////////////////////
////// tuples


const momseq_t *
mom_make_meta_tuple (momvalue_t metav, unsigned nbitems, ...)
{
  va_list args;
  if (MOM_UNLIKELY (nbitems > MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too big tuple %u", nbitems);
  momseq_t *tup = MOM_GC_ALLOC ("tuple",
				sizeof (momseq_t) +
				nbitems * sizeof (momitem_t *));
  unsigned cntitems = 0;
  va_start (args, nbitems);
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      if (itm && itm != MOM_EMPTY)
	tup->arritm[cntitems++] = itm;
    }
  va_end (args);
  if (MOM_UNLIKELY (cntitems < nbitems))
    {
      momseq_t *oldtup = tup;
      momseq_t *newtup = MOM_GC_ALLOC ("tuple",
				       sizeof (momseq_t) +
				       cntitems * sizeof (momitem_t *));
      memcpy (newtup->arritm, oldtup->arritm,
	      cntitems * sizeof (momitem_t *));
      tup = newtup;
      MOM_GC_FREE (oldtup);
    }
  tup->slen = cntitems;
  update_seq_hash_mom (tup);
  tup->meta = metav;
  return tup;
}

const momseq_t *
mom_make_sized_meta_tuple (momvalue_t metav, unsigned nbitems,
			   momitem_t **itmarr)
{
  if (MOM_UNLIKELY (nbitems && !itmarr))
    MOM_FATAPRINTF ("missing item array for sized %u meta tuple", nbitems);
  if (MOM_UNLIKELY (nbitems > MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too big tuple %u", nbitems);
  momseq_t *tup = MOM_GC_ALLOC ("tuple",
				sizeof (momseq_t) +
				nbitems * sizeof (momitem_t *));
  unsigned cntitems = 0;
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      momitem_t *itm = itmarr[ix];
      if (itm && itm != MOM_EMPTY)
	tup->arritm[cntitems++] = itm;
    }
  if (MOM_UNLIKELY (cntitems < nbitems))
    {
      momseq_t *oldtup = tup;
      momseq_t *newtup = MOM_GC_ALLOC ("shrinked tuple",
				       sizeof (momseq_t) +
				       cntitems * sizeof (momitem_t *));
      memcpy (newtup->arritm, oldtup->arritm,
	      cntitems * sizeof (momitem_t *));
      tup = newtup;
      MOM_GC_FREE (oldtup);
    }
  tup->slen = cntitems;
  update_seq_hash_mom (tup);
  tup->meta = metav;
  return tup;
}


////////////////////////////////////////////////////////////////
////// sets
static unsigned
sort_set_unique_items_mom (momitem_t **itmarr, unsigned nbitems)
{
  if (MOM_UNLIKELY (!itmarr || !nbitems))
    return 0;
  assert (itmarr);
  if (MOM_UNLIKELY (nbitems == 1))
    return itmarr[0] ? 1 : 0;
  if (MOM_UNLIKELY (nbitems == 2))
    {
      if (MOM_UNLIKELY (!itmarr[0] && !itmarr[1]))
	return 0;
      if (MOM_UNLIKELY (!itmarr[1]))
	return 1;
      if (MOM_UNLIKELY (!itmarr[0]))
	{
	  itmarr[0] = itmarr[1];
	  return 1;
	};
      int cmp = mom_item_cmp (itmarr[0], itmarr[1]);
      if (!cmp)
	{
	  return 1;
	};
      if (cmp > 0)
	{
	  momitem_t *tmpitm = itmarr[0];
	  itmarr[0] = itmarr[1];
	  itmarr[1] = tmpitm;
	};
      return 2;
    }
  mom_item_qsort (itmarr, nbitems);
  for (unsigned ix = 1; ix < nbitems; ix++)
    {
      momitem_t *previtm = itmarr[ix - 1];
      if (MOM_UNLIKELY (itmarr[ix] == previtm))
	{
	  unsigned nextix;
	  for (nextix = ix; nextix < nbitems; nextix++)
	    if (MOM_UNLIKELY (itmarr[nextix] != previtm))
	      break;
	  assert (nextix > ix);
	  memmove (itmarr + ix, itmarr + nextix,
		   (nextix - ix) * sizeof (momitem_t *));
	  nbitems -= (nextix - ix);
	}
    }
  return nbitems;
}



const momseq_t *
mom_make_meta_set (momvalue_t metav, unsigned nbitems, ...)
{
  va_list args;
  if (MOM_UNLIKELY (nbitems > MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too big set %u", nbitems);
  momseq_t *set = MOM_GC_ALLOC ("set",
				sizeof (momseq_t) +
				nbitems * sizeof (momitem_t *));
  unsigned cntitems = 0;
  va_start (args, nbitems);
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      if (itm && itm != MOM_EMPTY)
	set->arritm[cntitems++] = itm;
    }
  va_end (args);
  cntitems = sort_set_unique_items_mom (set->arritm, cntitems);
  if (MOM_UNLIKELY (cntitems < nbitems))
    {
      momseq_t *oldset = set;
      momseq_t *newset = MOM_GC_ALLOC ("shrinked set",
				       sizeof (momseq_t) +
				       cntitems * sizeof (momitem_t *));
      memcpy (newset->arritm, oldset->arritm,
	      cntitems * sizeof (momitem_t *));
      set = newset;
      MOM_GC_FREE (oldset);
    }
#ifndef NDEBUG
  for (unsigned ix = 1; ix < cntitems; ix++)
    assert (mom_item_cmp (set->arritm[ix - 1], set->arritm[ix]) < 0);
#endif
  set->slen = cntitems;
  update_seq_hash_mom (set);
  set->meta = metav;
  return set;
}

const momseq_t *
mom_make_sized_meta_set (momvalue_t metav, unsigned nbitems,
			 const momitem_t **itmarr)
{
  if (MOM_UNLIKELY (nbitems && !itmarr))
    MOM_FATAPRINTF ("missing item array for sized %u meta set", nbitems);
  if (MOM_UNLIKELY (nbitems > MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too big set %u", nbitems);
  momseq_t *set = MOM_GC_ALLOC ("set",
				sizeof (momseq_t) +
				nbitems * sizeof (momitem_t *));
  unsigned cntitems = 0;
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      const momitem_t *itm = itmarr[ix];
      if (itm && itm != MOM_EMPTY)
	set->arritm[cntitems++] = (momitem_t *) itm;
    }
  cntitems = sort_set_unique_items_mom (set->arritm, cntitems);
  if (MOM_UNLIKELY (cntitems < nbitems))
    {
      momseq_t *oldset = set;
      momseq_t *newset = MOM_GC_ALLOC ("shrinked set",
				       sizeof (momseq_t) +
				       cntitems * sizeof (momitem_t *));
      memcpy (newset->arritm, oldset->arritm,
	      cntitems * sizeof (momitem_t *));
      set = newset;
      MOM_GC_FREE (oldset);
    }
#ifndef NDEBUG
  for (unsigned ix = 1; ix < cntitems; ix++)
    assert (mom_item_cmp (set->arritm[ix - 1], set->arritm[ix]) < 0);
#endif
  set->slen = cntitems;
  update_seq_hash_mom (set);
  set->meta = metav;
  return set;
}

////////////////////////////////////////////////////////////////
////// nodes

static void
update_node_hash_mom (momnode_t *nod)
{
  momhash_t h = 0;
  assert (nod && nod->shash == 0);
  momhash_t h1 = 11 * mom_item_hash (nod->conn);
  momhash_t h2 = 337;
  unsigned nlen = nod->slen;
  for (unsigned ix = 0; ix + 1 < nlen; ix += 2)
    {
      h1 = ((h1 * 211) ^ (61 * mom_valueptr_hash (nod->arrsons + ix))) + ix;
      h2 =
	(h2 * 233) ^ ((73 * mom_valueptr_hash (nod->arrsons + ix + 1)) - ix);
    }
  if (nlen % 2)
    h1 =
      (163 * h1) ^ (73 * mom_valueptr_hash (nod->arrsons + nlen - 1) + nlen);
  h = h1 ^ h2;
  if (MOM_UNLIKELY (!h))
    h = h1;
  if (MOM_UNLIKELY (!h))
    h = h2;
  if (MOM_UNLIKELY (!h))
    h = ((nlen * 17) & 0xffffff) + 100;
  nod->shash = h;
}

const momnode_t *
mom_make_meta_node (momvalue_t metav, momitem_t *connitm, unsigned nbsons,
		    ...)
{
  va_list args;
  if (MOM_UNLIKELY (!connitm))
    return NULL;
  if (MOM_UNLIKELY (nbsons > MOM_MAX_NODE_LENGTH))
    MOM_FATAPRINTF ("too big node %u", nbsons);
  momnode_t *nod = MOM_GC_ALLOC ("node",
				 sizeof (momnode_t) +
				 nbsons * sizeof (momvalue_t));
  va_start (args, nbsons);
  for (unsigned ix = 0; ix < nbsons; ix++)
    {
      nod->arrsons[ix] = va_arg (args, momvalue_t);
    }
  va_end (args);
  nod->conn = connitm;
  nod->slen = nbsons;
  nod->meta = metav;
  update_node_hash_mom (nod);
  return nod;
}
