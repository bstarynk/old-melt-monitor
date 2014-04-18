// file dump-load.c

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

enum dumpstate_en
{
  dus_none = 0,
  dus_scan,
  dus_emit
};

#define DUMPER_MAGIC 0x572bb695	/* dumper magic 1462482581 */
struct mom_dumperqueue_st
{
  struct mom_dumperqueue_st *dq_next;
  mom_anyitem_t *dq_item;
};

void
mom_dumper_initialize (struct mom_dumper_st *dmp)
{
  const unsigned siz = 512;
  memset (dmp, 0, sizeof (struct mom_dumper_st));
  mom_anyitem_t **arr = GC_MALLOC (siz * sizeof (mom_anyitem_t *));
  if (MONIMELT_UNLIKELY (!arr))
    MONIMELT_FATAL ("cannot allocate dump array for %d items", siz);
  memset (arr, 0, siz * sizeof (mom_anyitem_t *));
  dmp->dmp_array = arr;
  dmp->dmp_size = siz;
  dmp->dmp_count = 0;
  dmp->dmp_qfirst = dmp->dmp_qlast = NULL;
  pthread_mutex_init (&dmp->dmp_mtx, NULL);
  dmp->dmp_magic = DUMPER_MAGIC;
  pthread_mutex_lock (&dmp->dmp_mtx);
  dmp->dmp_state = dus_scan;
  pthread_mutex_unlock (&dmp->dmp_mtx);
}

static inline void
add_dumped_item (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momhash_t h = itm->i_hash;
  unsigned size = dmp->dmp_size;
  unsigned istart = h % size;
  for (unsigned ix = istart; ix < size; ix++)
    {
      if (!dmp->dmp_array[ix])
	{
	  dmp->dmp_array[ix] = itm;
	  dmp->dmp_count++;
	  return;
	}
    }
  for (unsigned ix = 0; ix < istart; ix++)
    {
      if (!dmp->dmp_array[ix])
	{
	  dmp->dmp_array[ix] = itm;
	  dmp->dmp_count++;
	  return;
	}
    }
  // unreached
  MONIMELT_FATAL ("corrupted dump of %d items", dmp->dmp_count);
}


static inline bool
found_dumped_item (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  momhash_t h = itm->i_hash;
  unsigned size = dmp->dmp_size;
  unsigned istart = h % size;
  for (unsigned ix = istart; ix < size; ix++)
    {
      if (dmp->dmp_array[ix] == itm)
	return true;
    }
  for (unsigned ix = 0; ix < istart; ix++)
    {
      if (dmp->dmp_array[ix] == itm)
	return true;
    }
  return false;
}

void
mom_dump_add_item (struct mom_dumper_st *dmp, mom_anyitem_t * itm)
{
  if (!itm || itm->typnum < momty__itemlowtype)
    return;
  if (!dmp || dmp->dmp_magic != DUMPER_MAGIC)
    return;
  pthread_mutex_lock (&dmp->dmp_mtx);
  if (MONIMELT_UNLIKELY (dmp->dmp_state != dus_scan))
    MONIMELT_FATAL ("invalid dump state #%d", (int) dmp->dmp_state);
  if (MONIMELT_UNLIKELY (4 * dmp->dmp_count / 3 + 10 >= dmp->dmp_size))
    {
      unsigned oldsize = dmp->dmp_size;
      unsigned oldcount = dmp->dmp_count;
      mom_anyitem_t **oldarr = dmp->dmp_array;
      unsigned newsize = ((4 * oldcount / 3 + oldcount / 4 + 60) | 0x7f) + 1;
      mom_anyitem_t **newarr = GC_MALLOC (newsize * sizeof (mom_anyitem_t *));
      if (MONIMELT_UNLIKELY (!newarr))
	MONIMELT_FATAL ("cannot grow dumper to %d items", newsize);
      memset (newarr, 0, newsize * sizeof (mom_anyitem_t *));
      dmp->dmp_array = newarr;
      dmp->dmp_size = newsize;
      dmp->dmp_count = 0;
      for (unsigned ix = 0; ix < oldsize; ix++)
	{
	  mom_anyitem_t *curitm = oldarr[ix];
	  if (!curitm)
	    continue;
	  add_dumped_item (dmp, curitm);
	}
    }
  bool founditem = found_dumped_item (dmp, itm);
  // enqueue and add the item if it is not found
  if (!founditem)
    {
      struct mom_dumperqueue_st *qel =
	GC_MALLOC (sizeof (struct mom_dumperqueue_st));
      if (MONIMELT_UNLIKELY (!qel))
	MONIMELT_FATAL ("cannot add queue element to dumper of %d items",
			dmp->dmp_count);
      qel->dq_next = NULL;
      qel->dq_item = itm;
      if (MONIMELT_UNLIKELY (dmp->dmp_qlast == NULL))
	{
	  dmp->dmp_qfirst = dmp->dmp_qlast = qel;
	}
      else
	{
	  dmp->dmp_qlast->dq_next = qel;
	  dmp->dmp_qlast = qel;
	}
      add_dumped_item (dmp, itm);
    }
  pthread_mutex_unlock (&dmp->dmp_mtx);
}

void
mom_dump_scan_value (struct mom_dumper_st *dmp, const momval_t val)
{
  if (!dmp || !val.ptr)
    return;
  if (MONIMELT_UNLIKELY (dmp->dmp_magic != DUMPER_MAGIC))
    MONIMELT_FATAL ("bad dumper@%p when dumping value @%p", (void *) dmp,
		    val.ptr);
  if (MONIMELT_UNLIKELY (dmp->dmp_state != dus_scan))
    MONIMELT_FATAL ("invalid dump state #%d", (int) dmp->dmp_state);
  unsigned typ = *val.ptype;
  switch (typ)
    {
    case momty_int:
    case momty_float:
    case momty_string:
      return;
    case momty_jsonarray:
      {
	unsigned siz = val.pjsonarr->slen;
	for (unsigned ix = 0; ix < siz; ix++)
	  mom_dump_scan_value (dmp, val.pjsonarr->jarrtab[ix]);
	return;
      }
    case momty_jsonobject:
      {
	unsigned siz = val.pjsonobj->slen;
	for (unsigned ix = 0; ix < siz; ix++)
	  {
	    mom_dump_scan_value (dmp, val.pjsonobj->jobjtab[ix].je_name);
	    mom_dump_scan_value (dmp, val.pjsonobj->jobjtab[ix].je_attr);
	  }
	return;
      }
    case momty_node:
    case momty_closure:
      {
	unsigned siz = val.pnode->slen;
	mom_dump_add_item (dmp, val.pnode->connitm);
	for (unsigned ix = 0; ix < siz; ix++)
	  mom_dump_scan_value (dmp, val.pnode->sontab[ix]);
	return;
      }
    case momty_itemtuple:
    case momty_itemset:
      {
	unsigned siz = val.pseqitm->slen;
	for (unsigned ix = 0; ix < siz; ix++)
	  {
	    mom_anyitem_t *curitm = val.pseqitm->itemseq[ix];
	    if (!curitm)
	      continue;
	    mom_dump_add_item (dmp, curitm);
	  }
	return;
      }

#warning missing cases for in mom_dump_scan_value
    default:
      if (typ > momty__itemlowtype)
	mom_dump_add_item (dmp, val.panyitem);
      return;
    }
}

static momval_t raw_dump_emit_json (struct mom_dumper_st *dmp,
				    const momval_t val);
static const momjsonarray_t *
jsonarray_emit_itemseq (struct mom_dumper_st *dmp,
			const struct momseqitem_st *si)
{
  unsigned slen = si->slen;
  if (slen <= 8)
    {
      momval_t tab[8] = { MONIMELT_NULLV };
      for (unsigned ix = 0; ix < slen; ix++)
	tab[ix] = raw_dump_emit_json (dmp, (momval_t) (si->itemseq[ix]));
      return mom_make_json_array_count (slen, tab);
    }
  else
    {
      momval_t *arr = GC_MALLOC (sizeof (momval_t) * slen);
      if (MONIMELT_UNLIKELY (!arr))
	MONIMELT_FATAL ("failed to allocate array of %d", (int) slen);
      memset (arr, 0, sizeof (momval_t) * slen);
      for (unsigned ix = 0; ix < slen; ix++)
	arr[ix] = raw_dump_emit_json (dmp, (momval_t) (si->itemseq[ix]));
      const momjsonarray_t *jarr = mom_make_json_array_count (slen, arr);
      GC_FREE (arr);
      return jarr;
    }
}

static const momjsonarray_t *
jsonarray_emit_nodesons (struct mom_dumper_st *dmp,
			 const struct momnode_st *nd)
{
  unsigned slen = nd->slen;
  if (slen <= 8)
    {
      momval_t tab[8] = { MONIMELT_NULLV };
      for (unsigned ix = 0; ix < slen; ix++)
	tab[ix] = raw_dump_emit_json (dmp, (nd->sontab[ix]));
      return mom_make_json_array_count (slen, tab);
    }
  else
    {
      momval_t *arr = GC_MALLOC (sizeof (momval_t) * slen);
      if (MONIMELT_UNLIKELY (!arr))
	MONIMELT_FATAL ("failed to allocate son array of %d", (int) slen);
      memset (arr, 0, sizeof (momval_t) * slen);
      for (unsigned ix = 0; ix < slen; ix++)
	arr[ix] = raw_dump_emit_json (dmp, (nd->sontab[ix]));
      const momjsonarray_t *jarr = mom_make_json_array_count (slen, arr);
      GC_FREE (arr);
      return jarr;
    }
}

momval_t
raw_dump_emit_json (struct mom_dumper_st * dmp, const momval_t val)
{
  momval_t jsval = MONIMELT_NULLV;
  if (!val.ptr)
    goto end;
  unsigned typ = *val.ptype;
  switch (typ)
    {
    case momty_int:
    case momty_float:
    case momty_string:
      jsval = val;
      break;
    case momty_jsonarray:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__json_array,
	   MOMJSON_ENTRY, mom_item__json_array, val, MOMJSON_END);
      }
      break;
    case momty_jsonobject:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__json_object,
	   MOMJSON_ENTRY, mom_item__json_object, val, MOMJSON_END);
      }
      break;
    case momty_itemset:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__set,
	   MOMJSON_ENTRY, mom_item__set, jsonarray_emit_itemseq (dmp,
								 val.pitemset),
	   MOMJSON_END);
      }
      break;
    case momty_itemtuple:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__tuple,
	   MOMJSON_ENTRY, mom_item__tuple, jsonarray_emit_itemseq (dmp,
								   val.pitemtuple),
	   MOMJSON_END);
      }
      break;
    case momty_node:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__node,
	   MOMJSON_ENTRY, mom_item__conn, raw_dump_emit_json (dmp,
							      (momval_t)
							      (val.
							       pnode->connitm)),
	   MOMJSON_ENTRY, mom_item__sons, jsonarray_emit_nodesons (dmp,
								   val.pnode),
	   MOMJSON_END);
      }
      break;
    case momty_closure:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__closure,
	   MOMJSON_ENTRY, mom_item__conn, raw_dump_emit_json (dmp,
							      (momval_t)
							      (val.
							       pnode->connitm)),
	   MOMJSON_ENTRY, mom_item__sons, jsonarray_emit_nodesons (dmp,
								   val.pnode),
	   MOMJSON_END);
      }
      break;
    default:
      if (typ > momty__itemlowtype && val.panyitem->i_space > 0
	  && val.panyitem->i_space < MONIMELT_SPACE_MAX
	  && mom_spacedescr_array[val.panyitem->i_space])
	{
	  unsigned spacenum = val.panyitem->i_space;
	  struct momspacedescr_st *spadecr = mom_spacedescr_array[spacenum];
	  char ustr[40];
	  memset (ustr, 0, sizeof (ustr));
	  uuid_unparse (val.panyitem->i_uuid, ustr);
	  if (MONIMELT_UNLIKELY (!found_dumped_item (dmp, val.panyitem)))
	    MONIMELT_FATAL ("unknown dumped item @%p uuid %s", val.panyitem,
			    ustr);
	  if (MONIMELT_UNLIKELY (spadecr->spa_magic != SPACE_MAGIC))
	    MONIMELT_FATAL ("dumped item @%p uuid %s has bad space #%d",
			    val.ptr, ustr, spacenum);
	  if (MONIMELT_UNLIKELY (mom_spacename_array[spacenum] == NULL))
	    mom_spacename_array[spacenum] =
	      (momstring_t *) mom_make_string (spadecr->spa_name);
	  jsval =
	    (momval_t) mom_make_json_object (MOMJSON_ENTRY, mom_item__jtype,
					     mom_item__itemref, MOMJSON_ENTRY,
					     mom_item__uuid,
					     mom_make_string (ustr),
					     MOMJSON_ENTRY, mom_item__space,
					     mom_spacename_array[spacenum],
					     MOMJSON_END);
	}
      break;
    }
end:
  return jsval;
}

momval_t
mom_dump_emit_json (struct mom_dumper_st * dmp, const momval_t val)
{
  momval_t jsval = MONIMELT_NULLV;
  if (MONIMELT_UNLIKELY (!dmp || dmp->dmp_magic != DUMPER_MAGIC))
    MONIMELT_FATAL ("bad dumper@%p when dumping value @%p", (void *) dmp,
		    val.ptr);
  pthread_mutex_lock (&dmp->dmp_mtx);
  if (MONIMELT_UNLIKELY (dmp->dmp_state != dus_emit))
    MONIMELT_FATAL ("invalid dump state #%d", (int) dmp->dmp_state);
  jsval = raw_dump_emit_json (dmp, val);
  pthread_mutex_unlock (&dmp->dmp_mtx);
  return jsval;
}

momval_t
mom_attributes_emit_json (struct mom_dumper_st * dmp,
			  struct mom_itemattributes_st * iat)
{
  if (!iat)
    return MONIMELT_NULLV;
  momusize_t nbat = iat->nbattr;
  if (nbat < 8)
    {
      momval_t jatv[8];
      for (unsigned ix = 0; ix < nbat; ix++)
	jatv[ix] =
	  (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__attr,
	   raw_dump_emit_json (dmp, (momval_t) iat->itattrtab[ix].aten_itm),
	   MOMJSON_ENTRY, mom_item__val, raw_dump_emit_json (dmp,
							     iat->itattrtab
							     [ix].aten_val),
	   MOMJSON_END);
      return (momval_t) mom_make_json_array_count (nbat, jatv);
    }
  else
    {
      momval_t *jatarr = GC_MALLOC (nbat * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!jatarr))
	MONIMELT_FATAL ("failed to allocate json array for %d attributes",
			(int) nbat);
      memset (jatarr, 0, nbat * sizeof (momval_t));
      for (unsigned ix = 0; ix < nbat; ix++)
	jatarr[ix] =
	  (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__attr,
	   raw_dump_emit_json (dmp, (momval_t) iat->itattrtab[ix].aten_itm),
	   MOMJSON_ENTRY, mom_item__val, raw_dump_emit_json (dmp,
							     iat->itattrtab
							     [ix].aten_val),
	   MOMJSON_END);
      momval_t res = (momval_t) mom_make_json_array_count (nbat, jatarr);
      GC_FREE (jatarr);
      return res;
    }
}
