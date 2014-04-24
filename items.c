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
      else
	if (MONIMELT_UNLIKELY
	    (!memcmp
	     (newitm->i_uuid, items_data->items_arr[i]->i_uuid,
	      sizeof (uuid_t))))
	{
	  char uidstr[UUID_PARSED_LEN];
	  memset (uidstr, 0, sizeof (uidstr));
	  uuid_unparse (newitm->i_uuid, uidstr);
	  MONIMELT_FATAL ("duplicate uuid %s", uidstr);
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
      else
	if (MONIMELT_UNLIKELY
	    (!memcmp
	     (newitm->i_uuid, items_data->items_arr[i]->i_uuid,
	      sizeof (uuid_t))))
	{
	  char uidstr[UUID_PARSED_LEN];
	  memset (uidstr, 0, sizeof (uidstr));
	  uuid_unparse (newitm->i_uuid, uidstr);
	  MONIMELT_FATAL ("duplicate uuid %s", uidstr);
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
      else
	if (MONIMELT_UNLIKELY
	    (!memcmp
	     (newitm->i_uuid, items_data->items_arr[i]->i_uuid,
	      sizeof (uuid_t))))
	{
	  char uidstr[UUID_PARSED_LEN];
	  memset (uidstr, 0, sizeof (uidstr));
	  uuid_unparse (newitm->i_uuid, uidstr);
	  MONIMELT_FATAL ("duplicate uuid %s", uidstr);
	}
      if (!items_data->items_arr[i + 1]
	  || items_data->items_arr[i + 1] == MONIMELT_EMPTY)
	{
	  items_data->items_arr[i + 1] = newitm;
	  items_data->items_nb++;
	  return;
	}
      else
	if (MONIMELT_UNLIKELY
	    (!memcmp
	     (newitm->i_uuid, items_data->items_arr[i]->i_uuid,
	      sizeof (uuid_t))))
	{
	  char uidstr[UUID_PARSED_LEN];
	  memset (uidstr, 0, sizeof (uidstr));
	  uuid_unparse (newitm->i_uuid, uidstr);
	  MONIMELT_FATAL ("duplicate uuid %s", uidstr);
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


static void
mom_finalize_item (void *itmad, void *data)
{
  mom_anyitem_t *itm = itmad;
  const struct momitemtypedescr_st *idescr =
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
mom_allocate_item_with_uuid (unsigned type, size_t itemsize, unsigned space,
			     uuid_t uid)
{
  struct momanyitem_st *p = NULL;
  assert (itemsize >= sizeof (struct momanyitem_st));
  p = GC_MALLOC (itemsize);
  if (!p)
    MONIMELT_FATAL ("out of memory for item type %d size %ld",
		    type, (long) itemsize);
  memset (p, 0, itemsize);
  if (space != 0)
    {
      if (MONIMELT_UNLIKELY
	  (space > MONIMELT_SPACE_MAX || !mom_spacedescr_array[space]))
	space = 0;
    }
  pthread_mutex_lock (&mtx_global_items);
  p->typnum = type;
  p->i_space = space;
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
mom_allocate_item (unsigned type, size_t itemsize, unsigned space)
{
  assert (itemsize > sizeof (struct momanyitem_st));
  uuid_t uid;
  memset (uid, 0, sizeof (uid));
  uuid_generate (uid);
  return mom_allocate_item_with_uuid (type, itemsize, space, uid);
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
  return attrs;
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
	  mom_anyitem_t *curat = iat->itattrtab[ix].aten_itm;
	  if (!curat || !curat->i_space)
	    continue;
	  mom_dump_add_item (dmp, curat);
	  mom_dump_scan_value (dmp, iat->itattrtab[ix].aten_val);
	}
    }
  if (itm->i_content.ptr)
    mom_dump_scan_value (dmp, itm->i_content);
}


momit_json_name_t *
mom_make_item_json_name_of_uuid (uuid_t uid, const char *name, unsigned space)
{
  momit_json_name_t *itm
    = mom_allocate_item_with_uuid (momty_jsonitem, sizeof (momit_json_name_t),
				   space, uid);
  if (name)
    itm->ij_namejson = mom_make_string (name);
  return itm;
}

momit_json_name_t *
mom_make_item_json_name (const char *name, unsigned space)
{
  momit_json_name_t *itm
    = mom_allocate_item (momty_jsonitem, sizeof (momit_json_name_t), space);
  if (name)
    itm->ij_namejson = mom_make_string (name);
  return itm;
}

/// for json name item type descriptor
static mom_anyitem_t *json_name_itemloader (struct mom_loader_st *ld,
					    momval_t json, uuid_t uid,
					    unsigned space);
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


momit_bool_t *
mom_create_named_bool (uuid_t uid, const char *name)
{
  if (!strcmp (name, "true"))
    {
      momit_bool_t *itrue
	= mom_allocate_item_with_uuid (momty_boolitem, sizeof (momit_bool_t),
				       MONIMELT_SPACE_ROOT, uid);
      itrue->ib_bool = true;
      return itrue;
    }
  else if (!strcmp (name, "false"))
    {
      momit_bool_t *ifalse
	= mom_allocate_item_with_uuid (momty_boolitem, sizeof (momit_bool_t),
				       MONIMELT_SPACE_ROOT, uid);
      ifalse->ib_bool = false;
      return ifalse;
    }
  else
    MONIMELT_FATAL ("invalid boolean item name=%s", name);
}

////////////////////////////////////////////////////////////////
/// type routine for boolean items

/// for boolean item type descriptor
static mom_anyitem_t *bool_itemloader (struct mom_loader_st *ld,
				       momval_t json, uuid_t uid,
				       unsigned space);
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

static mom_anyitem_t *
bool_itemloader (struct mom_loader_st *ld, momval_t json
		 __attribute__ ((unused)), uuid_t uid, unsigned space)
{
  char ustr[UUID_PARSED_LEN];
  uuid_unparse (uid, ustr);
  MONIMELT_FATAL ("invalid call to bool_itemloader %s", ustr);
}

static void
bool_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm, momval_t json)
{
  mom_load_any_item_data (ld, itm, json);
}

static void
bool_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
}

static momval_t
bool_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return (momval_t) mom_make_json_object (	// the type:
					   MOMJSON_ENTRY, mom_item__jtype,
					   mom_item__bool_item,
					   // done
					   MOMJSON_END);
}

static momval_t
bool_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return
    (momval_t) mom_make_json_object
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     MOMJSON_END);
}

////////////////////////////////////////////////////////////////
/// type routine for json name items
static mom_anyitem_t *
json_name_itemloader (struct mom_loader_st *ld, momval_t json, uuid_t uid,
		      unsigned space)
{
  const char *name =
    mom_string_cstr (mom_jsonob_get (json, (momval_t) mom_item__name));
  if (name && name[0])
    return (mom_anyitem_t *) mom_make_item_json_name_of_uuid (uid, name,
							      space);
  MONIMELT_FATAL ("failed to load & build json name item");
}

static void
json_name_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		      momval_t json)
{
  mom_load_any_item_data (ld, itm, json);
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
  return (momval_t) mom_make_json_object (	// the type:
					   MOMJSON_ENTRY, mom_item__jtype,
					   mom_item__json_name_item,
					   // the name
					   MOMJSON_ENTRY, mom_item__name,
					   ((momit_json_name_t *)
					    itm)->ij_namejson,
					   // done
					   MOMJSON_END);
}

static momval_t
json_name_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return (momval_t) mom_make_json_object
    /// attributes
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     /// contents
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     /// done
     MOMJSON_END);
}

/// tasklets

momit_tasklet_t *
mom_make_item_tasklet_of_uuid (uuid_t uid, unsigned space)
{
  momit_tasklet_t *itm
    =
    mom_allocate_item_with_uuid (momty_taskletitem, sizeof (momit_tasklet_t),
				 space, uid);
  return itm;
}

momit_tasklet_t *
mom_make_item_tasklet (unsigned space)
{
  momit_tasklet_t *itm
    = mom_allocate_item (momty_taskletitem, sizeof (momit_tasklet_t), space);
  return itm;
}

/// for tasklet item type descriptor
static mom_anyitem_t *tasklet_itemloader (struct mom_loader_st *ld,
					  momval_t json, uuid_t uid,
					  unsigned space);
static void tasklet_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
				momval_t json);
static void tasklet_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm);
static momval_t tasklet_itemgetbuild (struct mom_dumper_st *dmp,
				      mom_anyitem_t * itm);
static momval_t tasklet_itemgetfill (struct mom_dumper_st *dmp,
				     mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_tasklet = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "tasklet",
  .ityp_loader = tasklet_itemloader,
  .ityp_filler = tasklet_itemfiller,
  .ityp_scan = tasklet_itemscan,
  .ityp_getbuild = tasklet_itemgetbuild,
  .ityp_getfill = tasklet_itemgetfill,
};

static mom_anyitem_t *
tasklet_itemloader (struct mom_loader_st *ld, momval_t json
		    __attribute__ ((unused)), uuid_t uid, unsigned space)
{
  return (mom_anyitem_t *) mom_make_item_tasklet_of_uuid (uid, space);
}




static momval_t
tasklet_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momval_t *jframarr = NULL;
  momval_t jframetiny[TINY_MAX] = { MONIMELT_NULLV };
  momit_tasklet_t *tskitm = (momit_tasklet_t *) itm;
  unsigned fratop = tskitm->itk_fratop;
  if (tskitm->itk_fratop >= TINY_MAX)
    {
      jframarr = GC_MALLOC (fratop * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!jframarr))
	MONIMELT_FATAL ("failed to allocate for %d frames", fratop);
      memset (jframarr, 0, fratop * sizeof (momval_t));
    }
  else
    jframarr = jframetiny;
  for (unsigned ix = 0; ix < fratop; ix++)
    {
      struct momframe_st *curfra = tskitm->itk_frames + ix;
      momclosure_t *curclo = tskitm->itk_closures[ix];
      if (MONIMELT_UNLIKELY (!curclo || curclo->typnum != momty_closure))
	MONIMELT_FATAL ("corrupted frame #%d", ix);
      unsigned nbnum = curfra->fr_dbloff - curfra->fr_intoff;
      unsigned nbdbl = 0, nbval = 0;
      if (ix + 1 >= fratop)
	{
	  nbdbl =
	    ((tskitm->itk_scaltop -
	      curfra->fr_dbloff) * sizeof (intptr_t)) / sizeof (double);
	  nbval = (tskitm->itk_valtop - curfra->fr_valoff);
	}
      else
	{
	  nbdbl =
	    (((tskitm->itk_frames[ix + 1].fr_intoff -
	       curfra->fr_dbloff) * sizeof (intptr_t)) / sizeof (double));
	  nbval = tskitm->itk_frames[ix + 1].fr_valoff - curfra->fr_valoff;
	}
      momval_t *valarr = NULL;
      momval_t *numarr = NULL;
      momval_t *dblarr = NULL;
      momval_t valtiny[TINY_MAX] = { MONIMELT_NULLV };
      momval_t numtiny[TINY_MAX] = { MONIMELT_NULLV };
      momval_t dbltiny[TINY_MAX] = { MONIMELT_NULLV };
      if (nbnum < TINY_MAX)
	numarr = numtiny;
      else
	{
	  numarr = GC_MALLOC (sizeof (momval_t) * nbnum);
	  if (MONIMELT_UNLIKELY (!numarr))
	    MONIMELT_FATAL ("failed to allocate for %d numbers", nbnum);
	  memset (numarr, 0, sizeof (momval_t) * nbnum);
	}
      if (nbdbl < TINY_MAX)
	dblarr = dbltiny;
      else
	{
	  dblarr = GC_MALLOC (sizeof (momval_t) * nbdbl);
	  if (MONIMELT_UNLIKELY (!dblarr))
	    MONIMELT_FATAL ("failed to allocate for %d doubles", nbdbl);
	  memset (dblarr, 0, sizeof (momval_t) * nbdbl);
	}
      if (nbval < TINY_MAX)
	valarr = valtiny;
      else
	{
	  valarr = GC_MALLOC (sizeof (momval_t) * nbval);
	  if (MONIMELT_UNLIKELY (!valarr))
	    MONIMELT_FATAL ("failed to allocate for %d values", nbval);
	  memset (valarr, 0, sizeof (momval_t) * nbval);
	}
      for (unsigned nix = 0; ix < nbnum; nix++)
	numarr[nix] =
	  (momval_t)
	  mom_make_int (((intptr_t *) (tskitm->itk_scalars +
				       curfra->fr_intoff))[nix]);
      for (unsigned dix = 0; dix < nbdbl; dix++)
	dblarr[dix] =
	  (momval_t)
	  mom_make_double (((double *) (tskitm->itk_scalars +
					curfra->fr_dbloff))[dix]);
      for (unsigned vix = 0; vix < nbval; vix++)
	valarr[vix] =
	  mom_dump_emit_json (dmp,
			      tskitm->itk_values[curfra->fr_valoff + vix]);
      jframarr[ix] =
	/// make the frame json
	(momval_t) mom_make_json_object
	/// 
	(
	  /// values
	  MOMJSON_ENTRY,
	  mom_item__values,
	  (momval_t) mom_make_json_array_count (nbval, valarr),
	  /// numbers
	  MOMJSON_ENTRY,
	  mom_item__numbers,
	  (momval_t) mom_make_json_array_count (nbnum, numarr),
	  /// doubles
	  MOMJSON_ENTRY,
	  mom_item__doubles,
	  (momval_t) mom_make_json_array_count (nbdbl, dblarr),
	  /// closure
	  MOMJSON_ENTRY,
	  mom_item__closure,
	  (momval_t)
	  mom_dump_emit_json
	  (dmp, (momval_t) (const momclosure_t *) (tskitm->itk_closures[ix])),
	  /// state
	  MOMJSON_ENTRY, mom_item__state, mom_make_int (curfra->fr_state),
	  /// end
	  MOMJSON_END);
    }
  return (momval_t) mom_make_json_object
    /// attributes
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     /// contents
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     /// frames
     MOMJSON_ENTRY, mom_item__frames,
     (momval_t) mom_make_json_array_count (fratop, jframarr),
     /// end
     MOMJSON_END);
}

static void
tasklet_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		    momval_t json)
{
  mom_load_any_item_data (ld, itm, json);
  momval_t jframes = mom_jsonob_get (json, (momval_t) mom_item__frames);
  unsigned nbframes = mom_json_array_size (jframes);
  assert (itm->typnum == momty_taskletitem);
  mom_tasklet_reserve ((momval_t) itm, 2 * nbframes, nbframes / 2,
		       3 * nbframes, nbframes);
  for (unsigned frix = 0; frix < nbframes; frix++)
    {
      momval_t jcurframe = mom_json_array_nth (jframes, frix);
      momval_t jclosure =
	mom_jsonob_get (jcurframe, (momval_t) mom_item__closure);
      momval_t jvalues =
	mom_jsonob_get (jcurframe, (momval_t) mom_item__values);
      momval_t jnumbers =
	mom_jsonob_get (jcurframe, (momval_t) mom_item__numbers);
      momval_t jdoubles =
	mom_jsonob_get (jcurframe, (momval_t) mom_item__doubles);
      momval_t jstate =
	mom_jsonob_get (jcurframe, (momval_t) mom_item__state);
      momval_t closv = mom_load_value_json (ld, jclosure);
      if (closv.ptr == NULL || *closv.ptype != momty_closure)
	continue;
      momval_t valtiny[TINY_MAX] = { MONIMELT_NULLV };
      intptr_t numtiny[TINY_MAX] = { 0 };
      double dbltiny[TINY_MAX] = { 0.0 };
      unsigned nbval = mom_json_array_size (jvalues);
      unsigned nbnum = mom_json_array_size (jnumbers);
      unsigned nbdbl = mom_json_array_size (jdoubles);
      int state = mom_int_of_value_else_0 (jstate);
      momval_t *valarr = NULL;
      intptr_t *numarr = NULL;
      double *dblarr = NULL;
      /// values
      if (nbval < TINY_MAX)
	valarr = valtiny;
      else
	{
	  valarr = GC_MALLOC (nbval * sizeof (momval_t));
	  if (!valarr)
	    MONIMELT_FATAL ("failed to allocate %d values", nbval);
	  memset (valarr, 0, nbval * sizeof (momval_t));
	}
      for (unsigned vix = 0; vix < nbval; vix++)
	valarr[vix] =
	  mom_load_value_json (ld, mom_json_array_nth (jvalues, vix));
      /// numbers
      if (nbnum < TINY_MAX)
	numarr = numtiny;
      else
	{
	  numarr = GC_MALLOC_ATOMIC (nbnum * sizeof (intptr_t));
	  if (!numarr)
	    MONIMELT_FATAL ("failed to allocate %d numbers", nbnum);
	  memset (numarr, 0, nbnum * sizeof (intptr_t));
	}
      for (unsigned nix = 0; nix < nbnum; nix++)
	numarr[nix] =
	  mom_int_of_value_else_0 (mom_json_array_nth (jnumbers, nix));
      /// doubles
      if (nbdbl < TINY_MAX)
	dblarr = dbltiny;
      else
	{
	  dblarr = GC_MALLOC_ATOMIC (nbdbl * sizeof (double));
	  if (!dblarr)
	    MONIMELT_FATAL ("failed to allocate %d doubles", nbdbl);
	  memset (dblarr, 0, nbdbl * sizeof (double));
	}
      for (unsigned dix = 0; dix < nbdbl; dix++)
	dblarr[dix] =
	  mom_double_of_value_else_0 (mom_json_array_nth (jdoubles, dix));
      /// push the frame
      mom_tasklet_push_frame ((momval_t) itm, closv,
			      // state
			      MOMPFR_STATE, state,
			      // values
			      MOMPFR_ARRAY_VALUES, nbval, valarr,
			      // numbers
			      MOMPFR_ARRAY_INTS, nbnum, numarr,
			      // doubles
			      MOMPFR_ARRAY_DOUBLES, nbdbl, dblarr,
			      // end
			      MOMPFR_END);
    }
}









static void
tasklet_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
  momit_tasklet_t *tskitm = (momit_tasklet_t *) itm;
  unsigned fratop = tskitm->itk_fratop;
  for (unsigned frix = 0; frix < fratop; frix++)
    {
      struct momframe_st *curfra = tskitm->itk_frames + frix;
      const momclosure_t *curclo = tskitm->itk_closures[frix];
      if (MONIMELT_UNLIKELY (!curclo || curclo->typnum != momty_closure))
	MONIMELT_FATAL ("corrupted frame #%d", frix);
      mom_dump_scan_value (dmp, (momval_t) curclo);
      unsigned nbval = 0;
      if (frix + 1 >= fratop)
	nbval = (tskitm->itk_valtop - curfra->fr_valoff);
      else
	nbval = tskitm->itk_frames[frix + 1].fr_valoff - curfra->fr_valoff;
      for (unsigned vix = 0; vix < nbval; vix++)
	mom_dump_scan_value (dmp,
			     tskitm->itk_values[tskitm->
						itk_frames[frix].fr_valoff +
						vix]);
    }
}

static momval_t
tasklet_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return (momval_t) mom_make_json_object (	// the type:
					   MOMJSON_ENTRY, mom_item__jtype,
					   mom_item__tasklet_item,
					   // done
					   MOMJSON_END);
}


#define SYMNAME_LEN 128
#define MOM_ROUTINE_NAME_FMT "momrout_%s"
momit_routine_t *
mom_make_item_routine_of_uuid (uuid_t uid, const char *name, unsigned space)
{
  char symname[SYMNAME_LEN];
  memset (symname, 0, sizeof (symname));
  if (MONIMELT_UNLIKELY (!name || !name[0]))
    MONIMELT_FATAL ("bad name for item routine");
  snprintf (symname, sizeof (symname), MOM_ROUTINE_NAME_FMT, name);
  struct momroutinedescr_st *rdescr = NULL;
  if (!g_module_symbol
      (mom_prog_module, symname, (gpointer *) & rdescr) || !rdescr)
    MONIMELT_FATAL ("failed to find routine descriptor %s : %s", symname,
		    g_module_error ());
  if (rdescr->rout_magic != ROUTINE_MAGIC || !rdescr->rout_code
      || strcmp (rdescr->rout_name, name))
    MONIMELT_FATAL ("bad routine descriptor %s", symname);
  momit_routine_t *itrout =
    mom_allocate_item_with_uuid (momty_routineitem, sizeof (momit_routine_t),
				 space, uid);
  itrout->irt_descr = rdescr;
  return itrout;
}

momit_routine_t *
mom_make_item_routine (const char *name, unsigned space)
{
  char symname[SYMNAME_LEN];
  memset (symname, 0, sizeof (symname));
  if (MONIMELT_UNLIKELY (!name || !name[0]))
    MONIMELT_FATAL ("bad name for item routine");
  snprintf (symname, sizeof (symname), MOM_ROUTINE_NAME_FMT, name);
  struct momroutinedescr_st *rdescr = NULL;
  if (!g_module_symbol
      (mom_prog_module, symname, (gpointer *) & rdescr) || !rdescr)
    MONIMELT_FATAL ("failed to find routine descriptor %s : %s", symname,
		    g_module_error ());
  if (rdescr->rout_magic != ROUTINE_MAGIC || !rdescr->rout_code
      || strcmp (rdescr->rout_name, name))
    MONIMELT_FATAL ("bad routine descriptor %s", symname);
  momit_routine_t *itrout =
    mom_allocate_item (momty_routineitem, sizeof (momit_routine_t), space);
  itrout->irt_descr = rdescr;
  return itrout;
}


momit_routine_t *
mom_try_make_item_routine (const char *name, unsigned space)
{
  char symname[SYMNAME_LEN];
  memset (symname, 0, sizeof (symname));
  if (MONIMELT_UNLIKELY (!name || !name[0]))
    return NULL;
  snprintf (symname, sizeof (symname), MOM_ROUTINE_NAME_FMT, name);
  struct momroutinedescr_st *rdescr = NULL;
  if (!g_module_symbol
      (mom_prog_module, symname, (gpointer *) & rdescr) || !rdescr)
    {
      MONIMELT_INFORM ("failed to find routine descriptor %s : %s", symname,
		       g_module_error ());
      return NULL;
    }
  if (rdescr->rout_magic != ROUTINE_MAGIC || !rdescr->rout_code
      || strcmp (rdescr->rout_name, name))
    MONIMELT_FATAL ("bad routine descriptor %s", symname);
  momit_routine_t *itrout =
    mom_allocate_item (momty_routineitem, sizeof (momit_routine_t), space);
  itrout->irt_descr = rdescr;
  return itrout;
}


/// for routine item type descriptor
static mom_anyitem_t *routine_itemloader (struct mom_loader_st *ld,
					  momval_t json, uuid_t uid,
					  unsigned space);
static void routine_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
				momval_t json);
static void routine_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm);
static momval_t routine_itemgetbuild (struct mom_dumper_st *dmp,
				      mom_anyitem_t * itm);
static momval_t routine_itemgetfill (struct mom_dumper_st *dmp,
				     mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_routine = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "routine",
  .ityp_loader = routine_itemloader,
  .ityp_filler = routine_itemfiller,
  .ityp_scan = routine_itemscan,
  .ityp_getbuild = routine_itemgetbuild,
  .ityp_getfill = routine_itemgetfill,
};

static momval_t
routine_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momit_routine_t *routitm = (momit_routine_t *) itm;
  return (momval_t) mom_make_json_object
    // build with type and routine name
    (				// the type:
      MOMJSON_ENTRY, mom_item__jtype, mom_item__routine_item,
      // the routine name
      MOMJSON_ENTRY, mom_item__name,
      mom_make_string (routitm->irt_descr->rout_name),
      // that's all!
      MOMJSON_END);
}

static mom_anyitem_t *
routine_itemloader (struct mom_loader_st *ld, momval_t json, uuid_t uid,
		    unsigned space)
{
  const char *name =
    mom_string_cstr (mom_jsonob_get (json, (momval_t) mom_item__name));
  if (MONIMELT_UNLIKELY (!name))
    MONIMELT_FATAL ("missing name for routine item");
  return (mom_anyitem_t *) mom_make_item_routine_of_uuid (uid, name, space);
}


static void
routine_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		    momval_t json)
{
  mom_load_any_item_data (ld, itm, json);
}

static void
routine_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
}

static momval_t
routine_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return
    (momval_t) mom_make_json_object
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     MOMJSON_END);
}

//////////////////////////////////////////////////// vector items
static void
reserve_vectoritem (momit_vector_t * vecitm, unsigned more)
{
  if (MONIMELT_UNLIKELY (vecitm->itv_count + more + 1 >= vecitm->itv_size))
    {
      unsigned newsiz = ((5 * vecitm->itv_count / 4 + 5 + more) | 7) + 1;
      momval_t *newarr = GC_MALLOC (newsiz * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!newarr))
	MONIMELT_FATAL ("failed to grow vector to %d", (int) newsiz);
      memset (newarr, 0, newsiz * sizeof (momval_t));
      if (vecitm->itv_count > 0)
	memcpy (newarr, vecitm->itv_arr,
		vecitm->itv_count * sizeof (momval_t));
      GC_FREE (vecitm->itv_arr);
      vecitm->itv_size = newsiz;
    }
}

momit_vector_t *
mom_make_item_vector (unsigned space, unsigned reserve)
{
  momit_vector_t *itmvec =
    mom_allocate_item (momty_vectoritem, sizeof (momit_vector_t), space);
  if (reserve > 0)
    reserve_vectoritem (itmvec, reserve);
  return itmvec;
}

momit_vector_t *
mom_make_item_vector_of_uuid (uuid_t uid, unsigned space, unsigned reserve)
{
  momit_vector_t *itmvec =
    mom_allocate_item_with_uuid (momty_vectoritem, sizeof (momit_vector_t),
				 space, uid);
  if (reserve > 0)
    reserve_vectoritem (itmvec, reserve);
  return itmvec;
}

unsigned
mom_item_vector_count (const momval_t vec)
{
  unsigned cnt = 0;
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return 0;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  cnt = vec.pvectitem->itv_count;
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
  return cnt;
}

const momval_t
mom_item_vector_nth (const momval_t vec, int rk)
{
  momval_t res = MONIMELT_NULLV;
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return MONIMELT_NULLV;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned cnt = vec.pvectitem->itv_count;
  if (rk < 0)
    rk += cnt;
  if (rk >= 0 && rk < (int) cnt)
    res = vec.pvectitem->itv_arr[rk];
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
  return res;
}

void
mom_item_vector_put_nth (const momval_t vec, int rk, const momval_t val)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned cnt = vec.pvectitem->itv_count;
  if (rk < 0)
    rk += cnt;
  if (rk >= 0 && rk < (int) cnt)
    vec.pvectitem->itv_arr[rk] = val;
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}

void
mom_item_vector_resize (momval_t vec, unsigned newcount)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned oldcnt = vec.pvectitem->itv_count;
  unsigned oldsiz = vec.pvectitem->itv_size;
  momval_t *oldarr = vec.pvectitem->itv_arr;
  if (newcount < oldcnt)
    {
      memset (vec.pvectitem->itv_arr + oldcnt, 0,
	      sizeof (momval_t) * (oldcnt - newcount));
      if (oldsiz > 24 && oldsiz > 2 * newcount)
	{
	  unsigned newsiz = ((9 * newcount / 8 + 3) | 7) + 1;
	  if (newsiz < oldsiz)
	    {
	      momval_t *newarr = GC_MALLOC (newsiz * sizeof (momval_t));
	      if (MONIMELT_UNLIKELY (!newarr))
		MONIMELT_FATAL ("failed to shrink vector to %d",
				(int) newsiz);
	      memset (newarr, 0, newsiz * sizeof (momval_t));
	      memcpy (newarr, oldarr, newcount * sizeof (momval_t));
	      vec.pvectitem->itv_size = newsiz;
	      vec.pvectitem->itv_arr = newarr;
	      GC_FREE (oldarr), oldarr = NULL;
	    }
	}
      vec.pvectitem->itv_count = newcount;
    }
  else if (newcount > oldcnt)
    {
      if (newcount + 1 >= oldsiz)
	reserve_vectoritem (vec.pvectitem, 9 * newcount / 8 - oldcnt + 1);
      vec.pvectitem->itv_count = newcount;
    };
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}

void
mom_item_vector_reserve (momval_t vec, unsigned more)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  reserve_vectoritem (vec.pvectitem, more);
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}

void
mom_item_vector_append1 (momval_t vec, momval_t val)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  if (MONIMELT_UNLIKELY
      (vec.pvectitem->itv_count + 1 >= vec.pvectitem->itv_size))
    reserve_vectoritem (vec.pvectitem, 1 + vec.pvectitem->itv_count / 8);
  vec.pvectitem->itv_arr[vec.pvectitem->itv_count] = val;
  vec.pvectitem->itv_count++;
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}

void
mom_item_vector_append_values (momval_t vec, unsigned nbval, ...)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned oldcnt = vec.pvectitem->itv_count;
  if (oldcnt + 1 + nbval >= vec.pvectitem->itv_size)
    reserve_vectoritem (vec.pvectitem, 2 + nbval + oldcnt / 8);
  va_list args;
  va_start (args, nbval);
  for (unsigned ix = 0; ix < nbval; ix++)
    {
      momval_t curval = va_arg (args, momval_t);
      vec.pvectitem->itv_arr[oldcnt + ix] = curval;
    }
  va_end (args);
  vec.pvectitem->itv_count += nbval;
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}

void
mom_item_vector_append_til_nil (momval_t vec, ...)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned oldcnt = vec.pvectitem->itv_count;
  unsigned nbval = 0;
  va_list args;
  va_start (args, vec);
  while (va_arg (args, momval_t).ptr != NULL)
    nbval++;
  va_end (args);
  if (oldcnt + 1 + nbval >= vec.pvectitem->itv_size)
    reserve_vectoritem (vec.pvectitem, 2 + nbval + oldcnt / 8);
  va_start (args, vec);
  for (unsigned ix = 0; ix < nbval; ix++)
    {
      momval_t curval = va_arg (args, momval_t);
      vec.pvectitem->itv_arr[oldcnt + ix] = curval;
    }
  va_end (args);
  vec.pvectitem->itv_count += nbval;
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}


void
mom_item_vector_append_count (momval_t vec, unsigned nbval, momval_t * arr)
{
  if (!vec.ptr || *vec.ptype != momty_vectoritem || !nbval || !arr)
    return;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned oldcnt = vec.pvectitem->itv_count;
  if (oldcnt + 1 + nbval >= vec.pvectitem->itv_size)
    reserve_vectoritem (vec.pvectitem, 2 + nbval + oldcnt / 8);
  for (unsigned ix = 0; ix < nbval; ix++)
    vec.pvectitem->itv_arr[oldcnt + ix] = arr[ix];
  vec.pvectitem->itv_count += nbval;
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
}

const momitemset_t *
mom_make_item_set_from_item_vector (momval_t vec)
{
  const momitemset_t *iset = NULL;
  assert (sizeof (mom_anyitem_t *) == sizeof (momval_t));
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return NULL;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned cnt = vec.pvectitem->itv_count;
  iset =
    mom_make_item_set_from_array (cnt,
				  (const mom_anyitem_t **) vec.
				  pvectitem->itv_arr);
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
  return iset;
}

const momitemtuple_t *
mom_make_item_tuple_from_item_vector (momval_t vec)
{
  const momitemtuple_t *ituple = NULL;
  assert (sizeof (mom_anyitem_t *) == sizeof (momval_t));
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return NULL;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned cnt = vec.pvectitem->itv_count;
  ituple =
    mom_make_item_tuple_from_array
    (cnt, (const mom_anyitem_t **) vec.pvectitem->itv_arr);
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
  return ituple;
}

const momnode_t *
mom_make_node_from_item_vector (momval_t conn, momval_t vec)
{
  const momnode_t *nd = NULL;
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return NULL;
  if (!conn.ptr || *conn.ptype < momty__itemlowtype)
    return NULL;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned cnt = vec.pvectitem->itv_count;
  nd = mom_make_node_from_array (conn.panyitem, cnt, vec.pvectitem->itv_arr);
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
  return nd;
}

const momclosure_t *
mom_make_closure_from_item_vector (momval_t conn, momval_t vec)
{
  const momclosure_t *clo = NULL;
  if (!vec.ptr || *vec.ptype != momty_vectoritem)
    return NULL;
  if (!conn.ptr || *conn.ptype != momty_routineitem)
    return NULL;
  pthread_mutex_lock (&vec.panyitem->i_mtx);
  unsigned cnt = vec.pvectitem->itv_count;
  clo =
    mom_make_closure_from_array (conn.panyitem, cnt, vec.pvectitem->itv_arr);
  pthread_mutex_unlock (&vec.panyitem->i_mtx);
  return clo;
}


/// for vector item type descriptor
static mom_anyitem_t *vector_itemloader (struct mom_loader_st *ld,
					 momval_t json, uuid_t uid,
					 unsigned space);
static void vector_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
			       momval_t json);
static void vector_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm);
static momval_t vector_itemgetbuild (struct mom_dumper_st *dmp,
				     mom_anyitem_t * itm);
static momval_t vector_itemgetfill (struct mom_dumper_st *dmp,
				    mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_vector = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "vector",
  .ityp_loader = vector_itemloader,
  .ityp_filler = vector_itemfiller,
  .ityp_scan = vector_itemscan,
  .ityp_getbuild = vector_itemgetbuild,
  .ityp_getfill = vector_itemgetfill,
};

static momval_t
vector_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return (momval_t) mom_make_json_object
    // build with type 
    (				// the type:
      MOMJSON_ENTRY, mom_item__jtype, mom_item__vector_item,
      // that's all!
      MOMJSON_END);
}

static mom_anyitem_t *
vector_itemloader (struct mom_loader_st *ld __attribute__ ((unused)),
		   momval_t json __attribute__ ((unused)),
		   uuid_t uid, unsigned space)
{
  return (mom_anyitem_t *) mom_make_item_vector_of_uuid (uid, space, 0);
}


static void
vector_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
  momit_vector_t *vecitm = (momit_vector_t *) itm;
  unsigned cnt = vecitm->itv_count;
  for (unsigned ix = 0; ix < cnt; ix++)
    mom_dump_scan_value (dmp, vecitm->itv_arr[ix]);
}


static momval_t
vector_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momit_vector_t *vecitm = (momit_vector_t *) itm;
  unsigned cnt = vecitm->itv_count;
  momval_t *jarr = NULL;
  momval_t tinyarr[TINY_MAX] = { MONIMELT_NULLV };
  if (cnt < TINY_MAX)
    jarr = tinyarr;
  else
    {
      jarr = GC_MALLOC (sizeof (momval_t) * cnt);
      if (MONIMELT_UNLIKELY (!jarr))
	MONIMELT_FATAL
	  ("failed to allocate json array for %d vector elements", (int) cnt);
      memset (jarr, 0, sizeof (momval_t) * cnt);
    }
  for (unsigned vix = 0; vix < cnt; vix++)
    jarr[vix] = mom_dump_emit_json (dmp, vecitm->itv_arr[vix]);
  momval_t jarrvec = (momval_t) mom_make_json_array_count (cnt, jarr);
  if (jarr != tinyarr)
    GC_FREE (jarr);
  jarr = NULL;
  return (momval_t) mom_make_json_object
    /// attributes
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     /// contents
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     /// vector elements
     MOMJSON_ENTRY, mom_item__vector, jarrvec,
     /// end
     MOMJSON_END);

}

static void
vector_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		   momval_t json)
{
  momit_vector_t *vecitm = (momit_vector_t *) itm;
  mom_load_any_item_data (ld, itm, json);
  momval_t jarr = mom_jsonob_get (json, (momval_t) mom_item__vector);
  unsigned cnt = mom_json_array_size (jarr);
  if (!cnt)
    return;
  reserve_vectoritem (vecitm, cnt + cnt / 16 + 1);
  for (unsigned vix = 0; vix < cnt; vix++)
    {
      momval_t jcurelem = mom_json_array_nth (jarr, vix);
      mom_item_vector_append1 ((momval_t) vecitm,
			       mom_load_value_json (ld, jcurelem));
    }
}

//////////////////////////////////////////////////// assoc items

// return the rank of given attribute in association item, or else negative
// start at the hinted index hintix
static int
assoc_get_rank (momit_assoc_t * itmassoc, mom_anyitem_t * itmattr, int hintix)
{
  unsigned siz = itmassoc->ita_size;
  momhash_t hat = itmattr->i_hash;
  if (!siz)
    return -1;
  if (hintix >= 0 && hintix < siz
      && itmassoc->ita_htab[hintix].aten_itm == itmattr)
    return hintix;
  unsigned istart = hat % siz;
  for (unsigned ix = istart; ix < siz; ix++)
    {
      mom_anyitem_t *curat = itmassoc->ita_htab[ix].aten_itm;
      if (curat == itmattr)
	return ix;
      if (!curat)
	return -1;
    }
  for (unsigned ix = 0; ix < istart; ix++)
    {
      mom_anyitem_t *curat = itmassoc->ita_htab[ix].aten_itm;
      if (curat == itmattr)
	return ix;
      if (!curat)
	return -1;
    }
  return -1;
}

void
assoc_put (momit_assoc_t * itmassoc, mom_anyitem_t * itmattr, momval_t val)
{
  unsigned siz = itmassoc->ita_size;
  momhash_t hat = itmattr->i_hash;
  unsigned istart = hat % siz;
  int pix = -1;
  for (unsigned ix = istart; ix < siz; ix++)
    {
      mom_anyitem_t *curat = itmassoc->ita_htab[ix].aten_itm;
      if (curat == itmattr)
	{
	  itmassoc->ita_htab[ix].aten_val = val;
	  return;
	}
      else if ((void *) curat == MONIMELT_EMPTY)
	{
	  if (pix < 0)
	    pix = (int) ix;
	  continue;
	}
      else if (!curat)
	{
	  if (pix < 0)
	    pix = (int) ix;
	  goto addatpix;
	}
    }
  for (unsigned ix = 0; ix < istart; ix++)
    {
      mom_anyitem_t *curat = itmassoc->ita_htab[ix].aten_itm;
      if (curat == itmattr)
	{
	  itmassoc->ita_htab[ix].aten_val = val;
	  return;
	}
      else if ((void *) curat == MONIMELT_EMPTY)
	{
	  if (pix < 0)
	    pix = (int) ix;
	  continue;
	}
      else if (!curat)
	{
	  if (pix < 0)
	    pix = (int) ix;
	  goto addatpix;
	}
    }
addatpix:
  if (pix >= 0)
    {
      itmassoc->ita_htab[pix].aten_itm = itmattr;
      itmassoc->ita_htab[pix].aten_val = val;
      itmassoc->ita_count++;
      return;
    }
  MONIMELT_FATAL ("corrupted assoc item");
}


momit_assoc_t *
mom_make_item_assoc (unsigned space)
{
  momit_assoc_t *itmasso =
    mom_allocate_item (momty_associtem, sizeof (momit_assoc_t), space);
  return itmasso;
}

momit_assoc_t *
mom_make_item_assoc_of_uuid (uuid_t uid, unsigned space)
{
  momit_assoc_t *itmasso =
    mom_allocate_item_with_uuid (momty_associtem, sizeof (momit_assoc_t),
				 space, uid);
  return itmasso;
}

unsigned
mom_item_assoc_count (const momval_t asso)
{
  unsigned cnt = 0;
  if (!asso.ptr || *asso.ptype != momty_associtem)
    return 0;
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  cnt = asso.passocitem->ita_count;
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
  return cnt;
}

momval_t
mom_item_assoc_get1 (momval_t asso, const momval_t attr)
{
  momval_t res = MONIMELT_NULLV;
  if (!asso.ptr || asso.panyitem->typnum != momty_associtem
      || !attr.ptr || attr.panyitem->typnum <= momty__itemlowtype)
    return MONIMELT_NULLV;
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  momit_assoc_t *assoc = asso.passocitem;
  int ix = assoc_get_rank (assoc, attr.panyitem, 0);
  if (ix >= 0)
    res = assoc->ita_htab[ix].aten_val;
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
  return res;
}

int
mom_item_assoc_get_several (momval_t asso, ...)
{
  int cnt = 0;
  if (!asso.ptr || asso.panyitem->typnum != momty_associtem)
    return 0;
  va_list args;
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  va_start (args, asso);
  mom_anyitem_t *itatt = NULL;
  while ((itatt = va_arg (args, mom_anyitem_t *)) != NULL)
    {
      momval_t *pval = va_arg (args, momval_t *);
      int ix = assoc_get_rank (asso.passocitem, itatt, 0);
      if (ix >= 0)
	{
	  cnt++;
	  if (pval)
	    *pval = asso.passocitem->ita_htab[ix].aten_val;
	}
    }
  va_end (args);
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
  return cnt;
}


void
mom_item_assoc_put1 (momval_t asso, const momval_t attr, const momval_t val)
{
  if (!asso.ptr || asso.panyitem->typnum != momty_associtem
      || !attr.ptr || attr.panyitem->typnum <= momty__itemlowtype)
    return;
  momit_assoc_t *assoc = asso.passocitem;
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  unsigned oldcount = assoc->ita_count;
  unsigned oldsize = assoc->ita_size;
  if (MONIMELT_UNLIKELY ((4 * oldcount + 2 >= 3 * oldsize && val.ptr != NULL)
			 || (oldsize > 32 && 4 * oldcount + 5 < oldsize)))
    {
      unsigned newsiz = ((3 * oldcount / 2 + oldcount / 16 + 5) | 7) + 1;
      struct mom_attrentry_st *oldarr = assoc->ita_htab;
      struct mom_attrentry_st *newarr =
	GC_MALLOC (newsiz * sizeof (struct mom_attrentry_st));
      if (MONIMELT_UNLIKELY (!newarr))
	MONIMELT_FATAL ("failed to grow assoc to size %d", (int) newsiz);
      memset (newarr, 0, newsiz * sizeof (struct mom_attrentry_st));
      assoc->ita_count = 0;
      assoc->ita_size = newsiz;
      assoc->ita_htab = newarr;
      for (unsigned oix = 0; oix < oldsize; oix++)
	{
	  mom_anyitem_t *curat = oldarr[oix].aten_itm;
	  if (!curat || (void *) curat == MONIMELT_EMPTY)
	    continue;
	  assoc_put (assoc, curat, oldarr[oix].aten_val);
	}
      assert (assoc->ita_count == oldcount);
    }
  if (val.ptr != NULL)
    assoc_put (assoc, attr.panyitem, val);
  else
    {
      int aix = assoc_get_rank (assoc, attr.panyitem, 0);
      if (aix >= 0)
	{
	  assoc->ita_htab[aix].aten_itm = (mom_anyitem_t *) MONIMELT_EMPTY;
	  assoc->ita_htab[aix].aten_val = (momval_t) MONIMELT_EMPTY;
	  assoc->ita_count--;
	}
    }
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
}

void
mom_item_assoc_put_several (momval_t asso, ...)
{
  int cnt = 0;
  if (!asso.ptr || asso.panyitem->typnum != momty_associtem)
    return;
  mom_anyitem_t *itatt = NULL;
  momit_assoc_t *assoc = asso.passocitem;
  va_list args;
  // first, count the arguments
  va_start (args, asso);
  while ((itatt = va_arg (args, mom_anyitem_t *)) != NULL)
    {
      (void) va_arg (args, momval_t);
      cnt++;
    };
  va_end (args);
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  unsigned oldcount = assoc->ita_count;
  unsigned oldsize = assoc->ita_size;
  unsigned newsize =
    ((4 * oldcount / 3 + oldcount / 16 + 3 * cnt / 2 + 3) | 7) + 1;
  if ((!cnt && newsize != oldsize) || (newsize > oldsize))
    {
      struct mom_attrentry_st *oldarr = assoc->ita_htab;
      struct mom_attrentry_st *newarr =
	GC_MALLOC (newsize * sizeof (struct mom_attrentry_st));
      if (MONIMELT_UNLIKELY (!newarr))
	MONIMELT_FATAL ("failed to resize assoc to size %d", (int) newsize);
      memset (newarr, 0, newsize * sizeof (struct mom_attrentry_st));
      assoc->ita_count = 0;
      assoc->ita_size = newsize;
      assoc->ita_htab = newarr;
      for (unsigned oix = 0; oix < oldsize; oix++)
	{
	  mom_anyitem_t *curat = oldarr[oix].aten_itm;
	  if (!curat || (void *) curat == MONIMELT_EMPTY)
	    continue;
	  assoc_put (assoc, curat, oldarr[oix].aten_val);
	}
      assert (assoc->ita_count == oldcount);
    }
  // second, put the arguments
  va_start (args, asso);
  while ((itatt = va_arg (args, mom_anyitem_t *)) != NULL)
    {
      momval_t curval = va_arg (args, momval_t);
      if (itatt->typnum < momty__itemlowtype)
	continue;
      if (curval.ptr)
	assoc_put (assoc, itatt, curval);
      else
	{
	  int aix = assoc_get_rank (assoc, itatt, 0);
	  if (aix >= 0)
	    {
	      assoc->ita_htab[aix].aten_itm =
		(mom_anyitem_t *) MONIMELT_EMPTY;
	      assoc->ita_htab[aix].aten_val = (momval_t) MONIMELT_EMPTY;
	      assoc->ita_count--;
	    }
	}
    };
  va_end (args);
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
}


mom_anyitem_t *
mom_item_assoc_first_attr (momval_t asso, int *phint)
{
  if (phint)
    *phint = 0;
  if (!asso.ptr || asso.panyitem->typnum != momty_associtem)
    return NULL;
  momit_assoc_t *assoc = asso.passocitem;
  mom_anyitem_t *first = NULL;
  int hintix = -1;
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  unsigned count = assoc->ita_count;
  unsigned size = assoc->ita_size;
  if (!count)
    goto end;
  for (unsigned ix = 0; ix < size; ix++)
    {
      mom_anyitem_t *curat = assoc->ita_htab[ix].aten_itm;
      if (!curat || (void *) curat == MONIMELT_EMPTY)
	continue;
      hintix = ix;
      first = curat;
      goto end;
    }
end:
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
  if (phint)
    *phint = hintix;
  return first;
}

mom_anyitem_t *
mom_item_assoc_next_attr (momval_t asso, mom_anyitem_t * attr, int *phint)
{
  int oldhint = phint ? (*phint) : -1;
  if (!asso.ptr || asso.panyitem->typnum != momty_associtem
      || !attr || attr->typnum <= momty__itemlowtype)
    return NULL;
  momit_assoc_t *assoc = asso.passocitem;
  mom_anyitem_t *next = NULL;
  int hintix = -1;
  pthread_mutex_lock (&asso.panyitem->i_mtx);
  unsigned count = assoc->ita_count;
  unsigned size = assoc->ita_size;
  if (!count)
    goto end;
  int rk = assoc_get_rank (assoc, attr, oldhint);
  if (rk >= 0 && rk < size)
    {
      for (unsigned ix = rk + 1; ix < size && hintix < 0; ix++)
	{
	  mom_anyitem_t *curitm = assoc->ita_htab[ix].aten_itm;
	  if (!curitm || (void *) curitm == MONIMELT_EMPTY)
	    continue;
	  hintix = ix;
	  next = curitm;
	  break;
	}
      for (unsigned ix = 0; ix < rk && hintix < 0; ix++)
	{
	  mom_anyitem_t *curitm = assoc->ita_htab[ix].aten_itm;
	  if (!curitm || (void *) curitm == MONIMELT_EMPTY)
	    continue;
	  hintix = ix;
	  next = curitm;
	  break;
	}
    }
  else
    goto end;
end:
  pthread_mutex_unlock (&asso.panyitem->i_mtx);
  if (phint)
    *phint = hintix;
  return next;
}



/// for assoc item type descriptor
static mom_anyitem_t *assoc_itemloader (struct mom_loader_st *ld,
					momval_t json, uuid_t uid,
					unsigned space);
static void assoc_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
			      momval_t json);
static void assoc_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm);
static momval_t assoc_itemgetbuild (struct mom_dumper_st *dmp,
				    mom_anyitem_t * itm);
static momval_t assoc_itemgetfill (struct mom_dumper_st *dmp,
				   mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_assoc = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "assoc",
  .ityp_loader = assoc_itemloader,
  .ityp_filler = assoc_itemfiller,
  .ityp_scan = assoc_itemscan,
  .ityp_getbuild = assoc_itemgetbuild,
  .ityp_getfill = assoc_itemgetfill,
};

static momval_t
assoc_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  return (momval_t) mom_make_json_object
    // build with type 
    (				// the type:
      MOMJSON_ENTRY, mom_item__jtype, mom_item__assoc_item,
      // that's all!
      MOMJSON_END);
}

static mom_anyitem_t *
assoc_itemloader (struct mom_loader_st *ld __attribute__ ((unused)),
		  momval_t json __attribute__ ((unused)),
		  uuid_t uid, unsigned space)
{
  return (mom_anyitem_t *) mom_make_item_assoc_of_uuid (uid, space);
}

static void
assoc_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
  momit_assoc_t *assitm = (momit_assoc_t *) itm;
  unsigned size = assitm->ita_size;
  if (assitm->ita_count == 0)
    return;
  for (unsigned ix = 0; ix < size; ix++)
    {
      mom_anyitem_t *curitm = assitm->ita_htab[ix].aten_itm;
      if (!curitm || (void *) curitm == MONIMELT_EMPTY)
	continue;
      momval_t curval = assitm->ita_htab[ix].aten_val;
      if (!curval.ptr || curval.ptr == MONIMELT_EMPTY)
	continue;
      mom_dump_scan_value (dmp, (momval_t) curitm);
      mom_dump_scan_value (dmp, curval);
    }
}

// we sort the attribute entries, to normalize the JSON
static int
cmp_attrentry (const void *p1, const void *p2)
{
  const struct mom_attrentry_st *e1 = p1;
  const struct mom_attrentry_st *e2 = p2;
  return mom_item_cmp (e1->aten_itm, e2->aten_itm);
}

static momval_t
assoc_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momit_assoc_t *assitm = (momit_assoc_t *) itm;
  unsigned count = assitm->ita_count;
  unsigned size = assitm->ita_size;
  struct mom_attrentry_st entiny[TINY_MAX] = { };
  struct mom_attrentry_st *entarr =
    (count <
     TINY_MAX) ? entiny : GC_MALLOC (count *
				     sizeof (struct mom_attrentry_st));
  if (MONIMELT_UNLIKELY (!entarr))
    MONIMELT_FATAL ("failed to allocate %d entries", (int) count);
  memset (entarr, 0, count * sizeof (struct mom_attrentry_st));
  unsigned nb = 0;
  for (unsigned ix = 0; ix < size; ix++)
    {
      mom_anyitem_t *curitm = assitm->ita_htab[ix].aten_itm;
      if (!curitm || (void *) curitm == MONIMELT_EMPTY)
	continue;
      assert (nb < count);
      assert (assitm->ita_htab[ix].aten_val.ptr != NULL
	      && (void *) assitm->ita_htab[ix].aten_val.ptr !=
	      MONIMELT_EMPTY);
      entarr[nb] = assitm->ita_htab[ix];
      nb++;
    }
  assert (nb == count);
  qsort (entarr, count, sizeof (struct mom_attrentry_st), cmp_attrentry);
  momval_t jarrtiny[TINY_MAX] = { };
  momval_t *jarr =
    (count < TINY_MAX) ? jarrtiny : GC_MALLOC (count * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!jarr))
    MONIMELT_FATAL ("failed to allocate %d json", (int) count);
  memset (jarr, 0, count * sizeof (momval_t));
  unsigned nbjent = 0;
  for (unsigned ix = 0; ix < count; ix++)
    {
      momval_t jcurat = mom_dump_emit_json (dmp, (momval_t)
					    entarr[ix].aten_itm);
      if (!jcurat.ptr)
	continue;
      momval_t jcurva = mom_dump_emit_json (dmp, (momval_t)
					    entarr[ix].aten_val);
      if (!jcurva.ptr)
	continue;
      jarr[nbjent] = (momval_t) mom_make_json_object (	// the attribute
						       MOMJSON_ENTRY,
						       mom_item__attr, jcurat,
						       // the value
						       MOMJSON_ENTRY,
						       mom_item__val, jcurva,
						       // that's all
						       MOMJSON_END);
      nbjent++;
    }
  momval_t jres = (momval_t) mom_make_json_object
    // attributes
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     // assocations
     MOMJSON_ENTRY, mom_item__associations,
     mom_make_json_array_count (nbjent, jarr),
     // content
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     // that's all
     MOMJSON_END);
  if (jarr != jarrtiny)
    GC_FREE (jarr), jarr = NULL;
  if (entarr != entiny)
    GC_FREE (entarr), entarr = NULL;
  return jres;
}

static void
assoc_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		  momval_t json)
{
  mom_load_any_item_data (ld, itm, json);
  assert (itm->typnum == momty_associtem);
  momit_assoc_t *assoc = (momit_assoc_t *) itm;
  momval_t jarr = mom_jsonob_get (json, (momval_t) mom_item__associations);
  unsigned cnt = mom_json_array_size (jarr);
  if (!cnt)
    return;
  {
    unsigned newsiz = ((5 * cnt / 4 + 5) | 3) + 1;
    struct mom_attrentry_st *entarr =
      GC_MALLOC (newsiz * sizeof (struct mom_attrentry_st));
    if (MONIMELT_UNLIKELY (!entarr))
      MONIMELT_FATAL ("failed to allocate %d entries", (int) newsiz);
    memset (entarr, 0, newsiz * sizeof (struct mom_attrentry_st));
    assoc->ita_htab = entarr;
    assoc->ita_size = newsiz;
    assoc->ita_count = 0;
  }
  for (unsigned eix = 0; eix < cnt; eix++)
    {
      momval_t jcurent = mom_json_array_nth (jarr, eix);
      momval_t curatt = mom_load_value_json (ld,
					     mom_jsonob_get (jcurent,
							     (momval_t)
							     mom_item__attr));
      momval_t curval = mom_load_value_json (ld,
					     mom_jsonob_get (jcurent,
							     (momval_t)
							     mom_item__val));
      if (curatt.ptr && curval.ptr)
	mom_item_assoc_put1 ((momval_t) assoc, curatt, curval);
    }
}




//////////////////////////////////////////////////////////////// queue items

momit_queue_t *
mom_make_item_queue (unsigned space)
{
  momit_queue_t *itmque =
    mom_allocate_item (momty_queueitem, sizeof (momit_queue_t), space);
  return itmque;
}

momit_queue_t *
mom_make_item_queue_of_uuid (uuid_t uid, unsigned space)
{
  momit_queue_t *itmque =
    mom_allocate_item_with_uuid (momty_queueitem, sizeof (momit_queue_t),
				 space, uid);
  return itmque;
}

unsigned
mom_item_queue_length (const momval_t que)
{
  unsigned len = 0;
  if (!que.ptr || *que.ptype != momty_queueitem)
    return 0;
  pthread_mutex_lock (&que.panyitem->i_mtx);
  len = que.pqueueitem->itq_len;
  pthread_mutex_unlock (&que.panyitem->i_mtx);
  return len;
}

static void
queue_push_back (struct momqueueitem_st *quitm, mom_anyitem_t * itm)
{
  struct mom_itqueue_st *qel = GC_MALLOC (sizeof (struct mom_itqueue_st));
  if (MONIMELT_UNLIKELY (!qel))
    MONIMELT_FATAL ("failed to allocate queue element");
  qel->iq_next = NULL;
  qel->iq_item = itm;
  if (MONIMELT_UNLIKELY (!quitm->itq_first))
    {
      quitm->itq_first = quitm->itq_last = qel;
      quitm->itq_len = 1;
    }
  else
    {
      struct mom_itqueue_st *oldlast = quitm->itq_last;
      oldlast->iq_next = qel;
      quitm->itq_last = qel;
      quitm->itq_len++;
    }
}

static void
queue_push_front (struct momqueueitem_st *quitm, mom_anyitem_t * itm)
{
  struct mom_itqueue_st *qel = GC_MALLOC (sizeof (struct mom_itqueue_st));
  if (MONIMELT_UNLIKELY (!qel))
    MONIMELT_FATAL ("failed to allocate queue element");
  qel->iq_next = NULL;
  qel->iq_item = itm;
  if (MONIMELT_UNLIKELY (!quitm->itq_first))
    {
      quitm->itq_first = quitm->itq_last = qel;
      quitm->itq_len = 1;
    }
  else
    {
      struct mom_itqueue_st *oldfirst = quitm->itq_first;
      qel->iq_next = oldfirst;
      quitm->itq_first = qel;
      quitm->itq_len++;
    }
}

void
mom_item_queue_push_back (momval_t quev, momval_t itmv)
{
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return;
  if (!itmv.ptr || *itmv.ptype <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  queue_push_back (quev.pqueueitem, itmv.panyitem);
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
}

void
mom_item_queue_push_front (momval_t quev, momval_t itmv)
{
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return;
  if (!itmv.ptr || *itmv.ptype <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  queue_push_front (quev.pqueueitem, itmv.panyitem);
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
}

void
mom_item_queue_push_many_back (momval_t quev, ...)
{
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return;
  mom_anyitem_t *curitm = NULL;
  va_list args;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  va_start (args, quev);
  while ((curitm = va_arg (args, mom_anyitem_t *)) != NULL)
    {
      if (curitm->typnum <= momty__itemlowtype)
	continue;
      queue_push_back (quev.pqueueitem, curitm);
    }
  va_end (args);
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
}


void
mom_item_queue_push_many_front (momval_t quev, ...)
{
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return;
  mom_anyitem_t *curitm = NULL;
  va_list args;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  va_start (args, quev);
  while ((curitm = va_arg (args, mom_anyitem_t *)) != NULL)
    {
      if (curitm->typnum <= momty__itemlowtype)
	continue;
      queue_push_front (quev.pqueueitem, curitm);
    }
  va_end (args);
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
}


void
mom_item_queue_push_counted_back (momval_t quev, unsigned count,
				  mom_anyitem_t * arr[])
{
  if (!quev.ptr || *quev.ptype != momty_queueitem || !count || !arr)
    return;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  for (unsigned ix = 0; ix < count; ix++)
    {
      mom_anyitem_t *curitm = arr[ix];
      if (!curitm || curitm->typnum <= momty__itemlowtype)
	continue;
      queue_push_back (quev.pqueueitem, curitm);
    }
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
}

void
mom_item_queue_push_counted_front (momval_t quev, unsigned count,
				   mom_anyitem_t * arr[])
{
  if (!quev.ptr || *quev.ptype != momty_queueitem || !count || !arr)
    return;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  for (unsigned ix = 0; ix < count; ix++)
    {
      mom_anyitem_t *curitm = arr[ix];
      if (!curitm || curitm->typnum <= momty__itemlowtype)
	continue;
      queue_push_front (quev.pqueueitem, curitm);
    }
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
}



mom_anyitem_t *
mom_item_queue_first (momval_t quev)
{
  mom_anyitem_t *res = NULL;
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return NULL;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  if (quev.pqueueitem->itq_first)
    res = quev.pqueueitem->itq_first->iq_item;
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
  return res;
}

mom_anyitem_t *
mom_item_queue_last (momval_t quev)
{
  mom_anyitem_t *res = NULL;
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return NULL;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  if (quev.pqueueitem->itq_last)
    res = quev.pqueueitem->itq_last->iq_item;
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
  return res;
}

mom_anyitem_t *
mom_item_queue_pop_front (momval_t quev)
{
  mom_anyitem_t *res = NULL;
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return NULL;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  if (quev.pqueueitem->itq_first)
    {
      res = quev.pqueueitem->itq_first->iq_item;
      if (quev.pqueueitem->itq_first == quev.pqueueitem->itq_last)
	{
	  quev.pqueueitem->itq_first = quev.pqueueitem->itq_last = NULL;
	  quev.pqueueitem->itq_len = 0;
	}
      else
	{
	  quev.pqueueitem->itq_first = quev.pqueueitem->itq_first->iq_next;
	  quev.pqueueitem->itq_len--;
	}
    }
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
  return res;
}

momval_t
mom_item_queue_tuple (momval_t quev)
{
  momval_t res = MONIMELT_NULLV;
  if (!quev.ptr || *quev.ptype != momty_queueitem)
    return MONIMELT_NULLV;
  const mom_anyitem_t *tinyarr[TINY_MAX] = { 0 };
  const mom_anyitem_t **arr = NULL;
  pthread_mutex_lock (&quev.panyitem->i_mtx);
  unsigned len = quev.pqueueitem->itq_len;
  if (len < TINY_MAX)
    arr = tinyarr;
  else
    arr = GC_MALLOC (len * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!arr))
    MONIMELT_FATAL ("failed to allocate array of %d values", (int) len);
  memset (arr, 0, len * sizeof (mom_anyitem_t *));
  unsigned nb = 0;
  for (struct mom_itqueue_st * iq = quev.pqueueitem->itq_first;
       iq != NULL && nb < len; iq = iq->iq_next)
    {
      const mom_anyitem_t *curitm = iq->iq_item;
      if (curitm != NULL && curitm->typnum > momty__itemlowtype)
	arr[nb++] = curitm;
    }
  pthread_mutex_unlock (&quev.panyitem->i_mtx);
  res = (momval_t) mom_make_item_tuple_from_array (nb, arr);
  if (arr != tinyarr)
    GC_FREE (arr);
  return res;
}


//////////////// queue type descriptor

static mom_anyitem_t *queue_itemloader (struct mom_loader_st *ld,
					momval_t json, uuid_t uid,
					unsigned space);
static void queue_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
			      momval_t json);
static void queue_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm);
static momval_t queue_itemgetbuild (struct mom_dumper_st *dmp,
				    mom_anyitem_t * itm);
static momval_t queue_itemgetfill (struct mom_dumper_st *dmp,
				   mom_anyitem_t * itm);

const struct momitemtypedescr_st momitype_queue = {
  .ityp_magic = ITEMTYPE_MAGIC,
  .ityp_name = "queue",
  .ityp_loader = queue_itemloader,
  .ityp_filler = queue_itemfiller,
  .ityp_scan = queue_itemscan,
  .ityp_getbuild = queue_itemgetbuild,
  .ityp_getfill = queue_itemgetfill,
};

static mom_anyitem_t *
queue_itemloader (struct mom_loader_st *ld __attribute__ ((unused)),
		  momval_t json __attribute__ ((unused)),
		  uuid_t uid, unsigned space)
{
  return (mom_anyitem_t *) mom_make_item_queue_of_uuid (uid, space);
}

static void
queue_itemscan (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  mom_scan_any_item_data (dmp, itm);
  momit_queue_t *queitm = (momit_queue_t *) itm;
  for (struct mom_itqueue_st * iq = queitm->itq_first; iq != NULL;
       iq = iq->iq_next)
    mom_dump_scan_value (dmp, (momval_t) iq->iq_item);
}

static momval_t
queue_itemgetfill (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momit_queue_t *queitm = (momit_queue_t *) itm;
  unsigned len = queitm->itq_len;
  momval_t tinyjarr[TINY_MAX] = { };
  momval_t *jarr = NULL;
  if (len < TINY_MAX)
    jarr = tinyjarr;
  else
    jarr = GC_MALLOC (len * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!jarr))
    MONIMELT_FATAL ("failed to allocate %d queued items", (int) len);
  unsigned nb = 0;
  for (struct mom_itqueue_st * iq = queitm->itq_first; iq != NULL && nb < len;
       iq = iq->iq_next)
    {
      momval_t jcuritm = mom_dump_emit_json (dmp, (momval_t) iq->iq_item);
      if (!jcuritm.ptr)
	continue;
      jarr[nb++] = jcuritm;
    }
  momval_t jres = (momval_t) mom_make_json_object
    // attributes
    (MOMJSON_ENTRY, mom_item__attributes,
     mom_attributes_emit_json (dmp, itm->i_attrs),
     // queue
     MOMJSON_ENTRY, mom_item__queue, mom_make_json_array_count (nb, jarr),
     // content
     MOMJSON_ENTRY, mom_item__content, mom_dump_emit_json (dmp,
							   itm->i_content),
     // end
     MOMJSON_END);
  if (jarr != tinyjarr)
    GC_FREE (jarr);
  return jres;
}

static void
queue_itemfiller (struct mom_loader_st *ld, mom_anyitem_t * itm,
		  momval_t json)
{
  mom_load_any_item_data (ld, itm, json);
  assert (itm->typnum == momty_queueitem);
  momit_queue_t *queitm = (momit_queue_t *) itm;
  momval_t jarr = mom_jsonob_get (json, (momval_t) mom_item__queue);
  unsigned cnt = mom_json_array_size (jarr);
  for (unsigned ix = 0; ix < cnt; ix++)
    {
      momval_t jcur = mom_json_array_nth (jarr, ix);
      momval_t curval = mom_load_value_json (ld, jcur);
      mom_item_queue_push_back ((momval_t) queitm, (momval_t) curval);
    }
}

static momval_t
queue_itemgetbuild (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{;
  assert (itm->typnum == momty_queueitem);
  return (momval_t) mom_make_json_object
    // build with type 
    (				// the type:
      MOMJSON_ENTRY, mom_item__jtype, mom_item__queue_item,
      // that's all!
      MOMJSON_END);
}

////////////////////////////////////////////////////////////////
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
  mom_typedescr_array[momty_jsonitem] = &momitype_json_name;
  mom_typedescr_array[momty_boolitem] = &momitype_bool;
  mom_typedescr_array[momty_taskletitem] = &momitype_tasklet;
  mom_typedescr_array[momty_routineitem] = &momitype_routine;
  mom_typedescr_array[momty_vectoritem] = &momitype_vector;
  mom_typedescr_array[momty_associtem] = &momitype_assoc;
  mom_typedescr_array[momty_queueitem] = &momitype_queue;
}
