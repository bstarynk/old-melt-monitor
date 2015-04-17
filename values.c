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

////////////////////////////////////////////////////////////////
////// tuples

static void
update_tuple_hash_mom (momseq_t *tup)
{
  assert (tup && tup->shash == 0);
  momhash_t h1 = 13, h2 = 1019;
  momhash_t h = 0;
  unsigned tlen = tup->slen;
  for (unsigned ix = 0; ix + 1 < tlen; ix += 2)
    {
      momhash_t hitm1 = mom_item_hash (tup->arritm[ix]);
      assert (hitm1 != 0);
      h1 = ((13 * h1) ^ (2027 * hitm1)) + ix;
      momhash_t hitm2 = mom_item_hash (tup->arritm[ix + 1]);
      assert (hitm2 != 0);
      h2 = ((31 * h2) ^ (1049 * hitm2)) - (unsigned) ix;
    }
  if (tlen % 2)
    h1 = (211 * h1) ^ (17 * mom_item_hash (tup->arritm[tlen - 1]));
  h = h1 ^ h2;
  if (MOM_UNLIKELY (!h))
    h = h1;
  if (MOM_UNLIKELY (!h))
    h = h2;
  if (MOM_UNLIKELY (!h))
    h = ((tlen * 11) & 0xffffff) + 13;
  tup->shash = h;
}


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
  update_tuple_hash_mom (tup);
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
  update_tuple_hash_mom (tup);
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


static void
update_set_hash_mom (momseq_t *set)
{
  assert (set && set->shash == 0);
  momhash_t h1 = 13, h2 = 1019;
  momhash_t h = 0;
  unsigned tlen = set->slen;
#ifndef NDEBUG
  for (unsigned ix = 1; ix < tlen; ix++)
    assert (mom_item_cmp (set->arritm[ix - 1], set->arritm[ix]) < 0);
#endif
  for (unsigned ix = 0; ix + 1 < tlen; ix += 2)
    {
      momhash_t hitm1 = mom_item_hash (set->arritm[ix]);
      assert (hitm1 != 0);
      h1 = ((17 * h1) ^ (2411 * hitm1)) + ix;
      momhash_t hitm2 = mom_item_hash (set->arritm[ix + 1]);
      assert (hitm2 != 0);
      h2 = ((37 * h2) ^ (2713 * hitm2)) - (unsigned) ix;
    }
  if (tlen % 2)
    h1 = (313 * h1) ^ (11 * mom_item_hash (set->arritm[tlen - 1]));
  h = h1 ^ h2;
  if (MOM_UNLIKELY (!h))
    h = h1;
  if (MOM_UNLIKELY (!h))
    h = h2;
  if (MOM_UNLIKELY (!h))
    h = ((tlen * 17) & 0xffffff) + 143;
  set->shash = h;
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
  set->slen = cntitems;
  update_set_hash_mom (set);
  set->meta = metav;
  return set;
}

const momseq_t *
mom_make_sized_meta_set (momvalue_t metav, unsigned nbitems,
			 momitem_t **itmarr)
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
      momitem_t *itm = itmarr[ix];
      if (itm && itm != MOM_EMPTY)
	set->arritm[cntitems++] = itm;
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
  set->slen = cntitems;
  update_set_hash_mom (set);
  set->meta = metav;
  return set;
}
