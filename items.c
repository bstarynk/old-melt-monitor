// file items.c

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

static pthread_mutex_t globitem_mtx_mom = PTHREAD_MUTEX_INITIALIZER;


#define  PREFERRED_BUCKET_SIZE_MOM 7
struct itembucket_mom_st
{
  unsigned buck_size;
  unsigned buck_count;
  const momitem_t *buck_items[];
};
static struct
{
  unsigned itbuck_size;
  unsigned itbuck_nbitems;
  struct itembucket_mom_st **itbuck_arr;
} buckets_mom;

void
mom_initialize_items (void)
{
  const unsigned inibuck = 128;
  buckets_mom.itbuck_arr =
    GC_MALLOC (inibuck * sizeof (struct itembucket_mom_st *));
  if (!buckets_mom.itbuck_arr)
    MOM_FATAPRINTF ("failed to create %d buckets", inibuck);
  memset (buckets_mom.itbuck_arr, 0,
	  inibuck * sizeof (struct itembucket_mom_st *));
  buckets_mom.itbuck_size = inibuck;
}


// return true if item is added
static bool
add_item_mom (const momitem_t * aitm)
{
  assert (mom_is_item ((momval_t) aitm));
  momhash_t h = aitm->i_hash;
  assert (h == mom_string_hash ((momval_t) aitm->i_idstr));
  unsigned bix = h % buckets_mom.itbuck_size;
  struct itembucket_mom_st *curbuck = buckets_mom.itbuck_arr[bix];
  if (!curbuck)
    {
      // no bucket, make one
      struct itembucket_mom_st *newbuck
	=
	GC_MALLOC_ATOMIC (sizeof (struct itembucket_mom_st) +
			  PREFERRED_BUCKET_SIZE_MOM * sizeof (momitem_t *));
      if (!newbuck)
	MOM_FATAPRINTF ("failed to allocate new item bucket");
      memset (newbuck, 0,
	      sizeof (struct itembucket_mom_st) +
	      PREFERRED_BUCKET_SIZE_MOM * sizeof (momitem_t *));
      newbuck->buck_size = PREFERRED_BUCKET_SIZE_MOM;
      curbuck = buckets_mom.itbuck_arr[bix] = newbuck;
    }
  else if (curbuck->buck_count >= curbuck->buck_size)
    {
      /// full bucket, grow it
      struct itembucket_mom_st *oldbuck = curbuck;
      struct itembucket_mom_st *newbuck = NULL;
      unsigned oldbuckcount = oldbuck->buck_count;
      unsigned oldbucksize = oldbuck->buck_size;
      unsigned newbucksize =
	(3 * oldbuckcount / 2 + PREFERRED_BUCKET_SIZE_MOM) | 7;
      newbuck =
	GC_MALLOC_ATOMIC (sizeof (struct itembucket_mom_st) +
			  newbucksize * sizeof (momitem_t *));
      if (!newbuck)
	MOM_FATAPRINTF ("failed to grow new item bucket of %d", newbucksize);
      memset (newbuck, 0,
	      sizeof (struct itembucket_mom_st) +
	      newbucksize * sizeof (momitem_t *));
      newbuck->buck_size = newbucksize;
      unsigned ncnt = 0;
      for (unsigned oix = 0; oix < oldbucksize; oix++)
	{
	  const momitem_t *olditm = oldbuck->buck_items[oix];
	  if (!olditm)
	    continue;
	  assert (ncnt < newbucksize);
	  GC_unregister_disappearing_link ((void **)
					   &oldbuck->buck_items[oix]);
	  oldbuck->buck_items[oix] = NULL;
	  newbuck->buck_items[ncnt] = olditm;
	  GC_general_register_disappearing_link
	    ((void **) &newbuck->buck_items[ncnt], (void *) olditm);
	  ncnt++;
	};
      newbuck->buck_count = ncnt;
      curbuck = buckets_mom.itbuck_arr[bix] = newbuck;
      GC_FREE (oldbuck);
    };
  int pos = -1;
  unsigned bucksize = curbuck->buck_size;
  for (unsigned pix = 0; pix < bucksize; pix++)
    {
      const momitem_t *curitm = curbuck->buck_items[pix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = pix;
	  continue;
	};
      if (curitm == aitm)
	return false;
      if (MOM_UNLIKELY (mom_item_cmp (curitm, aitm)))
	MOM_FATAPRINTF ("items @%p and @%p have same id %s", curitm, aitm,
			curitm->i_idstr->cstr);
    }
  assert (pos >= 0);
  curbuck->buck_items[pos] = aitm;
  GC_general_register_disappearing_link
    ((void **) &curbuck->buck_items[pos], (void *) aitm);
  curbuck->buck_count++;
  return true;
}



/// costly routine to reorganize the items buckets
/// complexity is proportional to total number of items
static void
reorganize_items_mom (unsigned morebuckets)
{
#define NB_ITEMS_MAX (64<<20)	/* 64 mega items is a big lot */
  unsigned long itemsize =
    ((5 * buckets_mom.itbuck_nbitems / 4 + 50) | 0x7f) + 1;
  unsigned long itemcount = 0;
  momitem_t **itemarr = GC_MALLOC (itemsize * sizeof (momitem_t *));
  if (MOM_UNLIKELY (!itemarr || itemsize > NB_ITEMS_MAX))
    MOM_FATAPRINTF ("failed to allocate item array for %ld items",
		    (long) itemsize);
  memset (itemarr, 0, itemsize * sizeof (momitem_t *));
  // big loop on each bucket, filling and growing itemarr to remember every item
  for (unsigned buckix = 0; buckix < buckets_mom.itbuck_size; buckix++)
    {
      struct itembucket_mom_st *curbucket = buckets_mom.itbuck_arr[buckix];
      if (!curbucket)
	continue;
      unsigned curbsize = curbucket->buck_size;
      if (MOM_UNLIKELY (itemcount + curbsize >= itemsize))
	{
	  unsigned long newitemsize =
	    ((5 * itemcount / 4 + curbsize + 100) | 0x7f) + 1;
	  momitem_t **newitemarr =
	    GC_MALLOC (newitemsize * sizeof (momitem_t *));
	  if (MOM_UNLIKELY (!newitemarr || newitemsize > NB_ITEMS_MAX))
	    MOM_FATAPRINTF ("failed to grow item array for %ld items",
			    (long) newitemsize);
	  memset (newitemarr, 0, newitemsize * sizeof (momitem_t *));
	  memcpy (newitemarr, itemarr, itemcount * sizeof (momitem_t *));
	  momitem_t **olditemarr = itemarr;
	  itemarr = newitemarr;
	  GC_FREE (olditemarr);
	}
      // inner loop inside the bucket
      for (unsigned oix = 0; oix < curbsize; oix++)
	{
	  const momitem_t *curitm = curbucket->buck_items[oix];
	  if (!curitm)
	    continue;
	  itemarr[itemcount++] = (momitem_t *) curitm;
	  GC_unregister_disappearing_link ((void **)
					   &curbucket->buck_items[oix]);
	  curbucket->buck_items[oix] = NULL;
	  curbucket->buck_count--;
	}
      buckets_mom.itbuck_arr[buckix] = NULL;
      GC_FREE (curbucket);
    };
  /// recreate the buffer array
  unsigned newnbbuckets =
    ((5 * (itemcount / PREFERRED_BUCKET_SIZE_MOM) / 4 + 10 +
      morebuckets) | 0x1f) + 1;
  GC_FREE (buckets_mom.itbuck_arr), buckets_mom.itbuck_arr = NULL;
  buckets_mom.itbuck_arr =
    GC_MALLOC (newnbbuckets * sizeof (struct itembucket_mom_st *));
  if (!buckets_mom.itbuck_arr)
    MOM_FATAPRINTF ("failed to create %d buckets", newnbbuckets);
  memset (buckets_mom.itbuck_arr, 0,
	  newnbbuckets * sizeof (struct itembucket_mom_st *));
  buckets_mom.itbuck_size = newnbbuckets;
  buckets_mom.itbuck_nbitems = 0;
  /// add the remembered items
  for (unsigned long itemix = 0; itemix < itemcount; itemix++)
    {
      const momitem_t *curitm = itemarr[itemix];
      assert (curitm != NULL && curitm->i_typnum == momty_item);
      if (add_item_mom (curitm))
	buckets_mom.itbuck_nbitems++;
      else
	MOM_FATAPRINTF ("item corruption, curitm @%p found twice", curitm);
    }
  memset (itemarr, 0, itemcount * sizeof (momitem_t *));
  GC_FREE (itemarr), itemarr = NULL;
}
