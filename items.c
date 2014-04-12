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



momhash_t
mom_hash_uuid (uuid_t uid)
{
  const unsigned char *u = (const unsigned char *) uid;
  momhash_t h1 =
    (13 * u[0]) + ((70201 * u[1]) ^ (311 * u[2] + 1071703)) - (171641 * u[3]);
  momhash_t h2 =
    (31 * u[4]) + (u[4] << (8 + (u[5] & 0xf))) - (521 * u[5]) +
    ((541 * u[6]) ^ (2071591 * u[7] + 156912));
  momhash_t h = (h1 * 3071633 - (h1 >> 10)) ^ (769 * h2 + 17);
  if (!h)
    {
      h = h1;
      if (!h)
	{
	  h = h2;
	  if (!h)
	    {
	      h = 3071741 * u[5] + u[7];
	      if (!h)
		{
		  h = 1071407 * u[6] + 31 * u[1];
		  if (!h)
		    {
		      h = u[1] + 2 * u[2] + 17 * u[3];
		      if (!h)
			h = 156;
		    }
		}
	    }
	}
    }
  return h;
}

static pthread_mutex_t mtx_global_items = PTHREAD_MUTEX_INITIALIZER;
#define EMPTY_ITEM_SLOT ((void*)(-1L))
static struct items_data_st
{
  uint32_t items_nb;
  uint32_t items_size;
  // should have one extra slot to ease loop unrolling
  mom_anyitem_t *items_arr[];
} *items_data;


static inline void
add_new_item (mom_anyitem_t * newitm)
{
  uint32_t imax = items_data->items_size;
  assert (5 * items_data->items_nb / 4 + 2 < imax);
  momhash_t newhash = newitm->i_hash;
  assert (newhash != 0);
  uint32_t istart = newhash % imax;
  for (uint32_t i = istart; i < imax; i += 2)
    {
      if (!items_data->items_arr[i]
	  || items_data->items_arr[i] == EMPTY_ITEM_SLOT)
	{
	  items_data->items_arr[i] = newitm;
	  items_data->items_nb++;
	  return;
	}
      if (!items_data->items_arr[i + 1]
	  || items_data->items_arr[i + 1] == EMPTY_ITEM_SLOT)
	{
	  if (i + 1 >= imax)
	    continue;
	  items_data->items_arr[i + 1] = newitm;
	  items_data->items_nb++;
	  return;
	}
    }
  for (uint32_t i = 0; i < istart; i += 2)
    {
      if (!items_data->items_arr[i]
	  || items_data->items_arr[i] == EMPTY_ITEM_SLOT)
	{
	  items_data->items_arr[i] = newitm;
	  items_data->items_nb++;
	  return;
	}
      if (!items_data->items_arr[i + 1]
	  || items_data->items_arr[i + 1] == EMPTY_ITEM_SLOT)
	{
	  items_data->items_arr[i + 1] = newitm;
	  items_data->items_nb++;
	  return;
	}
    }
}

static inline void
remove_old_item (mom_anyitem_t * olditm)
{
  uint32_t imax = items_data->items_size;
  assert (5 * items_data->items_nb / 4 + 2 < imax);
  momhash_t oldhash = olditm->i_hash;
  assert (oldhash != 0);
  uint32_t istart = oldhash % imax;
  for (uint32_t i = istart; i < imax; i += 2)
    {
      if (items_data->items_arr[i] == olditm)
	{
	  items_data->items_arr[i] = EMPTY_ITEM_SLOT;
	  items_data->items_nb--;
	  return;
	}
      if (items_data->items_arr[i + 1] == olditm && i + 1 < imax)
	{
	  items_data->items_arr[i + 1] = EMPTY_ITEM_SLOT;
	  items_data->items_nb--;
	  return;
	}
    }
  for (uint32_t i = 0; i < istart; i += 2)
    {
      if (items_data->items_arr[i] == olditm)
	{
	  items_data->items_arr[i] = EMPTY_ITEM_SLOT;
	  items_data->items_nb--;
	  return;
	}
      if (items_data->items_arr[i + 1] == olditm)
	{
	  items_data->items_arr[i + 1] = EMPTY_ITEM_SLOT;
	  items_data->items_nb--;
	  return;
	}
    }
}

static void
resize_items_data (uint32_t newsiz)
{
  assert (newsiz > items_data->items_nb);
  assert (5 * items_data->items_nb / 4 + 2 < newsiz);
  if (newsiz == items_data->items_size)
    return;
  struct items_data_st *newdata =
    GC_MALLOC_ATOMIC (sizeof (struct items_data_st) +
		      (newsiz + 1) * sizeof (mom_anyitem_t *));
  struct items_data_st *olddata = items_data;
  memset (newdata, 0,
	  sizeof (struct items_data_st) + (newsiz +
					   1) * sizeof (mom_anyitem_t *));
  newdata->items_nb = 0;
  newdata->items_size = newsiz;
  items_data = newdata;
  unsigned sizold = olddata->items_size;
  for (unsigned ix = 0; ix < sizold; ix++)
    {
      mom_anyitem_t *olditm = olddata->items_arr[ix];
      olddata->items_arr[ix] = NULL;
      if (!olditm || (void *) olddata == EMPTY_ITEM_SLOT)
	continue;
      add_new_item (olditm);
    }
  GC_FREE (olddata);
}

void
mom_initialize_items (void)
{
  const unsigned nb_items = 4090;
  pthread_mutex_lock (&mtx_global_items);
  items_data =
    GC_MALLOC_ATOMIC (sizeof (struct items_data_st) +
		      nb_items * sizeof (mom_anyitem_t *));
  memset (items_data, 0,
	  sizeof (struct items_data_st) +
	  nb_items * sizeof (mom_anyitem_t *));
  items_data->items_nb = 0;
  items_data->items_size = nb_items;
  pthread_mutex_unlock (&mtx_global_items);
}

static void
mom_finalize_item (void *itmad, void *data)
{
  mom_anyitem_t *itm = itmad;
  pthread_mutex_lock (&mtx_global_items);
  momhash_t itmhash = itm->i_hash;
  assert (itmhash != 0);
  pthread_mutex_destroy (&itm->i_mtx);
  if (items_data->items_nb < items_data->items_size / 4
      && items_data->items_size > 2000)
    {
      uint32_t newsiz = ((4 * items_data->items_nb / 3 + 100) | 0x3ff) - 2;
      if (newsiz < items_data->items_size)
	resize_items_data (newsiz);
    }
  remove_old_item (itm);
  itm->i_hash = 0;
  struct mom_itemattributes_st *iat = itm->i_attrs;
  if (iat)
    {
      itm->i_attrs = NULL;
      momusize_t nba = iat->nbattr;
      memset (iat, 0,
	      sizeof (struct mom_itemattributes_st) +
	      nba * sizeof (struct mom_attrentry_st));
      GC_FREE (iat);
    }
  pthread_mutex_unlock (&mtx_global_items);
}


void *
mom_allocate_item_with_uuid (unsigned type, size_t itemsize, uuid_t uid)
{
  struct momanyitem_st *p = NULL;
  assert (itemsize > sizeof (struct momanyitem_st));
  p = GC_MALLOC (itemsize);
  if (!p)
    MONIMELT_FATAL ("out of memory for item type %d size %ld",
		    type, (long) itemsize);
  memset (p, 0, itemsize);
  pthread_mutex_lock (&mtx_global_items);
  p->typnum = type;
  p->i_space = 0;
  p->i_attrs = NULL;
  momhash_t itmhash = mom_hash_uuid (uid);
  p->i_hash = itmhash;
  memcpy (p->i_uuid, uid, sizeof (uuid_t));
  if (items_data->items_nb >= 3 * items_data->items_size / 4)
    {
      uint32_t newsiz = ((3 * items_data->items_nb / 2 + 500) | 0x1ff) - 2;
      if (newsiz > items_data->items_size)
	resize_items_data (newsiz);
    }
  add_new_item (p);
  pthread_mutex_init (&p->i_mtx, NULL);
  GC_REGISTER_FINALIZER (p, mom_finalize_item, NULL, NULL, NULL);
  p->i_content.ptr = NULL;
  pthread_mutex_unlock (&mtx_global_items);
  return p;
}

void *
mom_allocate_item (unsigned type, size_t itemsize)
{
  assert (itemsize > sizeof (struct momanyitem_st));
  uuid_t uid;
  memset (uid, 0, sizeof (uid));
  uuid_generate (uid);
  return mom_allocate_item_with_uuid (type, itemsize, uid);
}


mom_anyitem_t *
mom_item_of_uuid (uuid_t uid)
{
  mom_anyitem_t *res = NULL;
  momhash_t h = mom_hash_uuid (uid);
  pthread_mutex_lock (&mtx_global_items);
  uint32_t imax = items_data->items_size;
  assert (5 * items_data->items_nb / 4 + 2 < imax);
  assert (h != 0);
  uint32_t istart = h % imax;
  for (uint32_t i = istart; i < imax; i++)
    {
      mom_anyitem_t *curitm = items_data->items_arr[i];
      if ((void *) curitm == EMPTY_ITEM_SLOT)
	continue;
      if (!curitm)
	goto end;
      if (!memcmp (curitm->i_uuid, uid, sizeof (uuid_t)))
	{
	  res = curitm;
	  goto end;
	}
    }
  for (uint32_t i = 0; i < istart; i++)
    {
      mom_anyitem_t *curitm = items_data->items_arr[i];
      if ((void *) curitm == EMPTY_ITEM_SLOT)
	continue;
      if (!curitm)
	goto end;
      if (!memcmp (curitm->i_uuid, uid, sizeof (uuid_t)))
	{
	  res = curitm;
	  goto end;
	}
    }
end:
  pthread_mutex_unlock (&mtx_global_items);
  return res;
}


momit_json_name_t *
mom_make_item_json_name_of_uuid (uuid_t uid, const char *name)
{
  momit_json_name_t *itm
    = mom_allocate_item_with_uuid (momty_jsonitem, sizeof (momit_json_name_t),
				   uid);
  if (name)
    itm->ij_namejson = mom_make_string (name);
  return itm;
}

momit_json_name_t *
mom_make_item_json_name (const char *name)
{
  momit_json_name_t *itm
    = mom_allocate_item (momty_jsonitem, sizeof (momit_json_name_t));
  if (name)
    itm->ij_namejson = mom_make_string (name);
  return itm;
}

momit_bool_t *
mom_create_named_bool (uuid_t uid, const char *name)
{
  if (!strcmp (name, "true"))
    {
      momit_bool_t *itrue
	= mom_allocate_item_with_uuid (momty_boolitem, sizeof (momit_bool_t),
				       uid);
      itrue->ib_bool = true;
      return itrue;
    }
  else if (!strcmp (name, "false"))
    {
      momit_bool_t *ifalse
	= mom_allocate_item_with_uuid (momty_boolitem, sizeof (momit_bool_t),
				       uid);
      ifalse->ib_bool = false;
      return ifalse;
    }
  else
    MONIMELT_FATAL ("invalid boolean item name=%s", name);
}
