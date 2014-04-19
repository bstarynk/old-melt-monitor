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


/// declare the named items
#define MONIMELT_NAMED(Name,Type,Uid) \
  extern momit_##Type##_t* mom_item__##Name;
#include "monimelt-names.h"

/// for boolean item type descriptor
static mom_anyitem_t *bool_itemloader (struct mom_loader_st *ld,
				       momval_t json, uuid_t uid);
static void bool_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
			     momval_t json);
static void bool_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm);
static momval_t bool_itemgetbuild (struct mom_dumper_st *dmp,
				   mom_anyitem_t * itm);
static momval_t bool_itemgetfill (struct mom_dumper_st *dmp,
				  mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_bool = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "bool",
  .ityp_loader = bool_itemloader,
  .ityp_filler = bool_itemfiller,
  .ityp_scan = bool_itemscan,
  .ityp_getbuild = bool_itemgetbuild,
  .ityp_getfill = bool_itemgetfill,
};

/// for json name item type descriptor
static mom_anyitem_t *json_name_itemloader (struct mom_loader_st *ld,
					    momval_t json, uuid_t uid);
static void json_name_itemfiller (struct mom_loader_st *ld,
				  mom_anyitem_t * itm, momval_t json);
static void json_name_itemscan (struct mom_dumper_st *dmp,
				mom_anyitem_t * itm);
static momval_t json_name_itemgetbuild (struct mom_dumper_st *dmp,
					mom_anyitem_t * itm);
static momval_t json_name_itemgetfill (struct mom_dumper_st *dmp,
				       mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_json_name = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "json_name",
  .ityp_loader = json_name_itemloader,
  .ityp_filler = json_name_itemfiller,
  .ityp_scan = json_name_itemscan,
  .ityp_getbuild = json_name_itemgetbuild,
  .ityp_getfill = json_name_itemgetfill,
};

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
	      h = 3071741 * u[5] + 7 * u[7];
	      if (!h)
		{
		  h = 1071407 * u[6] + 31 * u[1];
		  if (!h)
		    {
		      h = u[1] + 29 * u[2] + 17 * u[3];
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
	  || items_data->items_arr[i] == MONIMELT_EMPTY)
	{
	  items_data->items_arr[i] = newitm;
	  items_data->items_nb++;
	  return;
	}
      if (!items_data->items_arr[i + 1]
	  || items_data->items_arr[i + 1] == MONIMELT_EMPTY)
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
	  || items_data->items_arr[i] == MONIMELT_EMPTY)
	{
	  items_data->items_arr[i] = newitm;
	  items_data->items_nb++;
	  return;
	}
      if (!items_data->items_arr[i + 1]
	  || items_data->items_arr[i + 1] == MONIMELT_EMPTY)
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
	  items_data->items_arr[i] = MONIMELT_EMPTY;
	  items_data->items_nb--;
	  return;
	}
      if (items_data->items_arr[i + 1] == olditm && i + 1 < imax)
	{
	  items_data->items_arr[i + 1] = MONIMELT_EMPTY;
	  items_data->items_nb--;
	  return;
	}
    }
  for (uint32_t i = 0; i < istart; i += 2)
    {
      if (items_data->items_arr[i] == olditm)
	{
	  items_data->items_arr[i] = MONIMELT_EMPTY;
	  items_data->items_nb--;
	  return;
	}
      if (items_data->items_arr[i + 1] == olditm)
	{
	  items_data->items_arr[i + 1] = MONIMELT_EMPTY;
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
      if (!olditm || (void *) olddata == MONIMELT_EMPTY)
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
  struct momitemtypedescr_st *idescr =
    (itm->typnum > momty__itemlowtype && itm->typnum < momty__last) ?
    mom_typedescr_array[itm->typnum] : NULL;
  pthread_mutex_lock (&mtx_global_items);
  momhash_t itmhash = itm->i_hash;
  assert (itmhash != 0);
  if (idescr && idescr->ityp_destroy)
    {
      if (MONIMELT_UNLIKELY (idescr->ityp_magic != ITEMTYPE_MAGIC))
	MONIMELT_FATAL
	  ("corrupted item type descriptor #%d for finalized item@%p",
	   (int) (itm->typnum), (void *) itm);
      pthread_mutex_lock (&itm->i_mtx);
      idescr->ityp_destroy (itm);
      pthread_mutex_unlock (&itm->i_mtx);
    }
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
  pthread_mutex_init (&p->i_mtx, &mom_recursive_mutex_attr);
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

static inline momval_t
get_attribute (struct mom_itemattributes_st *attrs, mom_anyitem_t * itat)
{
  if (!attrs)
    return MONIMELT_NULLV;
  unsigned nbattr = attrs->nbattr;
  unsigned lo = 0, hi = nbattr, md;
  while (lo + 3 < hi)
    {
      md = (lo + hi) / 2;
      mom_anyitem_t *curatitm = attrs->itattrtab[md].aten_itm;
      if (curatitm == itat)
	return attrs->itattrtab[md].aten_val;
      else if (mom_item_cmp (itat, curatitm) < 0)
	hi = md;
      else
	lo = md;
    }
  for (md = lo; md < hi; md++)
    {
      mom_anyitem_t *curatitm = attrs->itattrtab[md].aten_itm;
      if (curatitm == itat)
	return attrs->itattrtab[md].aten_val;
    }
  return MONIMELT_NULLV;
}

static struct mom_itemattributes_st *
put_attribute (struct mom_itemattributes_st *attrs, mom_anyitem_t * itat,
	       momval_t val)
{
  if (MONIMELT_UNLIKELY (!attrs))
    {
      if (!itat || !val.ptr || itat->typnum <= momty__itemlowtype)
	return NULL;
      unsigned newsize = 3;
      struct mom_itemattributes_st *newattrs
	=
	GC_MALLOC (sizeof (struct mom_itemattributes_st) +
		   newsize * sizeof (struct mom_attrentry_st));
      if (MONIMELT_UNLIKELY (!newattrs))
	MONIMELT_FATAL ("failed to allocate initial attributes of %d",
			(int) newsize);
      memset (newattrs, 0,
	      sizeof (struct mom_itemattributes_st) +
	      newsize * sizeof (struct mom_attrentry_st));
      newattrs->nbattr = 1;
      newattrs->size = newsize;
      newattrs->itattrtab[0].aten_itm = itat;
      newattrs->itattrtab[0].aten_val = val;
      return newattrs;
    }
  unsigned nbattr = attrs->nbattr;
  unsigned size = attrs->size;
  unsigned lo = 0, hi = nbattr, md;
  assert (nbattr <= size);
  while (lo + 3 < hi)
    {
      md = (lo + hi) / 2;
      mom_anyitem_t *curatitm = attrs->itattrtab[md].aten_itm;
      if (curatitm == itat)
	{
	  if (val.ptr)
	    {
	      attrs->itattrtab[md].aten_val = val;
	      return attrs;
	    }
	  else
	    goto remove_at_md;
	}
      else if (mom_item_cmp (itat, curatitm) < 0)
	hi = md;
      else
	lo = md;
    }
  for (md = lo; md < hi; md++)
    {
      mom_anyitem_t *curatitm = attrs->itattrtab[md].aten_itm;
      if (curatitm == itat)
	{
	  if (val.ptr)
	    {
	      attrs->itattrtab[md].aten_val = val;
	      return attrs;
	    }
	  else
	    goto remove_at_md;
	}
      else if (mom_item_cmp (curatitm, itat) > 0)
	{
	  if (!val.ptr)
	    return attrs;
	  /// insert at md
	  if (nbattr >= size - 1)
	    {
	      unsigned newsize = ((5 * nbattr / 4 + 3) | 3) + 1;
	      struct mom_itemattributes_st *newattrs
		=
		GC_MALLOC (sizeof (struct mom_itemattributes_st) +
			   newsize * sizeof (struct mom_attrentry_st));
	      if (MONIMELT_UNLIKELY (!newattrs))
		MONIMELT_FATAL ("failed to grow attributes of %d",
				(int) newsize);
	      memset (newattrs, 0,
		      sizeof (struct mom_itemattributes_st) +
		      newsize * sizeof (struct mom_attrentry_st));
	      memcpy (newattrs, attrs,
		      sizeof (struct mom_itemattributes_st) +
		      md * sizeof (struct mom_attrentry_st));
	      newattrs->size = newsize;
	      newattrs->nbattr = nbattr + 1;
	      newattrs->itattrtab[md].aten_itm = itat;
	      newattrs->itattrtab[md].aten_val = val;
	      memcpy (newattrs->itattrtab + md + 1, attrs->itattrtab + md,
		      (nbattr - md) * sizeof (struct mom_itemattributes_st));
	      attrs->size = attrs->nbattr = 0;
	      GC_FREE (attrs);
	      return newattrs;
	    }
	  else
	    {
	      for (unsigned ix = md + 1; ix < nbattr; ix++)
		attrs->itattrtab[ix] = attrs->itattrtab[ix - 1];
	      attrs->nbattr = nbattr + 1;
	      attrs->itattrtab[md].aten_itm = itat;
	      attrs->itattrtab[md].aten_val = val;
	      return attrs;
	    }
	}
    }
  // this should never happen
  MONIMELT_FATAL ("corrupted attributes @%p", attrs);
remove_at_md:
  if (3 * nbattr / 2 + 1 < size && size > 4)
    {
      unsigned newsize = ((5 * nbattr / 4 + 3) | 3) + 1;
      if (newsize < size)
	{
	  struct mom_itemattributes_st *newattrs
	    =
	    GC_MALLOC (sizeof (struct mom_itemattributes_st) +
		       newsize * sizeof (struct mom_attrentry_st));
	  if (MONIMELT_UNLIKELY (!newattrs))
	    MONIMELT_FATAL ("failed to shrink attributes of %d",
			    (int) newsize);
	  memset (newattrs, 0,
		  sizeof (struct mom_itemattributes_st) +
		  newsize * sizeof (struct mom_attrentry_st));
	  memcpy (newattrs, attrs,
		  sizeof (struct mom_itemattributes_st) +
		  md * sizeof (struct mom_attrentry_st));
	  for (unsigned ix = md; ix + 1 < nbattr; ix++)
	    newattrs->itattrtab[ix] = attrs->itattrtab[ix + 1];
	  newattrs->size = newsize;
	  newattrs->nbattr = nbattr - 1;
	  attrs->size = 0;
	  attrs->nbattr = 0;
	  GC_FREE (attrs);
	  return newattrs;
	}
      else
	{
	  for (unsigned ix = md; ix + 1 < nbattr; ix++)
	    attrs->itattrtab[ix] = attrs->itattrtab[ix + 1];
	  attrs->itattrtab[nbattr].aten_itm = NULL;
	  attrs->itattrtab[nbattr].aten_val = MONIMELT_NULLV;
	  attrs->nbattr = nbattr - 1;
	  return attrs;
	}
    }
}

momval_t
mom_item_get_attr (mom_anyitem_t * itm, mom_anyitem_t * itat)
{
  momval_t res = MONIMELT_NULLV;
  if (!itm || !itat || itm->typnum <= momty__itemlowtype
      || itat->typnum <= momty__itemlowtype)
    return MONIMELT_NULLV;
  pthread_mutex_lock (&itm->i_mtx);
  res = get_attribute (itm->i_attrs, itat);
  pthread_mutex_unlock (&itm->i_mtx);
  return res;
}

int
mom_item_get_several_attrs (mom_anyitem_t * itm, ...)
{
  int cnt = 0;
  if (!itm || itm->typnum <= momty__itemlowtype)
    return 0;
  va_list args;
  pthread_mutex_lock (&itm->i_mtx);
  va_start (args, itm);
  for (;;)
    {
      mom_anyitem_t *curitat = va_arg (args, mom_anyitem_t *);
      if (!curitat || curitat->typnum <= momty__itemlowtype)
	break;
      momval_t *curvalp = va_arg (args, momval_t *);
      momval_t curval = get_attribute (itm->i_attrs, curitat);
      if (curval.ptr)
	cnt++;
      if (curvalp)
	*curvalp = curval;
      else
	break;
    }
  va_end (args);
  pthread_mutex_unlock (&itm->i_mtx);
  return cnt;
}

void
mom_item_put_attr (mom_anyitem_t * itm, mom_anyitem_t * itat, momval_t val)
{
  if (!itm || itm->typnum <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&itm->i_mtx);
  itm->i_attrs = put_attribute (itm->i_attrs, itat, val);
  pthread_mutex_unlock (&itm->i_mtx);
}

void
mom_item_put_several_attrs (mom_anyitem_t * itm, ...)
{
  if (!itm || itm->typnum <= momty__itemlowtype)
    return;
  va_list args;
  pthread_mutex_lock (&itm->i_mtx);
  va_start (args, itm);
  for (;;)
    {
      mom_anyitem_t *curitat = va_arg (args, mom_anyitem_t *);
      if (!curitat || curitat->typnum <= momty__itemlowtype)
	break;
      momval_t curval = va_arg (args, momval_t);
      itm->i_attrs = put_attribute (itm->i_attrs, curitat, curval);
    }
  va_end (args);
  pthread_mutex_unlock (&itm->i_mtx);
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
      if ((void *) curitm == MONIMELT_EMPTY)
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
      if ((void *) curitm == MONIMELT_EMPTY)
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


void
mom_scan_any_item_data (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  if (MONIMELT_UNLIKELY (!itm || itm->typnum <= momty__itemlowtype))
    return;
  struct mom_itemattributes_st *iat = itm->i_attrs;
  if (iat)
    {
      momusize_t nba = iat->nbattr;
      for (unsigned ix = 0; ix < nba; ix++)
	{
	  mom_dump_add_item (dmp, iat->itattrtab[ix].aten_itm);
	  mom_dump_scan_value (dmp, iat->itattrtab[ix].aten_val);
	}
    }
  if (itm->i_content.ptr)
    mom_dump_scan_value (dmp, itm->i_content);
}


void
mom_fill_any_item_data (mom_anyitem_t * it, momval_t jsob)
{
  momval_t jattrs = mom_jsonob_get (jsob, (momval_t) mom_item__attributes);
  if (jattrs.ptr && *jattrs.ptype == momty_jsonarray)
    {
    }
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

////////////////////////////////////////////////////////////////
/// type routine for boolean items

static mom_anyitem_t *
bool_itemloader (struct mom_loader_st *ld, momval_t json
		 __attribute__ ((unused)), uuid_t uid)
{
  char ustr[UUID_PARSED_LEN];
  uuid_unparse (uid, ustr);
  MONIMELT_FATAL ("invalid call to bool_itemloader %s", ustr);
}

static void
bool_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm, momval_t json)
{
#warning bool_itemfiller should fill the common data
}

static void
bool_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
}

static momval_t
bool_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return MONIMELT_NULLV;
}

static momval_t
bool_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return
    (momval_t) mom_make_json_object
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs), MOMJSON_ENTRY,
     mom_item__content, mom_dump_emit_json (dmp, itm->i_content),
     MOMJSON_END);
}

////////////////////////////////////////////////////////////////
/// type routine for json name items
static mom_anyitem_t *
json_name_itemloader (struct mom_loader_st *ld, momval_t json
		      __attribute__ ((unused)), uuid_t uid)
{
}

static void
json_name_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		      momval_t json)
{
}

static void
json_name_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
  // no need to scan ij_namejson, it is a string
}

static momval_t
json_name_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
}

static momval_t
json_name_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
}
