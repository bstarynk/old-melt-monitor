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

#define MONIMELT_VERSION_PARAM "dump_format_version"
#define MONIMELT_DUMP_VERSION "MoniMelt2014A"

enum dumpstate_en
{
  dus_none = 0,
  dus_scan,
  dus_emit
};

#define DUMPER_MAGIC 0x572bb695	/* dumper magic 1462482581 */

void
mom_dumper_initialize (struct mom_dumper_st *dmp)
{
  const unsigned siz = 512;
  memset (dmp, 0, sizeof (struct mom_dumper_st));
  const mom_anyitem_t **arr = GC_MALLOC (siz * sizeof (mom_anyitem_t *));
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
add_dumped_item (struct mom_dumper_st *dmp, const mom_anyitem_t * itm)
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
found_dumped_item (struct mom_dumper_st *dmp, const mom_anyitem_t * itm)
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
mom_dump_add_item (struct mom_dumper_st *dmp, const mom_anyitem_t * itm)
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
      const mom_anyitem_t **oldarr = dmp->dmp_array;
      unsigned newsize = ((4 * oldcount / 3 + oldcount / 4 + 60) | 0x7f) + 1;
      const mom_anyitem_t **newarr =
	GC_MALLOC (newsize * sizeof (mom_anyitem_t *));
      if (MONIMELT_UNLIKELY (!newarr))
	MONIMELT_FATAL ("cannot grow dumper to %d items", newsize);
      memset (newarr, 0, newsize * sizeof (mom_anyitem_t *));
      dmp->dmp_array = newarr;
      dmp->dmp_size = newsize;
      dmp->dmp_count = 0;
      for (unsigned ix = 0; ix < oldsize; ix++)
	{
	  const mom_anyitem_t *curitm = oldarr[ix];
	  if (!curitm)
	    continue;
	  add_dumped_item (dmp, curitm);
	}
    }
  bool founditem = found_dumped_item (dmp, itm);
  // enqueue and add the item if it is not found
  if (!founditem)
    {
      struct mom_itqueue_st *qel = GC_MALLOC (sizeof (struct mom_itqueue_st));
      if (MONIMELT_UNLIKELY (!qel))
	MONIMELT_FATAL ("cannot add queue element to dumper of %d items",
			dmp->dmp_count);
      qel->iq_next = NULL;
      qel->iq_item = (mom_anyitem_t *) itm;
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
	const mom_anyitem_t *curconn = val.pnode->connitm;
	if (curconn && curconn->i_space > 0)
	  {
	    unsigned siz = val.pnode->slen;
	    mom_dump_add_item (dmp, curconn);
	    for (unsigned ix = 0; ix < siz; ix++)
	      mom_dump_scan_value (dmp, val.pnode->sontab[ix]);
	  }
	return;
      }
    case momty_tuple:
    case momty_set:
      {
	unsigned siz = val.pseqitm->slen;
	for (unsigned ix = 0; ix < siz; ix++)
	  {
	    const mom_anyitem_t *curitm = val.pseqitm->itemseq[ix];
	    if (!curitm || curitm->i_space == 0)
	      continue;
	    mom_dump_add_item (dmp, curitm);
	  }
	return;
      }

    default:
      if (typ > momty__itemlowtype && val.panyitem->i_space > 0)
	mom_dump_add_item (dmp, val.panyitem);
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
	{
	  mom_anyitem_t *curitm = (mom_anyitem_t *) (si->itemseq[ix]);
	  if (curitm && curitm->i_space)
	    tab[ix] = raw_dump_emit_json (dmp, (momval_t) curitm);
	  else
	    tab[ix] = MONIMELT_NULLV;
	}
      return mom_make_json_array_count (slen, tab);
    }
  else
    {
      momval_t *arr = GC_MALLOC (sizeof (momval_t) * slen);
      if (MONIMELT_UNLIKELY (!arr))
	MONIMELT_FATAL ("failed to allocate array of %d", (int) slen);
      memset (arr, 0, sizeof (momval_t) * slen);
      for (unsigned ix = 0; ix < slen; ix++)
	{
	  mom_anyitem_t *curitm = (mom_anyitem_t *) (si->itemseq[ix]);
	  if (curitm && curitm->i_space)
	    arr[ix] = raw_dump_emit_json (dmp, (momval_t) curitm);
	  else
	    arr[ix].ptr = NULL;
	}
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
    case momty_set:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__set,
	   MOMJSON_ENTRY, mom_item__set, jsonarray_emit_itemseq (dmp,
								 val.pset),
	   MOMJSON_END);
      }
      break;
    case momty_tuple:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSON_ENTRY, mom_item__jtype, mom_item__tuple,
	   MOMJSON_ENTRY, mom_item__tuple, jsonarray_emit_itemseq (dmp,
								   val.ptuple),
	   MOMJSON_END);
      }
      break;
    case momty_node:
      {
	const mom_anyitem_t *curconn = val.pnode->connitm;
	if (curconn && curconn->i_space > 0)
	  jsval = (momval_t) mom_make_json_object
	    (MOMJSON_ENTRY, mom_item__jtype, mom_item__node,
	     MOMJSON_ENTRY, mom_item__conn, raw_dump_emit_json (dmp,
								(momval_t)
								(mom_anyitem_t
								 *) curconn),
	     MOMJSON_ENTRY, mom_item__sons, jsonarray_emit_nodesons (dmp,
								     val.pnode),
	     MOMJSON_END);
      }
      break;
    case momty_closure:
      {
	const mom_anyitem_t *curconn = val.pnode->connitm;
	if (curconn && curconn->i_space > 0)
	  jsval = (momval_t) mom_make_json_object
	    (MOMJSON_ENTRY, mom_item__jtype, mom_item__closure,
	     MOMJSON_ENTRY, mom_item__conn,
	     raw_dump_emit_json (dmp,
				 (momval_t) (mom_anyitem_t *) curconn),
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
	  if (!spacenum)
	    {
	      jsval.ptr = NULL;
	      goto end;
	    };
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
  unsigned spanum = 0;
  assert (uuid != NULL && !uuid_is_null (uuid));
  assert (ld != NULL && ld->ldr_magic == LOADER_MAGIC);
  itm = mom_item_of_uuid (uuid);
  if (itm)
    return itm;
  for (spanum = 0; spanum < MONIMELT_SPACE_MAX; spanum++)
    {
      struct momspacedescr_st *curspa = mom_spacedescr_array[spanum];
      if (!curspa)
	continue;
      assert (spanum > 0);
      if (MONIMELT_UNLIKELY (curspa->spa_magic != SPACE_MAGIC))
	MONIMELT_FATAL ("corrupted space #%d", (int) spanum);
      assert (curspa->spa_name != NULL);
      if (!strcmp (curspa->spa_name, space))
	{
	  char *buildstr = NULL;
	  char uuidstr[UUID_PARSED_LEN];
	  struct jsonparser_st jp = { 0 };
	  memset (uuidstr, 0, sizeof (uuidstr));
	  uuid_unparse (uuid, uuidstr);
	  assert (uuid[0] != (char) 0);
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
	      const struct momitemtypedescr_st *typdescr = NULL;
	      momval_t jval = mom_parse_json (&jp, &errmsg);
	      if (MONIMELT_UNLIKELY (!jval.ptr && errmsg))
		MONIMELT_FATAL
		  ("parsing of build of uid %s in space %s json %s failed : %s",
		   uuidstr, space, buildstr, errmsg);
	      mom_close_json_parser (&jp);
	      momval_t jcurtype = (momval_t) mom_jsonob_get (jval,
							     (momval_t)
							     mom_item__jtype);
	      if (MONIMELT_UNLIKELY (!jcurtype.ptr))
		MONIMELT_FATAL
		  ("build string %s of uid %s in space %s without jtype",
		   buildstr, uuidstr, space);
	      typestr = mom_jsonstring_cstr (jcurtype);
	      if (MONIMELT_UNLIKELY (!typestr || !typestr[0]))
		MONIMELT_FATAL
		  ("build string %s of uid %s in space %s with bad jtype",
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
		  itm = typdescr->ityp_loader (ld, jval, uuid, spanum);
		  // queue the loaded item to be filled
		  if (itm)
		    {
		      itm->i_space = spanum;
		      struct mom_itqueue_st *iq =
			GC_MALLOC (sizeof (struct mom_itqueue_st));
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
	      mom_jsonstring_cstr (mom_jsonob_get
				   (jval, (momval_t) mom_item__uuid));
	    const char *spastr =
	      mom_jsonstring_cstr (mom_jsonob_get
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
		const mom_anyitem_t *itemtab[TINY_MAX] = { NULL };
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemtab[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld, mom_json_array_nth (jsetv, ix)));
		return (momval_t) mom_make_set_from_array (nbsons, itemtab);
	      }
	    else
	      {
		const mom_anyitem_t **itemarr =
		  GC_MALLOC (sizeof (mom_anyitem_t *) * nbsons);
		if (MONIMELT_UNLIKELY (!itemarr))
		  MONIMELT_FATAL ("failed to load %d elements in set",
				  (unsigned) nbsons);
		memset (itemarr, 0, sizeof (momval_t) * nbsons);
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemarr[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld, mom_json_array_nth (jsetv, ix)));
		val = (momval_t) mom_make_set_from_array (nbsons, itemarr);
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
		const mom_anyitem_t *itemtab[TINY_MAX] = { NULL };
		for (unsigned ix = 0; ix < nbsons; ix++)
		  itemtab[ix] =
		    mom_value_as_item (mom_load_value_json
				       (ld,
					mom_json_array_nth (jtuplev, ix)));
		return (momval_t) mom_make_tuple_from_array (nbsons, itemtab);
	      }
	    else
	      {
		const mom_anyitem_t **itemarr =
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
		val = (momval_t) mom_make_tuple_from_array (nbsons, itemarr);
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
  unsigned cntat = 0;
  if (!iat)
    return MONIMELT_NULLV;
  momusize_t nbat = iat->nbattr;
  if (nbat < TINY_MAX)
    {
      momval_t jatv[TINY_MAX] = { };
      for (unsigned ix = 0; ix < nbat; ix++)
	{
	  mom_anyitem_t *curatitm = iat->itattrtab[ix].aten_itm;
	  if (!curatitm || !curatitm->i_space)
	    continue;
	  momval_t jcurat = raw_dump_emit_json (dmp, (momval_t) curatitm);
	  momval_t jcurva =
	    raw_dump_emit_json (dmp, iat->itattrtab[ix].aten_val);
	  if (!jcurat.ptr || !jcurva.ptr)
	    continue;
	  jatv[cntat] =
	    (momval_t) mom_make_json_object
	    (MOMJSON_ENTRY, mom_item__attr, jcurat,
	     MOMJSON_ENTRY, mom_item__val, jcurva, MOMJSON_END);
	  cntat++;
	}
      return (momval_t) mom_make_json_array_count (cntat, jatv);
    }
  else
    {
      momval_t *jatarr = GC_MALLOC (nbat * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!jatarr))
	MONIMELT_FATAL ("failed to allocate json array for %d attributes",
			(int) nbat);
      memset (jatarr, 0, nbat * sizeof (momval_t));
      for (unsigned ix = 0; ix < nbat; ix++)
	{
	  momval_t jcurat =
	    raw_dump_emit_json (dmp, (momval_t) iat->itattrtab[ix].aten_itm);
	  momval_t jcurva =
	    raw_dump_emit_json (dmp, iat->itattrtab[ix].aten_val);
	  if (!jcurat.ptr || !jcurva.ptr)
	    continue;
	  jatarr[cntat] =
	    (momval_t) mom_make_json_object
	    (MOMJSON_ENTRY, mom_item__attr, jcurat,
	     MOMJSON_ENTRY, mom_item__val, jcurva, MOMJSON_END);
	  cntat++;
	}
      momval_t res = (momval_t) mom_make_json_array_count (cntat, jatarr);
      GC_FREE (jatarr);
      return res;
    }
}


static int
setintptr_cb (void *data, int nbcol, char **colarrs, char **colnames)
{
  if (nbcol == 1 && data)
    {
      intptr_t *n = data;
      if (n)
	*n = atol (colarrs[0]);
    }
  return 0;
}


static sqlite3 *mom_dbsqlite = NULL;

static sqlite3_stmt *fetchbuild_loadstmt;
// fetch a GC_STRDUP-ed string to build an item of given uuid string
static char *
rootspace_fetch_build (unsigned spanum, const char *uuidstr)
{
  char *res = NULL;
  assert (spanum == MONIMELT_SPACE_ROOT);
  assert (mom_dbsqlite != NULL);
  if (MONIMELT_UNLIKELY (fetchbuild_loadstmt == NULL))
    {
      if (sqlite3_prepare_v2 (mom_dbsqlite,
			      "SELECT jbuild FROM t_item WHERE uid = ?1",
			      -1, &fetchbuild_loadstmt, NULL))
	MONIMELT_FATAL ("failed to prepare fetchbuild query: %s",
			sqlite3_errmsg (mom_dbsqlite));
    };
  // uuidstr at index 1
  if (sqlite3_bind_text (fetchbuild_loadstmt, 1, uuidstr, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind uuidstr: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  int stepres = sqlite3_step (fetchbuild_loadstmt);
  if (stepres == SQLITE_ROW)
    {
      const char *colbuild = (const char *)
	sqlite3_column_text (fetchbuild_loadstmt, 0);
      assert (colbuild != NULL);
      res = GC_STRDUP (colbuild);
      if (MONIMELT_UNLIKELY (!res))
	MONIMELT_FATAL ("failed to duplicate build of %s which is %s",
			uuidstr, colbuild);
      stepres = sqlite3_step (fetchbuild_loadstmt);
    }
  sqlite3_reset (fetchbuild_loadstmt);
  if (stepres == SQLITE_DONE)
    return res;
  else
    MONIMELT_FATAL ("failed to fetch build %s: %s (%d:%s)",
		    uuidstr, sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
}

static sqlite3_stmt *fetchfill_loadstmt;
// fetch a GC_STRDUP-ed string to fill an item of given uuid string
static char *
rootspace_fetch_fill (unsigned spanum, const char *uuidstr)
{
  char *res = NULL;
  assert (spanum == MONIMELT_SPACE_ROOT);
  assert (mom_dbsqlite != NULL);
  if (MONIMELT_UNLIKELY (fetchfill_loadstmt == NULL))
    {
      if (sqlite3_prepare_v2 (mom_dbsqlite,
			      "SELECT jfill FROM t_item WHERE uid = ?1",
			      -1, &fetchfill_loadstmt, NULL))
	MONIMELT_FATAL ("failed to prepare fetchfill query: %s",
			sqlite3_errmsg (mom_dbsqlite));
    };
  // uuidstr at index 1
  if (sqlite3_bind_text (fetchfill_loadstmt, 1, uuidstr, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind uuidstr: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  int stepres = sqlite3_step (fetchfill_loadstmt);
  if (stepres == SQLITE_ROW)
    {
      const char *colfill = (const char *)
	sqlite3_column_text (fetchfill_loadstmt, 0);
      assert (colfill != NULL);
      res = GC_STRDUP (colfill);
      if (MONIMELT_UNLIKELY (!res))
	MONIMELT_FATAL ("failed to duplicate fill of %s which is %s",
			uuidstr, colfill);
      stepres = sqlite3_step (fetchfill_loadstmt);
    }
  sqlite3_reset (fetchfill_loadstmt);
  if (stepres == SQLITE_DONE)
    return res;
  else
    MONIMELT_FATAL ("failed to fetch fill %s: %s (%d:%s)",
		    uuidstr, sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
}

static sqlite3_stmt *buildfill_dumpstmt;
static void
rootspace_store_build_fill (struct mom_dumper_st *dmp,
			    mom_anyitem_t * itm,
			    const char *buildstr, const char *fillstr)
{
  char ustr[UUID_PARSED_LEN];
  memset (ustr, 0, sizeof (ustr));
  assert (itm && itm->typnum > momty__itemlowtype
	  && itm->typnum < momty__last);
  uuid_unparse (itm->i_uuid, ustr);
  if (MONIMELT_UNLIKELY (buildfill_dumpstmt == NULL))
    {
      if (sqlite3_prepare_v2 (mom_dbsqlite,
			      "INSERT INTO t_item (uid, type, jbuild, jfill) VALUES (?1, ?2, ?3, ?4)",
			      -1, &buildfill_dumpstmt, NULL))
	MONIMELT_FATAL ("failed to prepare build&fill insertion: %s",
			sqlite3_errmsg (mom_dbsqlite));
    }
  // uidstr at index 1
  if (sqlite3_bind_text (buildfill_dumpstmt, 1, ustr, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind uuid: %s", sqlite3_errmsg (mom_dbsqlite));
  // type at index 2
  const struct momitemtypedescr_st *typdesc =
    mom_typedescr_array[itm->typnum];
  assert (typdesc != NULL && typdesc->ityp_magic == ITEMTYPE_MAGIC);
  const char *typename = typdesc->ityp_name;
  if (typdesc->ityp_mascarade_dump)
    typename = typdesc->ityp_mascarade_dump (dmp, itm);
  if (!typename || !typename[0])
    goto reset_step;
  if (sqlite3_bind_text
      (buildfill_dumpstmt, 2, typdesc->ityp_name, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind type: %s", sqlite3_errmsg (mom_dbsqlite));
  // build string at index 3
  if (sqlite3_bind_text
      (buildfill_dumpstmt, 3, buildstr ? buildstr : "", -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind build string: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  // fill string at index 4
  if (sqlite3_bind_text
      (buildfill_dumpstmt, 4, fillstr ? fillstr : "", -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind fill string: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  // now insert the build & fill
  int stepres = sqlite3_step (buildfill_dumpstmt);
  if (stepres != SQLITE_DONE)
    MONIMELT_FATAL ("failed to insert build & fill of uid %s: %s (%d:%s)",
		    ustr, sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
reset_step:
  if (sqlite3_reset (buildfill_dumpstmt))
    MONIMELT_FATAL ("failed to reset build & fill statement: %s",
		    sqlite3_errmsg (mom_dbsqlite));
}


static sqlite3_stmt *fetchparam_loadstmt;
static const char *
fetch_param (const char *parname)
{
  char *res = NULL;
  if (!parname || !parname[0])
    return NULL;
  if (MONIMELT_UNLIKELY (fetchparam_loadstmt == NULL))
    {
      if (sqlite3_prepare_v2 (mom_dbsqlite,
			      "SELECT parvalue FROM t_param WHERE parname = ?1",
			      -1, &fetchparam_loadstmt, NULL))
	MONIMELT_FATAL ("failed to prepare fetchparam query: %s",
			sqlite3_errmsg (mom_dbsqlite));
    }
  // parname at index 1
  if (sqlite3_bind_text (fetchparam_loadstmt, 1, parname, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind parrame: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  int stepres = sqlite3_step (fetchparam_loadstmt);
  if (stepres == SQLITE_ROW)
    {
      const char *colparam = (const char *)
	sqlite3_column_text (fetchparam_loadstmt, 0);
      assert (colparam != NULL);
      res = GC_STRDUP (colparam);
      if (MONIMELT_UNLIKELY (!res))
	MONIMELT_FATAL ("failed to duplicate param of name %s which is %s",
			parname, colparam);
      stepres = sqlite3_step (fetchparam_loadstmt);
    }
  sqlite3_reset (fetchparam_loadstmt);
  if (stepres == SQLITE_DONE)
    return res;
  else
    MONIMELT_FATAL ("failed to fetch parameter %s: %s (%d:%s)",
		    parname, sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
}

static sqlite3_stmt *putparam_dumpstmt;
static void param_printf (const char *paramname, const char *fmt, ...)
  __attribute__ ((format (printf, 2, 3)));
static void
param_printf (const char *paramname, const char *fmt, ...)
{
  if (!paramname || !paramname[0])
    return;
  char *buf = NULL;
  va_list args;
  va_start (args, fmt);
  vasprintf (&buf, fmt, args);
  va_end (args);
  if (MONIMELT_UNLIKELY (!buf))
    MONIMELT_FATAL ("failed to print param %s", fmt);
  if (MONIMELT_UNLIKELY (putparam_dumpstmt == NULL))
    {
      if (sqlite3_prepare_v2 (mom_dbsqlite,
			      "INSERT INTO t_param (parname, parvalue) VALUES (?1, ?2)",
			      -1, &putparam_dumpstmt, NULL))
	MONIMELT_FATAL ("failed to prepare putparam query: %s",
			sqlite3_errmsg (mom_dbsqlite));
    }
  // parname at index 1
  if (sqlite3_bind_text (putparam_dumpstmt, 1, paramname, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind parame: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  // buf at index 2
  if (sqlite3_bind_text (putparam_dumpstmt, 2, buf, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind buf: %s", sqlite3_errmsg (mom_dbsqlite));
  // now insert the param
  int stepres = sqlite3_step (putparam_dumpstmt);
  if (stepres != SQLITE_DONE)
    MONIMELT_FATAL ("failed to insert parameter %s: %s (%d:%s)",
		    paramname, sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
  if (sqlite3_reset (putparam_dumpstmt))
    MONIMELT_FATAL ("failed to reset put parameter statement: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  free (buf);
}

static struct momspacedescr_st mom_root_space_descr = {
  .spa_magic = SPACE_MAGIC,
  .spa_name = MONIMELT_ROOT_SPACE_NAME,
  .spa_data = NULL,
  .spa_fetch_build = rootspace_fetch_build,
  .spa_fetch_fill = rootspace_fetch_fill,
  .spa_store_build_fill = rootspace_store_build_fill,
};

#define LOADNAMING_MAGIC 0x148e62fd	/* loadnaming magic 344875773 */
struct loadnaming_st
{
  unsigned ldn_magic;
  unsigned ldn_nbitems;
  unsigned ldn_nbnames;
  unsigned ldn_count;
  char **ldn_names;
  char **ldn_uuids;
  char **ldn_spaces;
};

static int
ldn_cb (void *data, int nbcol, char **colarrs, char **colnames)
{
  struct loadnaming_st *ldn = data;
  assert (ldn->ldn_magic == LOADNAMING_MAGIC);
  assert (nbcol == 3);
  unsigned cnt = ldn->ldn_count;
  assert (cnt < ldn->ldn_nbnames);
  ldn->ldn_names[cnt] = GC_STRDUP (colarrs[0]);
  ldn->ldn_uuids[cnt] = GC_STRDUP (colarrs[1]);
  char *spaname = colarrs[2];
  assert (spaname != NULL);
  for (unsigned spix = 1; spix < MONIMELT_SPACE_MAX; spix++)
    {
      if (!mom_spacedescr_array[spix])
	MONIMELT_FATAL ("unknown space %s for uuid %s name %s", spaname,
			ldn->ldn_uuids[cnt], ldn->ldn_names[cnt]);
      if (!strcmp (spaname, mom_spacedescr_array[spix]->spa_name))
	{
	  ldn->ldn_spaces[cnt] =
	    (char *) mom_spacedescr_array[spix]->spa_name;
	  break;
	}
    }
  ldn->ldn_count = cnt + 1;
  return 0;
}


void
mom_initialize_spaces (void)
{
  mom_spacedescr_array[MONIMELT_SPACE_ROOT] = &mom_root_space_descr;
  mom_spacename_array[MONIMELT_SPACE_ROOT] =
    (momstring_t *) mom_make_string (MONIMELT_ROOT_SPACE_NAME);
}

static void load_modules ();
void
mom_initial_load (const char *state)
{
  int errcod = sqlite3_open (state, &mom_dbsqlite);
  struct mom_loader_st ld = { };
  intptr_t nbitems = 0, nbnames = 0, nbmodules = 0;
  if (errcod)
    MONIMELT_FATAL ("failed to open sqlite3 %s:%s", state,
		    sqlite3_errmsg (mom_dbsqlite));
  char *errmsg = NULL;
  const char *vers = fetch_param (MONIMELT_VERSION_PARAM);
  if (MONIMELT_UNLIKELY (vers && strcmp (vers, MONIMELT_DUMP_VERSION)))
    MONIMELT_FATAL
      ("in state %s dump format version mismatch got %s expected %s", state,
       vers, MONIMELT_DUMP_VERSION);
  if (sqlite3_exec
      (mom_dbsqlite, "SELECT COUNT(*) AS nb_items FROM t_item", setintptr_cb,
       &nbitems, &errmsg))
    MONIMELT_FATAL ("counting items in %s failed with %s", state, errmsg);
  if (sqlite3_exec (mom_dbsqlite,
		    "SELECT COUNT(*) AS nb_names FROM t_name",
		    setintptr_cb, &nbnames, &errmsg))
    MONIMELT_FATAL ("counting names in %s failed with %s", state, errmsg);
  if (sqlite3_exec (mom_dbsqlite,
		    "SELECT COUNT(*) AS nb_modules FROM t_module",
		    setintptr_cb, &nbmodules, &errmsg))
    MONIMELT_FATAL ("counting modules in %s failed with %s", state, errmsg);
  MONIMELT_INFORM ("state %s has %d items and %d names in %d modules", state,
		   (int) nbitems, (int) nbnames, (int) nbmodules);
  if (MONIMELT_UNLIKELY (nbitems == 0 || nbnames == 0))
    MONIMELT_FATAL ("no items or names to load from %s", state);
  load_modules ();
  memset (&ld, 0, sizeof (ld));
  ld.ldr_magic = LOADER_MAGIC;
  struct loadnaming_st ldn;
  memset (&ldn, 0, sizeof (ldn));
  ldn.ldn_nbitems = nbitems;
  ldn.ldn_nbnames = nbnames;
  ldn.ldn_count = 0;
  char **namesarr = GC_MALLOC (nbnames * sizeof (char *));
  char **uuidsarr = GC_MALLOC (nbnames * sizeof (char *));
  char **spacearr = GC_MALLOC (nbnames * sizeof (char *));
  if (!namesarr || !uuidsarr || !spacearr)
    MONIMELT_FATAL ("failed to allocate for %d names", (int) nbnames);
  memset (namesarr, 0, nbnames * sizeof (char *));
  memset (uuidsarr, 0, nbnames * sizeof (char *));
  memset (spacearr, 0, nbnames * sizeof (char *));
  ldn.ldn_names = namesarr;
  ldn.ldn_uuids = uuidsarr;
  ldn.ldn_spaces = spacearr;
  ldn.ldn_magic = LOADNAMING_MAGIC;
  if (sqlite3_exec (mom_dbsqlite,
		    "SELECT name, nuid, spacenam FROM t_name ORDER BY name",
		    ldn_cb, &ldn, &errmsg))
    MONIMELT_FATAL ("fetching names in %s failed with %s", state, errmsg);
  for (unsigned ix = 0; ix < ldn.ldn_count; ix++)
    {
      const char *curname = ldn.ldn_names[ix];
      const char *curuuidstr = ldn.ldn_uuids[ix];
      const char *curspace = ldn.ldn_spaces[ix];
      mom_anyitem_t *curitm = NULL;
      uuid_t curuid;
      memset (&curuid, 0, sizeof (uuid_t));
      if (!uuid_parse (curuuidstr, curuid))
	{
	  curitm = mom_load_item (&ld, curuid, curspace);
	  if (!curitm)
	    MONIMELT_FATAL ("failed to load named item %s of uid %s space %s",
			    curname, curuuidstr, curspace);
	}
    }
  while (ld.ldr_qfirst != NULL)
    {
      char curustr[UUID_PARSED_LEN];
      memset (curustr, 0, sizeof (curustr));
      assert (ld.ldr_magic == LOADER_MAGIC);
      mom_anyitem_t *itmld = ld.ldr_qfirst->iq_item;
      assert (itmld != NULL && itmld->typnum > momty__itemlowtype
	      && itmld->typnum < momty__last);
      assert (itmld->i_space > 0 && itmld->i_space < MONIMELT_SPACE_MAX);
      struct momspacedescr_st *curspad = mom_spacedescr_array[itmld->i_space];
      assert (curspad != NULL && curspad->spa_magic == SPACE_MAGIC);
      assert (curspad->spa_fetch_fill != NULL);
      uuid_unparse (itmld->i_uuid, curustr);
      const struct momitemtypedescr_st *ids =
	mom_typedescr_array[itmld->typnum];
      assert (ids && ids->ityp_magic == ITEMTYPE_MAGIC);
      char *fillstr = curspad->spa_fetch_fill (itmld->i_space, curustr);
      if (fillstr && fillstr[0] && ids->ityp_filler)
	{
	  struct jsonparser_st jp = { 0 };
	  FILE *fm = fmemopen (fillstr, strlen (fillstr), "r");
	  if (MONIMELT_UNLIKELY (!fm))
	    MONIMELT_FATAL ("fmemopen failed for uid %s fill string %s",
			    curustr, fillstr);
	  mom_initialize_json_parser (&jp, fm, NULL);
	  char *errmsg = NULL;
	  momval_t jval = mom_parse_json (&jp, &errmsg);
	  if (MONIMELT_UNLIKELY (!jval.ptr && errmsg))
	    MONIMELT_FATAL
	      ("parsing of fill of uid %s in space %s json %s failed : %s",
	       curustr, curspad->spa_name, fillstr, errmsg);
	  mom_close_json_parser (&jp);
	  ids->ityp_filler (&ld, itmld, jval);
	}
      ld.ldr_qfirst = ld.ldr_qfirst->iq_next;
      if (!ld.ldr_qfirst)
	ld.ldr_qlast = NULL;
    }
  if (fetchparam_loadstmt)
    sqlite3_finalize (fetchparam_loadstmt), fetchparam_loadstmt = NULL;
  if (fetchbuild_loadstmt)
    sqlite3_finalize (fetchbuild_loadstmt), fetchbuild_loadstmt = NULL;
  if (fetchfill_loadstmt)
    sqlite3_finalize (fetchfill_loadstmt), fetchbuild_loadstmt = NULL;
  sqlite3_close (mom_dbsqlite);
  mom_dbsqlite = NULL;
}


static void
dumpglobal_cb (const mom_anyitem_t * itm, const momstring_t * name,
	       void *data)
{
  sqlite3_stmt *stmt = data;
  assert (stmt != NULL);
  assert (name != NULL && name->typnum == momty_string);
  if (!itm || itm->typnum <= momty__itemlowtype || itm->i_space == 0
      || itm->i_space >= MONIMELT_SPACE_MAX
      || !mom_spacename_array[itm->i_space])
    return;
  // name at index 1
  if (sqlite3_bind_text
      (stmt, 1, mom_string_cstr ((momval_t) name), -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind name: %s", sqlite3_errmsg (mom_dbsqlite));
  // uid string at index 2
  char ustr[UUID_PARSED_LEN];
  memset (ustr, 0, sizeof (ustr));
  uuid_unparse (itm->i_uuid, ustr);
  if (sqlite3_bind_text (stmt, 2, ustr, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind uid: %s", sqlite3_errmsg (mom_dbsqlite));
  // spacename at index 3
  if (sqlite3_bind_text
      (stmt, 3,
       mom_string_cstr ((momval_t) mom_spacename_array[itm->i_space]), -1,
       SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind space name: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  int stepres = sqlite3_step (stmt);
  if (stepres != SQLITE_DONE)
    MONIMELT_FATAL ("failed to insert global name %s: %s (%d:%s)",
		    mom_string_cstr ((momval_t) name),
		    sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
  if (sqlite3_reset (stmt))
    MONIMELT_FATAL ("failed to reset statement: %s",
		    sqlite3_errmsg (mom_dbsqlite));
}

static GTree *dumped_module_tree;
static void dump_modules (void);

// we may want to avoid too long lines in the dumps. See
// http://programmers.stackexchange.com/q/236542/40065
#define BIGDUMP_THRESHOLD 250
void
mom_full_dump (const char *state)
{
  struct mom_dumper_st dmp = { };
  memset (&dmp, 0, sizeof (dmp));
  int errcod = sqlite3_open (state, &mom_dbsqlite);
  char *errmsg = NULL;
  if (errcod)
    MONIMELT_FATAL ("failed to open sqlite3 %s:%s", state,
		    sqlite3_errmsg (mom_dbsqlite));
  dumped_module_tree = g_tree_new ((GCompareFunc) strcmp);
  if (sqlite3_exec
      (mom_dbsqlite,
       "CREATE TABLE IF NOT EXISTS t_item (uid VARCHAR(38) PRIMARY KEY ASC NOT NULL UNIQUE,"
       " type VARCHAR(60) NOT NULL,"
       " jbuild TEXT NOT NULL," " jfill TEXT NOT NULL)", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to create t_item: %s", errmsg);
  if (sqlite3_exec
      (mom_dbsqlite,
       "CREATE TABLE IF NOT EXISTS t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE,"
       " nuid VARCHAR(38) UNIQUE NOT NULL REFERENCES t_id(uid),"
       " spacenam VARCHAR(30) NOT NULL)", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to create t_name: %s", errmsg);
  if (sqlite3_exec
      (mom_dbsqlite,
       "CREATE TABLE IF NOT EXISTS t_param (parname VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE,"
       " parvalue TEXT NOT NULL)", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to create t_param: %s", errmsg);
  if (sqlite3_exec
      (mom_dbsqlite,
       "CREATE TABLE IF NOT EXISTS t_module (modname VARCHAR(100) PRIMARY KEY ASC NOT NULL UNIQUE)",
       NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to create t_module: %s", errmsg);
  if (sqlite3_exec (mom_dbsqlite, "BEGIN TRANSACTION", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to BEGIN TRANSACTION: %s", errmsg);
  if (sqlite3_exec (mom_dbsqlite, "DELETE FROM t_name", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to DELETE FROM t_name: %s", errmsg);
  if (sqlite3_exec (mom_dbsqlite, "DELETE FROM t_item", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to DELETE FROM t_item: %s", errmsg);
  if (sqlite3_exec (mom_dbsqlite, "DELETE FROM t_param", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to DELETE FROM t_param: %s", errmsg);
  if (sqlite3_exec
      (mom_dbsqlite, "DELETE FROM t_module", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to DELETE FROM t_module: %s", errmsg);
  param_printf (MONIMELT_VERSION_PARAM, "%s", MONIMELT_DUMP_VERSION);
  mom_dumper_initialize (&dmp);
  sqlite3_stmt *stmt = NULL;
  if (sqlite3_prepare_v2 (mom_dbsqlite,
			  "INSERT INTO t_name (name, nuid, spacenam) VALUES (?1, ?2, ?3)",
			  -1, &stmt, NULL))
    MONIMELT_FATAL ("failed to prepare name insertion: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  mom_dump_globals (&dmp, dumpglobal_cb, stmt);
  unsigned nbdumpeditems = 0;
  dmp.dmp_state = dus_emit;
  while (dmp.dmp_qfirst != NULL)
    {
      struct jsonoutput_st outj = { };
      memset (&outj, 0, sizeof (outj));
      mom_anyitem_t *curitm = dmp.dmp_qfirst->iq_item;
      // debugging
      {
	char ustr[UUID_PARSED_LEN];
	memset (ustr, 0, sizeof (ustr));
	uuid_unparse (curitm->i_uuid, ustr);
      }
      //
      assert (curitm != NULL && curitm->i_space < momty__last);
      const struct momitemtypedescr_st *tydescr =
	mom_typedescr_array[curitm->typnum];
      assert (tydescr != NULL && tydescr->ityp_magic == ITEMTYPE_MAGIC);
      assert (curitm->i_space > 0 && curitm->i_space < MONIMELT_SPACE_MAX);
      struct momspacedescr_st *spadescr =
	mom_spacedescr_array[curitm->i_space];
      assert (spadescr && spadescr->spa_magic == SPACE_MAGIC);
      momval_t jsonbuild = tydescr->ityp_getbuild (&dmp, curitm);
      momval_t jsonfill = tydescr->ityp_getfill (&dmp, curitm);
      char *strbuild = NULL;
      char *strfill = NULL;
      {
	char *bufbuild = NULL;
	size_t sizbuild = 0;
	FILE *foutbuild = open_memstream (&bufbuild, &sizbuild);
	mom_json_output_initialize (&outj, foutbuild, NULL,
				    jsof_flush | jsof_halfindent);
	mom_output_json (&outj, jsonbuild);
	fflush (foutbuild);
	if (ftell (foutbuild) > BIGDUMP_THRESHOLD)
	  putc ('\n', foutbuild);
	mom_json_output_close (&outj);
	strbuild = GC_STRDUP (bufbuild);
	free (bufbuild), bufbuild = NULL, sizbuild = 0;
      }
      memset (&outj, 0, sizeof (outj));
      {
	char *buffill = NULL;
	size_t sizfill = 0;
	FILE *foutfill = open_memstream (&buffill, &sizfill);
	mom_json_output_initialize (&outj, foutfill, NULL,
				    jsof_flush | jsof_halfindent);
	mom_output_json (&outj, jsonfill);
	fflush (foutfill);
	if (ftell (foutfill) > BIGDUMP_THRESHOLD)
	  putc ('\n', foutfill);
	mom_json_output_close (&outj);
	strfill = GC_STRDUP (buffill);
	free (buffill), buffill = NULL, sizfill = 0;
      }
      assert (spadescr->spa_store_build_fill != NULL);
      spadescr->spa_store_build_fill (&dmp, curitm, strbuild, strfill);
      strbuild = NULL;
      strfill = NULL;
      dmp.dmp_qfirst = dmp.dmp_qfirst->iq_next;
      if (!dmp.dmp_qfirst)
	dmp.dmp_qlast = NULL;
      nbdumpeditems++;
    }
  dump_modules ();
  if (buildfill_dumpstmt)
    sqlite3_finalize (buildfill_dumpstmt), buildfill_dumpstmt = NULL;
  if (putparam_dumpstmt)
    sqlite3_finalize (putparam_dumpstmt), putparam_dumpstmt = NULL;
  if (sqlite3_exec (mom_dbsqlite, "END TRANSACTION", NULL, NULL, &errmsg))
    MONIMELT_FATAL ("failed to END TRANSACTION: %s", errmsg);
  sqlite3_close_v2 (mom_dbsqlite), mom_dbsqlite = NULL;
  MONIMELT_INFORM ("dumped %d items in %s", nbdumpeditems, state);
  g_tree_destroy (dumped_module_tree), dumped_module_tree = NULL;
}

void
mom_register_dumped_module (const char *modname)
{
  assert (dumped_module_tree != NULL);
  if (!g_tree_lookup (dumped_module_tree, modname))
    g_tree_insert (dumped_module_tree, (gpointer) modname,
		   (gpointer) modname);
}

gboolean
traverse_module_tree (gpointer key, gpointer val, gpointer data)
{
  const char *modname = key;
  sqlite3_stmt *modstmt = data;
  assert (key == val);
  // modname at index 1
  if (sqlite3_bind_text (modstmt, 1, modname, -1, SQLITE_STATIC))
    MONIMELT_FATAL ("failed to bind modname: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  int stepres = sqlite3_step (modstmt);
  if (stepres != SQLITE_DONE)
    MONIMELT_FATAL ("failed to insert module name %s: %s (%d:%s)",
		    modname,
		    sqlite3_errmsg (mom_dbsqlite),
		    stepres, sqlite3_errstr (stepres));
  if (sqlite3_reset (modstmt))
    MONIMELT_FATAL ("failed to reset statement: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  return FALSE;
}

static void
dump_modules (void)
{
  sqlite3_stmt *modstmt = NULL;
  assert (dumped_module_tree != NULL);
  if (sqlite3_prepare_v2 (mom_dbsqlite,
			  "INSERT INTO t_module (modname) VALUES (?1)",
			  -1, &modstmt, NULL))
    MONIMELT_FATAL ("failed to prepare module insertion query: %s",
		    sqlite3_errmsg (mom_dbsqlite));
  g_tree_foreach (dumped_module_tree, traverse_module_tree, modstmt);
  sqlite3_finalize (modstmt), modstmt = NULL;
}

static int
loadmodule_cb (void *data, int nbcol, char **colarrs, char **colnames)
{
  char *modname = colarrs[0];
  GModule *mod = g_module_open (modname, 0);
  if (!mod)
    {
      char bufname[128];
      memset (bufname, 0, sizeof (bufname));
      snprintf (bufname, sizeof (bufname), "./%s.%s", modname,
		G_MODULE_SUFFIX);
      mod = g_module_open (bufname, 0);
    }
  if (!mod)
    MONIMELT_FATAL ("failed to load module %s: %s", modname,
		    g_module_error ());
  else
    MONIMELT_INFORM ("loaded module %s", modname);
  return 0;
}

static void
load_modules (void)
{
  char *errmsg = NULL;
  if (sqlite3_exec
      (mom_dbsqlite, "SELECT modname FROM t_module", loadmodule_cb,
       NULL, &errmsg))
    MONIMELT_FATAL ("loading modules failed with %s", errmsg);
}
