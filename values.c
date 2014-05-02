// file values.c

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


momhash_t
mom_value_hash (const momval_t v)
{
  momhash_t h = 0;
  if (!v.ptr)
    return 0;
  unsigned vtype = *v.ptype;
  switch (vtype)
    {
    case momty_int:
      {
	intptr_t i = v.pint->intval;
	h = ((momhash_t) i ^ ((momhash_t) (i >> 27)));
	if (!h)
	  {
	    h = (momhash_t) i;
	    if (!h)
	      h = 12753;
	  }
	return h;
      }
    case momty_float:
      {
	double d = v.pfloat->floval;
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
      return v.pstring->hash;
    case momty_jsonarray:
      return v.pjsonarr->hash;
    case momty_jsonobject:
      return v.pjsonobj->hash;
    case momty_node:
      return v.pnode->hash;
    case momty_tuple:
    case momty_set:
      return v.pseqitm->hash;
    default:
      if (vtype >= momty__itemlowtype)
	return v.panyitem->i_hash;
      else
	MONIMELT_FATAL ("unimplemented hash of val@%p of type #%d", v.ptr,
			vtype);
    }
}



int
mom_itemptr_cmp (const void *l, const void *r)
{
  return mom_item_cmp (*(const mom_anyitem_t **) l,
		       *(const mom_anyitem_t **) r);
}

int
mom_value_cmp (const momval_t l, const momval_t r)
{
  if (l.ptr == r.ptr)
    return 0;
  if (!l.ptr)
    return -1;
  if (!r.ptr)
    return 1;
  unsigned ltype = *l.ptype;
  unsigned rtype = *r.ptype;
  if (ltype < rtype)
    return -1;
  if (ltype > rtype)
    return 1;
  switch (ltype)
    {
    case momty_int:
      {
	intptr_t lint = l.pint->intval;
	intptr_t rint = r.pint->intval;
	if (lint < rint)
	  return -1;
	if (lint > rint)
	  return 1;
	return 0;
      }
    case momty_float:
      {
	double ldbl = l.pfloat->floval;
	double rdbl = r.pfloat->floval;
	if (ldbl < rdbl)
	  return -1;
	if (ldbl > rdbl)
	  return 1;
	if (ldbl == rdbl)
	  return 0;
	if (isnan (ldbl))
	  {
	    if (isnan (rdbl))
	      return 0;
	    else
	      return -1;
	  }
	if (isnan (rdbl))
	  {
	    if (isnan (ldbl))
	      return 0;
	    else
	      return 1;
	  }
	return 0;
      }
    case momty_string:
      {
	momusize_t llen = l.pstring->slen;
	momusize_t rlen = r.pstring->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	return memcmp (l.pstring->cstr, r.pstring->cstr, minlen);
      }
    case momty_jsonarray:
      {
	momusize_t llen = l.pjsonarr->slen;
	momusize_t rlen = r.pjsonarr->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	unsigned ix = 0;
	for (ix = 0; ix < minlen; ix++)
	  {
	    int cmp = mom_value_cmp (l.pjsonarr->jarrtab[ix],
				     r.pjsonarr->jarrtab[ix]);
	    if (cmp)
	      return cmp;
	  }
	if (llen < rlen)
	  return -1;
	else if (llen > rlen)
	  return 1;
	return 0;
      }
    case momty_jsonobject:
      {
	momusize_t llen = l.pjsonobj->slen;
	momusize_t rlen = r.pjsonobj->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	unsigned ix = 0;
	for (ix = 0; ix < minlen; ix++)
	  {
	    int cmpat = mom_value_cmp (l.pjsonobj->jobjtab[ix].je_name,
				       r.pjsonobj->jobjtab[ix].je_name);
	    if (cmpat)
	      return cmpat;
	    int cmpval = mom_value_cmp (l.pjsonobj->jobjtab[ix].je_attr,
					r.pjsonobj->jobjtab[ix].je_attr);
	    if (cmpval)
	      return cmpval;
	  }
	if (llen < rlen)
	  return -1;
	else if (llen > rlen)
	  return 1;
	return 0;
      }
      // special case for JSON items: compare their names if different
    case momty_jsonitem:
      {
	int cmp = mom_value_cmp ((momval_t) (l.pjsonitem->ij_namejson),
				 (momval_t) (r.pjsonitem->ij_namejson));
	if (cmp)
	  return cmp;
	else
	  goto compare_item_by_uid;
      }
      // special case for boolean items: compare their boolean values if different
    case momty_boolitem:
      {
	bool lb = l.pboolitem->ib_bool;
	bool rb = r.pboolitem->ib_bool;
	if (lb < rb)
	  return -1;
	else if (lb > rb)
	  return 1;
	else
	  goto compare_item_by_uid;
      }
    case momty_node:
    case momty_closure:
      {
	momusize_t llen = l.pnode->slen;
	momusize_t rlen = r.pnode->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	int cmp =
	  mom_value_cmp ((momval_t) (mom_anyitem_t *) (l.pnode->connitm),
			 (momval_t) (mom_anyitem_t *) (r.pnode->connitm));
	if (cmp != 0)
	  return cmp;
	unsigned ix = 0;
	for (ix = 0; ix < minlen; ix++)
	  {
	    cmp = mom_value_cmp (l.pnode->sontab[ix], r.pnode->sontab[ix]);
	    if (cmp != 0)
	      return cmp;
	  }
	if (llen < rlen)
	  return -1;
	else if (llen > rlen)
	  return 1;
	return 0;
      }
    case momty_tuple:
    case momty_set:
      {
	momusize_t llen = l.pseqitm->slen;
	momusize_t rlen = r.pseqitm->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	int cmp = 0;
	unsigned ix = 0;
	for (ix = 0; ix < minlen; ix++)
	  {
	    cmp =
	      mom_value_cmp ((momval_t) (mom_anyitem_t *)
			     (l.pseqitm->itemseq[ix]),
			     (momval_t) (mom_anyitem_t *) (r.pseqitm->itemseq
							   [ix]));
	    if (cmp != 0)
	      return cmp;
	  }
	if (llen < rlen)
	  return -1;
	else if (llen > rlen)
	  return 1;
	return 0;
      }
    default:
      if (ltype > momty__itemlowtype)
	goto compare_item_by_uid;
      else
	MONIMELT_FATAL ("unimplemented compare of type #%d", (int) ltype);
    compare_item_by_uid:
      return memcmp (l.panyitem->i_uuid, r.panyitem->i_uuid, sizeof (uuid_t));
    }
}

momhash_t
mom_string_hash (const char *str, int len)
{
  if (!str)
    return 0;
  if (len < 0)
    len = strlen (str);
  momhash_t h1 = 0, h2 = len, h;
  int l = len;
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
mom_make_string_len (const char *str, int len)
{
  if (!str)
    return NULL;
  if (len < 0)
    len = strlen (str);
  momstring_t *sv = GC_MALLOC_ATOMIC (sizeof (momstring_t) + ((len | 3) + 1));
  memset (sv, 0, sizeof (momstring_t) + ((len | 3) + 1));
  sv->hash = mom_string_hash (str, len);
  sv->slen = len;
  memcpy (sv->cstr, str, len);
  sv->typnum = momty_string;
  return sv;
}

static const momint_t vintminus4 = {.typnum = momty_int,.intval = -4 };
static const momint_t vintminus3 = {.typnum = momty_int,.intval = -3 };
static const momint_t vintminus2 = {.typnum = momty_int,.intval = -2 };
static const momint_t vintminus1 = {.typnum = momty_int,.intval = -1 };
static const momint_t vint0 = {.typnum = momty_int,.intval = 0 };
static const momint_t vint1 = {.typnum = momty_int,.intval = 1 };
static const momint_t vint2 = {.typnum = momty_int,.intval = 2 };
static const momint_t vint3 = {.typnum = momty_int,.intval = 3 };
static const momint_t vint4 = {.typnum = momty_int,.intval = 4 };
static const momint_t vint5 = {.typnum = momty_int,.intval = 5 };
static const momint_t vint6 = {.typnum = momty_int,.intval = 6 };
static const momint_t vint7 = {.typnum = momty_int,.intval = 7 };
static const momint_t vint8 = {.typnum = momty_int,.intval = 8 };
static const momint_t vint9 = {.typnum = momty_int,.intval = 9 };

const momint_t *
mom_make_int (intptr_t n)
{
  switch (n)
    {
    case -4:
      return &vintminus4;
    case -3:
      return &vintminus3;
    case -2:
      return &vintminus2;
    case -1:
      return &vintminus1;
    case 0:
      return &vint0;
    case 1:
      return &vint1;
    case 2:
      return &vint2;
    case 3:
      return &vint3;
    case 4:
      return &vint4;
    case 5:
      return &vint5;
    case 6:
      return &vint6;
    case 7:
      return &vint7;
    case 8:
      return &vint8;
    case 9:
      return &vint9;
    default:
      {
	momint_t *iv = GC_MALLOC_ATOMIC (sizeof (momint_t));
	memset (iv, 0, sizeof (momint_t));
	iv->intval = n;
	iv->typnum = momty_int;
	return iv;
      }
    }
}

const momfloat_t *
mom_make_double (double x)
{
  momfloat_t *dv = GC_MALLOC_ATOMIC (sizeof (momfloat_t));
  memset (dv, 0, sizeof (momfloat_t));
  dv->floval = x;
  dv->typnum = momty_float;
  return dv;
}

static inline void
update_seqitem_hash (struct momseqitem_st *si)
{
  unsigned slen = si->slen;
  momhash_t h = (unsigned) 11 * si->typnum + 31 * slen;
  for (unsigned ix = 0; ix < slen; ix++)
    {
      const mom_anyitem_t *itm = si->itemseq[ix];
      h = (23473 * ix + 43499 * h) ^ (itm ? (itm->i_hash) : 43403);
    }
  if (MONIMELT_UNLIKELY (!h))
    h = (si->typnum + 13 * slen) | 1;
  si->hash = h;
}

const momset_t *
mom_make_set_til_nil (momval_t first, ...)
{
  va_list args;
  momval_t val = MONIMELT_NULLV;
  unsigned siz = 0, ix = 0;
  momset_t *iset = NULL;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      switch (*val.ptype)
	{
	case momty_set:
	case momty_tuple:
	  siz += val.pseqitm->slen;
	  continue;
	default:
	  if (*val.ptype > momty__itemlowtype)
	    siz++;
	}
      val = va_arg (args, momval_t);
    }
  va_end (args);
  iset = GC_MALLOC (sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!iset))
    MONIMELT_FATAL ("failed to allocate set of size %d", (int) siz);
  memset (iset, 0, sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
  siz = 0;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      switch (*val.ptype)
	{
	case momty_set:
	case momty_tuple:
	  {
	    const momseqitem_t *subsi = val.pseqitm;
	    unsigned subslen = subsi->slen;
	    for (unsigned j = 0; j < subslen; j++)
	      if (subsi->itemseq[j])
		iset->itemseq[ix++] = subsi->itemseq[j];
	  }
	  break;
	default:
	  if (*val.ptype > momty__itemlowtype)
	    iset->itemseq[ix++] = val.panyitem;
	  break;
	}
      val = va_arg (args, momval_t);
    }
  va_end (args);
  qsort (iset->itemseq, siz, sizeof (mom_anyitem_t *), mom_itemptr_cmp);
  bool shrink = false;
  for (unsigned ix = 0; siz > 0 && ix + 1 < siz; ix++)
    {
      if (iset->itemseq[ix] == iset->itemseq[ix + 1])
	{
	  shrink = true;
	  for (unsigned j = ix; j + 1 < siz; j++)
	    iset->itemseq[j] = iset->itemseq[j + 1];
	  iset->itemseq[siz - 1] = NULL;
	  siz--;
	}
    }
  iset->typnum = momty_set;
  iset->slen = siz;
  update_seqitem_hash (iset);
  if (MONIMELT_UNLIKELY (shrink))
    {
      momset_t *newiset =
	GC_MALLOC (sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
      if (newiset)
	{
	  memcpy (newiset, iset,
		  sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
	  GC_FREE (iset);
	  iset = newiset;
	}
    }
  return iset;
}

const momset_t *
mom_make_set_sized (unsigned siz, ...)
{
  va_list args;
  unsigned ix = 0, count = 0;
  momset_t *iset = NULL;
  bool shrink = false;
  iset = GC_MALLOC (sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!iset))
    MONIMELT_FATAL ("failed to build set of size %d", (int) siz);
  memset (iset, 0, sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
  va_start (args, siz);
  for (ix = 0; ix < siz; ix++)
    {
      mom_anyitem_t *itm = va_arg (args, mom_anyitem_t *);
      if (itm && itm->typnum >= momty__itemlowtype)
	iset->itemseq[count++] = itm;
    }
  va_end (args);
  shrink = count < siz;
  qsort (iset->itemseq, count, sizeof (mom_anyitem_t *), mom_itemptr_cmp);
  for (unsigned ix = 0; count > 0 && ix + 1 < count; ix++)
    {
      if (iset->itemseq[ix] == iset->itemseq[ix + 1])
	{
	  shrink = true;
	  for (unsigned j = ix; j + 1 < count; j++)
	    iset->itemseq[j] = iset->itemseq[j + 1];
	  iset->itemseq[siz - 1] = NULL;
	  count--;
	}
    }
  iset->typnum = momty_set;
  iset->slen = count;
  update_seqitem_hash (iset);
  if (MONIMELT_UNLIKELY (shrink))
    {
      momset_t *newiset =
	GC_MALLOC (sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
      if (newiset)
	{
	  memcpy (newiset, iset,
		  sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
	  GC_FREE (iset);
	  iset = newiset;
	}
    }
  return iset;
}

const momset_t *
mom_make_set_from_array (unsigned siz, const mom_anyitem_t ** itemarr)
{
  unsigned ix = 0, count = 0;
  momset_t *iset = NULL;
  bool shrink = false;
  iset = GC_MALLOC (sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!iset))
    MONIMELT_FATAL ("failed to build set of size %d", (int) siz);
  memset (iset, 0, sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
  for (ix = 0; ix < siz; ix++)
    {
      const mom_anyitem_t *itm = itemarr[ix];
      if (itm && itm->typnum >= momty__itemlowtype)
	iset->itemseq[count++] = itm;
    }
  shrink = count < siz;
  qsort (iset->itemseq, count, sizeof (mom_anyitem_t *), mom_itemptr_cmp);
  for (unsigned ix = 0; count > 0 && ix + 1 < count; ix++)
    {
      if (iset->itemseq[ix] == iset->itemseq[ix + 1])
	{
	  shrink = true;
	  for (unsigned j = ix; j + 1 < count; j++)
	    iset->itemseq[j] = iset->itemseq[j + 1];
	  iset->itemseq[siz - 1] = NULL;
	  count--;
	}
    }
  iset->typnum = momty_set;
  iset->slen = count;
  update_seqitem_hash (iset);
  if (MONIMELT_UNLIKELY (shrink))
    {
      momset_t *newiset =
	GC_MALLOC (sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
      if (newiset)
	{
	  memcpy (newiset, iset,
		  sizeof (momset_t) + siz * sizeof (mom_anyitem_t *));
	  GC_FREE (iset);
	  iset = newiset;
	}
    }
  return iset;
}

momval_t
mom_make_set_union (momval_t s1, momval_t s2)
{
  if (!s1.ptr)
    {
      if (s2.ptr && *s2.ptype == momty_set)
	return s2;
      else
	return MONIMELT_NULLV;
    }
  else if (!s2.ptr)
    {
      if (s1.ptr && *s1.ptype == momty_set)
	return s1;
      else
	return MONIMELT_NULLV;
    };
  if (*s1.ptype != momty_set || *s2.ptype != momty_set)
    return MONIMELT_NULLV;
  const momset_t *s1set = s1.pset;
  const momset_t *s2set = s2.pset;
  unsigned s1len = s1set->slen;
  unsigned s2len = s2set->slen;
  const mom_anyitem_t *tinyarr[TINY_MAX] = { };
  unsigned sumlen = s1len + s2len;
  const mom_anyitem_t **arr = NULL;
  if (sumlen < TINY_MAX)
    arr = tinyarr;
  else
    arr = GC_MALLOC (sizeof (mom_anyitem_t *) * sumlen);
  if (MONIMELT_UNLIKELY (!arr))
    MONIMELT_FATAL ("failed to allocate union temporary of %d items", sumlen);
  memset (arr, 0, sumlen * sizeof (mom_anyitem_t *));
  unsigned i1 = 0, i2 = 0;
  unsigned nbun = 0;
  while (i1 < s1len && i2 < s2len)
    {
      const mom_anyitem_t *itm1 = s1set->itemseq[i1];
      const mom_anyitem_t *itm2 = s1set->itemseq[i2];
      assert (itm1 != NULL && itm2 != NULL);
      assert (nbun < sumlen);
      int cmp = mom_item_cmp (itm1, itm2);
      if (cmp < 0)
	{
	  arr[nbun++] = itm1;
	  i1++;
	}
      else if (cmp > 0)
	{
	  arr[nbun++] = itm2;
	  i2++;
	}
      else
	{
	  assert (itm1 == itm2);
	  arr[nbun++] = itm1;
	  i1++, i2++;
	}
    }
  momset_t *rset =
    GC_MALLOC (sizeof (struct momseqitem_st) +
	       nbun * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!rset))
    MONIMELT_FATAL ("failed to allocate union of %d elements", (int) nbun);
  memset (rset, 0,
	  sizeof (struct momseqitem_st) + nbun * sizeof (mom_anyitem_t *));
  rset->slen = nbun;
  rset->typnum = momty_set;
  memcpy (rset->itemseq, arr, nbun * sizeof (mom_anyitem_t *));
  update_seqitem_hash (rset);
  if (arr != tinyarr)
    GC_FREE (arr);
  return (momval_t) (const momset_t *) rset;
}				// end mom_make_set_union



momval_t
mom_make_set_intersection (momval_t s1, momval_t s2)
{
  if (!s1.ptr || !s2.ptr || *s1.ptype != momty_set || *s2.ptype != momty_set)
    return MONIMELT_NULLV;
  const momset_t *s1set = s1.pset;
  const momset_t *s2set = s2.pset;
  unsigned s1len = s1set->slen;
  unsigned s2len = s2set->slen;
  const mom_anyitem_t *tinyarr[TINY_MAX] = { };
  unsigned maxlen = (s1len > s2len) ? s1len : s2len;
  const mom_anyitem_t **arr = NULL;
  if (maxlen < TINY_MAX)
    arr = tinyarr;
  else
    arr = GC_MALLOC (sizeof (mom_anyitem_t *) * maxlen);
  if (MONIMELT_UNLIKELY (!arr))
    MONIMELT_FATAL ("failed to allocate intersection temporary of %d items",
		    maxlen);
  memset (arr, 0, maxlen * sizeof (mom_anyitem_t *));
  unsigned i1 = 0, i2 = 0;
  unsigned nbin = 0;
  while (i1 < s1len && i2 < s2len)
    {
      const mom_anyitem_t *itm1 = s1set->itemseq[i1];
      const mom_anyitem_t *itm2 = s1set->itemseq[i2];
      assert (itm1 != NULL && itm2 != NULL);
      assert (nbin < maxlen);
      int cmp = mom_item_cmp (itm1, itm2);
      if (cmp < 0)
	i1++;
      else if (cmp > 0)
	i2++;
      else
	{
	  assert (itm1 == itm2);
	  arr[nbin++] = itm1;
	  i1++, i2++;
	}
    }
  momset_t *rset =
    GC_MALLOC (sizeof (struct momseqitem_st) +
	       nbin * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!rset))
    MONIMELT_FATAL ("failed to allocate intersection of %d elements",
		    (int) nbin);
  memset (rset, 0,
	  sizeof (struct momseqitem_st) + nbin * sizeof (mom_anyitem_t *));
  rset->slen = nbin;
  rset->typnum = momty_set;
  memcpy (rset->itemseq, arr, nbin * sizeof (mom_anyitem_t *));
  update_seqitem_hash (rset);
  if (arr != tinyarr)
    GC_FREE (arr);
  return (momval_t) (const momset_t *) rset;
}

const momitemtuple_t *
mom_make_tuple_til_nil (momval_t first, ...)
{
  va_list args;
  momval_t val = MONIMELT_NULLV;
  unsigned siz = 0, ix = 0;
  momitemtuple_t *itup = NULL;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      switch (*val.ptype)
	{
	case momty_set:
	case momty_tuple:
	  siz += val.pseqitm->slen;
	  continue;
	default:
	  if (*val.ptype > momty__itemlowtype)
	    siz++;
	}
      val = va_arg (args, momval_t);
    }
  va_end (args);
  itup = GC_MALLOC (sizeof (momitemtuple_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!itup))
    MONIMELT_FATAL ("failed to allocate tuple of size %d", (int) siz);
  memset (itup, 0, sizeof (momitemtuple_t) + siz * sizeof (mom_anyitem_t *));
  siz = 0;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      switch (*val.ptype)
	{
	case momty_set:
	case momty_tuple:
	  {
	    const momseqitem_t *subsi = val.pseqitm;
	    unsigned subslen = subsi->slen;
	    for (unsigned j = 0; j < subslen; j++)
	      itup->itemseq[ix++] = subsi->itemseq[j];
	  }
	  break;
	default:
	  if (*val.ptype > momty__itemlowtype)
	    itup->itemseq[ix++] = val.panyitem;
	  break;
	}
      val = va_arg (args, momval_t);
    }
  va_end (args);
  itup->typnum = momty_tuple;
  itup->slen = siz;
  update_seqitem_hash (itup);
  return itup;
}

const momitemtuple_t *
mom_make_tuple_sized (unsigned siz, ...)
{
  va_list args;
  unsigned ix = 0;
  momitemtuple_t *itup = NULL;
  itup = GC_MALLOC (sizeof (momitemtuple_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!itup))
    MONIMELT_FATAL ("failed to build tuple of size %d", (int) siz);
  memset (itup, 0, sizeof (momitemtuple_t) + siz * sizeof (mom_anyitem_t *));
  va_start (args, siz);
  for (ix = 0; ix < siz; ix++)
    {
      mom_anyitem_t *itm = va_arg (args, mom_anyitem_t *);
      itup->itemseq[ix] = itm;
    }
  va_end (args);
  itup->typnum = momty_tuple;
  itup->slen = siz;
  update_seqitem_hash (itup);
  return itup;
}

const momitemtuple_t *
mom_make_tuple_from_array (unsigned siz, const mom_anyitem_t ** itemarr)
{
  unsigned ix = 0;
  momitemtuple_t *ituple = NULL;
  ituple =
    GC_MALLOC (sizeof (momitemtuple_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!ituple))
    MONIMELT_FATAL ("failed to build tuple of size %d", (int) siz);
  memset (ituple, 0,
	  sizeof (momitemtuple_t) + siz * sizeof (mom_anyitem_t *));
  for (ix = 0; ix < siz; ix++)
    {
      const mom_anyitem_t *itm = itemarr[ix];
      if (itm && itm->typnum >= momty__itemlowtype)
	ituple->itemseq[ix] = itm;
    }
  ituple->typnum = momty_tuple;
  ituple->slen = siz;
  update_seqitem_hash (ituple);
  return ituple;
}

////////////////////////////////////////////////////////// nodes

static inline void
update_node_hash (struct momnode_st *nd)
{
  unsigned slen = nd->slen;
  momhash_t h = 11 * nd->connitm->i_hash + ((31 * slen) ^ (19 * nd->typnum));
  for (unsigned ix = 0; ix < slen; ix++)
    {
      h =
	(23053 * ix + 53171 * h) ^ (11 +
				    5309 * mom_value_hash (nd->sontab[ix]));
    }
  if (MONIMELT_UNLIKELY (!h))
    h = (13 * slen + 17 * nd->typnum + (nd->connitm->i_hash & 0xfff)) | 1;
  nd->hash = h;
}


const momnode_t *
mom_make_node_til_nil (mom_anyitem_t * conn, ...)
{
  momnode_t *nd = NULL;
  unsigned siz = 0;
  if (!conn || conn->typnum <= momty__itemlowtype)
    return NULL;
  va_list args;
  va_start (args, conn);
  while (va_arg (args, momval_t).ptr != NULL)
    siz++;
  va_end (args);
  nd = GC_MALLOC (sizeof (momnode_t) + siz * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!nd))
    MONIMELT_FATAL ("failed to allocate node of size %d", (int) siz);
  memset (nd, 0, sizeof (momnode_t) + siz * sizeof (momval_t));
  nd->connitm = conn;
  va_start (args, conn);
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) nd->sontab)[ix] = va_arg (args, momval_t);
  va_end (args);
  nd->typnum = momty_node;
  nd->slen = siz;
  update_node_hash (nd);
  return nd;
}

const momnode_t *
mom_make_node_sized (mom_anyitem_t * conn, unsigned siz, ...)
{
  momnode_t *nd = NULL;
  if (!conn || conn->typnum <= momty__itemlowtype)
    return NULL;
  va_list args;
  nd = GC_MALLOC (sizeof (momnode_t) + siz * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!nd))
    MONIMELT_FATAL ("failed to allocate node of size %d", (int) siz);
  memset (nd, 0, sizeof (momnode_t) + siz * sizeof (momval_t));
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) nd->sontab)[ix] = va_arg (args, momval_t);
  va_end (args);
  nd->typnum = momty_node;
  nd->connitm = conn;
  nd->slen = siz;
  update_node_hash (nd);
  return nd;
}

const momnode_t *
mom_make_node_from_array (mom_anyitem_t * conn, unsigned siz, momval_t * arr)
{
  momnode_t *nd = NULL;
  if (!conn || conn->typnum <= momty__itemlowtype)
    return NULL;
  nd = GC_MALLOC (sizeof (momnode_t) + siz * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!nd))
    MONIMELT_FATAL ("failed to allocate node of size %d", (int) siz);
  memset (nd, 0, sizeof (momnode_t) + siz * sizeof (momval_t));
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) nd->sontab)[ix] = arr[ix];
  nd->typnum = momty_node;
  nd->connitm = conn;
  nd->slen = siz;
  update_node_hash (nd);
  return nd;
}


///////////////////////////////////////////// closures


const momclosure_t *
mom_make_closure_til_nil (mom_anyitem_t * conn, ...)
{
  momclosure_t *clo = NULL;
  unsigned siz = 0;
  if (!conn || conn->typnum != momty_routineitem
      || !((const momit_routine_t *) conn)->irt_descr)
    return NULL;
  unsigned minsiz = ((momit_routine_t *) conn)->irt_descr->rout_minclosize;
  va_list args;
  va_start (args, conn);
  while (va_arg (args, momval_t).ptr != NULL)
    siz++;
  va_end (args);
  unsigned alsize = (minsiz > siz) ? minsiz : siz;
  clo = GC_MALLOC (sizeof (momnode_t) + alsize * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!clo))
    MONIMELT_FATAL ("failed to allocate closure of size %d", (int) alsize);
  memset (clo, 0, sizeof (momnode_t) + alsize * sizeof (momval_t));
  va_start (args, conn);
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) clo->sontab)[ix] = va_arg (args, momval_t);
  va_end (args);
  clo->typnum = momty_closure;
  clo->connitm = conn;
  clo->slen = alsize;
  update_node_hash (clo);
  return clo;
}

const momclosure_t *
mom_make_closure_sized (mom_anyitem_t * conn, unsigned siz, ...)
{
  momclosure_t *clo = NULL;
  if (!conn || conn->typnum != momty_routineitem
      || !((const momit_routine_t *) conn)->irt_descr)
    return NULL;
  unsigned minsiz = ((momit_routine_t *) conn)->irt_descr->rout_minclosize;
  unsigned alsize = (minsiz > siz) ? minsiz : siz;
  va_list args;
  clo = GC_MALLOC (sizeof (momclosure_t) + alsize * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!clo))
    MONIMELT_FATAL ("failed to allocate closure of size %d", (int) alsize);
  memset (clo, 0, sizeof (momclosure_t) + alsize * sizeof (momval_t));
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) clo->sontab)[ix] = va_arg (args, momval_t);
  va_end (args);
  clo->connitm = conn;
  clo->typnum = momty_closure;
  clo->slen = alsize;
  update_node_hash (clo);
  return clo;
}

const momclosure_t *
mom_make_closure_from_array (mom_anyitem_t * conn, unsigned siz,
			     momval_t * arr)
{
  momnode_t *clo = NULL;
  if (!conn || conn->typnum != momty_routineitem
      || !((const momit_routine_t *) conn)->irt_descr)
    return NULL;
  unsigned minsiz = ((momit_routine_t *) conn)->irt_descr->rout_minclosize;
  unsigned alsize = (minsiz > siz) ? minsiz : siz;
  clo = GC_MALLOC (sizeof (momnode_t) + alsize * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!clo))
    MONIMELT_FATAL ("failed to allocate closure of size %d", (int) siz);
  memset (clo, 0, sizeof (momnode_t) + alsize * sizeof (momval_t));
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) clo->sontab)[ix] = arr[ix];
  clo->typnum = momty_closure;
  clo->connitm = conn;
  clo->slen = alsize;
  update_node_hash (clo);
  return clo;
}

void
mom_debugprint_item (FILE * fil, const mom_anyitem_t * itm)
{
  assert (fil != NULL);
  if (!itm)
    fputs ("*nilitem*", fil);
  else
    {
      const momstring_t *namev = mom_name_of_item (itm);
      if (namev && namev->typnum == momty_string)
	fputs (namev->cstr, fil);
      else
	{
	  char uuidstr[UUID_PARSED_LEN];
	  memset (uuidstr, 0, sizeof (uuidstr));
	  fprintf (fil, "${%s}", mom_unparse_item_uuid (itm, uuidstr));
	}
    }
}

static void
debugprint_indent (FILE * fil, unsigned depth)
{
  putc ('\n', fil);
  for (unsigned ix = depth % 16; ix > 0; ix--)
    putc (' ', fil);
}

static void
debugprint_value (FILE * fil, const momval_t val, unsigned depth)
{
  assert (fil != NULL);
  if (!val.ptr)
    fputs ("*nil*", fil);
  else
    {
      unsigned typ = *val.ptype;
      switch (typ)
	{
	case momty_int:
	  fprintf (fil, "%ld", (long) val.pint->intval);
	  break;
	case momty_float:
	  fprintf (fil, "#%f", val.pfloat->floval);
	  break;
	case momty_string:
	  {
	    unsigned l = val.pstring->slen;
	    const char *s = val.pstring->cstr;
	    fputc ('"', fil);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		char c = s[ix];
		switch (c)
		  {
		  case '"':
		    fputs ("\\\"", fil);
		    break;
		  case '\\':
		    fputs ("\\\\", fil);
		    break;
		  case '\n':
		    fputs ("\\n", fil);
		    break;
		  case '\r':
		    fputs ("\\r", fil);
		    break;
		  case '\t':
		    fputs ("\\t", fil);
		    break;
		  case '\v':
		    fputs ("\\v", fil);
		    break;
		  case '\f':
		    fputs ("\\v", fil);
		    break;
		  default:
		    if (c >= ' ' && c < (char) 0x7f)
		      fputc (c, fil);
		    else
		      fprintf (fil, "\\x%02x", ((unsigned) c) & 0xff);
		    break;
		  }
	      }
	    fputc ('"', fil);
	  }
	  break;
	case momty_jsonarray:
	  {
	    unsigned l = val.pjsonarr->slen;
	    fprintf (fil, "*jsar%d[", (int) l);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		if (ix > 0)
		  {
		    putc (',', fil);
		    debugprint_indent (fil, depth + 1);
		  };
		debugprint_value (fil, val.pjsonarr->jarrtab[ix], depth + 1);
	      }
	    fputs ("]", fil);
	  }
	  break;
	case momty_jsonobject:
	  {
	    unsigned l = val.pjsonobj->slen;
	    fprintf (fil, "*jsob%d{", (int) l);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		if (ix > 0)
		  {
		    putc (',', fil);
		    debugprint_indent (fil, depth + 1);
		  };
		debugprint_value (fil, val.pjsonobj->jobjtab[ix].je_name,
				  depth + 1);
		fputs (": ", fil);
		debugprint_value (fil, val.pjsonobj->jobjtab[ix].je_attr,
				  depth + 1);
	      }
	    fputs ("}", fil);
	  }
	  break;
	case momty_set:
	  {
	    unsigned l = val.pset->slen;
	    fprintf (fil, "*set%d{", (int) l);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		if (ix > 0)
		  {
		    putc (';', fil);
		    debugprint_indent (fil, depth + 1);
		  };
		mom_debugprint_item (fil, val.pset->itemseq[ix]);
	      }
	    fputs ("}", fil);
	  }
	  break;
	case momty_tuple:
	  {
	    unsigned l = val.ptuple->slen;
	    fprintf (fil, "*tuple%d(", (int) l);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		if (ix > 0)
		  {
		    putc (',', fil);
		    debugprint_indent (fil, depth + 1);
		  };
		mom_debugprint_item (fil, val.ptuple->itemseq[ix]);
	      }
	    fputs (")", fil);
	  }
	  break;
	case momty_node:
	  {
	    unsigned l = val.pnode->slen;
	    fprintf (fil, "*nod%d:", (int) l);
	    mom_debugprint_item (fil, val.pnode->connitm);
	    fputs ("/(", fil);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		if (ix > 0)
		  {
		    putc (',', fil);
		    debugprint_indent (fil, depth + 1);
		  };
		debugprint_value (fil, val.pnode->sontab[ix], depth + 1);
	      }
	    fputs (")", fil);
	  }
	  break;
	case momty_closure:
	  {
	    unsigned l = val.pnode->slen;
	    fprintf (fil, "*clo%d:", (int) l);
	    mom_debugprint_item (fil, val.pnode->connitm);
	    fputs ("/(", fil);
	    for (unsigned ix = 0; ix < l; ix++)
	      {
		if (ix > 0)
		  {
		    putc (',', fil);
		    debugprint_indent (fil, depth + 1);
		  };
		debugprint_value (fil, val.pnode->sontab[ix], depth + 1);
	      }
	    fputs (")", fil);
	  }
	  break;
	case momty_routineitem:
	  {
	    mom_debugprint_item (fil, val.panyitem);
	    if (val.proutitem->irt_descr
		&& val.proutitem->irt_descr->rout_magic == ROUTINE_MAGIC)
	      fprintf (fil, "@r!%s", val.proutitem->irt_descr->rout_name);
	  }
	  break;
	default:
	  if (typ > momty__itemlowtype && typ < momty__last)
	    mom_debugprint_item (fil, val.panyitem);
	  else
	    {
	      if (typ > 0 && typ < momty__last && mom_typedescr_array[typ])
		fprintf (fil, "?*%s@%p",
			 mom_typedescr_array[typ]->ityp_name, val.ptr);
	      else
		fprintf (fil, "?type#%d@%p", typ, val.ptr);
	    }
	}
    }
}


void
mom_debugprint_value (FILE * fil, const momval_t val)
{
  debugprint_value (fil, val, 0);
}


void
mom_dbgout_item (const mom_anyitem_t * itm)
{
  mom_debugprint_item (stdout, itm);
  putchar ('\n');
  fflush (stdout);
}

void
mom_dbgout_value (const momval_t val)
{
  mom_debugprint_value (stdout, val);
  putchar ('\n');
  fflush (stdout);
}
