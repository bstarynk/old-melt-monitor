// file sequ.c - manage sequence of value components, or of items


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


void
mom_components_put_nth (struct momcomponents_st *csq, int rk,
			const momvalue_t val)
{
  if (!csq || csq == MOM_EMPTY)
    return;
  unsigned cnt = csq->cp_cnt;
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    csq->cp_comps[rk] = val;
}

void
mom_components_scan_dump (struct momcomponents_st *csq)
{
  if (!csq || csq == MOM_EMPTY)
    return;
  unsigned cnt = csq->cp_cnt;
  for (unsigned ix = 0; ix < cnt; ix++)
    mom_scan_dumped_valueptr (&csq->cp_comps[ix]);
}

struct momcomponents_st *
mom_components_append1 (struct momcomponents_st *csq, const momvalue_t val)
{
  if (csq == MOM_EMPTY)
    csq = NULL;
  unsigned cnt = csq ? csq->cp_cnt : 0;
  unsigned len = csq ? csq->cp_len : 0;
  assert (cnt <= len);
  if (MOM_UNLIKELY (cnt >= MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too huge components sequence %d values", cnt);
  if (MOM_UNLIKELY (!csq || cnt + 1 >= len))
    {
      unsigned newsiz = ((5 * cnt / 4 + 3) | 0xf) + 1;
      struct momcomponents_st *newcsq =	//
	MOM_GC_ALLOC ("new components seq",
		      sizeof (struct momcomponents_st) +
		      newsiz * sizeof (momvalue_t));
      newcsq->cp_len = newsiz;
      if (csq)
	memcpy (newcsq->cp_comps, csq->cp_comps, cnt * sizeof (momvalue_t));
      newcsq->cp_cnt = cnt;
      MOM_GC_FREE (csq,
		   sizeof (struct momcomponents_st) +
		   len * sizeof (momvalue_t));
      csq = newcsq;
    }
  csq->cp_comps[cnt++] = val;
  csq->cp_cnt = cnt;
  return csq;
}

struct momcomponents_st *
mom_components_append_values (struct momcomponents_st *csq, unsigned nbval,
			      ... /*values */ )
{
  va_list args;
  if (csq == MOM_EMPTY)
    csq = NULL;
  unsigned cnt = csq ? csq->cp_cnt : 0;
  unsigned len = csq ? csq->cp_len : 0;
  assert (cnt <= len);
  if (MOM_UNLIKELY (nbval == 0))
    return csq;
  if (MOM_UNLIKELY (cnt + nbval >= MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too many components : %d values", cnt);
  if (MOM_UNLIKELY (!csq || cnt + nbval >= len))
    {
      unsigned newsiz = ((5 * cnt / 4 + nbval + 3) | 0xf) + 1;
      struct momcomponents_st *newcsq =	//
	MOM_GC_ALLOC ("new components seq",
		      sizeof (struct momcomponents_st) +
		      newsiz * sizeof (momvalue_t));
      newcsq->cp_len = newsiz;
      if (csq)
	memcpy (newcsq->cp_comps, csq->cp_comps, cnt * sizeof (momvalue_t));
      newcsq->cp_cnt = cnt;
      MOM_GC_FREE (csq,
		   sizeof (struct momcomponents_st) +
		   len * sizeof (momvalue_t));
      csq = newcsq;
    }
  va_start (args, nbval);
  for (unsigned ix = 0; ix < nbval; ix++)
    csq->cp_comps[cnt++] = va_arg (args, momvalue_t);
  va_end (args);
  csq->cp_cnt = cnt;
  return csq;
}

struct momcomponents_st *
mom_components_append_sized_array (struct momcomponents_st *csq,
				   unsigned nbval, const momvalue_t *valarr)
{
  if (csq == MOM_EMPTY)
    csq = NULL;
  if (!nbval || !valarr)
    return csq;
  unsigned cnt = csq ? csq->cp_cnt : 0;
  unsigned len = csq ? csq->cp_len : 0;
  assert (cnt <= len);
  if (MOM_UNLIKELY (cnt + nbval >= MOM_MAX_SEQ_LENGTH))
    MOM_FATAPRINTF ("too many components : %d values", cnt);
  if (MOM_UNLIKELY (!csq || cnt + nbval >= len))
    {
      unsigned newsiz = ((5 * cnt / 4 + nbval + 3) | 0xf) + 1;
      struct momcomponents_st *newcsq =	//
	MOM_GC_ALLOC ("new components seq",
		      sizeof (struct momcomponents_st) +
		      newsiz * sizeof (momvalue_t));
      newcsq->cp_len = newsiz;
      if (csq)
	memcpy (newcsq->cp_comps, csq->cp_comps, cnt * sizeof (momvalue_t));
      newcsq->cp_cnt = cnt;
      MOM_GC_FREE (csq,
		   sizeof (struct momcomponents_st) +
		   len * sizeof (momvalue_t));
      csq = newcsq;
    }
  for (unsigned ix = 0; ix < nbval; ix++)
    csq->cp_comps[cnt++] = valarr[ix];
  return csq;
}


struct momcomponents_st *
mom_components_reserve (struct momcomponents_st *csq, unsigned nbcomp)
{
  if (!nbcomp)
    return csq;
  unsigned cnt = csq ? csq->cp_cnt : 0;
  unsigned len = csq ? csq->cp_len : 0;
  if (nbcomp > len)
    {
      unsigned newsiz = ((nbcomp + len / 8 + 2) | 0xf) + 1;
      struct momcomponents_st *newcsq =	//
	MOM_GC_ALLOC ("new components seq",
		      sizeof (struct momcomponents_st) +
		      newsiz * sizeof (momvalue_t));
      if (cnt > 0)
	memcpy (newcsq->cp_comps, csq->cp_comps, cnt * sizeof (momvalue_t));
      newcsq->cp_len = newsiz;
      newcsq->cp_cnt = nbcomp;
      MOM_GC_FREE (csq,
		   sizeof (struct momcomponents_st) +
		   len * sizeof (momvalue_t));
      return newcsq;
    };
  assert (len >= cnt);
  if (len > cnt)
    memset (csq->cp_comps + cnt, 0, (len - cnt) * sizeof (momvalue_t));
  csq->cp_len = nbcomp;
  return csq;
}


void
mom_itemvec_put_nth (struct momitemvec_st *ivec, int rk, const momitem_t *itm)
{
  if (!ivec || ivec == MOM_EMPTY)
    return;
  if (!itm || itm == MOM_EMPTY)
    return;
  unsigned cnt = ivec->ivec_cnt;
  if (rk < 0)
    rk += cnt;
  if (rk >= 0 && rk < (int) cnt)
    ivec->ivec_arr[rk] = itm;
}

struct momitemvec_st *
mom_itemvec_reserve (struct momitemvec_st *ivec, unsigned gap)
{
  if (ivec == MOM_EMPTY)
    ivec = NULL;
  unsigned cnt = ivec ? ivec->ivec_cnt : 0;
  unsigned len = ivec ? ivec->ivec_len : 0;
  assert (cnt <= len);
  if (cnt + gap >= len)
    {
      unsigned newsiz = ((cnt + gap + cnt / 16 + 10) | 0xf) + 1;
      assert (newsiz > 1);
      if (newsiz > len)
	{
	  struct momitemvec_st *newivec =	//
	    MOM_GC_ALLOC ("newivec",
			  sizeof (struct momitemvec_st) +
			  newsiz * sizeof (momitem_t *));
	  newivec->ivec_len = newsiz;
	  if (cnt > 0)
	    {
	      memcpy (newivec->ivec_arr, ivec->ivec_arr,
		      cnt * sizeof (momitem_t *));
	      newivec->ivec_cnt = cnt;
	    }
	  struct momitemvec_st *oldivec = ivec;
	  ivec = newivec;
	  MOM_GC_FREE (oldivec,
		       sizeof (struct momitemvec_st) +
		       len * sizeof (momitem_t *));
	}
    }
  return ivec;
}				// end of mom_itemvec_reserve

struct momitemvec_st *
mom_itemvec_append1 (struct momitemvec_st *ivec, const momitem_t *itm)
{
  if (ivec == MOM_EMPTY)
    ivec = NULL;
  if (!itm || itm == MOM_EMPTY)
    return ivec;
  unsigned cnt = ivec ? ivec->ivec_cnt : 0;
  unsigned len = ivec ? ivec->ivec_len : 0;
  if (cnt >= len)
    {
      ivec = mom_itemvec_reserve (ivec, 1 + cnt / 64);
      len = ivec->ivec_len;
    }
  ivec->ivec_arr[cnt] = itm;
  ivec->ivec_cnt = cnt + 1;
  return ivec;
}				// end of mom_itemvec_append1

struct momitemvec_st *
mom_itemvec_append_items (struct momitemvec_st *ivec, unsigned nbitems, ...)
{
  va_list args;
  if (ivec == MOM_EMPTY)
    ivec = NULL;
  if (!nbitems)
    return ivec;
  unsigned cnt = ivec ? ivec->ivec_cnt : 0;
  unsigned len = ivec ? ivec->ivec_len : 0;
  if (cnt + nbitems >= len)
    {
      ivec = mom_itemvec_reserve (ivec, nbitems + cnt / 64 + 1);
      len = ivec->ivec_len;
    }
  va_start (args, nbitems);
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      if (!itm || itm == MOM_EMPTY)
	continue;
      ivec->ivec_arr[cnt++] = itm;
    };
  va_end (args);
  ivec->ivec_cnt = cnt;
  return ivec;
}				/* end of mom_itemvec_append_items */



struct momitemvec_st *
mom_itemvec_append_sized_item_array (struct momitemvec_st *ivec,
				     unsigned nbitems,
				     const momitem_t **itemarr)
{
  if (ivec == MOM_EMPTY)
    ivec = NULL;
  if (!nbitems || !itemarr)
    return ivec;
  unsigned cnt = ivec ? ivec->ivec_cnt : 0;
  unsigned len = ivec ? ivec->ivec_len : 0;
  if (cnt + nbitems >= len)
    {
      ivec = mom_itemvec_reserve (ivec, nbitems + cnt / 64 + 1);
      assert (ivec != NULL);
      len = ivec->ivec_len;
    }
  assert (ivec != NULL && ivec->ivec_cnt + nbitems <= ivec->ivec_len);
  for (unsigned ix = 0; ix < nbitems; ix++)
    {
      const momitem_t *itm = itemarr[ix];
      if (!itm || itm == MOM_EMPTY)
	continue;
      ivec->ivec_arr[cnt++] = itm;
    };
  ivec->ivec_cnt = cnt;
  return ivec;
}				/* end of mom_itemvec_append_sized_item_array */
