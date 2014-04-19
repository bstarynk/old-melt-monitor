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

// below TINY_MAX we try to allocate on stack temporary vectors
#define TINY_MAX 8
enum dumpstate_en
{
  dus_none = 0,
  dus_scan,
  dus_emit
};

#define DUMPER_MAGIC 0x572bb695	/* dumper magic 1462482581 */
struct mom_itemqueue_st
{
  struct mom_itemqueue_st *iq_next;
  mom_anyitem_t *iq_item;
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
  dmp->dmp_magic = DUMPER_MAGIC;
  dmp->dmp_state = dus_scan;
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
      struct mom_itemqueue_st *qel =
	GC_MALLOC (sizeof (struct mom_itemqueue_st));
      if (MONIMELT_UNLIKELY (!qel))
	MONIMELT_FATAL ("cannot add queue element to dumper of %d items",
			dmp->dmp_count);
      qel->iq_next = NULL;
      qel->iq_item = itm;
      if (MONIMELT_UNLIKELY (dmp->dmp_qlast == NULL))
	{
	  dmp->dmp_qfirst = dmp->dmp_qlast = qel;
	}
      else
	{
	  dmp->dmp_qlast->iq_next = qel;
	  dmp->dmp_qlast = qel;
	}
      add_dumped_item (dmp, itm);
    }
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

    default:
      if (typ > momty__itemlowtype)
	mom_dump_add_item (dmp, val.panyitem);
      else
	MONIMELT_FATAL ("unimplemented dump for type %d", (int) typ);
    }
}

static momval_t raw_dump_emit_json (struct mom_dumper_st *dmp,
				    const momval_t val);
static const momjsonarray_t *
jsonarray_emit_itemseq (struct mom_dumper_st *dmp,
			const struct momseqitem_st *si)
{
  unsigned slen = si->slen;
  if (slen <= TINY_MAX)
    {
      momval_t tab[TINY_MAX] = { MONIMELT_NULLV };
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
  if (slen <= TINY_MAX)
    {
      momval_t tab[TINY_MAX] = { MONIMELT_NULLV };
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
	  char ustr[UUID_PARSED_LEN];
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
  if (MONIMELT_UNLIKELY (dmp->dmp_state != dus_emit))
    MONIMELT_FATAL ("invalid dump state #%d", (int) dmp->dmp_state);
  jsval = raw_dump_emit_json (dmp, val);
  return jsval;
}


mom_anyitem_t *
mom_load_item (struct mom_loader_st * ld, uuid_t uuid, const char *space)
{
  mom_anyitem_t *itm = NULL;
  itm = mom_item_of_uuid (uuid);
  if (itm)
    return itm;
  for (unsigned spanum = 1; spanum < MONIMELT_SPACE_MAX; spanum++)
    {
      struct momspacedescr_st *curspa = mom_spacedescr_array[spanum];
      if (!curspa)
	continue;
      if (MONIMELT_UNLIKELY (curspa->spa_magic != SPACE_MAGIC))
	MONIMELT_FATAL ("corrupted space #%d", (int) spanum);
      if (!strcmp (curspa->spa_name, space))
	{
	  char *buildstr = NULL;
	  char uuidstr[UUID_PARSED_LEN];
	  struct jsonparser_st jp = { 0 };
	  memset (uuidstr, 0, sizeof (uuidstr));
	  uuid_unparse (uuid, uuidstr);
	  if (curspa->spa_fetch_build)
	    buildstr = curspa->spa_fetch_build (spanum, uuidstr);
	  if (buildstr)
	    {
	      FILE *fm = fmemopen (buildstr, strlen (buildstr), "r");
	      if (MONIMELT_UNLIKELY (!fm))
		MONIMELT_FATAL ("fmemopen failed for build string %s",
				buildstr);
	      mom_initialize_json_parser (&jp, fm, NULL);
	      char *errmsg = NULL;
	      const char *typestr = NULL;
	      struct momitemtypedescr_st *typdescr = NULL;
	      momval_t jval = mom_parse_json (&jp, &errmsg);
	      if (MONIMELT_UNLIKELY (!jval.ptr && errmsg))
		MONIMELT_FATAL
		  ("parsing of build of uid %s in space %s json %s failed : %s",
		   uuidstr, space, buildstr, errmsg);
	      mom_close_json_parser (&jp);
	      typestr =
		mom_string_cstr ((momval_t)
				 mom_jsonob_get (jval,
						 (momval_t) mom_item__jtype));
	      if (MONIMELT_UNLIKELY (!typestr || !typestr[0]))
		MONIMELT_FATAL
		  ("build string %s of uid %s in space %s without jtype",
		   buildstr, uuidstr, space);
	      for (unsigned ix = momty__itemlowtype;
		   ix < momty__last && !typdescr; ix++)
		if (mom_typedescr_array[ix]
		    && !strcmp (mom_typedescr_array[ix]->ityp_name, typestr))
		  typdescr = mom_typedescr_array[ix];
	      if (MONIMELT_UNLIKELY (!typdescr))
		MONIMELT_FATAL
		  ("build string %s of uid %s in space %s with unknown type %s",
		   buildstr, uuidstr, space, typestr);
	      if (typdescr->ityp_loader)
		{
		  itm = typdescr->ityp_loader (ld, jval, uuid);
		  // queue the loaded item to be filled
		  if (itm)
		    {
		      itm->i_space = spanum;
		      struct mom_itemqueue_st *iq =
			GC_MALLOC (sizeof (struct mom_itemqueue_st));
		      if (MONIMELT_UNLIKELY (!iq))
			MONIMELT_FATAL ("failed to queue item in loader");
		      iq->iq_item = itm;
		      iq->iq_next = NULL;
		      if (MONIMELT_UNLIKELY (!ld->ldr_qlast))
			ld->ldr_qfirst = ld->ldr_qlast = iq;
		      else
			{
			  ld->ldr_qlast->iq_next = iq;
			  ld->ldr_qlast = iq;
			}
		    }
		}
	      return itm;
	    }
	}
    };
  return NULL;
}



void
mom_load_any_item_data (struct mom_loader_st *ld, mom_anyitem_t * itm,
			momval_t jsob)
{
  if (MONIMELT_UNLIKELY (!ld || ld->ldr_magic != LOADER_MAGIC))
    MONIMELT_FATAL ("invalid loader @%p to fill item data", ld);
  if (MONIMELT_UNLIKELY (!itm || itm->typnum <= momty__itemlowtype))
    MONIMELT_FATAL ("invalid item @%p to fill item data", itm);
  // fill the attributes
  momval_t jattrs = mom_jsonob_get (jsob, (momval_t) mom_item__attributes);
  unsigned nbat = 0;
  if (jattrs.ptr && *jattrs.ptype == momty_jsonarray
      && (nbat = mom_json_array_size (jattrs)) > 0)
    {
      unsigned sizat = 2 + 9 * nbat / 8;
      struct mom_itemattributes_st *attrs =
	GC_MALLOC (sizeof (struct mom_itemattributes_st) +
		   sizat * sizeof (struct mom_attrentry_st));
      if (MONIMELT_UNLIKELY (!attrs))
	MONIMELT_FATAL ("cannot allocate %d attributes", (int) sizat);
      memset (attrs, 0,
	      sizeof (struct mom_itemattributes_st) +
	      sizat * sizeof (struct mom_attrentry_st));
      attrs->size = sizat;
      attrs->nbattr = 0;
      itm->i_attrs = attrs;
      for (unsigned ix = 0; ix < nbat; ix++)
	{
	  momval_t jent = mom_json_array_nth (jattrs, ix);
	  if (MONIMELT_UNLIKELY
	      (!jent.ptr || *jent.ptype != momty_jsonobject))
	    continue;
	  momval_t jat = mom_jsonob_get (jent, (momval_t) mom_item__attr);
	  if (MONIMELT_UNLIKELY (!jat.ptr))
	    continue;
	  momval_t jval = mom_jsonob_get (jent, (momval_t) mom_item__val);
	  if (MONIMELT_UNLIKELY (!jval.ptr))
	    continue;
	  mom_anyitem_t *itat =
	    mom_value_as_item (mom_load_value_json (ld, jat));
	  if (MONIMELT_UNLIKELY (!itat))
	    continue;
	  momval_t aval = mom_load_value_json (ld, jval);
	  if (MONIMELT_UNLIKELY (!aval.ptr))
	    continue;
	  mom_item_put_attr (itm, itat, aval);
	}
    };
  // fill the contents
  momval_t jcontents = mom_jsonob_get (jsob, (momval_t) mom_item__content);
  if (jcontents.ptr)
    itm->i_content = mom_load_value_json (ld, jcontents);
}

momval_t
mom_load_value_json (struct mom_loader_st *ld, const momval_t jval)
{
  if (!jval.ptr)
    return MONIMELT_NULLV;
  unsigned jtype = *jval.ptype;
  switch (jtype)
    {
    case momty_int:
    case momty_float:
    case momty_string:
      return jval;
    case momty_jsonobject:
      {
	momval_t val = MONIMELT_NULLV;
	momval_t jtypv = mom_jsonob_get (jval, (momval_t) mom_item__jtype);
	if (jtypv.panyitem == (mom_anyitem_t *) mom_item__itemref)
	  {
	    const char *uidstr =
	      mom_string_cstr (mom_jsonob_get
			       (jval, (momval_t) mom_item__uuid));
	    const char *spastr =
	      mom_string_cstr (mom_jsonob_get
			       (jval, (momval_t) mom_item__space));
	    uuid_t uuid;
	    memset (uuid, 0, sizeof (uuid));
	    if (uidstr && !uuid_parse (uidstr, uuid))
	      {
		return (momval_t) mom_load_item (ld, uuid, spastr);
	      }
	    else
	      return MONIMELT_NULLV;
	  }
	else if (jtypv.panyitem == (mom_anyitem_t *) mom_item__closure)
	  {
	    momval_t jconnv =
	      mom_jsonob_get (jval, (momval_t) mom_item__conn);
	    if (!jconnv.ptr)
	      return MONIMELT_NULLV;
	    mom_anyitem_t *connitm =
	      mom_value_as_item (mom_load_value_json (ld, jconnv));
	    if (!connitm || connitm->typnum != momty_routineitem)
	      return MONIMELT_NULLV;
	    momval_t jsonsv =
	      mom_jsonob_get (jval, (momval_t) mom_item__sons);
	    unsigned nbsons = mom_json_array_size (jsonsv);
	    if (nbsons < TINY_MAX)
	      {
		momval_t sontab[TINY_MAX] = { MONIMELT_NULLV };
		for (unsigned ix = 0; ix < nbsons; ix++)
		  sontab[ix] =
		    mom_load_value_json (ld, mom_json_array_nth (jsonsv, ix));
		return (momval_t) mom_make_closure_from_array (connitm,
							       nbsons,
							       sontab);
	      }
	    else
	      {
		momval_t *sonarr = GC_MALLOC (sizeof (momval_t) * nbsons);
		if (MONIMELT_UNLIKELY (!sonarr))
		  MONIMELT_FATAL ("failed to load %d sons in closure",
				  (unsigned) nbsons);
		memset (sonarr, 0, sizeof (momval_t) * nbsons);
		for (unsigned ix = 0; ix < nbsons; ix++)
		  sonarr[ix] =
		    mom_load_value_json (ld, mom_json_array_nth (jsonsv, ix));
		val =
		  (momval_t) mom_make_closure_from_array (connitm, nbsons,
							  sonarr);
		GC_FREE (sonarr);
		return val;
	      }
	  }
	else if (jtypv.panyitem == (mom_anyitem_t *) mom_item__node)
	  {
	    momval_t jconnv =
	      mom_jsonob_get (jval, (momval_t) mom_item__conn);
	    if (!jconnv.ptr)
	      return MONIMELT_NULLV;
	    mom_anyitem_t *connitm =
	      mom_value_as_item (mom_load_value_json (ld, jconnv));
	    if (!connitm)
	      return MONIMELT_NULLV;
	    momval_t jsonsv =
	      mom_jsonob_get (jval, (momval_t) mom_item__sons);
	    unsigned nbsons = mom_json_array_size (jsonsv);
	    if (nbsons < TINY_MAX)
	      {
		momval_t sontab[TINY_MAX] = { MONIMELT_NULLV };
		for (unsigned ix = 0; ix < nbsons; ix++)
		  sontab[ix] =
		    mom_load_value_json (ld, mom_json_array_nth (jsonsv, ix));
		return (momval_t) mom_make_node_from_array (connitm, nbsons,
							    sontab);
	      }
	    else
	      {
		momval_t *sonarr = GC_MALLOC (sizeof (momval_t) * nbsons);
		if (MONIMELT_UNLIKELY (!sonarr))
		  MONIMELT_FATAL ("failed to load %d sons in node",
				  (unsigned) nbsons);
		memset (sonarr, 0, sizeof (momval_t) * nbsons);
		for (unsigned ix = 0; ix < nbsons; ix++)
		  sonarr[ix] =
		    mom_load_value_json (ld, mom_json_array_nth (jsonsv, ix));
		val =
		  (momval_t) mom_make_node_from_array (connitm, nbsons,
						       sonarr);
		GC_FREE (sonarr);
		return val;
	      }
	  }
	else if (jtypv.panyitem == (mom_anyitem_t *) mom_item__set)
	  {
	    momval_t jsetv = mom_jsonob_get (jval, (momval_t) mom_item__set);
	    unsigned nbsons = mom_json_array_size (jsetv);
	    if (nbsons < TINY_MAX)
	      {
		mom_anyitem_t *itemtab[TINY_MAX] = { NULL };
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemtab[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld, mom_json_array_nth (jsetv, ix)));
		return (momval_t) mom_make_item_set_from_array (nbsons,
								itemtab);
	      }
	    else
	      {
		mom_anyitem_t **itemarr =
		  GC_MALLOC (sizeof (mom_anyitem_t *) * nbsons);
		if (MONIMELT_UNLIKELY (!itemarr))
		  MONIMELT_FATAL ("failed to load %d elements in set",
				  (unsigned) nbsons);
		memset (itemarr, 0, sizeof (momval_t) * nbsons);
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemarr[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld, mom_json_array_nth (jsetv, ix)));
		val =
		  (momval_t) mom_make_item_set_from_array (nbsons, itemarr);
		GC_FREE (itemarr);
		return val;
	      }
	  }
	else if (jtypv.panyitem == (mom_anyitem_t *) mom_item__tuple)
	  {
	    momval_t jtuplev =
	      mom_jsonob_get (jval, (momval_t) mom_item__tuple);
	    unsigned nbsons = mom_json_array_size (jtuplev);
	    if (nbsons < TINY_MAX)
	      {
		mom_anyitem_t *itemtab[TINY_MAX] = { NULL };
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemtab[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld,
					mom_json_array_nth (jtuplev, ix)));
		return (momval_t) mom_make_item_tuple_from_array (nbsons,
								  itemtab);
	      }
	    else
	      {
		mom_anyitem_t **itemarr =
		  GC_MALLOC (sizeof (mom_anyitem_t *) * nbsons);
		if (MONIMELT_UNLIKELY (!itemarr))
		  MONIMELT_FATAL ("failed to load %d elements in tuple",
				  (unsigned) nbsons);
		memset (itemarr, 0, sizeof (momval_t) * nbsons);
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemarr[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld,
					mom_json_array_nth (jtuplev, ix)));
		val =
		  (momval_t) mom_make_item_tuple_from_array (nbsons, itemarr);
		GC_FREE (itemarr);
		return val;
	      }
	  }
	else if (jtypv.panyitem == (mom_anyitem_t *) mom_item__json_array)
	  {
	    return (momval_t) mom_jsonob_get (jval,
					      (momval_t)
					      mom_item__json_array);
	  }
	else if (jtypv.panyitem == (mom_anyitem_t *) mom_item__json_object)
	  {
	    return (momval_t) mom_jsonob_get (jval,
					      (momval_t)
					      mom_item__json_object);
	  }
	return val;
      }
    default:
      return jval;
    }
}

momval_t
mom_attributes_emit_json (struct mom_dumper_st * dmp,
			  struct mom_itemattributes_st * iat)
{
  if (!iat)
    return MONIMELT_NULLV;
  momusize_t nbat = iat->nbattr;
  if (nbat < TINY_MAX)
    {
      momval_t jatv[TINY_MAX];
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



void
mom_initial_load (const char *state)
{
  MONIMELT_FATAL ("unimplemented initial load of %s", state);
}
