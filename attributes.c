// file attributes.c

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

//// attributes are seggregated between small attributes, which are
//// scanned linearly, and bigger ones, which are hashtables.
#define SMALL_ATTR_LEN_MOM 8

static struct momentry_st *
attributes_find_entry_pos_mom (const struct momattributes_st *attrs,
			       const momitem_t *itma, int *ppos)
{
  if (ppos)
    *ppos = -1;
  if (!attrs || !itma)
    return NULL;
  unsigned len = attrs->at_len;
  int pos = -1;
  if (len <= SMALL_ATTR_LEN_MOM)
    {
      for (unsigned ix = 0; ix < len; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (curitm == itma)
	    return (struct momentry_st *) &attrs->at_entries[ix];
	  else if ((!curitm || curitm == MOM_EMPTY) && pos < 0)
	    pos = (int) ix;
	}
      if (ppos && pos >= 0)
	*ppos = pos;
      return NULL;
    }
  else
    {
      momhash_t hi = mom_item_hash (itma);
      unsigned startix = hi % len;
      for (unsigned ix = startix; ix < len; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (!curitm)
	    {
	      if (pos < 0)
		pos = ix;
	      if (ppos && pos >= 0)
		*ppos = pos;
	      return NULL;
	    }
	  if (curitm == MOM_EMPTY)
	    {
	      if (pos < 0)
		pos = ix;
	      continue;
	    }
	  if (curitm == itma)
	    return (struct momentry_st *) attrs->at_entries + ix;
	}
      for (unsigned ix = 0; ix < startix; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (!curitm)
	    {
	      if (pos < 0)
		pos = ix;
	      if (ppos && pos >= 0)
		*ppos = pos;
	      return NULL;
	    }
	  if (curitm == MOM_EMPTY)
	    {
	      if (pos < 0)
		pos = ix;
	      continue;
	    }
	  if (curitm == itma)
	    return (struct momentry_st *) attrs->at_entries + ix;
	}
      if (ppos && pos >= 0)
	*ppos = pos;
      return NULL;
    }
}

struct momentry_st *
mom_attributes_find_entry (const struct momattributes_st *attrs,
			   const momitem_t *itma)
{
  return attributes_find_entry_pos_mom (attrs, itma, NULL);
}

struct momattributes_st *
mom_attributes_put (struct momattributes_st *attrs,
		    const momitem_t *itma, const momvalue_t *pval)
{
  if (!attrs)
    {
      if (!itma || !pval || pval->typnum == momty_null)
	return NULL;
      unsigned siz = SMALL_ATTR_LEN_MOM / 2;
      struct momattributes_st *newattrs =	//
	MOM_GC_ALLOC ("new attributes",	//
		      sizeof (struct momattributes_st)
		      + siz * sizeof (struct momentry_st));
      newattrs->at_len = siz;
      newattrs->at_cnt = 1;
      newattrs->at_entries[0].ent_itm = itma;
      newattrs->at_entries[0].ent_val = *pval;
      return newattrs;
    }
  if (!itma)
    return (struct momattributes_st *) attrs;
  if (!pval)
    return mom_attributes_remove (attrs, itma);
  int pos = -1;
  unsigned alen = attrs->at_len;
  unsigned acnt = attrs->at_cnt;
  struct momentry_st *ent = attributes_find_entry_pos_mom (attrs, itma, &pos);
  if (ent)
    {				/* in-place replacement */
      ent->ent_val = *pval;
      return attrs;
    };
  if (pos >= 0)
    {
      if (alen <= SMALL_ATTR_LEN_MOM || 5 * acnt + 1 < 4 * alen)
	{
	  assert (pos < (int) alen);
	  attrs->at_entries[pos].ent_itm = itma;
	  attrs->at_entries[pos].ent_val = *pval;
	  attrs->at_cnt++;
	  return attrs;
	}
    }
  if (alen < SMALL_ATTR_LEN_MOM)
    {
      unsigned newsiz = SMALL_ATTR_LEN_MOM;
      struct momattributes_st *newattrs =	//
	MOM_GC_ALLOC ("new attributes",	//
		      sizeof (struct momattributes_st)
		      + newsiz * sizeof (struct momentry_st));
      memcpy (newattrs, attrs,
	      sizeof (struct momattributes_st) +
	      alen * sizeof (struct momentry_st));
      newattrs->at_len = newsiz;
      newattrs->at_entries[alen].ent_itm = itma;
      newattrs->at_entries[alen].ent_val = *pval;
      newattrs->at_cnt++;
      return newattrs;
    }
  else
    {
      unsigned newsiz = ((4 * acnt / 3 + 2) | 0xf) + 1;
      struct momattributes_st *newattrs =	//
	MOM_GC_ALLOC ("new attributes",	//
		      sizeof (struct momattributes_st)
		      + newsiz * sizeof (struct momentry_st));
      newattrs->at_len = newsiz;
      for (unsigned ix = 0; ix < alen; ix++)
	{
	  int newpos = -1;
	  const momitem_t *olditm = attrs->at_entries[ix].ent_itm;
	  if (!olditm || olditm == MOM_EMPTY)
	    continue;
	  struct momentry_st *newent =
	    attributes_find_entry_pos_mom (newattrs, itma, &newpos);
	  if (MOM_UNLIKELY (newent != NULL || newpos < 0))	// should never happen
	    MOM_FATAPRINTF ("corrupted new attributes of size %u", newsiz);
	  assert (newpos < (int) newsiz);
	  newattrs->at_entries[newpos] = attrs->at_entries[ix];
	  newattrs->at_cnt++;
	}
      MOM_GC_FREE (attrs,
		   sizeof (struct momattributes_st) +
		   alen * sizeof (struct momentry_st));
      return newattrs;
    }
}

struct momattributes_st *
mom_attributes_remove (struct momattributes_st *attrs, const momitem_t *itma)
{
  if (!itma || itma == MOM_EMPTY)
    return attrs;
  if (!attrs)
    return NULL;
  struct momentry_st *ent = attributes_find_entry_pos_mom (attrs, itma, NULL);
  if (!ent)
    return attrs;
  ent->ent_itm = MOM_EMPTY;
  ent->ent_val = MOM_NONEV;
  attrs->at_cnt--;
  unsigned alen = attrs->at_len;
  unsigned acnt = attrs->at_cnt;
  if (alen <= SMALL_ATTR_LEN_MOM)
    return attrs;
  if (MOM_UNLIKELY (3 * acnt + 1 < alen))
    {
      if (acnt < SMALL_ATTR_LEN_MOM - 1)
	{
	  unsigned newsiz = SMALL_ATTR_LEN_MOM;
	  unsigned newcnt = 0;
	  struct momattributes_st *newattrs =	//
	    MOM_GC_ALLOC ("new attributes",	//
			  sizeof (struct momattributes_st)
			  + newsiz * sizeof (struct momentry_st));
	  newattrs->at_len = newsiz;
	  for (unsigned ix = 0; ix < alen; ix++)
	    {
	      assert (newcnt < newsiz);
	      const momitem_t *olditm = attrs->at_entries[ix].ent_itm;
	      if (!olditm || olditm == MOM_EMPTY)
		continue;
	      newattrs->at_entries[newcnt++] = attrs->at_entries[ix];
	    };
	  newattrs->at_cnt = newcnt;
	  MOM_GC_FREE (attrs,
		       sizeof (struct momattributes_st)
		       + alen * sizeof (struct momentry_st));
	  return newattrs;
	}
      else			// acnt >= SMALL_ATTR_LEN_MOM-1
	{
	  unsigned newsiz = (((3 * acnt / 2) + 2) | 0xf) + 1;
	  if (newsiz >= alen)
	    return attrs;
	  struct momattributes_st *newattrs =	//
	    MOM_GC_ALLOC ("new attributes",	//
			  sizeof (struct momattributes_st)
			  + newsiz * sizeof (struct momentry_st));
	  newattrs->at_len = newsiz;
	  for (unsigned ix = 0; ix < alen; ix++)
	    {
	      const momitem_t *olditm = attrs->at_entries[ix].ent_itm;
	      if (!olditm || olditm == MOM_EMPTY)
		continue;
	      int newpos = -1;
	      struct momentry_st *newent =
		attributes_find_entry_pos_mom (newattrs, itma, &newpos);
	      if (MOM_UNLIKELY (newent != NULL || newpos < 0))	// should never happen
		MOM_FATAPRINTF ("corrupted new attributes of size %u",
				newsiz);
	      assert (newpos < (int) newsiz);
	      newattrs->at_entries[newpos] = attrs->at_entries[ix];
	      newattrs->at_cnt++;
	    }
	  MOM_GC_FREE (attrs,	//
		       sizeof (struct momattributes_st)
		       + alen * sizeof (struct momentry_st));
	  return newattrs;

	}
    }
  return attrs;
}

struct momattributes_st *
mom_attributes_make_atva (unsigned nbent, ...
			  /* item1, val1, item2, val2, ... */
  )
{
  va_list args;
  unsigned siz =
    (nbent < SMALL_ATTR_LEN_MOM / 2) ? (SMALL_ATTR_LEN_MOM / 2)
    : (nbent <
       SMALL_ATTR_LEN_MOM - 1) ? (SMALL_ATTR_LEN_MOM) : (1 +
							 ((4 * nbent / 3 +
							   2) | 0xf));
  struct momattributes_st *attrs =	//
    MOM_GC_ALLOC ("new attributes",	//
		  sizeof (struct momattributes_st)
		  + siz * sizeof (struct momentry_st));
  attrs->at_len = siz;
  va_start (args, nbent);
  for (unsigned ix = 0; ix < nbent; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      momvalue_t val = va_arg (args, momvalue_t);
      if (!itm || itm == MOM_EMPTY || val.typnum == momty_null)
	continue;
      attrs = mom_attributes_put (attrs, itm, &val);
    }
  va_end (args);
  return attrs;
}

void
mom_attributes_scan_dump (struct momattributes_st *attrs,
			  struct momdumper_st *du)
{
  if (!attrs)
    return;
  assert (du != NULL);
  unsigned alen = attrs->at_len;
  for (unsigned ix = 0; ix < alen; ix++)
    {
      const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      if (!mom_scan_dumped_item (du, curitm))
	continue;
      mom_scan_dumped_value (du, attrs->at_entries[ix].ent_val);
    }
}

const momseq_t *
mom_attributes_set (struct momattributes_st *attrs, momvalue_t meta)
{
  if (!attrs || attrs == MOM_EMPTY)
    return NULL;
  unsigned acnt = attrs->at_cnt;
  unsigned asiz = attrs->at_len;
  unsigned count = 0;
  const momitem_t **arr =	//
    MOM_GC_ALLOC ("attributes set", (acnt + 1) * sizeof (momitem_t *));
  for (unsigned ix = 0; ix < asiz; ix++)
    {
      const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      assert (count < acnt);
      arr[count++] = curitm;
    };
  assert (count == acnt);
  const momseq_t *set = mom_make_sized_meta_set (meta, count, arr);
  MOM_GC_FREE (arr, (acnt + 1) * sizeof (momitem_t *));
  return set;
}
