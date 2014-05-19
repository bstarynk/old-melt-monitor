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

static int64_t nb_creation_items_mom;
static int64_t nb_destruction_items_mom;

struct dictent_mom_st
{
  const momstring_t *dicent_name;
  momitem_t *dicent_item;
};

static struct
{
  unsigned dict_size;
  unsigned dict_count;
  struct dictent_mom_st *dict_array;
} dict_mom;

static void finalize_item_mom (void *itmad, void *data);

void
mom_initialize_items (void)
{
  const unsigned inibuck = 128;
  buckets_mom.itbuck_arr =
    GC_MALLOC (inibuck * sizeof (struct itembucket_mom_st *));
  if (!buckets_mom.itbuck_arr)
    MOM_FATAPRINTF ("failed to create %d buckets for items", inibuck);
  memset (buckets_mom.itbuck_arr, 0,
	  inibuck * sizeof (struct itembucket_mom_st *));
  buckets_mom.itbuck_size = inibuck;
  const unsigned inidict = 32;
  dict_mom.dict_array = GC_MALLOC (inidict * sizeof (struct dictent_mom_st));
  if (!dict_mom.dict_array)
    MOM_FATAPRINTF ("failed to create dictionnary of %d", inidict);
  memset (dict_mom.dict_array, 0, inidict * sizeof (struct dictent_mom_st));
  dict_mom.dict_size = inidict;
}


static struct itembucket_mom_st *
find_bucket_for_idstr_mom (const momstring_t *idstr)
{
  if (!idstr || idstr->typnum != momty_string
      || idstr->slen != MOM_IDSTRING_LEN
      || idstr->cstr[0] != '_' || !isdigit (idstr->cstr[1]))
    return NULL;
  momhash_t h = idstr->hash;
  unsigned bix = h % buckets_mom.itbuck_size;
  struct itembucket_mom_st *curbuck = buckets_mom.itbuck_arr[bix];
  return curbuck;
}

static struct itembucket_mom_st *
find_bucket_for_cstr_mom (const char *cstr, momhash_t h)
{
  if (!cstr || cstr[0] != '_' || !isdigit (cstr[1]))
    return NULL;
  if (h)
    h = mom_cstring_hash (cstr);
  unsigned bix = h % buckets_mom.itbuck_size;
  struct itembucket_mom_st *curbuck = buckets_mom.itbuck_arr[bix];
  return curbuck;
}

// return true if item is added
static bool
add_item_mom (const momitem_t *aitm)
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


#define REORGANIZE_ITEM_PERIOD_MOM 2048

momitem_t *
mom_make_item (void)
{
  momitem_t *newitm = GC_MALLOC (sizeof (momitem_t));
  if (MOM_UNLIKELY (!newitm))
    MOM_FATAPRINTF ("failed to allocate item");
  memset (newitm, 0, sizeof (momitem_t));
  const momstring_t *ids = mom_make_random_idstr ();
  assert (ids && ids->typnum == momty_string
	  && ids->slen == MOM_IDSTRING_LEN);
  momhash_t h = ids->hash;
  assert (h != 0);
  pthread_mutex_init (&newitm->i_mtx, &mom_recursive_mutex_attr);
  *(momtynum_t *) (&newitm->i_typnum) = momty_item;
  *(momhash_t *) (&newitm->i_hash) = h;
  *(momstring_t **) (&newitm->i_idstr) = (momstring_t *) ids;
  *(unsigned *) (&newitm->i_magic) = MOM_ITEM_MAGIC;
  pthread_mutex_lock (&globitem_mtx_mom);
  nb_creation_items_mom++;
  if (MOM_UNLIKELY (nb_creation_items_mom % REORGANIZE_ITEM_PERIOD_MOM == 0
		    || buckets_mom.itbuck_size * (3 *
						  PREFERRED_BUCKET_SIZE_MOM) >
		    2 * buckets_mom.itbuck_nbitems))
    reorganize_items_mom (15 +
			  buckets_mom.itbuck_nbitems / (8 *
							PREFERRED_BUCKET_SIZE_MOM));
  if (MOM_UNLIKELY (!add_item_mom (newitm)))
    MOM_FATAPRINTF ("failed to add new item @%p", newitm);
  // see
  // http://www.hpl.hp.com/hosted/linux/mail-archives/gc/2009-June/002786.html
  GC_REGISTER_FINALIZER (newitm, finalize_item_mom, NULL, NULL, NULL);
  goto end;
end:
  pthread_mutex_unlock (&globitem_mtx_mom);
  return newitm;
}


momitem_t *
mom_make_item_of_ident (const momstring_t *idstr)
{
  momitem_t *itm = NULL;
  if (!idstr || idstr->typnum != momty_string
      || idstr->slen != MOM_IDSTRING_LEN)
    return NULL;
  {
    const char *end = NULL;
    if (!mom_looks_like_random_id_cstr (idstr->cstr, &end) || !end || *end)
      return NULL;
  }
  momhash_t idh = idstr->hash;
  pthread_mutex_lock (&globitem_mtx_mom);
  {
    struct itembucket_mom_st *curbuck = find_bucket_for_idstr_mom (idstr);
    if (curbuck)
      {
	unsigned bsiz = curbuck->buck_size;
	for (unsigned bix = 0; bix < bsiz; bix++)
	  {
	    const momitem_t *curitm = curbuck->buck_items[bix];
	    if (!curitm)
	      continue;
	    if (curitm->i_hash == idh
		&& !memcmp (idstr->cstr, curitm->i_idstr->cstr,
			    MOM_IDSTRING_LEN))
	      {
		itm = (momitem_t *) curitm;
		goto end;
	      }
	  }
      }
  }
  itm = GC_MALLOC (sizeof (momitem_t));
  if (MOM_UNLIKELY (!itm))
    MOM_FATAPRINTF ("failed to allocate item of ident %s", idstr->cstr);
  memset (itm, 0, sizeof (momitem_t));
  pthread_mutex_init (&itm->i_mtx, &mom_recursive_mutex_attr);
  *(momtynum_t *) (&itm->i_typnum) = momty_item;
  *(momhash_t *) (&itm->i_hash) = idh;
  *(momstring_t **) (&itm->i_idstr) = (momstring_t *) idstr;
  *(unsigned *) (&itm->i_magic) = MOM_ITEM_MAGIC;
  nb_creation_items_mom++;
  if (MOM_UNLIKELY (nb_creation_items_mom % REORGANIZE_ITEM_PERIOD_MOM == 0
		    || buckets_mom.itbuck_size * (3 *
						  PREFERRED_BUCKET_SIZE_MOM) >
		    2 * buckets_mom.itbuck_nbitems))
    reorganize_items_mom (15 +
			  buckets_mom.itbuck_nbitems / (8 *
							PREFERRED_BUCKET_SIZE_MOM));
  if (MOM_UNLIKELY (!add_item_mom (itm)))
    MOM_FATAPRINTF ("failed to add new item @%p", itm);
  // see
  // http://www.hpl.hp.com/hosted/linux/mail-archives/gc/2009-June/002786.html
  GC_REGISTER_FINALIZER (itm, finalize_item_mom, NULL, NULL, NULL);
end:
  pthread_mutex_unlock (&globitem_mtx_mom);
  return itm;
}

momitem_t *
mom_make_item_of_identcstr (const char *idstr)
{
  if (!idstr || idstr[0] != '_')
    return NULL;
  {
    const char *end = NULL;
    if (!mom_looks_like_random_id_cstr (idstr, &end) || !end || *end)
      return NULL;
  }
  return mom_make_item_of_ident (mom_make_string (idstr));
}


momitem_t *
mom_get_item_of_ident (const momstring_t *idstr)
{
  momitem_t *itm = NULL;
  if (!idstr || idstr->typnum != momty_string
      || idstr->slen != MOM_IDSTRING_LEN)
    return NULL;
  {
    const char *end = NULL;
    if (!mom_looks_like_random_id_cstr (idstr->cstr, &end) || !end || *end)
      return NULL;
  }
  momhash_t idh = idstr->hash;
  pthread_mutex_lock (&globitem_mtx_mom);
  {
    struct itembucket_mom_st *curbuck = find_bucket_for_idstr_mom (idstr);
    if (curbuck)
      {
	unsigned bsiz = curbuck->buck_size;
	for (unsigned bix = 0; bix < bsiz; bix++)
	  {
	    const momitem_t *curitm = curbuck->buck_items[bix];
	    if (!curitm)
	      continue;
	    if (curitm->i_hash == idh
		&& !memcmp (idstr->cstr, curitm->i_idstr->cstr,
			    MOM_IDSTRING_LEN))
	      {
		itm = (momitem_t *) curitm;
		goto end;
	      }
	  }
      }
  }
end:
  pthread_mutex_unlock (&globitem_mtx_mom);
  return itm;
}



momitem_t *
mom_get_item_of_identcstr (const char *idcstr)
{
  {
    const char *end = NULL;
    if (!mom_looks_like_random_id_cstr (idcstr, &end) || !end || *end)
      return NULL;
  }
  return mom_get_item_of_ident (mom_make_string (idcstr));
}



static void
finalize_item_mom (void *itmad, void *data)
{
  momitem_t *itm = (momitem_t *) itmad;
  assert (itm->i_typnum == momty_item && itm->i_magic == MOM_ITEM_MAGIC);
  pthread_mutex_destroy (&itm->i_mtx);
  pthread_mutex_lock (&globitem_mtx_mom);
  nb_destruction_items_mom++;
  pthread_mutex_unlock (&globitem_mtx_mom);
}


// return the index of a given string, or -1 if not found
static int
index_dict_mom (const char *namcstr, momhash_t namh)
{
  if (!namcstr || !namcstr[0])
    return -1;
  if (!namh)
    namh = mom_cstring_hash (namcstr);
  unsigned dsize = dict_mom.dict_size;
  struct dictent_mom_st *darr = dict_mom.dict_array;
  unsigned istart = namh / dsize;
  for (unsigned i = istart; i < dsize; i++)
    {
      const momstring_t *curnam = darr[i].dicent_name;
      if ((void *) curnam == MOM_EMPTY
	  || (void *) darr[i].dicent_item == MOM_EMPTY)
	continue;
      if (!curnam)
	return -1;
      if (curnam->hash == namh && !strcmp (curnam->cstr, namcstr))
	return i;
    }
  for (unsigned i = 0; i < istart; i++)
    {
      const momstring_t *curnam = darr[i].dicent_name;
      if ((void *) curnam == MOM_EMPTY
	  || (void *) darr[i].dicent_item == MOM_EMPTY)
	continue;
      if (!curnam)
	return -1;
      if (curnam->hash == namh && !strcmp (curnam->cstr, namcstr))
	return i;
    }
  return -1;
}

static int
add_dict_mom (const momstring_t *nam, const momitem_t *itm)
{
  if (!nam || !itm || nam->typnum != momty_string || nam->slen == 0
      || itm->i_typnum != momty_item)
    return -1;
  momhash_t h = nam->hash;
  assert (h != 0);
  unsigned dsize = dict_mom.dict_size;
  struct dictent_mom_st *darr = dict_mom.dict_array;
  assert (dict_mom.dict_count + 1 < dsize);
  unsigned istart = h / dsize;
  int pos = -1;
  for (unsigned i = istart; i < dsize; i++)
    {
      const momstring_t *curnam = darr[i].dicent_name;
      if ((void *) curnam == MOM_EMPTY
	  || (void *) darr[i].dicent_item == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = i;
	  continue;
	}
      if (!curnam)
	{
	  if (pos < 0)
	    pos = i;
	  break;
	};
      if (curnam->hash == h && !strcmp (curnam->cstr, nam->cstr))
	return i;
    }
  for (unsigned i = 0; i < istart; i++)
    {
      const momstring_t *curnam = darr[i].dicent_name;
      if ((void *) curnam == MOM_EMPTY
	  || (void *) darr[i].dicent_item == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = i;
	  continue;
	}
      if (!curnam)
	{
	  if (pos < 0)
	    pos = i;
	  break;
	};
      if (curnam->hash == h && !strcmp (curnam->cstr, nam->cstr))
	return i;
    }
  if (pos >= 0)
    {
      darr[pos].dicent_item = (momitem_t *) itm;
      darr[pos].dicent_name = nam;
      dict_mom.dict_count++;
    }
  return pos;
}

void
reorganize_dict (unsigned more)
{
  unsigned oldsize = dict_mom.dict_size;
  unsigned oldcnt = dict_mom.dict_count;
  struct dictent_mom_st *oldarr = dict_mom.dict_array;
  unsigned newsize = ((3 * oldcnt / 2 + more + 10) | 0x1f) + 1;
  struct dictent_mom_st *newarr =
    GC_MALLOC (newsize * sizeof (struct dictent_mom_st));
  if (MOM_UNLIKELY (!newarr))
    MOM_FATAPRINTF ("failed to grow dictionnary to %d", newsize);
  memset (newarr, 0, newsize * sizeof (struct dictent_mom_st));
  dict_mom.dict_size = newsize;
  dict_mom.dict_count = 0;
  dict_mom.dict_array = newarr;
  for (unsigned i = 0; i < oldsize; i++)
    {
      const momstring_t *curnam = oldarr[i].dicent_name;
      if (!curnam)
	continue;
      if ((void *) curnam == MOM_EMPTY)
	continue;
      const momitem_t *curitm = oldarr[i].dicent_item;
      if (!curitm || (void *) curitm == MOM_EMPTY)
	continue;
      add_dict_mom (curnam, curitm);
    }
  assert (dict_mom.dict_count == oldcnt);
  memset (oldarr, 0, oldsize * sizeof (struct dictent_mom_st));
  GC_FREE (oldarr);
}



void
mom_register_item_named (momitem_t *itm, const momstring_t *name)
{
  if (!itm || itm->i_typnum != momty_item || !name
      || name->typnum != momty_string || isalpha (name->cstr[0]))
    return;
  unsigned nlen = name->slen;
  for (unsigned cix = 0; cix < nlen; cix++)
    if (!isalnum (name->cstr[cix]) && name->cstr[cix] != '_')
      return;
  pthread_mutex_lock (&globitem_mtx_mom);
  if (5 * dict_mom.dict_count + 10 > 4 * dict_mom.dict_size)
    reorganize_dict (dict_mom.dict_count / 8 + 2);
  int nix = index_dict_mom (name->cstr, name->hash);
  if (nix >= 0)
    {
      momitem_t *olditm = dict_mom.dict_array[nix].dicent_item;
      assert (olditm && olditm != MOM_EMPTY
	      && olditm->i_typnum == momty_item);
      pthread_mutex_lock (&olditm->i_mtx);
      assert (olditm->i_name == dict_mom.dict_array[nix].dicent_name);
      olditm->i_name = NULL;
      pthread_mutex_unlock (&olditm->i_mtx);
      dict_mom.dict_array[nix].dicent_item = itm;
    }
  else
    {
      add_dict_mom (name, itm);
    };
  pthread_mutex_lock (&itm->i_mtx);
  itm->i_name = name;
  pthread_mutex_unlock (&itm->i_mtx);
  pthread_mutex_unlock (&globitem_mtx_mom);
}



void
mom_forget_name (const char *namestr)
{
  if (!namestr || !isalpha (namestr[0]))
    return;
  pthread_mutex_lock (&globitem_mtx_mom);
  int nix = index_dict_mom (namestr, 0);
  if (nix >= 0)
    {
      momitem_t *olditm = dict_mom.dict_array[nix].dicent_item;
      assert (olditm && olditm != MOM_EMPTY
	      && olditm->i_typnum == momty_item);
      pthread_mutex_lock (&olditm->i_mtx);
      assert (olditm->i_name == dict_mom.dict_array[nix].dicent_name);
      olditm->i_name = NULL;
      pthread_mutex_unlock (&olditm->i_mtx);
      dict_mom.dict_count--;
      if (dict_mom.dict_size > 100
	  && 4 * dict_mom.dict_count < dict_mom.dict_size)
	reorganize_dict (dict_mom.dict_count / 8 + 2);
    }
  pthread_mutex_unlock (&globitem_mtx_mom);
}

const momstring_t *
mom_item_get_name (momitem_t *itm)
{
  const momstring_t *namev = NULL;
  if (!itm || !itm->i_typnum == momty_item)
    return NULL;
  pthread_mutex_lock (&itm->i_mtx);
  namev = itm->i_name;
  pthread_mutex_unlock (&itm->i_mtx);
  return namev;
}

const momstring_t *
mom_item_get_idstr (momitem_t *itm)
{
  const momstring_t *idsv = NULL;
  if (!itm || !itm->i_typnum == momty_item)
    return NULL;
  pthread_mutex_lock (&itm->i_mtx);
  idsv = itm->i_idstr;
  pthread_mutex_unlock (&itm->i_mtx);
  return idsv;
}


const momstring_t *
mom_item_get_name_or_idstr (momitem_t *itm)
{
  const momstring_t *strv = NULL;
  if (!itm || !itm->i_typnum == momty_item)
    return NULL;
  pthread_mutex_lock (&itm->i_mtx);
  strv = itm->i_name;
  if (!strv)
    strv = itm->i_idstr;
  pthread_mutex_unlock (&itm->i_mtx);
  return strv;
}

momitem_t *
mom_get_item_of_name_hash (const char *s, momhash_t h)
{
  momitem_t *itm = NULL;
  if (!s || !isalpha (s[0]))
    return NULL;
  if (!h)
    h = mom_cstring_hash (s);
  pthread_mutex_lock (&globitem_mtx_mom);
  int nix = index_dict_mom (s, h);
  if (nix >= 0)
    {
      itm = dict_mom.dict_array[nix].dicent_item;
      assert (itm && itm != MOM_EMPTY && itm->i_typnum == momty_item);
    };
  pthread_mutex_unlock (&globitem_mtx_mom);
  return itm;
}

momitem_t *
mom_get_item_of_name_or_ident_cstr_hash (const char *s, momhash_t h)
{
  momitem_t *itm = NULL;
  const char *end = NULL;
  if (!s || !s[0])
    return NULL;
  if (!(isalpha (s[0]) || (s[0] == '_' && isdigit (s[1]))))
    return NULL;
  if (!h)
    h = mom_cstring_hash (s);
  pthread_mutex_lock (&globitem_mtx_mom);
  if (s[0] == '_' && mom_looks_like_random_id_cstr (s, &end)
      && end && *end == (char) 0)
    {
      struct itembucket_mom_st *curbuck = find_bucket_for_cstr_mom (s, h);
      if (curbuck)
	{
	  unsigned bsiz = curbuck->buck_size;
	  for (unsigned bix = 0; bix < bsiz; bix++)
	    {
	      const momitem_t *curitm = curbuck->buck_items[bix];
	      if (!curitm)
		continue;
	      if (curitm->i_hash == h
		  && !memcmp (s, curitm->i_idstr->cstr, MOM_IDSTRING_LEN))
		{
		  itm = (momitem_t *) curitm;
		  goto end;
		}
	    }
	}
    }
  else if (isalpha (s[0]))
    {
      int nix = index_dict_mom (s, h);
      if (nix >= 0)
	{
	  itm = dict_mom.dict_array[nix].dicent_item;
	  assert (itm && itm != MOM_EMPTY && itm->i_typnum == momty_item);
	};
    }
end:
  pthread_mutex_unlock (&globitem_mtx_mom);
  return itm;
}

////////////////////////////////////////////////////////////////
const momset_t *
mom_set_of_named_items (void)
{
  const momset_t *set = NULL;
  const momitem_t **arr = NULL;
  unsigned siz = 0;
  unsigned cnt = 0;
  pthread_mutex_lock (&globitem_mtx_mom);
  siz = dict_mom.dict_count + 2;
  unsigned dicsiz = dict_mom.dict_size;
  assert (dicsiz > 0 && dict_mom.dict_array != NULL);
  arr = MOM_GC_ALLOC ("named array", siz * sizeof (momitem_t *));
  for (unsigned dix = 0; dix < dicsiz; dix++)
    {
      const momitem_t *curitm = dict_mom.dict_array[dix].dicent_item;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      assert (cnt < siz);
      arr[cnt++] = curitm;
    };
  assert (cnt == dict_mom.dict_count);
  set = mom_make_set_from_array (cnt, arr);
  MOM_GC_FREE (arr);
  pthread_mutex_unlock (&globitem_mtx_mom);
  return set;
}

static int
item_name_cmp_mom (const void *l, const void *r)
{
  const momitem_t *litm = *(momitem_t **) l;
  const momitem_t *ritm = *(momitem_t **) r;
  assert (litm && litm->i_typnum == momty_item && litm->i_name);
  assert (ritm && ritm->i_typnum == momty_item && ritm->i_name);
  return strcmp (litm->i_name->cstr, ritm->i_name->cstr);
}

const momtuple_t *
mom_alpha_ordered_tuple_of_named_items (void)
{
  const momtuple_t *tup = NULL;
  const momitem_t **arr = NULL;
  unsigned siz = 0;
  unsigned cnt = 0;
  pthread_mutex_lock (&globitem_mtx_mom);
  siz = dict_mom.dict_count + 2;
  unsigned dicsiz = dict_mom.dict_size;
  assert (dicsiz > 0 && dict_mom.dict_array != NULL);
  arr = MOM_GC_ALLOC ("named array", siz * sizeof (momitem_t *));
  for (unsigned dix = 0; dix < dicsiz; dix++)
    {
      const momitem_t *curitm = dict_mom.dict_array[dix].dicent_item;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      assert (cnt < siz);
      arr[cnt++] = curitm;
    };
  assert (cnt == dict_mom.dict_count);
  qsort (arr, cnt, sizeof (momitem_t *), item_name_cmp_mom);
  tup = mom_make_tuple_from_array (cnt, arr);
  MOM_GC_FREE (arr);
  pthread_mutex_unlock (&globitem_mtx_mom);
  return tup;
}

////////////////////////////////////////////////////////////////
const momitem_t *
mom_get_item_bool (bool v)
{
  if (v)
    return mom_named__json_true;
  else
    return mom_named__json_false;
}


////////////////////////////////////////////////////////////////
void
mom_create_predefined_items (void)
{
  int nbnamed = 0, nbanonym = 0;
#define MOM_PREDEFINED_NAMED(Nam,Id) do {			\
    mom_named__##Nam = mom_make_item_of_identcstr(#Id);		\
    mom_named__##Nam->i_space = momspa__predefined;		\
    mom_register_item_named_cstr (mom_named__##Nam, #Nam);	\
    nbnamed ++;							\
  } while(0);
#define MOM_PREDEFINED_ANONYMOUS(Id) do {			\
  mom_anonymous_##Id =  mom_make_item_of_identcstr (#Id);	\
  mom_anonymous_##Id->i_space = momspa__predefined;		\
  nbanonym ++;							\
  } while(0);
#include "predefined.h"
  MOM_INFORMPRINTF ("created predefined %d named, %d anonymous items",
		    nbnamed, nbanonym);
}				// end of mom_create_predefined_items

// declare the predefined
#define MOM_PREDEFINED_NAMED(Nam,Id) momitem_t* mom_named__##Nam;
#define MOM_PREDEFINED_ANONYMOUS(Id) momitem_t* mom_anonymous_##Id;
#include "predefined.h"
