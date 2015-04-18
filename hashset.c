// file hashset.c

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

// we seggregate small hashsets, scanned linearly, from bigger hashed set
#define SMALL_HASHSET_LEN_MOM 12

const momseq_t *
mom_hashset_elements_set_meta (struct momhashset_st *hset, momvalue_t metav)
{
  if (!hset)
    return NULL;
  return mom_make_sized_meta_set (metav, hset->hset_len, hset->hset_elems);
}

bool
mom_hashset_contains (const struct momhashset_st * hset, const momitem_t *itm)
{
  if (!hset || hset == MOM_EMPTY || !itm || itm == MOM_EMPTY)
    return false;
  unsigned hslen = hset->hset_len;
  if (hslen <= SMALL_HASHSET_LEN_MOM)
    {
      for (unsigned ix = 0; ix < hslen; ix++)
	if (hset->hset_elems[ix] == itm)
	  return true;
    }
  else
    {
      unsigned startix = mom_item_hash (itm) % hslen;
      for (unsigned ix = startix; ix < hslen; ix++)
	{
	  const momitem_t *curitm = hset->hset_elems[ix];
	  if (!curitm)
	    return false;
	  else if (curitm == itm)
	    return true;
	}
      for (unsigned ix = 0; ix < startix; ix++)
	{
	  const momitem_t *curitm = hset->hset_elems[ix];
	  if (!curitm)
	    return false;
	  else if (curitm == itm)
	    return true;
	}
    }
  return false;
}


static void
hashset_raw_hash_add_mom (struct momhashset_st *hset, const momitem_t *itm)
{
  assert (hset && itm);
  unsigned hslen = hset->hset_len;
  assert (hslen > SMALL_HASHSET_LEN_MOM);
  unsigned hscnt = hset->hset_cnt;
  assert (hscnt < hslen);
  int pos = -1;
  unsigned startix = mom_item_hash (itm) % hslen;
  for (unsigned ix = startix; ix < hslen; ix++)
    {
      const momitem_t *curitm = hset->hset_elems[ix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = ix;
	  hset->hset_elems[pos] = itm;
	  hset->hset_cnt = hscnt + 1;
	  return;
	}
      else if (curitm == itm)
	return;
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const momitem_t *curitm = hset->hset_elems[ix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = ix;
	  hset->hset_elems[pos] = itm;
	  hset->hset_cnt = hscnt + 1;
	  return;
	}
      else if (curitm == itm)
	return;
    }
  // never reached
  MOM_FATAPRINTF ("corrupted hashset@%p", (void *) hset);
}


struct momhashset_st *
mom_hashset_put (struct momhashset_st *hset, momitem_t *itm)
{
  if (!hset || hset == MOM_EMPTY)
    return NULL;
  if (!itm || itm == MOM_EMPTY)
    return hset;
  unsigned hslen = hset->hset_len;
  unsigned hscnt = hset->hset_cnt;
  if (hslen <= SMALL_HASHSET_LEN_MOM)
    {
      if (hscnt + 1 < hslen)
      small_nonfull_hset:
	{

	  int pos = -1;
	  for (unsigned ix = 0; ix < hslen; ix++)
	    {
	      const momitem_t *curitm = hset->hset_elems[ix];
	      if (!curitm)
		{
		  if (pos < 0)
		    pos = ix;
		  break;
		}
	      else if (curitm == MOM_EMPTY)
		{
		  if (pos < 0)
		    pos = ix;
		}
	      else if (curitm == itm)
		return hset;
	    }
	  assert (pos >= 0);
	  hset->hset_elems[pos] = itm;
	  hset->hset_cnt++;
	  return hset;
	}
      else
	{
	  if (hslen < SMALL_HASHSET_LEN_MOM)
	    {
	      unsigned siz = SMALL_HASHSET_LEN_MOM;
	      struct momhashset_st *newhset	//
		= MOM_GC_ALLOC ("new small hashset",
				sizeof (struct momhashset_st) +
				siz * sizeof (momitem_t *));
	      newhset->hset_len = siz;
	      memcpy (newhset->hset_elems, hset->hset_elems,
		      hslen * sizeof (momitem_t *));
	      newhset->hset_cnt = hset->hset_cnt;
	      memset (hset, 0,
		      sizeof (struct momhashset_st) +
		      hslen * sizeof (momitem_t *));
	      MOM_GC_FREE (hset);
	      hset = newhset;
	      hslen = siz;
	      goto small_nonfull_hset;
	    }
	  else
	    {
	      unsigned siz = ((4 * hscnt / 3 + 10) | 0xf) + 1;
	      struct momhashset_st *newhset	//
		= MOM_GC_ALLOC ("new  hashset",
				sizeof (struct momhashset_st) +
				siz * sizeof (momitem_t *));
	      newhset->hset_len = siz;
	      for (unsigned ix = 0; ix < hslen; ix++)
		{
		  const momitem_t *olditm = hset->hset_elems[ix];
		  if (!olditm || olditm == MOM_EMPTY)
		    continue;
		  hashset_raw_hash_add_mom (newhset, olditm);
		}
	      hashset_raw_hash_add_mom (newhset, itm);
	      memset (hset, 0,
		      sizeof (struct momhashset_st) +
		      hslen * sizeof (momitem_t *));
	      MOM_GC_FREE (hset);
	      hset = newhset;
	      return hset;
	    }
	}
    }
  else
    {
      if (4 * hscnt + 2 > 3 * hslen)
	{
	  unsigned siz = ((3 * hscnt / 2 + 10) | 0xf) + 1;
	  struct momhashset_st *newhset	//
	    = MOM_GC_ALLOC ("new hashset",
			    sizeof (struct momhashset_st) +
			    siz * sizeof (momitem_t *));
	  newhset->hset_len = siz;
	  for (unsigned ix = 0; ix < hslen; ix++)
	    {
	      const momitem_t *olditm = hset->hset_elems[ix];
	      if (!olditm || olditm == MOM_EMPTY)
		continue;
	      hashset_raw_hash_add_mom (newhset, olditm);
	    }
	  hashset_raw_hash_add_mom (newhset, itm);
	  memset (hset, 0,
		  sizeof (struct momhashset_st) +
		  hslen * sizeof (momitem_t *));
	  MOM_GC_FREE (hset);
	  hset = newhset;
	  return hset;
	}
      else
	{
	  hashset_raw_hash_add_mom (hset, itm);
	  return hset;
	}
    }
}



struct momhashset_st *
mom_hashset_remove (struct momhashset_st *hset, momitem_t *itm)
{
#warning unimplemented mom_hashset_remove
}


struct momhashset_st *
mom_hashset_add_items (struct momhashset_st *hset,
		       unsigned nbitems, ... /* items */ )
{
#warning unimplemented mom_hashset_add_items
}


struct momhashset_st *
mom_hashset_add_sized_items (struct momhashset_st *hset,
			     unsigned siz, const momitem_t **itmarr)
{
#warning unimplemented mom_hashset_add_sized_items
}
