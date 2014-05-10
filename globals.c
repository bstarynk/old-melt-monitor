// file globals.c

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
// the glob_mtx should be recursive, since dump is locking it too..
static pthread_mutex_t glob_mtx = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;


struct mom_name_item_entry_st	// for dictionnary 
{
  const momstring_t *nme_str;
  const mom_anyitem_t *nme_itm;
};

static struct glob_dict_st
{
  unsigned name_count;		// number of named entries
  unsigned name_size;		// size of hash tables
  struct mom_name_item_entry_st *name_hashitem;	// hash table on items
  struct mom_name_item_entry_st *name_hashstr;	// hash table on strings
} glob_dict;


void
mom_initialize_globals (void)
{
  const unsigned dictsiz = 32;
  pthread_mutex_lock (&glob_mtx);
  glob_dict.name_hashitem =
    GC_MALLOC (sizeof (struct mom_name_item_entry_st) * dictsiz);
  glob_dict.name_hashstr =
    GC_MALLOC (sizeof (struct mom_name_item_entry_st) * dictsiz);
  if (!glob_dict.name_hashitem || !glob_dict.name_hashstr)
    MOM_FATAL ("failed to allocate %d globals", dictsiz);
  memset (glob_dict.name_hashitem, 0,
	  sizeof (struct mom_name_item_entry_st) * dictsiz);
  memset (glob_dict.name_hashstr, 0,
	  sizeof (struct mom_name_item_entry_st) * dictsiz);
  glob_dict.name_size = dictsiz;
  glob_dict.name_count = 0;
  pthread_mutex_unlock (&glob_mtx);
}

// internal routine to find the name index by string or -1 if not found
static inline int
find_name_index (const char *str, momhash_t h)
{
  if (!h)
    h = mom_string_hash (str, -1);
  unsigned size = glob_dict.name_size;
  unsigned istart = h % size;
  struct mom_name_item_entry_st *arrname = glob_dict.name_hashstr;
  for (unsigned i = istart; i < size; i++)
    {
      if (!arrname[i].nme_str)
	return -1;
      if (arrname[i].nme_str == MOM_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == h
	  && !strcmp (arrname[i].nme_str->cstr, str))
	return (int) i;
    }
  for (unsigned i = 0; i < istart; i++)
    {
      if (!arrname[i].nme_str)
	return -1;
      if (arrname[i].nme_str == MOM_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == h
	  && !strcmp (arrname[i].nme_str->cstr, str))
	return (int) i;
    }
  return -1;
}

// internal routine to find the item index by item or -1 if not found
static inline int
find_item_index (const mom_anyitem_t * itm)
{
  momhash_t h = itm->i_hash;
  unsigned size = glob_dict.name_size;
  unsigned istart = h % size;
  struct mom_name_item_entry_st *arritem = glob_dict.name_hashitem;
  for (unsigned i = istart; i < size; i++)
    {
      if (arritem[i].nme_itm == itm)
	return (int) i;
      if (!arritem[i].nme_itm)
	return -1;
      if (arritem[i].nme_itm == MOM_EMPTY)
	continue;
    }
  for (unsigned i = 0; i < istart; i++)
    {
      if (arritem[i].nme_itm == itm)
	return (int) i;
      if (!arritem[i].nme_itm)
	return -1;
      if (arritem[i].nme_itm == MOM_EMPTY)
	continue;
    }
  return -1;
}

// return a GC_MALLOC_ATOMIC array of indexes of names starting with a
// given prefix, and set *pnbindex to the number of found indexes...
static inline int *
indexes_of_name_starting (const char *prefix, int *pnbindex)
{
  int *res = NULL;
  if (!prefix)
    prefix = "";
  unsigned lenprefix = strlen (prefix);
  if (pnbindex)
    *pnbindex = 0;
  unsigned size = glob_dict.name_size;
  struct mom_name_item_entry_st *arritem = glob_dict.name_hashitem;
  int count = 0;
  // first loop to find the number of matching indexes
  for (unsigned ix = 0; ix < size; ix++)
    {
      if (!arritem[ix].nme_itm || arritem[ix].nme_itm == MOM_EMPTY)
	continue;
      if (!arritem[ix].nme_str || arritem[ix].nme_str == MOM_EMPTY)
	continue;
      assert (arritem[ix].nme_str->typnum == momty_string);
      if (strncmp (arritem[ix].nme_str->cstr, prefix, lenprefix) == 0)
	count++;
    }
  // allocate one extra element, useless except for assert in second
  // loop
  res = count ? GC_MALLOC_ATOMIC ((count + 1) * sizeof (int)) : NULL;
  if (MOM_UNLIKELY (!res && count > 0))
    MOM_FATAL ("failed to allocate %d integers", count);
  if (res)
    for (int i = 0; i <= count; i++)
      res[i] = -1;
  int nbres = 0;
  // second loop to fill the index table
  if (count > 0)
    for (unsigned ix = 0; ix < size; ix++)
      {
	assert (nbres <= count);
	if (!arritem[ix].nme_itm || arritem[ix].nme_itm == MOM_EMPTY)
	  continue;
	if (!arritem[ix].nme_str || arritem[ix].nme_str == MOM_EMPTY)
	  continue;
	assert (arritem[ix].nme_str->typnum == momty_string);
	if (strncmp (arritem[ix].nme_str->cstr, prefix, lenprefix) == 0)
	  res[nbres++] = ix;
      };
  assert (nbres == count);
  if (pnbindex)
    *pnbindex = count;
  return res;
}


mom_anyitem_t *
mom_item_named (const char *name)
{
  const mom_anyitem_t *itm = NULL;
  if (!name || !name[0])
    return NULL;
  pthread_mutex_lock (&glob_mtx);
  int ix = find_name_index (name, 0);
  if (ix >= 0)
    itm = glob_dict.name_hashstr[ix].nme_itm;
  pthread_mutex_unlock (&glob_mtx);
  return (mom_anyitem_t *) itm;
}


mom_anyitem_t *
mom_item_of_name_string (momval_t namev)
{
  const mom_anyitem_t *itm = NULL;
  if (!namev.ptr || *namev.ptype != momty_string)
    return NULL;
  pthread_mutex_lock (&glob_mtx);
  int ix = find_name_index (namev.pstring->cstr, namev.pstring->hash);
  if (ix >= 0)
    itm = glob_dict.name_hashstr[ix].nme_itm;
  pthread_mutex_unlock (&glob_mtx);
  return (mom_anyitem_t *) itm;
}

mom_anyitem_t *
mom_item_named_with_string (const char *name, const momstring_t ** pstr)
{
  const mom_anyitem_t *itm = NULL;
  if (!name || !name[0])
    return NULL;
  if (pstr)
    *pstr = NULL;
  pthread_mutex_lock (&glob_mtx);
  int ix = find_name_index (name, 0);
  if (ix >= 0)
    {
      itm = glob_dict.name_hashstr[ix].nme_itm;
      if (pstr)
	*pstr = glob_dict.name_hashstr[ix].nme_str;
    }
  pthread_mutex_unlock (&glob_mtx);
  return (mom_anyitem_t *) itm;
}

const momstring_t *
mom_name_of_item (const mom_anyitem_t * itm)
{
  const momstring_t *str = NULL;
  if (!itm || itm->typnum < momty__itemlowtype)
    return NULL;
  pthread_mutex_lock (&glob_mtx);
  int ix = find_item_index (itm);
  if (ix >= 0)
    str = glob_dict.name_hashitem[ix].nme_str;
  pthread_mutex_unlock (&glob_mtx);
  return str;
}

int
index_name_cmp (const void *p1, const void *p2)
{
  int i1 = *(int *) p1;
  int i2 = *(int *) p2;
  struct mom_name_item_entry_st *arritem = glob_dict.name_hashitem;
  assert (i1 >= 0 && i1 < glob_dict.name_size && arritem[i1].nme_str != NULL
	  && arritem[i1].nme_str != MOM_EMPTY);
  assert (i2 >= 0 && i2 < glob_dict.name_size && arritem[i2].nme_str != NULL
	  && arritem[i2].nme_str != MOM_EMPTY);
  return strcmp (arritem[i1].nme_str->cstr, arritem[i2].nme_str->cstr);
}

momval_t
mom_node_sorted_names_prefixed (const mom_anyitem_t * conn,
				const char *prefix)
{
  momval_t res = MOM_NULLV;
  if (!conn || conn->typnum < momty__itemlowtype)
    return MOM_NULLV;
  int nbindex = -1;
  if (!prefix)
    prefix = "";
  pthread_mutex_lock (&glob_mtx);
  int *arrindexes = indexes_of_name_starting (prefix, &nbindex);
  assert (nbindex >= 0);
  if (nbindex > 0)
    qsort (arrindexes, nbindex, sizeof (int), index_name_cmp);
  momval_t *arrsons =
    (nbindex > 0) ? GC_MALLOC (nbindex * sizeof (momval_t)) : NULL;
  if (MOM_UNLIKELY (nbindex > 0 && !arrsons))
    MOM_FATAL ("failed to allocate %d sons", nbindex);
  if (arrsons)
    memset (arrsons, 0, nbindex * sizeof (momval_t));
  for (int ix = 0; ix < nbindex; ix++)
    {
      int cix = arrindexes[ix];
      assert (cix >= 0 && cix < glob_dict.name_size);
      arrsons[ix] = (momval_t) glob_dict.name_hashitem[cix].nme_str;
      assert (arrsons[ix].ptr && arrsons[ix].ptr != MOM_EMPTY
	      && *arrsons[ix].ptype == momty_string);
    };
  pthread_mutex_unlock (&glob_mtx);
  res =
    (momval_t) mom_make_node_from_array (conn, (unsigned) nbindex, arrsons);
  GC_FREE (arrindexes), arrindexes = NULL;
  GC_FREE (arrsons), arrsons = NULL;
  return res;
}

momval_t
mom_set_named_items_prefixed (const char *prefix)
{
  momval_t res = MOM_NULLV;
  int nbindex = -1;
  if (!prefix)
    prefix = "";
  pthread_mutex_lock (&glob_mtx);
  int *arrindexes = indexes_of_name_starting (prefix, &nbindex);
  assert (nbindex >= 0);
  const mom_anyitem_t **arritems =
    (nbindex > 0) ? GC_MALLOC (nbindex * sizeof (mom_anyitem_t *)) : NULL;
  if (MOM_UNLIKELY (nbindex > 0 && !arritems))
    MOM_FATAL ("failed to allocate %d items", nbindex);
  if (arritems)
    memset (arritems, 0, nbindex * sizeof (mom_anyitem_t *));
  for (int ix = 0; ix < nbindex; ix++)
    {
      arritems[ix] = glob_dict.name_hashitem[ix].nme_itm;
      assert (arritems[ix] && arritems[ix] != MOM_EMPTY
	      && arritems[ix]->typnum > momty__itemlowtype);
    };
  res = (momval_t) mom_make_set_from_array (nbindex, arritems);
  pthread_mutex_unlock (&glob_mtx);
  GC_FREE (arritems), arritems = NULL;
  GC_FREE (arrindexes), arrindexes = NULL;
  return res;
}

// internal routine to add a name entry which is known to be new
static inline void
add_new_name_entry (const momstring_t * name, const mom_anyitem_t * item)
{
  unsigned size = glob_dict.name_size;
  momhash_t hashname = name->hash;
  momhash_t hashitem = item->i_hash;
  unsigned istartname = hashname % size;
  unsigned istartitem = hashitem % size;
  struct mom_name_item_entry_st *arrname = glob_dict.name_hashstr;
  struct mom_name_item_entry_st *arritem = glob_dict.name_hashitem;
  for (unsigned i = istartname; i < size; i++)
    {
      if (!arrname[i].nme_str || arrname[i].nme_str == MOM_EMPTY)
	{
	  arrname[i].nme_str = name;
	  arrname[i].nme_itm = item;
	  goto additem;
	}
    }
  for (unsigned i = 0; i < istartname; i++)
    {
      if (!arrname[i].nme_str || arrname[i].nme_str == MOM_EMPTY)
	{
	  arrname[i].nme_str = name;
	  arrname[i].nme_itm = item;
	  goto additem;
	}
    }
  // this should never happen
  MOM_FATAL ("corrupted dictionnary for names of size %d", (int) size);
additem:
  for (unsigned i = istartitem; i < size; i++)
    {
      if (!arritem[i].nme_str || arritem[i].nme_str == MOM_EMPTY)
	{
	  arritem[i].nme_str = name;
	  arritem[i].nme_itm = item;
	  goto end;
	}
    }
  for (unsigned i = 0; i < istartitem; i++)
    {
      if (!arritem[i].nme_str || arritem[i].nme_str == MOM_EMPTY)
	{
	  arritem[i].nme_str = name;
	  arritem[i].nme_itm = item;
	  goto end;
	}
    }
  // this should never happen
  MOM_FATAL ("corrupted dictionnary for items of size %d", (int) size);
end:
  glob_dict.name_count++;
}

static void
resize_dict (unsigned newsize)
{
  unsigned oldcount = glob_dict.name_count;
  unsigned oldsize = glob_dict.name_size;
  if (newsize + 5 < 9 * oldcount / 8 || newsize == 0)
    MOM_FATAL ("invalid newsize %u for dictonnary count %u",
	       newsize, oldcount);
  if (newsize == oldsize)
    return;
  struct mom_name_item_entry_st *oldarrname = glob_dict.name_hashstr;
  struct mom_name_item_entry_st *oldarritem = glob_dict.name_hashitem;
  glob_dict.name_hashstr =
    GC_MALLOC (sizeof (struct mom_name_item_entry_st) * newsize);
  if (!glob_dict.name_hashstr)
    MOM_FATAL ("failed to grow dictionnary string hash to %u", newsize);
  memset (glob_dict.name_hashstr, 0,
	  sizeof (struct mom_name_item_entry_st) * newsize);
  glob_dict.name_hashitem =
    GC_MALLOC (sizeof (struct mom_name_item_entry_st) * newsize);
  if (!glob_dict.name_hashitem)
    MOM_FATAL ("failed to grow dictionnary item hash to %u", newsize);
  memset (glob_dict.name_hashitem, 0,
	  sizeof (struct mom_name_item_entry_st) * newsize);
  glob_dict.name_count = 0;
  glob_dict.name_size = newsize;
  for (unsigned i = 0; i < oldsize; i++)
    {
      struct mom_name_item_entry_st *curent = oldarrname + i;
      if (!curent->nme_str || curent->nme_str == MOM_EMPTY
	  || !curent->nme_itm || curent->nme_itm == MOM_EMPTY)
	continue;
      add_new_name_entry (curent->nme_str, curent->nme_itm);
    }
  if (glob_dict.name_count != oldcount)
    MOM_FATAL ("corrupted resized dictionnary old count=%u new count=%u",
	       oldcount, (unsigned) glob_dict.name_count);
  GC_FREE (oldarrname);
  GC_FREE (oldarritem);
}


void
mom_register_new_name_string (const momstring_t * namestr,
			      mom_anyitem_t * item)
{
  if (!namestr || namestr->typnum != momty_string)
    return;
  if (!item || item->typnum <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&glob_mtx);
  if (find_name_index (namestr->cstr, namestr->hash) >= 0)
    goto end;
  if (find_item_index (item) >= 0)
    goto end;
  if (4 * glob_dict.name_count > 3 * glob_dict.name_size)
    {
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 100) | 0x7f) + 1;
      resize_dict (newsize);
    }
  add_new_name_entry (namestr, item);
end:
  pthread_mutex_unlock (&glob_mtx);
}

void
mom_register_new_name_item (const char *name, mom_anyitem_t * item)
{
  if (!name || !name[0] || !item || item->typnum <= momty__itemlowtype)
    return;
  unsigned namelen = strlen (name);
  pthread_mutex_lock (&glob_mtx);
  if (find_name_index (name, mom_string_hash (name, namelen)) >= 0)
    goto end;
  if (find_item_index (item) >= 0)
    goto end;
  if (4 * glob_dict.name_count > 3 * glob_dict.name_size)
    {
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 100) | 0x7f) + 1;
      resize_dict (newsize);
    }
  const momstring_t *namestr = mom_make_string_len (name, namelen);
  add_new_name_entry (namestr, item);
end:
  pthread_mutex_unlock (&glob_mtx);
}

static void
remove_entry (const momstring_t * name, const mom_anyitem_t * itm)
{
  unsigned size = glob_dict.name_size;
  momhash_t hashname = name->hash;
  momhash_t hashitem = itm->i_hash;
  unsigned istartname = hashname % size;
  unsigned istartitem = hashitem % size;
  struct mom_name_item_entry_st *arrname = glob_dict.name_hashstr;
  struct mom_name_item_entry_st *arritem = glob_dict.name_hashitem;
  for (unsigned i = istartname; i < size; i++)
    {
      if (!arrname[i].nme_str)
	return;
      if (arrname[i].nme_str == MOM_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == hashname
	  && (arrname[i].nme_str == name
	      || !strcmp (arrname[i].nme_str->cstr, name->cstr)))
	{
	  arrname[i].nme_str = MOM_EMPTY;
	  arrname[i].nme_itm = MOM_EMPTY;
	  goto remove_item;
	}
    }
  for (unsigned i = 0; i < istartname; i++)
    {
      if (!arrname[i].nme_str)
	return;
      if (arrname[i].nme_str == MOM_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == hashname
	  && (arrname[i].nme_str == name
	      || !strcmp (arrname[i].nme_str->cstr, name->cstr)))
	{
	  arrname[i].nme_str = MOM_EMPTY;
	  arrname[i].nme_itm = MOM_EMPTY;
	  goto remove_item;
	}
    }
  // the name was not found
  return;
remove_item:
  for (unsigned i = istartitem; i < size; i++)
    {
      if (arritem[i].nme_itm == itm)
	{
	  arritem[i].nme_str = MOM_EMPTY;
	  arritem[i].nme_itm = MOM_EMPTY;
	  goto end;
	}
      else if (!arritem[i].nme_itm)	// should not happen
	MOM_FATAL ("corrupted item dict of size %u", size);
      else if (arritem[i].nme_itm == MOM_EMPTY)
	continue;
    }
  for (unsigned i = 0; i < istartitem; i++)
    {
      if (arritem[i].nme_itm == itm)
	{
	  arritem[i].nme_str = MOM_EMPTY;
	  arritem[i].nme_itm = MOM_EMPTY;
	  goto end;
	}
      else if (!arritem[i].nme_itm)	// should not happen
	MOM_FATAL ("corrupted item dict of size %u", size);
      else if (arritem[i].nme_itm == MOM_EMPTY)
	continue;
    }
  // should never happen
  MOM_FATAL ("corrupted dict of size %u", size);
end:
  glob_dict.name_count--;
}

void
mom_replace_name_string (const momstring_t * namestr, mom_anyitem_t * item)
{
  if (!namestr || namestr->typnum != momty_string)
    return;
  if (!item || item->typnum <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&glob_mtx);
  int namix = find_name_index (namestr->cstr, namestr->hash);
  if (namix >= 0)
    {
      remove_entry (glob_dict.name_hashstr[namix].nme_str,
		    glob_dict.name_hashstr[namix].nme_itm);
    }
  int itmix = find_item_index (item);
  if (itmix >= 0)
    {
      remove_entry (glob_dict.name_hashitem[itmix].nme_str,
		    glob_dict.name_hashitem[itmix].nme_itm);
    }

  if (4 * glob_dict.name_count > 3 * glob_dict.name_size)
    {
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 400) | 0xff) + 1;
      resize_dict (newsize);
    }
  add_new_name_entry (namestr, item);
  pthread_mutex_unlock (&glob_mtx);
}


void
mom_replace_named_item (const char *name, mom_anyitem_t * item)
{
  if (!name || !name[0])
    return;
  if (!item || item->typnum <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&glob_mtx);
  unsigned namelen = strlen (name);
  momhash_t namehash = mom_string_hash (name, namelen);
  const momstring_t *namestr = NULL;
  int namix = find_name_index (name, namehash);
  if (namix >= 0)
    {
      namestr = glob_dict.name_hashstr[namix].nme_str;
      remove_entry (namestr, glob_dict.name_hashstr[namix].nme_itm);
    }
  int itmix = find_item_index (item);
  if (itmix >= 0)
    {
      remove_entry (glob_dict.name_hashitem[itmix].nme_str,
		    glob_dict.name_hashitem[itmix].nme_itm);
    }

  if (4 * glob_dict.name_count > 3 * glob_dict.name_size)
    {
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 400) | 0xff) + 1;
      resize_dict (newsize);
    }
  if (!namestr)
    namestr = mom_make_string_len (name, namelen);
  add_new_name_entry (namestr, item);
  pthread_mutex_unlock (&glob_mtx);
}


void
mom_forget_name (const char *name)
{
  if (!name || !name[0])
    return;
  pthread_mutex_lock (&glob_mtx);
  unsigned namelen = strlen (name);
  momhash_t namehash = mom_string_hash (name, namelen);
  int namix = find_name_index (name, namehash);
  if (namix >= 0)
    {
      remove_entry (glob_dict.name_hashstr[namix].nme_str,
		    glob_dict.name_hashstr[namix].nme_itm);
    }
  pthread_mutex_unlock (&glob_mtx);
}

void
mom_forget_string (const momstring_t * namestr)
{
  if (!namestr || namestr->typnum != momty_string)
    return;
  pthread_mutex_lock (&glob_mtx);
  int namix = find_name_index (namestr->cstr, namestr->hash);
  if (namix >= 0)
    {
      remove_entry (glob_dict.name_hashstr[namix].nme_str,
		    glob_dict.name_hashstr[namix].nme_itm);
    }
  else
    goto unlock;
  if (4 * glob_dict.name_count > 3 * glob_dict.name_size)
    {
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 400) | 0xff) + 1;
      resize_dict (newsize);
    }
unlock:
  pthread_mutex_unlock (&glob_mtx);
}





void
mom_forget_item (mom_anyitem_t * item)
{
  if (!item || item->typnum <= momty__itemlowtype)
    return;
  assert (item->typnum < momty__last);
  pthread_mutex_lock (&glob_mtx);
  int itmix = find_item_index (item);
  if (itmix >= 0)
    {
      remove_entry (glob_dict.name_hashitem[itmix].nme_str,
		    glob_dict.name_hashitem[itmix].nme_itm);
      if (5 * glob_dict.name_count < glob_dict.name_size
	  && glob_dict.name_size > 350)
	{
	  unsigned newsize =
	    ((13 * glob_dict.name_count / 8 + 400) | 0xff) + 1;
	  if (newsize != glob_dict.name_size)
	    resize_dict (newsize);
	}
    }
  pthread_mutex_unlock (&glob_mtx);
}


void
mom_dump_globals (struct mom_dumper_st *dmp, mom_dumpglobal_sig_t * globcb,
		  void *data)
{
  // add global data
  pthread_mutex_lock (&glob_mtx);
  /// add the named items
  for (unsigned ix = 0; ix < glob_dict.name_size; ix++)
    {
      const mom_anyitem_t *itm = glob_dict.name_hashitem[ix].nme_itm;
      const momstring_t *nam = glob_dict.name_hashitem[ix].nme_str;
      if (!itm || (void *) itm == MOM_EMPTY || !nam
	  || (void *) nam == MOM_EMPTY)
	continue;
      mom_dump_add_item (dmp, (mom_anyitem_t *) itm);
      if (globcb)
	globcb (itm, nam, data);
    }
  pthread_mutex_unlock (&glob_mtx);
}


int
mom_tasklet_step (momit_tasklet_t * tskitm)
{
  int res = 0;
  if (!tskitm || tskitm->itk_item.typnum != momty_taskletitem)
    return 0;
  pthread_mutex_lock (&((mom_anyitem_t *) tskitm)->i_mtx);
  unsigned fratop = tskitm->itk_fratop;
  tskitm->itk_thread = pthread_self ();
  if (fratop == 0)
    goto end;
  struct momframe_st *curfram = tskitm->itk_frames + fratop - 1;
  uint32_t curstate = curfram->fr_state;
  uint32_t curintoff = curfram->fr_intoff;
  uint32_t curdbloff = curfram->fr_dbloff;
  uint32_t curvaloff = curfram->fr_valoff;
  momclosure_t *curclo = tskitm->itk_closures[fratop - 1];
  if (MOM_UNLIKELY (!curclo || curclo->typnum != momty_closure))
    {
      tskitm->itk_closures[fratop] = NULL;
      memset (tskitm->itk_frames + fratop, 0, sizeof (struct momframe_st));
      if (tskitm->itk_valtop > curvaloff)
	{
	  memset (tskitm->itk_values + curvaloff, 0,
		  sizeof (momval_t) * (tskitm->itk_valtop - curvaloff));
	  tskitm->itk_valtop = curvaloff;
	}
      if (tskitm->itk_scaltop > curintoff)
	{
	  memset (tskitm->itk_scalars + curintoff, 0,
		  sizeof (intptr_t) * (tskitm->itk_scaltop - curintoff));
	  tskitm->itk_scaltop = curintoff;
	}
      tskitm->itk_fratop = fratop - 1;
      goto end;
    }
  momit_routine_t *curout = (momit_routine_t *) (curclo->connitm);
  const struct momroutinedescr_st *rdescr = NULL;
  const momrout_sig_t *rcode = NULL;
  int nextstate = 0;
  // an incomplete routine would not have made a closure
  if (MOM_UNLIKELY
      (!curout || curout->irt_item.typnum != momty_routineitem
       || !(rdescr = curout->irt_descr) || rdescr->rout_magic != ROUTINE_MAGIC
       || !(rcode = rdescr->rout_code)))
    MOM_FATAL ("corrupted closure in tasklet");
  nextstate = rcode (curstate, tskitm, curclo,
		     tskitm->itk_values + curvaloff,
		     tskitm->itk_scalars + curintoff,
		     (double *) tskitm->itk_scalars + curdbloff);
  MOM_DEBUG (run, "tasklet_step nextstate=%d", (int) nextstate);
  if (nextstate > 0)
    {
      // the rcode might have changed the itk_frames so we can't use curfram
      tskitm->itk_frames[fratop - 1].fr_state = nextstate;
      // enqueue the current task into the agenda
      mom_agenda_add_tasklet_back ((momval_t) tskitm);
    }
  else if (nextstate == routres_pop)
    {
      // pop one frame
      tskitm->itk_closures[fratop] = NULL;
      memset (tskitm->itk_frames + fratop, 0, sizeof (struct momframe_st));
      if (tskitm->itk_valtop > curvaloff)
	{
	  memset (tskitm->itk_values + curvaloff, 0,
		  sizeof (momval_t) * (tskitm->itk_valtop - curvaloff));
	  tskitm->itk_valtop = curvaloff;
	}
      if (tskitm->itk_scaltop > curintoff)
	{
	  memset (tskitm->itk_scalars + curintoff, 0,
		  sizeof (intptr_t) * (tskitm->itk_scaltop - curintoff));
	  tskitm->itk_scaltop = curintoff;
	}
      tskitm->itk_fratop = fratop - 1;
      // if the tasklet is not empty, enqueue it again
      if (tskitm->itk_fratop > 0)
	mom_agenda_add_tasklet_back ((momval_t) tskitm);
    }
  res = nextstate;
end:
  tskitm->itk_thread = (pthread_t) 0;
  pthread_mutex_unlock (&((mom_anyitem_t *) tskitm)->i_mtx);
  return res;
}

int
mom_tasklet_depth (momval_t tsk)
{
  int top = 0;
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return 0;
  momit_tasklet_t *tskitm = tsk.ptaskitem;
  pthread_mutex_lock (&((mom_anyitem_t *) tskitm)->i_mtx);
  top = (int) tskitm->itk_fratop;
  pthread_mutex_unlock (&((mom_anyitem_t *) tskitm)->i_mtx);
  return top;
}


static void
compute_pushed_data_size (const momclosure_t * closure, unsigned *pnbval,
			  unsigned *pnbnum, unsigned *pnbdbl, int *pnewstate,
			  enum mom_pushframedirective_en dir, va_list args)
{
  unsigned nbval = 0;
  unsigned nbnum = 0;
  unsigned nbdbl = 0;
  int newstate = 0;
  bool again = true;
  while (again && dir != MOMPFR__END)
    {
      switch (dir)
	{
	case MOMPFR__END:
	  again = false;
	  break;
	case MOMPFR_STATE /*, int state  */ :
	  newstate = va_arg (args, int);
	  break;
	case MOMPFR_VALUE /*, momval_t val */ :
	  (void) va_arg (args, momval_t);
	  nbval++;
	  break;
	case MOMPFR_TWO_VALUES /*, momval_t val1, val2 */ :
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 2;
	  break;
	case MOMPFR_THREE_VALUES /*, momval_t val1, val2, val3 */ :
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 3;
	  break;
	case MOMPFR_FOUR_VALUES /*, momval_t val1, val2, val3, val4 */ :
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 4;
	  break;
	case MOMPFR_FIVE_VALUES /*, momval_t val1, val2, val3, val4, val5 */ :
	  (void) va_arg (args,
			 momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  nbval += 5;
	  break;
	case MOMPFR_ARRAY_VALUES /* unsigned count, momval_t valarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    momval_t *arr = va_arg (args, momval_t *);
	    if (MOM_UNLIKELY (!arr))
	      MOM_FATAL ("invalid array value to push");
	    nbval += count;
	  }
	  break;
	case MOMPFR_NODE_VALUES /* momnode_st* node, -- to push the sons of a node or closure */ :
	  {
	    momnode_t *nod = va_arg (args, momnode_t *);
	    if (nod
		&& (nod->typnum == momty_node
		    || nod->typnum == momty_closure))
	      nbval += nod->slen;
	  }
	  break;
	case MOMPFR_INT /*, intptr_t num */ :
	  {
	    (void) va_arg (args, intptr_t);
	    nbnum++;
	  }
	  break;
	case MOMPFR_TWO_INTS /*, intptr_t num1, num2 */ :
	  {
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    nbnum += 2;
	  }
	  break;
	case MOMPFR_THREE_INTS /*, intptr_t num1, num2, num3 */ :
	  {
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    nbnum += 3;
	  }
	  break;
	case MOMPFR_FOUR_INTS /*, intptr_t num1, num2, num3, num4 */ :
	  {
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    nbnum += 4;
	  }
	  break;
	case MOMPFR_FIVE_INTS /*, intptr_t num1, num2, num3, num4, num5 */ :
	  {
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    (void) va_arg (args, intptr_t);
	    nbnum += 5;
	  }
	  break;
	case MOMPFR_ARRAY_INTS /* unsigned count, intptr_t numarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    momval_t *arr = va_arg (args, intptr_t *);
	    if (MOM_UNLIKELY (!arr))
	      MOM_FATAL ("invalid integer value to push");
	    nbnum += count;
	  }
	  break;
	case MOMPFR_DOUBLE /*, double d */ :
	  {
	    (void) va_arg (args, double);
	    nbdbl++;
	  }
	  break;
	case MOMPFR_TWO_DOUBLES /*, double d1, d2 */ :
	  {
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    nbdbl += 2;
	  }
	  break;
	case MOMPFR_THREE_DOUBLES /*, double d1, d2, d3 */ :
	  {
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    nbdbl += 3;
	  }
	  break;
	case MOMPFR_FOUR_DOUBLES /*, double d1, d2, d3, d4 */ :
	  {
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    nbdbl += 4;
	  }
	  break;
	case MOMPFR_FIVE_DOUBLES /*, double d1, d2, d3, d4, d5 */ :
	  {
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    (void) va_arg (args, double);
	    nbdbl += 5;
	  }
	  break;
	case MOMPFR_ARRAY_DOUBLES /* unsigned count, double dblarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    double *arr = va_arg (args, double *);
	    if (MOM_UNLIKELY (!arr))
	      MOM_FATAL ("invalid double value to push");
	    nbdbl += count;
	  }
	  break;
	default:
	  MOM_FATAL ("unexpected push directive #%d", (int) dir);
	}
      if (again)
	dir = va_arg (args, enum mom_pushframedirective_en);
    }
  momit_routine_t *rout = (momit_routine_t *) closure->connitm;
  if (MOM_UNLIKELY (!rout || rout->irt_item.typnum != momty_routineitem))
    MOM_FATAL ("bad routine in closure");
  // an incomplete routine would not have made a closure
  struct momroutinedescr_st *rdescr = rout->irt_descr;
  if (MOM_UNLIKELY (!rdescr || rdescr->rout_magic != ROUTINE_MAGIC))
    MOM_FATAL ("corrupted routine in closure");
  if (nbval < rdescr->rout_frame_nbval)
    nbval = rdescr->rout_frame_nbval;
  if (nbnum < rdescr->rout_frame_nbnum)
    nbnum = rdescr->rout_frame_nbnum;
  if (nbdbl < rdescr->rout_frame_nbdbl)
    nbdbl = rdescr->rout_frame_nbdbl;
  if (nbnum % 2 != 0)
    nbnum++;
  if (nbval % 2 != 0)
    nbval++;
  if (nbdbl % 2 != 0)
    nbdbl++;
  *pnbval = nbval;
  *pnbnum = nbnum;
  *pnbdbl = nbdbl;
  *pnewstate = newstate;
}




static void
fill_frame_data (intptr_t * numdata, double *dbldata, momval_t * valdata,
		 enum mom_pushframedirective_en dir, va_list args)
{
  bool again = true;
  while (again && dir != MOMPFR__END)
    {
      switch (dir)
	{
	case MOMPFR__END:
	  again = false;
	  break;
	case MOMPFR_STATE /*, int state  */ :
	  break;
	case MOMPFR_VALUE /*, momval_t val */ :
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	case MOMPFR_TWO_VALUES /*, momval_t val1, val2 */ :
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	case MOMPFR_THREE_VALUES /*, momval_t val1, val2, val3 */ :
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	case MOMPFR_FOUR_VALUES /*, momval_t val1, val2, val3, val4 */ :
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	case MOMPFR_FIVE_VALUES /*, momval_t val1, val2, val3, val4, val5 */ :
	  *(valdata++) =
	    va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  *(valdata++) = va_arg (args, momval_t);
	  break;
	case MOMPFR_ARRAY_VALUES /* unsigned count, momval_t valarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    momval_t *arr = va_arg (args, momval_t *);
	    memcpy (valdata, arr, count * sizeof (momval_t));
	    valdata += count;
	  }
	  break;
	case MOMPFR_NODE_VALUES /* momnode_st* node, -- to push the sons of a node or closure */ :
	  {
	    momnode_t *nod = va_arg (args, momnode_t *);
	    if (nod
		&& (nod->typnum == momty_node
		    || nod->typnum == momty_closure))
	      {
		memcpy (valdata, nod->sontab, nod->slen * sizeof (momval_t));
		valdata += nod->slen;
	      }
	  }
	  break;
	case MOMPFR_INT /*, intptr_t num */ :
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	case MOMPFR_TWO_INTS /*, intptr_t num1, num2 */ :
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	case MOMPFR_THREE_INTS /*, intptr_t num1, num2, num3 */ :
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	case MOMPFR_FOUR_INTS /*, intptr_t num1, num2, num3, num4 */ :
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	case MOMPFR_FIVE_INTS /*, intptr_t num1, num2, num3, num4, num5 */ :
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  *(numdata++) = va_arg (args, intptr_t);
	  break;
	case MOMPFR_ARRAY_INTS /* unsigned count, intptr_t numarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    momval_t *arr = va_arg (args, intptr_t *);
	    memcpy (numdata, arr, count * sizeof (intptr_t));
	    numdata += count;
	  }
	  break;
	case MOMPFR_DOUBLE /*, double d */ :
	  *(dbldata++) = va_arg (args, double);
	  break;
	case MOMPFR_TWO_DOUBLES /*, double d1, d2 */ :
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	case MOMPFR_THREE_DOUBLES /*, double d1, d2, d3 */ :
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	case MOMPFR_FOUR_DOUBLES /*, double d1, d2, d3, d4 */ :
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	case MOMPFR_FIVE_DOUBLES /*, double d1, d2, d3, d4, d5 */ :
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  *(dbldata++) = va_arg (args, double);
	  break;
	case MOMPFR_ARRAY_DOUBLES /* unsigned count, double dblarr[count] */ :
	  {
	    unsigned count = va_arg (args, unsigned);
	    double *arr = va_arg (args, double *);
	    memcpy (dbldata, arr, count * sizeof (double));
	    dbldata += count;
	  }
	  break;
	default:
	  MOM_FATAL ("unexpected push directive #%d", (int) dir);
	}
      if (again)
	dir = va_arg (args, enum mom_pushframedirective_en);
    }
}




void
mom_tasklet_push_frame (momval_t tsk, momval_t clo,
			enum mom_pushframedirective_en firstdir, ...)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  if (!clo.ptr || *clo.ptype != momty_closure)
    return;
  unsigned nbval = 0;
  unsigned nbnum = 0;
  unsigned nbdbl = 0;
  int newstate = 0;
  const momclosure_t *closure = clo.pclosure;
  va_list args;
  // first, compute the data size
  va_start (args, firstdir);
  compute_pushed_data_size (closure, &nbval, &nbnum, &nbdbl, &newstate,
			    firstdir, args);
  va_end (args);
  momit_tasklet_t *tskitm = tsk.ptaskitem;
  pthread_mutex_lock (&((mom_anyitem_t *) tskitm)->i_mtx);
  if (MOM_UNLIKELY
      (tskitm->itk_scaltop +
       (sizeof (intptr_t) * nbnum +
	sizeof (double) * nbdbl) / sizeof (intptr_t) >= tskitm->itk_scalsize))
    {
      unsigned newscalsize =
	((5 * tskitm->itk_scaltop / 4 +
	  (sizeof (intptr_t *) * nbnum +
	   sizeof (double) * nbdbl) / sizeof (intptr_t) + 5) | 7) + 1;
      intptr_t *newscalars =
	GC_MALLOC_ATOMIC (newscalsize * sizeof (intptr_t));
      if (MOM_UNLIKELY (!newscalars))
	MOM_FATAL ("failed to grow scalars of task to %d", (int) newscalsize);
      memset (newscalars, 0, newscalsize * sizeof (intptr_t));
      memcpy (newscalars, tskitm->itk_scalars,
	      tskitm->itk_scaltop * sizeof (intptr_t));
      GC_FREE (tskitm->itk_scalars);
      tskitm->itk_scalars = newscalars;
      tskitm->itk_scalsize = newscalsize;
    }
  if (MOM_UNLIKELY (tskitm->itk_valtop + nbval >= tskitm->itk_valsize))
    {
      unsigned newvalsize =
	((5 * tskitm->itk_valtop / 4 + nbval + 6) | 7) + 1;
      momval_t *newvalues = GC_MALLOC (newvalsize * sizeof (momval_t));
      if (MOM_UNLIKELY (!newvalues))
	MOM_FATAL ("failed to grow values of task to %d", (int) newvalsize);
      memset (newvalues, 0, newvalsize * sizeof (momval_t));
      memcpy (newvalues, tskitm->itk_values,
	      tskitm->itk_valtop * sizeof (momval_t));
      GC_FREE (tskitm->itk_values);
      tskitm->itk_values = newvalues;
      tskitm->itk_valsize = newvalsize;
    }
  if (MOM_UNLIKELY (tskitm->itk_fratop + 1 >= tskitm->itk_frasize))
    {
      unsigned newfrasize = ((5 * tskitm->itk_frasize / 4 + 6) | 7) + 1;
      struct momframe_st *newframes =
	GC_MALLOC_ATOMIC (sizeof (struct momframe_st) * newfrasize);
      momclosure_t **newclosures =
	GC_MALLOC (sizeof (momclosure_t *) * newfrasize);
      if (MOM_UNLIKELY (!newframes || !newclosures))
	MOM_FATAL ("failed to grow frames of task to %d", (int) newfrasize);
      memset (newframes, 0, sizeof (struct momframe_st) * newfrasize);
      memset (newclosures, 0, sizeof (momclosure_t *) * newfrasize);
      memcpy (newframes, tskitm->itk_frames,
	      tskitm->itk_fratop * sizeof (struct momframe_st));
      memcpy (newclosures, tskitm->itk_closures,
	      tskitm->itk_fratop * sizeof (momclosure_t *));
      GC_FREE (tskitm->itk_frames);
      GC_FREE (tskitm->itk_closures);
      tskitm->itk_frames = newframes;
      tskitm->itk_closures = newclosures;
      tskitm->itk_frasize = newfrasize;
    }
  struct momframe_st *newframe = tskitm->itk_frames + tskitm->itk_fratop;
  newframe->fr_state = newstate;
  unsigned froint = newframe->fr_intoff = tskitm->itk_scaltop;
  unsigned frodbl = newframe->fr_dbloff = tskitm->itk_scaltop + nbnum;
  unsigned froval = newframe->fr_valoff = tskitm->itk_valtop;
  unsigned fratop = tskitm->itk_fratop;
  memset (tskitm->itk_scalars + tskitm->itk_scaltop, 0,
	  (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)));
  memset (tskitm->itk_values + tskitm->itk_valtop, 0,
	  nbval * sizeof (momval_t));
  tskitm->itk_scaltop +=
    (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)) / sizeof (intptr_t);
  tskitm->itk_valtop += nbval;
  tskitm->itk_closures[tskitm->itk_fratop] = (momclosure_t *) closure;
  tskitm->itk_fratop = fratop + 1;
  va_start (args, firstdir);
  fill_frame_data ((intptr_t *) (tskitm->itk_scalars + froint),
		   (double *) (tskitm->itk_scalars + frodbl),
		   (momval_t *) (tskitm->itk_values + froval), firstdir,
		   args);
  va_end (args);
  pthread_mutex_unlock (&((mom_anyitem_t *) tskitm)->i_mtx);
}




void
mom_tasklet_replace_top_frame (momval_t tsk, momval_t clo,
			       enum mom_pushframedirective_en firstdir, ...)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  if (!clo.ptr || *clo.ptype != momty_closure)
    return;
  unsigned nbval = 0;
  unsigned nbnum = 0;
  unsigned nbdbl = 0;
  int newstate = 0;
  const momclosure_t *closure = clo.pclosure;
  va_list args;
  // first, compute the data size
  va_start (args, firstdir);
  compute_pushed_data_size (closure, &nbval, &nbnum, &nbdbl, &newstate,
			    firstdir, args);
  va_end (args);
  momit_tasklet_t *tskitm = tsk.ptaskitem;
  pthread_mutex_lock (&((mom_anyitem_t *) tskitm)->i_mtx);
  if (tskitm->itk_fratop == 0)
    goto end;
  struct momframe_st *prevframe = tskitm->itk_frames + tskitm->itk_fratop - 1;
  unsigned ofpscal = prevframe->fr_intoff;
  unsigned ofpvalu = prevframe->fr_valoff;
  if (tskitm->itk_scaltop > ofpscal)
    memset (tskitm->itk_scalars + ofpscal, 0,
	    (tskitm->itk_scaltop - ofpscal) * sizeof (intptr_t));
  if (tskitm->itk_valtop > ofpvalu)
    memset (tskitm->itk_values + ofpvalu, 0,
	    (tskitm->itk_valtop - ofpvalu) * sizeof (momval_t));
  tskitm->itk_scaltop = ofpscal;
  tskitm->itk_valtop = ofpvalu;
  memset (prevframe, 0, sizeof (struct momframe_st));
  tskitm->itk_closures[tskitm->itk_fratop - 1] = NULL;
  if (MOM_UNLIKELY
      (tskitm->itk_scaltop +
       (sizeof (intptr_t) * nbnum +
	sizeof (double) * nbdbl) / sizeof (intptr_t) >= tskitm->itk_scalsize))
    {
      unsigned newscalsize =
	((5 * tskitm->itk_scaltop / 4 +
	  (sizeof (intptr_t *) * nbnum +
	   sizeof (double) * nbdbl) / sizeof (intptr_t) + 5) | 7) + 1;
      intptr_t *newscalars =
	GC_MALLOC_ATOMIC (newscalsize * sizeof (intptr_t));
      if (MOM_UNLIKELY (!newscalars))
	MOM_FATAL ("failed to grow scalars of task to %d", (int) newscalsize);
      memset (newscalars, 0, newscalsize * sizeof (intptr_t));
      memcpy (newscalars, tskitm->itk_scalars,
	      tskitm->itk_scaltop * sizeof (intptr_t));
      GC_FREE (tskitm->itk_scalars);
      tskitm->itk_scalars = newscalars;
      tskitm->itk_scalsize = newscalsize;
    }
  if (MOM_UNLIKELY (tskitm->itk_valtop + nbval >= tskitm->itk_valsize))
    {
      unsigned newvalsize =
	((5 * tskitm->itk_valtop / 4 + nbval + 6) | 7) + 1;
      momval_t *newvalues = GC_MALLOC (newvalsize * sizeof (momval_t));
      if (MOM_UNLIKELY (!newvalues))
	MOM_FATAL ("failed to grow values of task to %d", (int) newvalsize);
      memset (newvalues, 0, newvalsize * sizeof (momval_t));
      memcpy (newvalues, tskitm->itk_values,
	      tskitm->itk_valtop * sizeof (momval_t));
      GC_FREE (tskitm->itk_values);
      tskitm->itk_values = newvalues;
      tskitm->itk_valsize = newvalsize;
    }
  if (MOM_UNLIKELY (tskitm->itk_fratop + 1 >= tskitm->itk_frasize))
    {
      unsigned newfrasize = ((5 * tskitm->itk_frasize / 4 + 6) | 7) + 1;
      struct momframe_st *newframes =
	GC_MALLOC_ATOMIC (sizeof (struct momframe_st) * newfrasize);
      momclosure_t **newclosures =
	GC_MALLOC (sizeof (momclosure_t *) * newfrasize);
      if (MOM_UNLIKELY (!newframes || !newclosures))
	MOM_FATAL ("failed to grow frames of task to %d", (int) newfrasize);
      memset (newframes, 0, sizeof (struct momframe_st) * newfrasize);
      memset (newclosures, 0, sizeof (momclosure_t *) * newfrasize);
      memcpy (newframes, tskitm->itk_frames,
	      tskitm->itk_fratop * sizeof (struct momframe_st));
      memcpy (newclosures, tskitm->itk_closures,
	      tskitm->itk_fratop * sizeof (momclosure_t *));
      GC_FREE (tskitm->itk_frames);
      GC_FREE (tskitm->itk_closures);
      tskitm->itk_frames = newframes;
      tskitm->itk_closures = newclosures;
      tskitm->itk_frasize = newfrasize;
    }
  struct momframe_st *newframe = tskitm->itk_frames + tskitm->itk_fratop;
  newframe->fr_state = newstate;
  unsigned froint = newframe->fr_intoff = tskitm->itk_scaltop;
  unsigned frodbl = newframe->fr_dbloff = tskitm->itk_scaltop + nbnum;
  unsigned froval = newframe->fr_valoff = tskitm->itk_valtop;
  unsigned fratop = tskitm->itk_fratop;
  memset (tskitm->itk_scalars + tskitm->itk_scaltop, 0,
	  (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)));
  memset (tskitm->itk_values + tskitm->itk_valtop, 0,
	  nbval * sizeof (momval_t));
  tskitm->itk_scaltop +=
    (nbnum * sizeof (intptr_t) + nbdbl * sizeof (double)) / sizeof (intptr_t);
  tskitm->itk_valtop += nbval;
  tskitm->itk_closures[tskitm->itk_fratop] = (momclosure_t *) closure;
  tskitm->itk_fratop = fratop + 1;
  va_start (args, firstdir);
  fill_frame_data ((intptr_t *) (tskitm->itk_scalars + froint),
		   (double *) (tskitm->itk_scalars + frodbl),
		   (momval_t *) (tskitm->itk_values + froval), firstdir,
		   args);
  va_end (args);
end:
  pthread_mutex_unlock (&((mom_anyitem_t *) tskitm)->i_mtx);
}

void
mom_tasklet_reserve (momval_t tsk, unsigned nbint, unsigned nbdbl,
		     unsigned nbval, unsigned nbfram)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  momit_tasklet_t *tskitm = tsk.ptaskitem;
  pthread_mutex_lock (&((mom_anyitem_t *) tskitm)->i_mtx);
  unsigned scalwant =
    tskitm->itk_scaltop +
    ((nbint * sizeof (intptr_t) +
      nbdbl * sizeof (double)) / sizeof (intptr_t));
  unsigned valuwant = tskitm->itk_valtop + nbval;
  if (MOM_UNLIKELY
      (scalwant > tskitm->itk_scalsize
       || (tskitm->itk_scalsize > 64 && 2 * scalwant < tskitm->itk_scalsize)))
    {
      unsigned newscalsize =
	((5 * tskitm->itk_scaltop / 4 + 3 +
	  ((nbint * sizeof (intptr_t) +
	    nbdbl * sizeof (double)) / sizeof (intptr_t))) | 7) + 1;
      if (newscalsize != tskitm->itk_scalsize)
	{
	  intptr_t *newscalars =
	    GC_MALLOC_ATOMIC (newscalsize * sizeof (intptr_t));
	  if (MOM_UNLIKELY (!newscalars))
	    MOM_FATAL ("failed to resize scalars of task to %d",
		       (int) newscalsize);
	  memset (newscalars, 0, newscalsize * sizeof (intptr_t));
	  memcpy (newscalars, tskitm->itk_scalars,
		  tskitm->itk_scaltop * sizeof (intptr_t));
	  GC_FREE (tskitm->itk_scalars);
	  tskitm->itk_scalars = newscalars;
	  tskitm->itk_scalsize = newscalsize;
	}
    }
  if (MOM_UNLIKELY
      (valuwant > tskitm->itk_valsize
       || (tskitm->itk_valsize > 64 && 2 * valuwant < tskitm->itk_valsize)))
    {
      unsigned newvalsize =
	((5 * tskitm->itk_valtop / 4 + 3 + nbval) | 7) + 1;
      if (newvalsize != tskitm->itk_valsize)
	{
	  momval_t *newvalues = GC_MALLOC (newvalsize * sizeof (momval_t));
	  if (MOM_UNLIKELY (!newvalues))
	    MOM_FATAL ("failed to resize values of task to %d",
		       (int) newvalsize);
	  memset (newvalues, 0, newvalsize * sizeof (momval_t));
	  memcpy (newvalues, tskitm->itk_values,
		  tskitm->itk_valtop * sizeof (momval_t));
	  GC_FREE (tskitm->itk_values);
	  tskitm->itk_values = newvalues;
	  tskitm->itk_valsize = newvalsize;
	}
    }
  unsigned framwant = tskitm->itk_fratop + nbfram;
  if (MOM_UNLIKELY
      (framwant > tskitm->itk_frasize
       || (tskitm->itk_frasize > 64 && 2 * framwant < tskitm->itk_frasize)))
    {
      unsigned newfrasize =
	((5 * tskitm->itk_fratop / 4 + 3 + nbfram) | 7) + 1;
      if (newfrasize != tskitm->itk_frasize)
	{
	  struct momframe_st *newframes =
	    GC_MALLOC_ATOMIC (sizeof (struct momframe_st) * newfrasize);
	  momclosure_t **newclosures =
	    GC_MALLOC (sizeof (momclosure_t *) * newfrasize);
	  if (MOM_UNLIKELY (!newframes || !newclosures))
	    MOM_FATAL ("failed to resize frames of task to %d",
		       (int) newfrasize);
	  memset (newframes, 0, sizeof (struct momframe_st) * newfrasize);
	  memset (newclosures, 0, sizeof (momclosure_t *) * newfrasize);
	  memcpy (newframes, tskitm->itk_frames,
		  tskitm->itk_fratop * sizeof (struct momframe_st));
	  memcpy (newclosures, tskitm->itk_closures,
		  tskitm->itk_fratop * sizeof (momclosure_t *));
	  GC_FREE (tskitm->itk_frames);
	  GC_FREE (tskitm->itk_closures);
	  tskitm->itk_frames = newframes;
	  tskitm->itk_closures = newclosures;
	  tskitm->itk_frasize = newfrasize;
	}
    }
  pthread_mutex_unlock (&((mom_anyitem_t *) tskitm)->i_mtx);
}



void
mom_tasklet_pop_frame (momval_t tsk)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  momit_tasklet_t *tskitm = tsk.ptaskitem;
  pthread_mutex_lock (&((mom_anyitem_t *) tskitm)->i_mtx);
  if (tskitm->itk_fratop == 0)
    goto end;
  struct momframe_st *prevframe = tskitm->itk_frames + tskitm->itk_fratop - 1;
  unsigned ofpscal = prevframe->fr_intoff;
  unsigned ofpvalu = prevframe->fr_valoff;
  if (tskitm->itk_scaltop > ofpscal)
    memset (tskitm->itk_scalars + ofpscal, 0,
	    (tskitm->itk_scaltop - ofpscal) * sizeof (intptr_t));
  if (tskitm->itk_valtop > ofpvalu)
    memset (tskitm->itk_values + ofpvalu, 0,
	    (tskitm->itk_valtop - ofpvalu) * sizeof (momval_t));
  tskitm->itk_scaltop = ofpscal;
  tskitm->itk_valtop = ofpvalu;
  memset (prevframe, 0, sizeof (struct momframe_st));
  tskitm->itk_closures[tskitm->itk_fratop - 1] = NULL;
  tskitm->itk_fratop--;
  if (MOM_UNLIKELY
      (tskitm->itk_scalsize > 64
       && 2 * tskitm->itk_scaltop < tskitm->itk_scalsize))
    {
      unsigned newscalsize = ((5 * tskitm->itk_scaltop / 4 + 3) | 7) + 1;
      if (newscalsize != tskitm->itk_scalsize)
	{
	  intptr_t *newscalars =
	    GC_MALLOC_ATOMIC (newscalsize * sizeof (intptr_t));
	  if (MOM_UNLIKELY (!newscalars))
	    MOM_FATAL ("failed to shrink scalars of task to %d",
		       (int) newscalsize);
	  memset (newscalars, 0, newscalsize * sizeof (intptr_t));
	  memcpy (newscalars, tskitm->itk_scalars,
		  tskitm->itk_scaltop * sizeof (intptr_t));
	  GC_FREE (tskitm->itk_scalars);
	  tskitm->itk_scalars = newscalars;
	  tskitm->itk_scalsize = newscalsize;
	}
    }
  if (MOM_UNLIKELY
      (tskitm->itk_valsize > 64
       && 2 * tskitm->itk_valtop < tskitm->itk_valsize))
    {
      unsigned newvalsize = ((5 * tskitm->itk_valtop / 4 + 3) | 7) + 1;
      if (newvalsize != tskitm->itk_valsize)
	{
	  momval_t *newvalues = GC_MALLOC (newvalsize * sizeof (momval_t));
	  if (MOM_UNLIKELY (!newvalues))
	    MOM_FATAL ("failed to shrink values of task to %d",
		       (int) newvalsize);
	  memset (newvalues, 0, newvalsize * sizeof (momval_t));
	  memcpy (newvalues, tskitm->itk_values,
		  tskitm->itk_valtop * sizeof (momval_t));
	  GC_FREE (tskitm->itk_values);
	  tskitm->itk_values = newvalues;
	  tskitm->itk_valsize = newvalsize;
	}
    }
  if (MOM_UNLIKELY
      (tskitm->itk_frasize > 64
       && 2 * tskitm->itk_fratop < tskitm->itk_frasize))
    {
      unsigned newfrasize = ((5 * tskitm->itk_fratop / 4 + 3) | 7) + 1;
      if (newfrasize != tskitm->itk_frasize)
	{
	  struct momframe_st *newframes =
	    GC_MALLOC_ATOMIC (sizeof (struct momframe_st) * newfrasize);
	  momclosure_t **newclosures =
	    GC_MALLOC (sizeof (momclosure_t *) * newfrasize);
	  if (MOM_UNLIKELY (!newframes || !newclosures))
	    MOM_FATAL ("failed to shrink frames of task to %d",
		       (int) newfrasize);
	  memset (newframes, 0, sizeof (struct momframe_st) * newfrasize);
	  memset (newclosures, 0, sizeof (momclosure_t *) * newfrasize);
	  memcpy (newframes, tskitm->itk_frames,
		  tskitm->itk_fratop * sizeof (struct momframe_st));
	  memcpy (newclosures, tskitm->itk_closures,
		  tskitm->itk_fratop * sizeof (momclosure_t *));
	  GC_FREE (tskitm->itk_frames);
	  GC_FREE (tskitm->itk_closures);
	  tskitm->itk_frames = newframes;
	  tskitm->itk_closures = newclosures;
	  tskitm->itk_frasize = newfrasize;
	}
    }
end:
  pthread_mutex_unlock (&((mom_anyitem_t *) tskitm)->i_mtx);
}

momval_t
mom_run_closure (momval_t clo, enum mom_pushframedirective_en firstdir, ...)
{
  momval_t res = MOM_NULLV;
  if (!clo.ptr || *clo.ptype != momty_closure)
    return MOM_NULLV;
  unsigned nbval = 0;
  unsigned nbnum = 0;
  unsigned nbdbl = 0;
  int newstate = 0;
  const momclosure_t *closure = clo.pclosure;
  const momit_routine_t *routitm = (momit_routine_t *) closure->connitm;
  if (!routitm || ((mom_anyitem_t *) routitm)->typnum != momty_routineitem)
    return MOM_NULLV;
  va_list args;
  const struct momroutinedescr_st *rdescr = routitm->irt_descr;
  // an incomplete routine would not have made a closure
  if (MOM_UNLIKELY (!rdescr || rdescr->rout_magic != ROUTINE_MAGIC
		    || !rdescr->rout_code))
    MOM_FATAL ("corrupted routine descriptor in closure's routine");
  // first, compute the data size
  va_start (args, firstdir);
  compute_pushed_data_size (closure, &nbval, &nbnum, &nbdbl, &newstate,
			    firstdir, args);
  va_end (args);
  momval_t valtiny[TINY_MAX] = { MOM_NULLV };
  intptr_t numtiny[TINY_MAX] = { 0 };
  double dbltiny[TINY_MAX] = { 0.0 };
  momval_t *valarr = NULL;
  intptr_t *numarr = NULL;
  double *dblarr = NULL;
  if (closure->slen < rdescr->rout_minclosize)
    return MOM_NULLV;
  if (nbval < rdescr->rout_frame_nbval)
    nbval = rdescr->rout_frame_nbval;
  if (nbnum < rdescr->rout_frame_nbnum)
    nbnum = rdescr->rout_frame_nbnum;
  if (nbdbl < rdescr->rout_frame_nbdbl)
    nbdbl = rdescr->rout_frame_nbdbl;
  if (nbval == 0)
    nbval = 1;
  if (nbval < TINY_MAX)
    valarr = valtiny;
  else
    {
      valarr = GC_MALLOC (nbval * sizeof (momval_t));
      if (MOM_UNLIKELY (!valarr))
	MOM_FATAL ("failed to allocate %d values", (int) nbval);
      memset (valarr, 0, nbval * sizeof (momval_t));
    };
  if (nbnum < TINY_MAX)
    numarr = numtiny;
  else
    {
      numarr = GC_MALLOC_ATOMIC (sizeof (intptr_t) * nbnum);
      if (MOM_UNLIKELY (!numarr))
	MOM_FATAL ("failed to allocate %d numbers", (int) nbnum);
      memset (numarr, 0, nbnum * sizeof (intptr_t));
    };
  if (nbdbl < TINY_MAX)
    dblarr = dbltiny;
  else
    {
      dblarr = GC_MALLOC_ATOMIC (sizeof (double) * nbdbl);
      if (MOM_UNLIKELY (!dblarr))
	MOM_FATAL ("failed to allocate %d doubles", (int) nbdbl);
      memset (dblarr, 0, nbdbl * sizeof (double));
    };
  // fill the pseudo frame data
  va_start (args, firstdir);
  fill_frame_data (numarr, dblarr, valarr, firstdir, args);
  va_end (args);
  // invoke the routine
  if (rdescr->rout_code (0, NULL, (momclosure_t *) closure, valarr, numarr,
			 dblarr) < 0)
    return MOM_NULLV;
  res = valarr[0];
  return res;
}
