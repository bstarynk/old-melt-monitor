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
    case momty_itemtuple:
    case momty_itemset:
      return v.pseqitm->hash;
    default:
      if (vtype >= momty__itemlowtype)
	return v.panyitem->i_hash;
#warning missing hash of nodes, sets, ...
      else
	MONIMELT_FATAL ("unimplemented hash of val@%p of type #%d", v.ptr,
			vtype);
    }
}


int
mom_item_cmp (const mom_anyitem_t * l, const mom_anyitem_t * r)
{
  if (l == r)
    return 0;
  if (!l)
    return -1;
  if (!r)
    return 1;
  return memcmp (l->i_uuid, r->i_uuid, sizeof (uuid_t));
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
      {
	momusize_t llen = l.pnode->slen;
	momusize_t rlen = r.pnode->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	int cmp = mom_value_cmp ((momval_t) (l.pnode->connitm),
				 (momval_t) (r.pnode->connitm));
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
    case momty_itemtuple:
    case momty_itemset:
      {
	momusize_t llen = l.pseqitm->slen;
	momusize_t rlen = r.pseqitm->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	int cmp = 0;
	unsigned ix = 0;
	for (ix = 0; ix < minlen; ix++)
	  {
	    cmp =
	      mom_value_cmp ((momval_t) (l.pseqitm->itemseq[ix]),
			     (momval_t) (r.pseqitm->itemseq[ix]));
	    if (cmp != 0)
	      return cmp;
	  }
	if (llen < rlen)
	  return -1;
	else if (llen > rlen)
	  return 1;
	return 0;
      }
#warning missing compare of nodes, closures, etc...
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
      mom_anyitem_t *itm = si->itemseq[ix];
      h = (23473 * ix + 43499 * h) ^ (itm ? (itm->i_hash) : 43403);
    }
  if (MONIMELT_UNLIKELY (!h))
    h = (si->typnum + 13 * slen) | 1;
  si->hash = h;
}

const momitemset_t *
mom_make_item_set_til_nil (momval_t first, ...)
{
  va_list args;
  momval_t val = MONIMELT_NULLV;
  unsigned siz = 0, ix = 0;
  momitemset_t *iset = NULL;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      switch (*val.ptype)
	{
	case momty_itemset:
	case momty_itemtuple:
	  siz += val.pseqitm->slen;
	  continue;
	default:
	  if (*val.ptype > momty__itemlowtype)
	    siz++;
	}
      val = va_arg (args, momval_t);
    }
  va_end (args);
  iset = GC_MALLOC (sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!iset))
    MONIMELT_FATAL ("failed to allocate set of size %d", (int) siz);
  memset (iset, 0, sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
  siz = 0;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      switch (*val.ptype)
	{
	case momty_itemset:
	case momty_itemtuple:
	  {
	    momseqitem_t *subsi = val.pseqitm;
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
  iset->typnum = momty_itemset;
  update_seqitem_hash (iset);
  if (MONIMELT_UNLIKELY (shrink))
    {
      momitemset_t *newiset =
	GC_MALLOC (sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
      if (newiset)
	{
	  memcpy (newiset, iset,
		  sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
	  GC_FREE (iset);
	  iset = newiset;
	}
    }
  return iset;
}

const momitemset_t *
mom_make_item_set_sized (unsigned siz, ...)
{
  va_list args;
  momval_t val = MONIMELT_NULLV;
  unsigned ix = 0, count = 0;
  momitemset_t *iset = NULL;
  bool shrink = false;
  iset = GC_MALLOC (sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!iset))
    MONIMELT_FATAL ("failed to build set of size %d", (int) siz);
  memset (iset, 0, sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
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
  iset->typnum = momty_itemset;
  update_seqitem_hash (iset);
  if (MONIMELT_UNLIKELY (shrink))
    {
      momitemset_t *newiset =
	GC_MALLOC (sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
      if (newiset)
	{
	  memcpy (newiset, iset,
		  sizeof (momitemset_t) + siz * sizeof (mom_anyitem_t *));
	  GC_FREE (iset);
	  iset = newiset;
	}
    }
  return iset;
}

const momitemtuple_t *
mom_make_item_tuple_til_nil (momval_t first, ...)
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
	case momty_itemset:
	case momty_itemtuple:
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
	case momty_itemset:
	case momty_itemtuple:
	  {
	    momseqitem_t *subsi = val.pseqitm;
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
  itup->typnum = momty_itemtuple;
  update_seqitem_hash (itup);
  return itup;
}

const momitemset_t *
mom_make_item_tuple_sized (unsigned siz, ...)
{
  va_list args;
  momval_t val = MONIMELT_NULLV;
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
  itup->typnum = momty_itemtuple;
  update_seqitem_hash (itup);
  return itup;
}
