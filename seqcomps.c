// file seqcomps.c


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
mom_components_scan_dump (struct momcomponents_st *csq,
			  struct momdumper_st *du)
{
  if (!csq || csq == MOM_EMPTY)
    return;
  assert (du);
  unsigned cnt = csq->cp_cnt;
  for (unsigned ix = 0; ix < cnt; ix++)
    mom_scan_dumped_value (du, csq->cp_comps[ix]);
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
