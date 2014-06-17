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

/***************** integers ****************/
static const momint_t int0_mom = {.typnum = momty_int,.intval = 0 };
static const momint_t int1_mom = {.typnum = momty_int,.intval = 1 };
static const momint_t int2_mom = {.typnum = momty_int,.intval = 2 };
static const momint_t int3_mom = {.typnum = momty_int,.intval = 3 };
static const momint_t int4_mom = {.typnum = momty_int,.intval = 4 };
static const momint_t int5_mom = {.typnum = momty_int,.intval = 5 };
static const momint_t int6_mom = {.typnum = momty_int,.intval = 6 };
static const momint_t int7_mom = {.typnum = momty_int,.intval = 7 };
static const momint_t int8_mom = {.typnum = momty_int,.intval = 8 };
static const momint_t int9_mom = {.typnum = momty_int,.intval = 9 };
static const momint_t int10_mom = {.typnum = momty_int,.intval = 10 };
static const momint_t int11_mom = {.typnum = momty_int,.intval = 11 };
static const momint_t int12_mom = {.typnum = momty_int,.intval = 12 };
static const momint_t int13_mom = {.typnum = momty_int,.intval = 13 };
static const momint_t int14_mom = {.typnum = momty_int,.intval = 14 };
static const momint_t int15_mom = {.typnum = momty_int,.intval = 15 };
static const momint_t int16_mom = {.typnum = momty_int,.intval = 16 };
static const momint_t intm1_mom = {.typnum = momty_int,.intval = -1 };
static const momint_t intm2_mom = {.typnum = momty_int,.intval = -2 };
static const momint_t intm3_mom = {.typnum = momty_int,.intval = -3 };
static const momint_t intm4_mom = {.typnum = momty_int,.intval = -4 };
static const momint_t intm5_mom = {.typnum = momty_int,.intval = -5 };
static const momint_t intm6_mom = {.typnum = momty_int,.intval = -6 };
static const momint_t intm7_mom = {.typnum = momty_int,.intval = -7 };
static const momint_t intm8_mom = {.typnum = momty_int,.intval = -8 };
static const momint_t intm9_mom = {.typnum = momty_int,.intval = -9 };

momval_t
mom_make_integer (int64_t c)
{
  if (c > -127L && c <= 128L)
    switch ((int) c)
      {
      case 0:
	return (momval_t) &int0_mom;
      case 1:
	return (momval_t) &int1_mom;
      case 2:
	return (momval_t) &int2_mom;
      case 3:
	return (momval_t) &int3_mom;
      case 4:
	return (momval_t) &int4_mom;
      case 5:
	return (momval_t) &int5_mom;
      case 6:
	return (momval_t) &int6_mom;
      case 7:
	return (momval_t) &int7_mom;
      case 8:
	return (momval_t) &int8_mom;
      case 9:
	return (momval_t) &int9_mom;
      case 10:
	return (momval_t) &int10_mom;
      case 11:
	return (momval_t) &int11_mom;
      case 12:
	return (momval_t) &int12_mom;
      case 13:
	return (momval_t) &int13_mom;
      case 14:
	return (momval_t) &int14_mom;
      case 15:
	return (momval_t) &int15_mom;
      case 16:
	return (momval_t) &int16_mom;
      case -1:
	return (momval_t) &intm1_mom;
      case -2:
	return (momval_t) &intm2_mom;
      case -3:
	return (momval_t) &intm3_mom;
      case -4:
	return (momval_t) &intm4_mom;
      case -5:
	return (momval_t) &intm5_mom;
      case -6:
	return (momval_t) &intm6_mom;
      case -7:
	return (momval_t) &intm7_mom;
      case -8:
	return (momval_t) &intm8_mom;
      case -9:
	return (momval_t) &intm9_mom;
      default:
	break;
      };
  momint_t *v = GC_MALLOC_ATOMIC (sizeof (momint_t));
  if (MOM_UNLIKELY (!v))
    MOM_FATAPRINTF ("cannot allocate boxed integer");
  memset (v, 0, sizeof (momint_t));
  v->typnum = momty_int;
  v->intval = c;
  return (momval_t) (const momint_t *) v;
}


/*************************** boxed doubles ***************************/
const momdouble_t dbl0_mom = {.typnum = momty_double,.dblval = 0.0 };
const momdouble_t dbl1_mom = {.typnum = momty_double,.dblval = 1.0 };
const momdouble_t dblm1_mom = {.typnum = momty_double,.dblval = -1.0 };

momval_t
mom_make_double (double d)
{
  if (d == 0.0)
    return (momval_t) &dbl0_mom;
  else if (d == 1.0)
    return (momval_t) &dbl1_mom;
  else if (d == -1.0)
    return (momval_t) &dblm1_mom;
  momdouble_t *vd = GC_MALLOC_ATOMIC (sizeof (momdouble_t));
  if (MOM_UNLIKELY (!vd))
    MOM_FATAPRINTF ("failed to make double");
  memset (vd, 0, sizeof (momdouble_t));
  vd->typnum = momty_double;
  vd->dblval = d;
  return (momval_t) (const momdouble_t *) vd;
}



/********************* strings ********************/
momhash_t
mom_cstring_hash (const char *str)
{
  if (!str)
    return 0;
  int len = strlen (str);
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
  const gchar *end = NULL;
  unsigned slen = strlen (str);
  if (MOM_UNLIKELY (slen > MOM_MAX_STRING_LENGTH))
    MOM_FATAPRINTF ("too long %d string to make %.50s", slen, str);
  if (MOM_UNLIKELY (!g_utf8_validate ((const gchar *) str, slen, &end)))
    MOM_FATAPRINTF ("invalid UTF8 in %d-sized string %.50s", slen, str);
  momstring_t *res = GC_MALLOC_ATOMIC (sizeof (momstring_t) + slen + 1);
  if (MOM_UNLIKELY (!res))
    MOM_FATAPRINTF ("failed to allocate string of %d bytes", slen);
  memset (res, 0, sizeof (momstring_t) + slen + 1);
  res->slen = slen;
  res->hash = mom_cstring_hash (str);
  memcpy (res->cstr, str, slen);
  res->typnum = momty_string;
  return res;
}

const momstring_t *
mom_make_string_len (const char *str, unsigned slen)
{
  if (!str)
    return NULL;
  const gchar *end = NULL;
  if (MOM_UNLIKELY (slen > MOM_MAX_STRING_LENGTH))
    MOM_FATAPRINTF ("too long %d string to make %.50s", slen, str);
  if (MOM_UNLIKELY (!g_utf8_validate ((const gchar *) str, slen, &end)))
    MOM_FATAPRINTF ("invalid UTF8 in %d-sized string %.50s", slen, str);
  momstring_t *res = GC_MALLOC_ATOMIC (sizeof (momstring_t) + slen + 1);
  if (MOM_UNLIKELY (!res))
    MOM_FATAPRINTF ("failed to allocate string of %d bytes", slen);
  memset (res, 0, sizeof (momstring_t) + slen + 1);
  res->slen = slen;
  res->hash = mom_cstring_hash (str);
  memcpy (res->cstr, str, slen);
  res->typnum = momty_string;
  return res;
}

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
	int64_t i = v.pint->intval;
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
	double d = v.pdouble->dblval;
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
    case momty_tuple:
    case momty_set:
      return v.pseqitems->hash;
    case momty_item:
      return v.pitem->i_hash;
    case momty_node:
      return v.pnode->hash;
    default:
      MOM_FATAPRINTF ("unimplemented hash of val@%p of type #%d", v.ptr,
		      vtype);
    }
  return 0;
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
  switch ((enum momvaltype_en) ltype)
    {
    case momty_null:
      MOM_FATAPRINTF ("corrupted values to compare");
      return 0;
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
    case momty_double:
      {
	double ldbl = l.pdouble->dblval;
	double rdbl = r.pdouble->dblval;
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
    case momty_item:
      {
	return mom_item_cmp (l.pitem, r.pitem);
      }
    case momty_node:
      {
	momusize_t llen = l.pnode->slen;
	momusize_t rlen = r.pnode->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	int cmp = mom_item_cmp ((l.pnode->connitm), (r.pnode->connitm));
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
      break;
    case momty_set:
    case momty_tuple:
      {
	momusize_t llen = l.pseqitems->slen;
	momusize_t rlen = r.pseqitems->slen;
	momusize_t minlen = (llen > rlen) ? rlen : llen;
	int cmp = 0;
	unsigned ix = 0;
	for (ix = 0; ix < minlen; ix++)
	  {
	    cmp =
	      mom_item_cmp ((l.pseqitems->itemseq[ix]),
			    (r.pseqitems->itemseq[ix]));
	    if (cmp != 0)
	      return cmp;
	  }
	if (llen < rlen)
	  return -1;
	else if (llen > rlen)
	  return 1;
	return 0;
      }
      break;
    }
  MOM_FATAPRINTF ("corrupted values to compare");
  return -2;
}



/// be careful, the hash of empty set or tuple should be consistent
/// with what update_seqitem_hash_mom would compute!
static const momset_t empty_set_mom = {
  .typnum = momty_set,
  .hash = 11 * (unsigned) momty_set,
  .slen = 0
};


static const momtuple_t empty_tuple_mom = {
  .typnum = momty_tuple,
  .hash = 11 * (unsigned) momty_tuple,
  .slen = 0
};

static inline void
update_seqitem_hash_mom (struct momseqitem_st *si)
{
  unsigned slen = si->slen;
  momhash_t h = (unsigned) 11 * si->typnum + 31 * slen;
  for (unsigned ix = 0; ix < slen; ix++)
    {
      const momitem_t *itm = si->itemseq[ix];
      h = (23473 * ix + 43499 * h) ^ (itm ? (itm->i_hash) : 43403);
    }
  if (MOM_UNLIKELY (!h))
    h = (si->typnum + 13 * slen) | 1;
  si->hash = h;
}

static int
itemptr_cmp_mom (const void *l, const void *r)
{
  return mom_item_cmp (*(const momitem_t **) l, *(const momitem_t **) r);
}




const momset_t *
mom_make_set_til_nil (momval_t first, ...)
{
  va_list args;
  momval_t val = MOM_NULLV;
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
	  siz += val.pseqitems->slen;
	  continue;
	case momty_item:
	  siz++;
	default:
	  continue;
	}
      val = va_arg (args, momval_t);
    };
  va_end (args);
  if (siz == 0)
    return &empty_set_mom;
  iset =
    MOM_GC_ALLOC ("new set til nil",
		  sizeof (momset_t) + siz * sizeof (momitem_t *));
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
	    const momseqitem_t *subsi = val.pseqitems;
	    unsigned subslen = subsi->slen;
	    for (unsigned j = 0; j < subslen; j++)
	      if (subsi->itemseq[j])
		iset->itemseq[ix++] = subsi->itemseq[j];
	  }
	  break;
	case momty_item:
	  iset->itemseq[ix++] = val.pitem;
	  break;
	default:
	  break;
	}
      val = va_arg (args, momval_t);
    }
  va_end (args);
  qsort (iset->itemseq, siz, sizeof (momitem_t *), itemptr_cmp_mom);
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
  update_seqitem_hash_mom (iset);
  if (MOM_UNLIKELY (shrink))
    {
      momset_t *newiset = MOM_GC_ALLOC ("new shrinked set til nil",
					sizeof (momset_t) +
					siz * sizeof (momitem_t *));
      memcpy (newiset, iset, sizeof (momset_t) + siz * sizeof (momitem_t *));
      MOM_GC_FREE (iset);
      iset = newiset;
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
  if (siz == 0)
    return &empty_set_mom;
  iset =
    MOM_GC_ALLOC ("new sized set",
		  sizeof (momset_t) + siz * sizeof (momitem_t *));
  va_start (args, siz);
  for (ix = 0; ix < siz; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      if (itm && itm->i_typnum == momty_item)
	iset->itemseq[count++] = itm;
    }
  va_end (args);
  shrink = count < siz;
  qsort (iset->itemseq, count, sizeof (momitem_t *), itemptr_cmp_mom);
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
  if (count == 0)
    return &empty_set_mom;
  iset->typnum = momty_set;
  iset->slen = count;
  update_seqitem_hash_mom (iset);
  if (MOM_UNLIKELY (shrink))
    {
      momset_t *newiset = MOM_GC_ALLOC ("new shrinked sized set",
					sizeof (momset_t) +
					siz * sizeof (momitem_t *));
      memcpy (newiset, iset, sizeof (momset_t) + siz * sizeof (momitem_t *));
      MOM_GC_FREE (iset);
      iset = newiset;
    }
  return iset;
}




const momset_t *
mom_make_set_from_array (unsigned siz, const momitem_t **itemarr)
{
  unsigned ix = 0, count = 0;
  momset_t *iset = NULL;
  bool shrink = false;
  if (siz == 0 || !itemarr)
    return &empty_set_mom;
  iset =
    MOM_GC_ALLOC ("new set from array",
		  sizeof (momset_t) + siz * sizeof (momitem_t *));
  for (ix = 0; ix < siz; ix++)
    {
      const momitem_t *itm = itemarr[ix];
      if (itm && itm->i_typnum == momty_item)
	iset->itemseq[count++] = itm;
    }
  if (count == 0)
    return &empty_set_mom;
  shrink = count < siz;
  qsort (iset->itemseq, count, sizeof (momitem_t *), itemptr_cmp_mom);
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
  update_seqitem_hash_mom (iset);
  if (MOM_UNLIKELY (shrink))
    {
      momset_t *newiset = MOM_GC_ALLOC ("new shrinked set from array",
					sizeof (momset_t) +
					siz * sizeof (momitem_t *));
      memcpy (newiset, iset, sizeof (momset_t) + siz * sizeof (momitem_t *));
      MOM_GC_FREE (iset);
      iset = newiset;
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
	return MOM_NULLV;
    }
  else if (!s2.ptr)
    {
      if (s1.ptr && *s1.ptype == momty_set)
	return s1;
      else
	return MOM_NULLV;
    };
  if (*s1.ptype != momty_set || *s2.ptype != momty_set)
    return MOM_NULLV;
  const momset_t *s1set = s1.pset;
  const momset_t *s2set = s2.pset;
  unsigned s1len = s1set->slen;
  unsigned s2len = s2set->slen;
  const momitem_t *tinyarr[MOM_TINY_MAX] = { };
  unsigned sumlen = s1len + s2len;
  if (sumlen == 0)
    return (momval_t) &empty_set_mom;
  const momitem_t **arr = NULL;
  if (sumlen < MOM_TINY_MAX)
    arr = tinyarr;
  else
    arr =
      MOM_GC_ALLOC ("new temporary set union elements",
		    sizeof (momitem_t *) * sumlen);
  unsigned i1 = 0, i2 = 0;
  unsigned nbun = 0;
  while (i1 < s1len && i2 < s2len)
    {
      const momitem_t *itm1 = s1set->itemseq[i1];
      const momitem_t *itm2 = s1set->itemseq[i2];
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
  if (nbun == 0)
    return (momval_t) &empty_set_mom;
  momset_t *rset =
    MOM_GC_ALLOC ("result union set", sizeof (struct momseqitem_st) +
		  nbun * sizeof (momitem_t *));
  rset->slen = nbun;
  rset->typnum = momty_set;
  memcpy (rset->itemseq, arr, nbun * sizeof (momitem_t *));
  update_seqitem_hash_mom (rset);
  if (arr != tinyarr)
    MOM_GC_FREE (arr);
  return (momval_t) (const momset_t *) rset;
}				// end mom_make_set_union



momval_t
mom_make_set_intersection (momval_t s1, momval_t s2)
{
  if (!s1.ptr || !s2.ptr || *s1.ptype != momty_set || *s2.ptype != momty_set)
    return MOM_NULLV;
  const momset_t *s1set = s1.pset;
  const momset_t *s2set = s2.pset;
  unsigned s1len = s1set->slen;
  unsigned s2len = s2set->slen;
  const momitem_t *tinyarr[MOM_TINY_MAX] = { };
  unsigned maxlen = (s1len > s2len) ? s1len : s2len;
  if (s1len == 0 || s2len == 0)
    return (momval_t) &empty_set_mom;
  const momitem_t **arr = NULL;
  if (maxlen < MOM_TINY_MAX)
    arr = tinyarr;
  else
    arr =
      MOM_GC_ALLOC ("new set intersection array",
		    sizeof (momitem_t *) * maxlen);
  unsigned i1 = 0, i2 = 0;
  unsigned nbin = 0;
  while (i1 < s1len && i2 < s2len)
    {
      const momitem_t *itm1 = s1set->itemseq[i1];
      const momitem_t *itm2 = s1set->itemseq[i2];
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
  if (nbin == 0)
    {
      if (arr != tinyarr)
	MOM_GC_FREE (arr);
      return (momval_t) &empty_set_mom;
    };
  momset_t *rset =
    MOM_GC_ALLOC ("result set intersection", sizeof (struct momseqitem_st) +
		  nbin * sizeof (momitem_t *));
  rset->slen = nbin;
  rset->typnum = momty_set;
  memcpy (rset->itemseq, arr, nbin * sizeof (momitem_t *));
  update_seqitem_hash_mom (rset);
  if (arr != tinyarr)
    MOM_GC_FREE (arr);
  return (momval_t) (const momset_t *) rset;
}



/// in set S1 remove the items from set, tuple V2 or remove the item
/// V2 if it is an item...
momval_t
mom_make_set_without (momval_t s1, momval_t v2)
{
  momset_t *sres = NULL;
  if (!s1.ptr || *s1.ptype != momty_set)
    return MOM_NULLV;
  if (!v2.ptr)
    return s1;
  const momset_t *s1set = s1.pset;
  unsigned s1len = s1set->slen;
  unsigned numtyp2 = *v2.ptype;
  if (s1len == 0)
    return s1;
  switch (numtyp2)
    {
    case momty_set:
    merge_two_sets:
      {
	const momset_t *s2set = v2.pset;
	unsigned s2len = s2set->slen;
	if (s2len == 0)
	  return s1;
	const momitem_t *tinyarr[MOM_TINY_MAX] = { };
	const momitem_t **arr =
	  (s1len <
	   MOM_TINY_MAX) ? tinyarr : MOM_GC_ALLOC ("set without array",
						   s1len *
						   sizeof (momitem_t *));
	int ix1 = 0, ix2 = 0, ixres = 0;
	while (ix1 < (int) s1len || ix2 < (int) s2len)
	  {
	    assert (ixres < (int) s1len);
	    const momitem_t *curitm1 =
	      (ix1 < (int) s1len) ? s1set->itemseq[ix1] : NULL;
	    const momitem_t *curitm2 =
	      (ix2 < (int) s2len) ? s2set->itemseq[ix2] : NULL;
	    if (!curitm1)
	      break;
	    if (!curitm2)
	      {
		arr[ixres++] = curitm1;
		ix1++;
		continue;
	      }
	    if (curitm1 == curitm2)
	      {
		ix1++, ix2++;
	      }
	    else
	      {
		int cmp = mom_item_cmp (curitm1, curitm2);
		assert (cmp != 0);
		if (cmp < 0)
		  {
		    arr[ixres++] = curitm1;
		    ix1++;
		  }
		else
		  ix2++;
	      }
	  }
	if (ixres == 0)
	  return (momval_t) &empty_set_mom;
	sres =
	  MOM_GC_ALLOC ("result of set without",
			sizeof (momset_t) + ixres * sizeof (momitem_t *));
	sres->typnum = momty_set;
	if (ixres > 0)
	  memcpy (sres->itemseq, arr, ixres * sizeof (momitem_t *));
	sres->slen = ixres;
	update_seqitem_hash_mom (sres);
	if (arr != tinyarr)
	  GC_FREE (arr), arr = NULL;
	return (momval_t) (const momset_t *) sres;
      }
      break;
    case momty_tuple:
      {
	const momset_t *set2 = mom_make_set_from_array (v2.ptuple->slen,
							(const momitem_t
							 **) (v2.
							      ptuple->itemseq));
	if (!set2 || set2->slen == 0)
	  return s1;
	v2.pset = set2;
	goto merge_two_sets;
      }
      break;
    default:
      if (numtyp2 == momty_item)
	{			// remove the single item itm2 from set s1
	  const momitem_t *itm2 = v2.pitem;
	  int ix1 = -1;
	  unsigned lo = 0, hi = s1len, md = 0;
	  while (lo + 2 < hi)
	    {
	      md = (lo + hi) / 2;
	      int cmp = mom_item_cmp (s1set->itemseq[md], itm2);
	      if (cmp < 0)
		lo = md;
	      else if (cmp > 0)
		hi = md;
	      else
		{
		  assert (s1set->itemseq[md] == itm2);
		  ix1 = (int) md;
		  break;
		};
	    }
	  for (md = lo; md < hi; md++)
	    if (s1set->itemseq[md] == itm2)
	      ix1 = (int) md;
	  if (ix1 >= 0)
	    {
	      assert (s1len > 0);
	      if (s1len == 1)
		return (momval_t) &empty_set_mom;
	      sres =
		MOM_GC_ALLOC ("result of set without", sizeof (momset_t) +
			      (s1len - 1) * sizeof (momitem_t *));
	      memset (sres, 0,
		      sizeof (momset_t) + (s1len - 1) * sizeof (momitem_t *));
	      if (ix1 > 0)
		memcpy (sres->itemseq, s1set->itemseq,
			sizeof (momitem_t *) * (ix1 - 1));
	      memcpy (sres->itemseq + ix1, s1set->itemseq + ix1 + 1,
		      s1len - ix1 - 1);
	      sres->typnum = momty_set;
	      sres->slen = s1len - 1;
	      update_seqitem_hash_mom (sres);
	      return (momval_t) (const momset_t *) sres;
	    }
	  else
	    return s1;
	}
      else			// v2 is not an item, set or tuple
	return s1;
    }
  return s1;
}


const momtuple_t *
mom_make_tuple_til_nil (momval_t first, ...)
{
  va_list args;
  momval_t val = MOM_NULLV;
  unsigned siz = 0, ix = 0;
  momtuple_t *itup = NULL;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      if (val.ptr == MOM_EMPTY)
	siz++;
      else
	switch (*val.ptype)
	  {
	  case momty_set:
	  case momty_tuple:
	    siz += val.pseqitems->slen;
	    break;
	  case momty_item:
	    siz++;
	    break;
	  default:
	    break;
	  }
      val = va_arg (args, momval_t);
    }
  va_end (args);
  if (siz == 0)
    return &empty_tuple_mom;
  itup =
    MOM_GC_ALLOC ("new tuple til nil",
		  sizeof (momtuple_t) + siz * sizeof (momitem_t *));
  siz = 0;
  ix = 0;
  val = first;
  va_start (args, first);
  while (val.ptr != NULL)
    {
      if (val.ptr == MOM_EMPTY)
	{
	  itup->itemseq[ix++] = NULL;
	}
      else
	switch (*val.ptype)
	  {
	  case momty_set:
	  case momty_tuple:
	    {
	      const momseqitem_t *subsi = val.pseqitems;
	      unsigned subslen = subsi->slen;
	      for (unsigned j = 0; j < subslen; j++)
		itup->itemseq[ix++] = subsi->itemseq[j];
	    }
	    break;
	  case momty_item:
	    itup->itemseq[ix++] = val.pitem;
	    break;
	  default:
	    break;
	  }
      val = va_arg (args, momval_t);
    }
  va_end (args);
  itup->typnum = momty_tuple;
  assert (ix == siz);
  itup->slen = siz;
  update_seqitem_hash_mom (itup);
  return itup;
}

const momtuple_t *
mom_make_tuple_sized (unsigned siz, ...)
{
  va_list args;
  unsigned ix = 0;
  momtuple_t *itup = NULL;
  if (siz == 0)
    return &empty_tuple_mom;
  itup =
    MOM_GC_ALLOC ("new tuple sized",
		  sizeof (momtuple_t) + siz * sizeof (momitem_t *));
  va_start (args, siz);
  for (ix = 0; ix < siz; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      if (itm && itm->i_typnum != momty_item)
	itm = NULL;
      itup->itemseq[ix] = itm;
    }
  va_end (args);
  itup->typnum = momty_tuple;
  itup->slen = siz;
  update_seqitem_hash_mom (itup);
  return itup;
}

const momtuple_t *
mom_make_tuple_from_array (unsigned siz, const momitem_t **itemarr)
{
  unsigned ix = 0;
  momtuple_t *ituple = NULL;
  if (siz == 0 || !itemarr)
    return &empty_tuple_mom;
  ituple =
    MOM_GC_ALLOC ("new tuple from array",
		  sizeof (momtuple_t) + siz * sizeof (momitem_t *));
  for (ix = 0; ix < siz; ix++)
    {
      const momitem_t *itm = itemarr[ix];
      if (itm && itm->i_typnum == momty_item)
	ituple->itemseq[ix] = itm;
    }
  ituple->typnum = momty_tuple;
  ituple->slen = siz;
  update_seqitem_hash_mom (ituple);
  return ituple;
}



static inline void
update_node_hash_mom (struct momnode_st *nd)
{
  unsigned slen = nd->slen;
  momhash_t h = 11 * nd->connitm->i_hash + ((31 * slen) ^ (19 * nd->typnum));
  for (unsigned ix = 0; ix < slen; ix++)
    {
      h =
	(23053 * ix + 53171 * h) ^ (11 +
				    5309 * mom_value_hash (nd->sontab[ix]));
    }
  if (MOM_UNLIKELY (!h))
    h = (13 * slen + 17 * nd->typnum + (nd->connitm->i_hash & 0xfff)) | 1;
  nd->hash = h;
}

const momnode_t *
mom_make_node_til_nil (const momitem_t *conn, ...)
{
  momnode_t *nd = NULL;
  unsigned siz = 0;
  if (!conn || conn->i_typnum != momty_item)
    return NULL;
  va_list args;
  va_start (args, conn);
  while (va_arg (args, momval_t).ptr != NULL)
      siz++;
  va_end (args);
  nd =
    MOM_GC_ALLOC ("new node til nil",
		  sizeof (momnode_t) + siz * sizeof (momval_t));
  nd->connitm = conn;
  va_start (args, conn);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      momval_t v = va_arg (args, momval_t);
      if (v.ptr == MOM_EMPTY)
	v.ptr = NULL;
      ((momval_t *) nd->sontab)[ix] = v;
    }
  va_end (args);
  nd->typnum = momty_node;
  nd->slen = siz;
  update_node_hash_mom (nd);
  return nd;
}

const momnode_t *
mom_make_node_sized (const momitem_t *conn, unsigned siz, ...)
{
  momnode_t *nd = NULL;
  if (!conn || conn->i_typnum != momty_item)
    return NULL;
  va_list args;
  nd =
    MOM_GC_ALLOC ("new node sized",
		  sizeof (momnode_t) + siz * sizeof (momval_t));
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    ((momval_t *) nd->sontab)[ix] = va_arg (args, momval_t);
  va_end (args);
  nd->typnum = momty_node;
  nd->connitm = conn;
  nd->slen = siz;
  update_node_hash_mom (nd);
  return nd;
}

const momnode_t *
mom_make_node_from_array (const momitem_t *conn, unsigned siz, momval_t *arr)
{
  momnode_t *nd = NULL;
  if (!conn || conn->i_typnum != momty_item)
    return NULL;
  nd =
    MOM_GC_ALLOC ("new node from array",
		  sizeof (momnode_t) + siz * sizeof (momval_t));
  if (arr)
    for (unsigned ix = 0; ix < siz; ix++)
      ((momval_t *) nd->sontab)[ix] = arr[ix];
  nd->typnum = momty_node;
  nd->connitm = conn;
  nd->slen = siz;
  update_node_hash_mom (nd);
  return nd;
}

const char *
mom_type_cstring (momtynum_t ty)
{
  switch (ty)
    {
    case momty_null:
      return "null";
    case momty_int:
      return "int";
    case momty_double:
      return "double";
    case momty_string:
      return "string";
    case momty_jsonarray:
      return "jsonarray";
    case momty_jsonobject:
      return "jsonobject";
    case momty_set:
      return "set";
    case momty_tuple:
      return "tuple";
    case momty_node:
      return "node";
    case momty_item:
      return "item";
    default:
      {
	char tbuf[16];
	memset (tbuf, 0, sizeof (tbuf));
	snprintf (tbuf, sizeof (tbuf), "?typ#%d?", (int) ty);
	return MOM_GC_STRDUP ("strange type cstring", tbuf);
      }
    }
}
