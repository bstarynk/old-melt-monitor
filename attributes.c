// file attributes.c - manage attribute hash tables

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
#define SMALL_ATTR_LEN_MOM 8	/* should be a power of two */

struct momentry_st *
mom_attributes_find_entry (const struct momattributes_st *attrs,
			   const momitem_t *itma)
{
  if (!attrs || !itma || itma == MOM_EMPTY || attrs == MOM_EMPTY)
    return NULL;
  unsigned len = attrs->at_len;
  assert (len >= attrs->at_cnt);
  if (len <= SMALL_ATTR_LEN_MOM)
    {
      for (unsigned ix = 0; ix < len; ix++)
	if (attrs->at_entries[ix].ent_itm == itma)
	  return (struct momentry_st *) attrs->at_entries + ix;
    }
  else
    {
      momhash_t hi = mom_item_hash (itma);
      unsigned startix = hi % len;
      for (unsigned ix = startix; ix < len; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (curitm == itma)
	    return (struct momentry_st *) attrs->at_entries + ix;
	  if (!curitm)
	    return NULL;
	}
      for (unsigned ix = 0; ix < startix; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (curitm == itma)
	    return (struct momentry_st *) attrs->at_entries + ix;
	  if (!curitm)
	    return NULL;
	}
      return NULL;
    }
  return NULL;
}				/* end mom_attributes_find_entry */



static void
attributes_raw_put_mom (struct momattributes_st *attrs, const momitem_t *itma,
			const momvalue_t *pval)
{
  unsigned len = attrs->at_len;
  assert (len >= attrs->at_cnt);
  if (len <= SMALL_ATTR_LEN_MOM)
    {
      for (unsigned ix = 0; ix < len; ix++)
	if (attrs->at_entries[ix].ent_itm == itma)
	  {
	    attrs->at_entries[ix].ent_val = *pval;
	    return;
	  };
      for (unsigned ix = 0; ix < len; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (!curitm || curitm == MOM_EMPTY)
	    {
	      attrs->at_entries[ix].ent_itm = itma;
	      attrs->at_entries[ix].ent_val = *pval;
	      attrs->at_cnt++;
	      return;
	    }
	}
    }
  else
    {
      momhash_t hi = mom_item_hash (itma);
      unsigned startix = hi % len;
      int pos = -1;
      for (unsigned ix = startix; ix < len; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (curitm == itma)
	    {
	      attrs->at_entries[ix].ent_val = *pval;
	      return;
	    };
	  if (curitm == MOM_EMPTY)
	    {
	      if (pos < 0)
		pos = (int) ix;
	      continue;
	    }
	  else if (curitm == NULL)
	    {
	      if (pos < 0)
		pos = (int) ix;
	      break;
	    }
	}
      for (unsigned ix = 0; ix < startix; ix++)
	{
	  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	  if (curitm == itma)
	    {
	      attrs->at_entries[ix].ent_val = *pval;
	      return;
	    };
	  if (curitm == MOM_EMPTY)
	    {
	      if (pos < 0)
		pos = (int) ix;
	      continue;
	    }
	  else if (curitm == NULL)
	    {
	      if (pos < 0)
		pos = (int) ix;
	      break;
	    }
	}
      if (pos >= 0)
	{
	  attrs->at_entries[pos].ent_itm = itma;
	  attrs->at_entries[pos].ent_val = *pval;
	  attrs->at_cnt++;
	  return;
	}
    }
  // should never be reached
  MOM_FATAPRINTF ("corrupted attributes @%p count %u len %u", attrs,
		  (unsigned) attrs->at_cnt, len);
}				/* end attributes_raw_put_mom */


struct momattributes_st *
mom_attributes_put (struct momattributes_st *attrs,
		    const momitem_t *itma, const momvalue_t *pval)
{
  if (!attrs)
    {
      if (!itma || !pval || pval->typnum == momty_null)
	return NULL;
      unsigned siz = SMALL_ATTR_LEN_MOM / 2;
      struct momattributes_st *newattrs = mom_attributes_make (siz);
      newattrs->at_cnt = 1;
      newattrs->at_entries[0].ent_itm = itma;
      newattrs->at_entries[0].ent_val = *pval;
      return newattrs;
    }
  unsigned len = attrs->at_len;
  unsigned cnt = attrs->at_cnt;
  assert (cnt <= len);
  if (len <= SMALL_ATTR_LEN_MOM)
    {
      if (cnt + 1 <= len)
	{
	  attributes_raw_put_mom (attrs, itma, pval);
	  return attrs;
	}
      else
	{
	  if (len < SMALL_ATTR_LEN_MOM)
	    {
	      struct momattributes_st *newattrs =
		mom_attributes_make (SMALL_ATTR_LEN_MOM);
	      for (unsigned ix = 0; ix < len; ix++)
		{
		  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
		  if (!curitm || curitm == MOM_EMPTY)
		    continue;
		  attributes_raw_put_mom (newattrs, curitm,
					  &attrs->at_entries[ix].ent_val);
		}
	      attributes_raw_put_mom (newattrs, itma, pval);
	      return newattrs;
	    }
	  else
	    {
	      unsigned newlen =
		((4 * cnt / 3 + 3) | (SMALL_ATTR_LEN_MOM - 1)) + 1;
	      assert (newlen > len);
	      struct momattributes_st *newattrs =
		mom_attributes_make (newlen);
	      for (unsigned ix = 0; ix < len; ix++)
		{
		  const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
		  if (!curitm || curitm == MOM_EMPTY)
		    continue;
		  attributes_raw_put_mom (newattrs, curitm,
					  &attrs->at_entries[ix].ent_val);
		}
	      attributes_raw_put_mom (newattrs, itma, pval);
	      return newattrs;
	    }
	}
    }
  else
    {				// len > SMALL_ATTR_LEN_MOM
      if (4 * cnt + 1 < 3 * len)
	{
	  attributes_raw_put_mom (attrs, itma, pval);
	  return attrs;
	}
      else
	{
	  unsigned newlen =
	    ((3 * cnt / 2 + 5) | (SMALL_ATTR_LEN_MOM - 1)) + 1;
	  assert (newlen > len);
	  struct momattributes_st *newattrs = mom_attributes_make (newlen);
	  for (unsigned ix = 0; ix < len; ix++)
	    {
	      const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	      if (!curitm || curitm == MOM_EMPTY)
		continue;
	      attributes_raw_put_mom (newattrs, curitm,
				      &attrs->at_entries[ix].ent_val);
	    }
	  attributes_raw_put_mom (newattrs, itma, pval);
	  return newattrs;
	}
    }
}				/* end of mom_attributes_put */


struct momattributes_st *
mom_attributes_remove (struct momattributes_st *attrs, const momitem_t *itma)
{
  if (!itma || itma == MOM_EMPTY)
    return attrs;
  if (!attrs)
    return NULL;
  assert (attrs->at_cnt <= attrs->at_len);
  struct momentry_st *ent = mom_attributes_find_entry (attrs, itma);
  if (!ent)
    return attrs;
  ent->ent_itm = MOM_EMPTY;
  ent->ent_val = MOM_NONEV;
  attrs->at_cnt--;
  unsigned alen = attrs->at_len;
  unsigned acnt = attrs->at_cnt;
  if (acnt < SMALL_ATTR_LEN_MOM - 2)
    {
      if (alen > SMALL_ATTR_LEN_MOM)
	{
	  struct momattributes_st *newattrs =
	    mom_attributes_make (SMALL_ATTR_LEN_MOM);
	  for (unsigned ix = 0; ix < alen; ix++)
	    {
	      const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	      if (!curitm || curitm == MOM_EMPTY)
		continue;
	      attributes_raw_put_mom (newattrs, curitm,
				      &attrs->at_entries[ix].ent_val);
	    };
	  return newattrs;
	}
      else
	return attrs;
    };
  if (3 * acnt < alen)
    {
      unsigned newlen = ((3 * acnt / 2 + 4) | (SMALL_ATTR_LEN_MOM - 1)) + 1;
      if (newlen < alen)
	{
	  struct momattributes_st *newattrs = mom_attributes_make (newlen);
	  for (unsigned ix = 0; ix < alen; ix++)
	    {
	      const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
	      if (!curitm || curitm == MOM_EMPTY)
		continue;
	      attributes_raw_put_mom (newattrs, curitm,
				      &attrs->at_entries[ix].ent_val);
	    };
	  return newattrs;
	}
      else
	return attrs;
    }
  return attrs;
}				/* end mom_attributes_remove */



struct momattributes_st *
mom_attributes_make_atva (unsigned nbent, ...
			  /* item1, val1, item2, val2, ... */
  )
{
  va_list args;
  unsigned siz = 0;
  if (nbent < SMALL_ATTR_LEN_MOM / 2)
    siz = SMALL_ATTR_LEN_MOM / 2;
  else if (nbent < SMALL_ATTR_LEN_MOM)
    siz = SMALL_ATTR_LEN_MOM;
  else
    siz = ((3 * nbent / 2 + 4) | (SMALL_ATTR_LEN_MOM - 1)) + 1;
  struct momattributes_st *attrs = mom_attributes_make (siz);
  va_start (args, nbent);
  for (unsigned ix = 0; ix < nbent; ix++)
    {
      momitem_t *itm = va_arg (args, momitem_t *);
      momvalue_t val = va_arg (args, momvalue_t);
      if (!itm || itm == MOM_EMPTY || val.typnum == momty_null)
	continue;
      attributes_raw_put_mom (attrs, itm, &val);
    }
  va_end (args);
  assert (attrs->at_cnt <= attrs->at_len);
  return attrs;
}				/* end mom_attributes_make_atva */

void
mom_attributes_scan_dump (struct momattributes_st *attrs)
{
  if (!attrs)
    return;
  unsigned alen = attrs->at_len;
  assert (attrs->at_cnt <= attrs->at_len);
  for (unsigned ix = 0; ix < alen; ix++)
    {
      const momitem_t *curitm = attrs->at_entries[ix].ent_itm;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      if (!mom_scan_dumped_item (curitm))
	continue;
      mom_scan_dumped_valueptr (&attrs->at_entries[ix].ent_val);
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
  MOM_DEBUGPRINTF (item, "attributes_set count %u", count);
  if (MOM_IS_DEBUGGING (item))
    {
      for (unsigned ix = 0; ix < count; ix++)
	MOM_DEBUGPRINTF (item, "attributes_set arr[%u] = %s",
			 ix, mom_item_cstring (arr[ix]));
    }
  assert (count == acnt);
  const momseq_t *set = mom_make_sized_meta_set (meta, count, arr);
  assert (acnt == 0 || set->slen == acnt);
  MOM_GC_FREE (arr, (acnt + 1) * sizeof (momitem_t *));
  return set;
}				/* end mom_attributes_set  */
