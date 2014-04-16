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

static pthread_mutex_t glob_mtx = PTHREAD_MUTEX_INITIALIZER;

struct name_entry_st
{
  const momstring_t *nme_str;
  const mom_anyitem_t *nme_itm;
};

static struct glob_dict_st
{
  unsigned name_count;		// number of named entries
  unsigned name_size;		// size of hash tables
  struct name_entry_st *name_hashitem;	// hash table on items
  struct name_entry_st *name_hashstr;	// hash table on strings
} glob_dict;


void
mom_initialize_globals (void)
{
  const unsigned dictsiz = 1024;
  pthread_mutex_lock (&glob_mtx);
  glob_dict.name_hashitem =
    GC_MALLOC (sizeof (struct name_entry_st) * dictsiz);
  memset (glob_dict.name_hashitem, 0,
	  sizeof (struct name_entry_st) * dictsiz);
  glob_dict.name_hashstr =
    GC_MALLOC (sizeof (struct name_entry_st) * dictsiz);
  memset (glob_dict.name_hashstr, 0, sizeof (struct name_entry_st) * dictsiz);
  glob_dict.name_size = dictsiz;
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
  struct name_entry_st *arrname = glob_dict.name_hashstr;
  for (unsigned i = istart; i < size; i++)
    {
      if (!arrname[i].nme_str)
	return -1;
      if (arrname[i].nme_str == MONIMELT_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == h
	  && !strcmp (arrname[i].nme_str->cstr, str))
	return (int) i;
    }
  for (unsigned i = 0; i < istart; i++)
    {
      if (!arrname[i].nme_str)
	return -1;
      if (arrname[i].nme_str == MONIMELT_EMPTY)
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
  struct name_entry_st *arritem = glob_dict.name_hashitem;
  for (unsigned i = istart; i < size; i++)
    {
      if (arritem[i].nme_itm == itm)
	return (int) i;
      if (!arritem[i].nme_itm)
	return -1;
      if (arritem[i].nme_itm == MONIMELT_EMPTY)
	continue;
    }
  for (unsigned i = 0; i < istart; i++)
    {
      if (arritem[i].nme_itm == itm)
	return (int) i;
      if (!arritem[i].nme_itm)
	return -1;
      if (arritem[i].nme_itm == MONIMELT_EMPTY)
	continue;
    }
  return -1;
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


// internal routine to add a name entry which is known to be new
static inline void
add_new_name_entry (const momstring_t * name, const mom_anyitem_t * item)
{
  unsigned size = glob_dict.name_size;
  momhash_t hashname = name->hash;
  momhash_t hashitem = item->i_hash;
  unsigned istartname = hashname % size;
  unsigned istartitem = hashitem % size;
  struct name_entry_st *arrname = glob_dict.name_hashstr;
  struct name_entry_st *arritem = glob_dict.name_hashitem;
  for (unsigned i = istartname; i < size; i++)
    {
      if (!arrname[i].nme_str || arrname[i].nme_str == MONIMELT_EMPTY)
	{
	  arrname[i].nme_str = name;
	  arrname[i].nme_itm = item;
	  goto additem;
	}
    }
  for (unsigned i = 0; i < istartname; i++)
    {
      if (!arrname[i].nme_str || arrname[i].nme_str == MONIMELT_EMPTY)
	{
	  arrname[i].nme_str = name;
	  arrname[i].nme_itm = item;
	  goto additem;
	}
    }
  // this should never happen
  MONIMELT_FATAL ("corrupted dictionnary for names of size %d", (int) size);
additem:
  for (unsigned i = istartitem; i < size; i++)
    {
      if (!arritem[i].nme_str || arritem[i].nme_str == MONIMELT_EMPTY)
	{
	  arritem[i].nme_str = name;
	  arritem[i].nme_itm = item;
	  goto end;
	}
    }
  for (unsigned i = 0; i < istartitem; i++)
    {
      if (!arritem[i].nme_str || arritem[i].nme_str == MONIMELT_EMPTY)
	{
	  arritem[i].nme_str = name;
	  arritem[i].nme_itm = item;
	  goto end;
	}
    }
  // this should never happen
  MONIMELT_FATAL ("corrupted dictionnary for items of size %d", (int) size);
end:
  glob_dict.name_count++;
}

static void
resize_dict (unsigned newsize)
{
  unsigned oldcount = glob_dict.name_count;
  unsigned oldsize = glob_dict.name_size;
  if (newsize + 5 < 9 * oldcount / 8)
    MONIMELT_FATAL ("invalid newsize %u for dictonnary count %u",
		    newsize, oldcount);
  if (newsize == oldsize)
    return;
  struct name_entry_st *oldarrname = glob_dict.name_hashstr;
  struct name_entry_st *oldarritem = glob_dict.name_hashitem;
  glob_dict.name_hashstr =
    GC_MALLOC (sizeof (struct name_entry_st) * newsize);
  if (!glob_dict.name_hashstr)
    MONIMELT_FATAL ("failed to grow dictionnary string hash to %u", newsize);
  memset (glob_dict.name_hashstr, 0, sizeof (struct name_entry_st) * newsize);
  glob_dict.name_hashitem =
    GC_MALLOC (sizeof (struct name_entry_st) * newsize);
  if (!glob_dict.name_hashitem)
    MONIMELT_FATAL ("failed to grow dictionnary item hash to %u", newsize);
  memset (glob_dict.name_hashitem, 0,
	  sizeof (struct name_entry_st) * newsize);
  glob_dict.name_count = 0;
  glob_dict.name_size = newsize;
  for (unsigned i = 0; i < oldsize; i++)
    {
      struct name_entry_st *curent = oldarrname + i;
      if (!curent->nme_str || curent->nme_str == MONIMELT_EMPTY
	  || !curent->nme_itm || curent->nme_itm == MONIMELT_EMPTY)
	continue;
      add_new_name_entry (curent->nme_str, curent->nme_itm);
    }
  if (glob_dict.name_count != oldcount)
    MONIMELT_FATAL ("corrupted resized dictionnary old count=%u new count=%u",
		    oldcount, (unsigned) glob_dict.name_count);
  GC_FREE (oldarrname);
  GC_FREE (oldarritem);
}


void
mom_register_new_name_string (momstring_t * namestr, mom_anyitem_t * item)
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
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 400) | 0xff) + 1;
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
      unsigned newsize = ((13 * glob_dict.name_count / 8 + 400) | 0xff) + 1;
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
  struct name_entry_st *arrname = glob_dict.name_hashstr;
  struct name_entry_st *arritem = glob_dict.name_hashitem;
  for (unsigned i = istartname; i < size; i++)
    {
      if (!arrname[i].nme_str)
	return;
      if (arrname[i].nme_str == MONIMELT_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == hashname
	  && (arrname[i].nme_str == name
	      || !strcmp (arrname[i].nme_str->cstr, name->cstr)))
	{
	  arrname[i].nme_str = MONIMELT_EMPTY;
	  arrname[i].nme_itm = MONIMELT_EMPTY;
	  goto remove_item;
	}
    }
  for (unsigned i = 0; i < istartname; i++)
    {
      if (!arrname[i].nme_str)
	return;
      if (arrname[i].nme_str == MONIMELT_EMPTY)
	continue;
      if (arrname[i].nme_str->hash == hashname
	  && (arrname[i].nme_str == name
	      || !strcmp (arrname[i].nme_str->cstr, name->cstr)))
	{
	  arrname[i].nme_str = MONIMELT_EMPTY;
	  arrname[i].nme_itm = MONIMELT_EMPTY;
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
	  arritem[i].nme_str = MONIMELT_EMPTY;
	  arritem[i].nme_itm = MONIMELT_EMPTY;
	  goto end;
	}
      else if (!arritem[i].nme_itm)	// should not happen
	MONIMELT_FATAL ("corrupted item dict of size %u", size);
      else if (arritem[i].nme_itm == MONIMELT_EMPTY)
	continue;
    }
  for (unsigned i = 0; i < istartitem; i++)
    {
      if (arritem[i].nme_itm == itm)
	{
	  arritem[i].nme_str = MONIMELT_EMPTY;
	  arritem[i].nme_itm = MONIMELT_EMPTY;
	  goto end;
	}
      else if (!arritem[i].nme_itm)	// should not happen
	MONIMELT_FATAL ("corrupted item dict of size %u", size);
      else if (arritem[i].nme_itm == MONIMELT_EMPTY)
	continue;
    }
  // should never happen
  MONIMELT_FATAL ("corrupted dict of size %u", size);
end:
  glob_dict.name_count--;
}

void
mom_replace_name_string (momstring_t * namestr, mom_anyitem_t * item)
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
  pthread_mutex_lock (&glob_mtx);
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
