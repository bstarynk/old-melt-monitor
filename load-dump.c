// file load-dump.c

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

#define MOM_VERSION_PARAM "dump_format_version"
#define MOM_DUMP_VERSION "MoniMelt2014B"
//// loader
struct mom_loader_st
{
  unsigned ldr_magic;		/* always LOADER_MAGIC */
  /// hash table of loaded items
  unsigned ldr_hsize;
  unsigned ldr_hcount;
  const momitem_t **ldr_htable;
  momspaceid_t ldr_curspace;
  // the load directory
  const char *ldr_dirpath;
  // the sqlite3 path
  const char *ldr_sqlpath;
  // the sqlite database handle
  sqlite3 *ldr_sqlite;
  // statement for param fetching
  sqlite3_stmt *ldr_sqlstmt_param_fetch;
  // statement for named fetching
  sqlite3_stmt *ldr_sqlstmt_named_fetch;
  // statement for item fetching
  sqlite3_stmt *ldr_sqlstmt_item_fetch;
  /// queue of items whose content should be loaded:
  struct mom_itqueue_st *ldr_qfirst;
  struct mom_itqueue_st *ldr_qlast;
};


//// dumper, see file load-dump.c
struct mom_dumper_st
{
  unsigned dmp_magic;		/* always DUMPER_MAGIC */
  unsigned dmp_count;
  unsigned dmp_size;
  uint8_t dmp_state;
  momspaceid_t dmp_curspace;
  const momitem_t **dmp_array;	// hash table with dmp_count & dmp_size
  // of dumped items
  unsigned dmp_predefsize;
  unsigned dmp_predefnb;
  const momitem_t **dmp_predefarray;	/* table of size dmp_predefsize
					   for predefined items, last
					   index is dmp_predefnb */
  ///
  const char *dmp_reason;
  const char *dmp_dirpath;	/* the directory */
  const char *dmp_sqlpath;	/* the .dbsqlite file */
  const char *dmp_predefhpath;	/* the predef-monimelt.h file */
  sqlite3 *dmp_sqlite;
  /// statements for Sqlite3
  sqlite3_stmt *dmp_sqlstmt_param_insert;
  sqlite3_stmt *dmp_sqlstmt_item_insert;
  sqlite3_stmt *dmp_sqlstmt_name_insert;
  sqlite3_stmt *dmp_sqlstmt_module_insert;
  struct mom_itqueue_st *dmp_qfirst;
  struct mom_itqueue_st *dmp_qlast;
};
static pthread_mutex_t dump_mtx_mom = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

enum dumpstate_en
{
  dus_none = 0,
  dus_scan,
  dus_emit
};

#define DUMPER_MAGIC 0x572bb695	/* dumper magic 1462482581 */

#define LOADER_MAGIC 0x169128bb	/* loader magic 378611899 */

static inline void
add_dumped_item_mom (struct mom_dumper_st *dmp, const momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
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
  MOM_FATAPRINTF ("corrupted dump of %d items", dmp->dmp_count);
}


static inline bool
found_dumped_item_mom (struct mom_dumper_st *dmp, const momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item);
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  momhash_t h = itm->i_hash;
  unsigned size = dmp->dmp_size;
  unsigned istart = h % size;
  for (unsigned ix = istart; ix < size; ix++)
    {
      if (dmp->dmp_array[ix] == itm)
	return true;
      else if (!dmp->dmp_array[ix])
	return false;
    }
  for (unsigned ix = 0; ix < istart; ix++)
    {
      if (dmp->dmp_array[ix] == itm)
	return true;
      else if (!dmp->dmp_array[ix])
	return false;
    }
  return false;
}

void
mom_dump_add_item (struct mom_dumper_st *dmp, const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return;
  if (!dmp || dmp->dmp_magic != DUMPER_MAGIC)
    return;
  MOM_DEBUG (dump, MOMOUT_LITERAL ("adding item:"), MOMOUT_ITEM (itm));
  if (MOM_UNLIKELY (dmp->dmp_state != dus_scan))
    MOM_FATAPRINTF ("invalid dump state #%d", (int) dmp->dmp_state);
  if (MOM_UNLIKELY (4 * dmp->dmp_count / 3 + 10 >= dmp->dmp_size))
    {
      unsigned oldsize = dmp->dmp_size;
      unsigned oldcount = dmp->dmp_count;
      const momitem_t **oldarr = dmp->dmp_array;
      unsigned newsize = ((4 * oldcount / 3 + oldcount / 4 + 60) | 0x7f) + 1;
      const momitem_t **newarr = MOM_GC_ALLOC ("growing dumper array",
					       newsize *
					       sizeof (momitem_t *));
      dmp->dmp_array = newarr;
      dmp->dmp_size = newsize;
      dmp->dmp_count = 0;
      for (unsigned ix = 0; ix < oldsize; ix++)
	{
	  const momitem_t *curitm = oldarr[ix];
	  if (!curitm)
	    continue;
	  add_dumped_item_mom (dmp, curitm);
	}
    }
  bool founditem = found_dumped_item_mom (dmp, itm);
  // enqueue and add the item if it is not found
  if (!founditem)
    {
      MOM_DEBUG (dump, MOMOUT_LITERAL ("enqueue item:"), MOMOUT_ITEM (itm));
      struct mom_itqueue_st *qel = MOM_GC_ALLOC ("dumped item queue element",
						 sizeof (struct
							 mom_itqueue_st));
      qel->iq_next = NULL;
      qel->iq_item = (momitem_t *) itm;
      if (MOM_UNLIKELY (dmp->dmp_qlast == NULL))
	{
	  dmp->dmp_qfirst = dmp->dmp_qlast = qel;
	}
      else
	{
	  dmp->dmp_qlast->iq_next = qel;
	  dmp->dmp_qlast = qel;
	}
      add_dumped_item_mom (dmp, itm);
    }
}

void
mom_dump_scan_value (struct mom_dumper_st *dmp, const momval_t val)
{
  if (!dmp || !val.ptr)
    return;
  if (MOM_UNLIKELY (dmp->dmp_magic != DUMPER_MAGIC))
    MOM_FATAPRINTF ("bad dumper@%p when dumping value @%p", (void *) dmp,
		    val.ptr);
  if (MOM_UNLIKELY (dmp->dmp_state != dus_scan))
    MOM_FATAPRINTF ("invalid dump state #%d", (int) dmp->dmp_state);
  unsigned typ = *val.ptype;
  switch ((enum momvaltype_en) typ)
    {
    case momty_null:
      MOM_FATAPRINTF ("corrupted value @%p to dump", val.ptr);
      break;
    case momty_int:
    case momty_double:
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
      {
	const momitem_t *curconn = val.pnode->connitm;
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
	unsigned siz = val.pseqitems->slen;
	for (unsigned ix = 0; ix < siz; ix++)
	  {
	    const momitem_t *curitm = val.pseqitems->itemseq[ix];
	    if (!curitm || curitm->i_space == 0)
	      continue;
	    mom_dump_add_item (dmp, curitm);
	  }
	return;
      }
    case momty_item:
      {
	if (val.pitem && val.pitem->i_space > 0)
	  mom_dump_add_item (dmp, val.pitem);
      }
      break;
    }
}				// end mom_dump_scan_value


static momval_t
raw_dump_emit_json_mom (struct mom_dumper_st *dmp, const momval_t val);

momval_t
mom_dump_emit_json (struct mom_dumper_st *dmp, const momval_t val)
{
  momval_t jsval = MOM_NULLV;
  if (MOM_UNLIKELY (!dmp || dmp->dmp_magic != DUMPER_MAGIC))
    MOM_FATAPRINTF ("bad dumper@%p when dumping value @%p", (void *) dmp,
		    val.ptr);
  if (MOM_UNLIKELY (dmp->dmp_state != dus_emit))
    MOM_FATAPRINTF ("invalid dump state #%d", (int) dmp->dmp_state);
  jsval = raw_dump_emit_json_mom (dmp, val);
  return jsval;
}


// emit a short representation of an item: if it is in the current
// space, just its id string...
static momval_t
emit_short_item_json_mom (struct mom_dumper_st *dmp, const momitem_t *itm)
{
  if (!itm)
    return MOM_NULLV;
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  assert (dmp->dmp_state == dus_emit);
  if (itm->i_space == momspa_none || !found_dumped_item_mom (dmp, itm))
    return MOM_NULLV;
  assert (itm->i_typnum == momty_item && itm->i_magic == MOM_ITEM_MAGIC);
  momval_t idstr = (momval_t) mom_item_get_idstr ((momitem_t *) itm);
  if (dmp->dmp_curspace == itm->i_space
      || (itm->i_space == momspa_predefined
	  && dmp->dmp_curspace == momspa_root))
    return idstr;
  else
    {
      assert (itm->i_space <= momspa__last);
      struct mom_spacedescr_st *spad = mom_spacedescr_array[itm->i_space];
      assert (spad && spad->space_magic == MOM_SPACE_MAGIC);
      assert (spad->space_name && spad->space_name[0]);
      if (MOM_UNLIKELY (!spad->space_namestr))
	spad->space_namestr = mom_make_string (spad->space_name);
      return
	(momval_t) mom_make_json_object
	(MOMJSOB_ENTRY
	 ((momval_t) mom_named__jtype, (momval_t) mom_named__item_ref),
	 MOMJSOB_ENTRY ((momval_t) mom_named__item_ref, idstr),
	 MOMJSOB_ENTRY ((momval_t) mom_named__space,
			(momval_t) spad->space_namestr), MOMJSON_END);
    }
}

void
mom_dump_scan_inside_item (struct mom_dumper_st *dmp, momitem_t *itm)
{
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  assert (dmp->dmp_state == dus_scan);
  assert (itm && itm->i_typnum == momty_item
	  && itm->i_magic == MOM_ITEM_MAGIC);
  if (itm->i_space == momspa_none)
    return;
  pthread_mutex_lock (&itm->i_mtx);
  if (itm->i_content.ptr)
    mom_dump_scan_value (dmp, itm->i_content);
  if (itm->i_attrs)
    {
      unsigned atsiz = itm->i_attrs->size;
      struct mom_attrentry_st *atent = itm->i_attrs->itattrtab;
      for (unsigned aix = 0; aix < atsiz; aix++)
	{
	  if (atent[aix].aten_itm && atent[aix].aten_itm != MOM_EMPTY
	      && atent[aix].aten_val.ptr
	      && atent[aix].aten_val.ptr != MOM_EMPTY
	      && atent[aix].aten_itm->i_space != momspa_none)
	    {
	      add_dumped_item_mom (dmp, atent[aix].aten_itm);
	      mom_dump_scan_value (dmp, atent[aix].aten_val);
	    }
	}
    }
  if (itm->i_payload != NULL)
    {
      assert (itm->i_paylkind > 0 && itm->i_paylkind < mompayl__last);
      struct mom_payload_descr_st *payld = mom_payloadescr[itm->i_paylkind];
      assert (payld && payld->dpayl_magic == MOM_PAYLOAD_MAGIC);
      if (payld->dpayl_dumpscanfun)
	payld->dpayl_dumpscanfun (dmp, itm);
    }
  pthread_mutex_unlock (&itm->i_mtx);
}

static const momjsonarray_t *
jsonarray_emit_itemseq_mom (struct mom_dumper_st *dmp,
			    const struct momseqitem_st *si)
{
  unsigned slen = si->slen;
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  assert (dmp->dmp_state == dus_emit);
  if (slen <= MOM_TINY_MAX)
    {
      momval_t tab[MOM_TINY_MAX] = { MOM_NULLV };
      for (unsigned ix = 0; ix < slen; ix++)
	{
	  momitem_t *curitm = (momitem_t *) (si->itemseq[ix]);
	  if (curitm)
	    tab[ix] = emit_short_item_json_mom (dmp, curitm);
	  else
	    tab[ix] = MOM_NULLV;
	}
      return mom_make_json_array_count (slen, tab);
    }
  else
    {
      momval_t *arr =
	MOM_GC_ALLOC ("jsonarray emit", sizeof (momval_t) * slen);
      for (unsigned ix = 0; ix < slen; ix++)
	{
	  momitem_t *curitm = (momitem_t *) (si->itemseq[ix]);
	  if (curitm && curitm->i_space > 0)
	    arr[ix] = emit_short_item_json_mom (dmp, curitm);
	  else
	    arr[ix].ptr = NULL;
	}
      const momjsonarray_t *jarr = mom_make_json_array_count (slen, arr);
      GC_FREE (arr);
      return jarr;
    }
}


static const momjsonarray_t *
jsonarray_emit_nodesons_mom (struct mom_dumper_st *dmp,
			     const struct momnode_st *nd)
{
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  assert (dmp->dmp_state == dus_emit);
  unsigned slen = nd->slen;
  if (slen <= MOM_TINY_MAX)
    {
      momval_t tab[MOM_TINY_MAX] = { MOM_NULLV };
      for (unsigned ix = 0; ix < slen; ix++)
	tab[ix] = mom_dump_emit_json (dmp, (nd->sontab[ix]));
      return mom_make_json_array_count (slen, tab);
    }
  else
    {
      momval_t *arr =
	MOM_GC_ALLOC ("jsonarray emit node", sizeof (momval_t) * slen);
      for (unsigned ix = 0; ix < slen; ix++)
	arr[ix] = mom_dump_emit_json (dmp, (nd->sontab[ix]));
      const momjsonarray_t *jarr = mom_make_json_array_count (slen, arr);
      MOM_GC_FREE (arr);
      return jarr;
    }
}

static momval_t
raw_dump_emit_json_mom (struct mom_dumper_st *dmp, const momval_t val)
{
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  assert (dmp->dmp_state == dus_emit);
  momval_t jsval = MOM_NULLV;
  if (!val.ptr)
    return MOM_NULLV;
  unsigned typ = *val.ptype;
  switch ((enum momvaltype_en) typ)
    {
    case momty_null:
      MOM_FATAPRINTF ("corrupted value@%p", val.ptr);
      break;
    case momty_int:
    case momty_double:
      jsval = val;
      break;
    case momty_string:
      // we avoid very big strings in output by chunking them
      {
#define BIG_STRING_THRESHOLD 128
#define STRING_CHUNK_SIZE 40
	const momstring_t *str = val.pstring;
	unsigned slen = str->slen;
	const gchar *cstrb = (const gchar *) str->cstr;
	assert (g_utf8_validate (cstrb, slen, NULL));
	if (slen > BIG_STRING_THRESHOLD)
	  {
	    unsigned chksize = 3 + slen / STRING_CHUNK_SIZE;
	    momval_t *chkarr =
	      MOM_GC_ALLOC ("jstring big chunk", chksize * sizeof (momval_t));
	    const gchar *curb = cstrb;
	    const gchar *endb = cstrb + slen;
	    const gchar *nextb = NULL;
	    unsigned nbchk = 0;
	    for (curb = cstrb; curb && curb < endb; curb = nextb)
	      {
		momval_t chunkv = MOM_NULLV;
		assert (nbchk + 1 < chksize);
		nextb = g_utf8_next_char (curb);
		unsigned curchklen = 0;
		// skip STRING_CHUNK_SIZE UTF-8 characters if possible
		while (curchklen < STRING_CHUNK_SIZE && nextb < endb
		       && *nextb)
		  {
		    nextb = g_utf8_next_char (nextb);
		    curchklen++;
		  };
		const gchar *endchk = NULL;
		const gchar *lastb = nextb;
		// skip again STRING_CHUNK_SIZE UTF-8 up to an Unicode space or punctuation
		while (curchklen < 2 * STRING_CHUNK_SIZE && nextb < endb
		       && *nextb)
		  {
		    gunichar uc = g_utf8_get_char (nextb);
		    if (g_unichar_isspace (uc) || g_unichar_ispunct (uc))
		      {
			endchk = nextb;
			break;
		      }
		  }
		if (endchk)
		  {
		    chunkv =
		      (momval_t) mom_make_string_len (curb, endchk - curb);
		    nextb = endchk;
		  }
		else
		  {
		    chunkv =
		      (momval_t) mom_make_string_len (curb, lastb - curb);
		    nextb = lastb;
		  }
		chkarr[nbchk++] = chunkv;
	      };
	    momval_t jarrchk =
	      (momval_t) mom_make_json_array_count (nbchk, chkarr);
	    jsval =
	      (momval_t) mom_make_json_object
	      (MOMJSOB_ENTRY
	       ((momval_t) mom_named__jtype, (momval_t) mom_named__string),
	       MOMJSOB_ENTRY ((momval_t) mom_named__string, jarrchk),
	       MOMJSON_END);
	    GC_FREE (chkarr), chkarr = NULL;
	  }
	else
	  jsval = val;
	break;
      }
      break;
    case momty_item:
      {
	if (val.pitem->i_space == momspa_none
	    || !found_dumped_item_mom (dmp, val.pitem))
	  jsval = MOM_NULLV;
	else
	  {
	    const momstring_t *itemids = mom_item_get_idstr (val.pitem);
	    assert (val.pitem->i_space <= momspa__last);
	    struct mom_spacedescr_st *spad =
	      mom_spacedescr_array[val.pitem->i_space];
	    assert (spad && spad->space_magic == MOM_SPACE_MAGIC);
	    assert (spad->space_name && spad->space_name[0]);
	    if (MOM_UNLIKELY (!spad->space_namestr))
	      spad->space_namestr = mom_make_string (spad->space_name);
	    assert (mom_is_string ((momval_t) itemids));
	    jsval = (momval_t) mom_make_json_object
	      (MOMJSOB_ENTRY
	       ((momval_t) mom_named__jtype, (momval_t) mom_named__item_ref),
	       MOMJSOB_ENTRY ((momval_t) mom_named__item_ref,
			      (momval_t) itemids),
	       MOMJSOB_ENTRY ((momval_t) mom_named__space,
			      (momval_t) spad->space_namestr), MOMJSON_END);
	  }
      }
      break;
    case momty_jsonarray:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSOB_ENTRY
	   ((momval_t) mom_named__jtype, (momval_t) mom_named__json_array),
	   MOMJSOB_ENTRY ((momval_t) mom_named__json_array, val),
	   MOMJSON_END);
      }
      break;
    case momty_jsonobject:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSOB_ENTRY
	   ((momval_t) mom_named__jtype, (momval_t) mom_named__json_object),
	   MOMJSOB_ENTRY ((momval_t) mom_named__json_object, (momval_t) val),
	   MOMJSON_END);
      }
      break;
    case momty_set:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSOB_ENTRY
	   ((momval_t) mom_named__jtype, (momval_t) mom_named__set),
	   MOMJSOB_ENTRY ((momval_t) mom_named__set,
			  (momval_t) jsonarray_emit_itemseq_mom (dmp,
								 val.pset)),
	   MOMJSON_END);
      }
      break;
    case momty_tuple:
      {
	jsval = (momval_t) mom_make_json_object
	  (MOMJSOB_ENTRY
	   ((momval_t) mom_named__jtype, (momval_t) mom_named__tuple),
	   MOMJSOB_ENTRY ((momval_t) mom_named__tuple,
			  (momval_t) jsonarray_emit_itemseq_mom (dmp,
								 val.ptuple)),
	   MOMJSON_END);
      }
      break;
    case momty_node:
      {
	const momitem_t *curconn = val.pnode->connitm;
	if (curconn && curconn->i_space > 0)
	  {
	    momval_t jconn = emit_short_item_json_mom (dmp, curconn);
	    if (jconn.ptr)
	      jsval = (momval_t) mom_make_json_object
		(MOMJSOB_ENTRY
		 ((momval_t) mom_named__jtype, (momval_t) mom_named__node),
		 MOMJSOB_ENTRY ((momval_t) mom_named__node, jconn),
		 MOMJSOB_ENTRY ((momval_t) mom_named__sons,
				(momval_t) jsonarray_emit_nodesons_mom (dmp,
									val.pnode)),
		 MOMJSON_END);
	  }
      }
      break;
    }
  return jsval;
}

momval_t
mom_dump_data_inside_item (struct mom_dumper_st *dmp, momitem_t *itm)
{
  momval_t jdata = MOM_NULLV;
  momval_t jpayl = MOM_NULLV;
  momval_t jkind = MOM_NULLV;
  momval_t jarrent = MOM_NULLV;
  momval_t jcontent = MOM_NULLV;
  assert (dmp && dmp->dmp_magic == DUMPER_MAGIC);
  assert (dmp->dmp_state == dus_emit);
  assert (itm && itm->i_typnum == momty_item
	  && itm->i_magic == MOM_ITEM_MAGIC);
  if (itm->i_space == momspa_none)
    return MOM_NULLV;
  pthread_mutex_lock (&itm->i_mtx);
  if (itm->i_payload)
    {
      unsigned kindpayl = itm->i_paylkind;
      struct mom_payload_descr_st *payld = NULL;
      if (kindpayl > 0 && kindpayl < mompayl__last
	  && (payld = mom_payloadescr[kindpayl]) != NULL)
	{
	  assert (payld->dpayl_magic == MOM_PAYLOAD_MAGIC);
	  if (payld->dpayl_dumpjsonfun)
	    jpayl = payld->dpayl_dumpjsonfun (dmp, itm);
	  const momitem_t *kinditm = mom_get_item_of_name (payld->dpayl_name);
	  jkind =
	    kinditm ? ((momval_t) kinditm) : ((momval_t)
					      mom_make_string
					      (payld->dpayl_name));
	}
    }
  if (itm->i_content.ptr)
    jcontent = mom_dump_emit_json (dmp, itm->i_content);
  unsigned nbattrs = itm->i_attrs ? (itm->i_attrs->nbattr) : 0;
  unsigned cntattr = 0;
  momval_t tinyattr[MOM_TINY_MAX] = { };
  memset (tinyattr, 0, sizeof (tinyattr));
  momval_t *arrattr = (nbattrs < MOM_TINY_MAX)
    ? tinyattr : MOM_GC_ALLOC ("dumped attributes",
			       sizeof (momval_t) * (nbattrs + 1));
  for (unsigned aix = 0; aix < nbattrs; aix++)
    {
      assert (cntattr < nbattrs);
      momitem_t *curatitm = itm->i_attrs->itattrtab[aix].aten_itm;
      if (!curatitm || curatitm == MOM_EMPTY)
	continue;
      momval_t curval = itm->i_attrs->itattrtab[aix].aten_val;
      if (!curval.ptr || curval.ptr == MOM_EMPTY)
	continue;
      momval_t jattr = emit_short_item_json_mom (dmp, curatitm);
      if (!jattr.ptr)
	continue;
      momval_t jval = mom_dump_emit_json (dmp, curval);
      if (!jval.ptr)
	continue;
      momval_t jent = (momval_t) mom_make_json_object
	(MOMJSOB_ENTRY ((momval_t) mom_named__attr, (momval_t) jattr),
	 MOMJSOB_ENTRY ((momval_t) mom_named__val, (momval_t) jval),
	 MOMJSON_END);
      arrattr[cntattr++] = jent;
    }
  jarrent = (momval_t) mom_make_json_array_count (cntattr, arrattr);
  if (arrattr != tinyattr)
    MOM_GC_FREE (arrattr);
  jdata = (momval_t) mom_make_json_object
    (MOMJSOB_ENTRY ((momval_t) mom_named__attr, jarrent),
     MOMJSOB_ENTRY ((momval_t) mom_named__payload, jpayl),
     MOMJSOB_ENTRY ((momval_t) mom_named__content, jcontent),
     MOMJSOB_ENTRY ((momval_t) mom_named__kind, jkind), MOMJSON_END);
  pthread_mutex_unlock (&itm->i_mtx);
  return jdata;
}

static void
load_data_inside_item_mom (struct mom_loader_st *ld, momitem_t *itm,
			   momval_t jdata)
{
  assert (ld && ld->ldr_magic == LOADER_MAGIC);
  assert (itm && itm->i_typnum == momty_item);
  momval_t jarrent = mom_jsonob_get (jdata, (momval_t) mom_named__attr);
  momval_t jpayl = mom_jsonob_get (jdata, (momval_t) mom_named__payload);
  momval_t jcontent = mom_jsonob_get (jdata, (momval_t) mom_named__content);
  momval_t jkind = mom_jsonob_get (jdata, (momval_t) mom_named__kind);
  unsigned nbent = mom_json_array_size (jarrent);
  if (nbent > 0)
    {
      itm->i_attrs = mom_reserve_attribute (itm->i_attrs, 5 * nbent / 4 + 1);
      for (unsigned aix = 0; aix < nbent; aix++)
	{
	  momval_t jent = mom_json_array_nth (jarrent, (int) aix);
	  momval_t jcurattr =
	    mom_jsonob_get (jent, (momval_t) mom_named__attr);
	  momval_t jcurval = mom_jsonob_get (jent, (momval_t) mom_named__val);
	  momitem_t *curitm = mom_load_item_json (ld, jcurattr);
	  if (!curitm)
	    continue;
	  momval_t curval = mom_load_value_json (ld, jcurval);
	  if (curval.ptr)
	    itm->i_attrs = mom_put_attribute (itm->i_attrs, curitm, curval);
	}
    }
  if (jcontent.ptr)
    {
      momval_t curcont = mom_load_value_json (ld, jcontent);
      if (curcont.ptr)
	itm->i_content = curcont;
    }
  const momstring_t *skind = NULL;
  if (mom_is_string (jkind))
    skind = jkind.pstring;
  else if (mom_is_item (jkind))
    skind = mom_item_get_name (jkind.pitem);
  for (unsigned kix = 1; kix < mompayl__last; kix++)
    {
      struct mom_payload_descr_st *payld = mom_payloadescr[kix];
      if (!payld)
	continue;
      assert (payld->dpayl_magic == MOM_PAYLOAD_MAGIC);
      assert (payld->dpayl_name != NULL && payld->dpayl_name[0]);
      if (mom_string_same ((momval_t) skind, payld->dpayl_name)
	  && payld->dpayl_loadfun)
	{
	  payld->dpayl_loadfun (ld, itm, jpayl);
	  break;
	}
    }
}

static bool
add_loaded_item_mom (struct mom_loader_st *ld, const momitem_t *litm)
{
  assert (ld != NULL && ld->ldr_magic == LOADER_MAGIC);
  assert (litm != NULL && litm->i_typnum == momty_item);
  unsigned siz = ld->ldr_hsize;
  const momitem_t **htbl = ld->ldr_htable;
  assert (siz > 0 && htbl != NULL && ld->ldr_hcount < siz);
  unsigned lh = litm->i_hash;
  assert (lh != 0);
  unsigned istart = lh % siz;
  for (unsigned ix = istart; ix < siz; ix++)
    {
      if (!htbl[ix])
	{
	  htbl[ix] = litm;
	  ld->ldr_hcount++;
	  return true;
	}
      else if (htbl[ix] == litm)
	return false;
    }
  for (unsigned ix = 0; ix < istart; ix++)
    {
      if (!htbl[ix])
	{
	  htbl[ix] = litm;
	  ld->ldr_hcount++;
	  return true;
	}
      else if (htbl[ix] == litm)
	return false;
    }
  // should never reach here
  MOM_FATAPRINTF ("load htable is corrupted and full");
  return false;
}

momitem_t *
mom_load_item_json (struct mom_loader_st *ld, const momval_t jval)
{
  momitem_t *litm = NULL;
  if (!jval.ptr || !ld)
    return NULL;
  assert (ld->ldr_magic == LOADER_MAGIC);
  if (mom_is_item (jval))
    litm = jval.pitem;
  else if (mom_is_string (jval))
    {
      if (mom_looks_like_random_id_cstr (jval.pstring->cstr, NULL))
	litm = mom_make_item_of_ident (jval.pstring);
      else if (isalpha (jval.pstring->cstr[0]))
	litm = mom_get_item_of_name_hash (jval.pstring->cstr,
					  jval.pstring->hash);
      if (litm->i_space == momspa_none && ld->ldr_curspace != momspa_none)
	((momitem_t *) litm)->i_space = ld->ldr_curspace;
    }
  else if (mom_jsonob_get (jval, (momval_t) mom_named__jtype).pitem ==
	   mom_named__item_ref)
    {
      momval_t idrefv = mom_jsonob_get (jval, (momval_t) mom_named__item_ref);
      momval_t spav = mom_jsonob_get (jval, (momval_t) mom_named__space);
      if (mom_is_string (idrefv))
	{
	  if (mom_looks_like_random_id_cstr (idrefv.pstring->cstr, NULL))
	    litm = mom_make_item_of_ident (idrefv.pstring);
	  else if (isalpha (idrefv.pstring->cstr[0]))
	    litm = mom_get_item_of_name_hash (idrefv.pstring->cstr,
					      idrefv.pstring->hash);
	  if (MOM_UNLIKELY (litm->i_space == momspa_none
			    && mom_is_string (spav)))
	    {
	      struct mom_spacedescr_st *spad = NULL;
	      for (unsigned spix = momspa_root; spix <= momspa__last; spix++)
		if ((spad = mom_spacedescr_array[spix]) != NULL)
		  {
		    assert (spad->space_magic == MOM_SPACE_MAGIC);
		    assert (spad->space_index == spix);
		    assert (spad->space_name != NULL);
		    if (!strcmp (spad->space_name, spav.pstring->cstr))
		      {
			((momitem_t *) litm)->i_space = spix;
			break;
		      }
		  };
	    }
	}
    }
  if (MOM_UNLIKELY (4 * ld->ldr_hcount + 10 > 3 * ld->ldr_hsize))
    {
      // grow the hash table
      const momitem_t **oldhtbl = ld->ldr_htable;
      unsigned oldsize = ld->ldr_hsize;
      unsigned oldcount = ld->ldr_hcount;
      unsigned newsize = (((3 * oldcount / 2) + 50) | 0x1f) + 1;
      assert (newsize > oldsize);
      const momitem_t **newhtbl =
	MOM_GC_ALLOC ("load hash table", newsize * sizeof (momitem_t *));
      ld->ldr_hsize = newsize;
      ld->ldr_htable = newhtbl;
      ld->ldr_hcount = 0;
      for (unsigned oix = 0; oix < oldsize; oix++)
	{
	  const momitem_t *curolditm = oldhtbl[oix];
	  if (!curolditm)
	    continue;
	  if (!add_loaded_item_mom (ld, curolditm))
	    MOM_FATAPRINTF ("corrupted old load hash table");
	}
      assert (ld->ldr_hcount == oldcount);
    }
  // add and enqueue the item if new
  if (add_loaded_item_mom (ld, litm))
    {
      struct mom_itqueue_st *elq =
	MOM_GC_ALLOC ("load queue element", sizeof (struct mom_itqueue_st));
      elq->iq_item = (momitem_t *) litm;
      if (MOM_UNLIKELY (!ld->ldr_qfirst))
	{
	  ld->ldr_qfirst = ld->ldr_qlast = elq;
	}
      else
	{
	  assert (ld->ldr_qlast != NULL);
	  ld->ldr_qlast->iq_next = elq;
	  ld->ldr_qlast = elq;
	}
    }
  return litm;
}

static const momseqitem_t *
load_seqitem_json_mom (unsigned mtyp, struct mom_loader_st *ld,
		       const momjsonarray_t *jarr)
{
  const momseqitem_t *seqres = NULL;
  assert (mtyp == momty_set || mtyp == momty_tuple);
  assert (ld && ld->ldr_magic == LOADER_MAGIC);
  assert (jarr && jarr->typnum == momty_jsonarray);
  unsigned jlen = jarr->slen;
  const momitem_t **arritems = NULL;
  const momitem_t *tinyarr[MOM_TINY_MAX] = { NULL };
  if (jlen < MOM_TINY_MAX)
    arritems = tinyarr;
  else
    arritems =
      MOM_GC_ALLOC ("loaded items in sequence", jlen * sizeof (momitem_t *));
  for (unsigned ix = 0; ix < jlen; ix++)
    arritems[ix] = mom_load_item_json (ld, jarr->jarrtab[ix]);
  if (mtyp == momty_set)
    seqres = mom_make_set_from_array (jlen, arritems);
  else if (mtyp == momty_tuple)
    seqres = mom_make_tuple_from_array (jlen, arritems);
  else
    MOM_FATAPRINTF ("corrupted mtyp#%d", mtyp);
  if (arritems != tinyarr)
    MOM_GC_FREE (arritems);
  return seqres;
}

momval_t
mom_load_value_json (struct mom_loader_st *ld, const momval_t jval)
{
  momval_t jres = MOM_NULLV;
  if (!jval.ptr)
    return MOM_NULLV;
  if (!ld)
    return MOM_NULLV;
  assert (ld->ldr_magic == LOADER_MAGIC);
  unsigned jvaltype = *jval.ptype;
  switch ((enum momvaltype_en) jvaltype)
    {
    case momty_int:
    case momty_double:
    case momty_string:
    case momty_item:
      return jval;
    case momty_null:
    case momty_set:
    case momty_tuple:
    case momty_node:
    case momty_jsonarray:
      MOM_WARNPRINTF ("unexpected jvaltype#%d", jvaltype);
      return MOM_NULLV;
    case momty_jsonobject:
      {
	momval_t jtypv = mom_jsonob_get (jval, (momval_t) mom_named__jtype);
	/// chunked strings
	if (jtypv.pitem == mom_named__string)
	  {
	    momval_t jarrchk =
	      mom_jsonob_get (jval, (momval_t) mom_named__string);
	    unsigned nbchk = mom_json_array_size (jarrchk);
	    unsigned cumsiz = 0;
	    for (unsigned cix = 0; cix < nbchk; cix++)
	      {
		momval_t curchk = mom_json_array_nth (jarrchk, cix);
		cumsiz += mom_string_slen (curchk);
	      }
	    char *buf = MOM_GC_SCALAR_ALLOC ("chunk buffer", cumsiz + 2);
	    unsigned curoff = 0;
	    for (unsigned cix = 0; cix < nbchk; cix++)
	      {
		momval_t curchk = mom_json_array_nth (jarrchk, cix);
		if (!mom_is_string (curchk))
		  continue;
		unsigned chklen = mom_string_slen (curchk);
		assert (curoff + chklen <= cumsiz);
		memcpy (buf + curoff, mom_string_cstr (curchk), chklen);
		curoff += chklen;
	      }
	    assert (curoff == cumsiz);
	    jres = (momval_t) mom_make_string_len (buf, cumsiz);
	    MOM_GC_FREE (buf);
	  }
	else if (jtypv.pitem == mom_named__item_ref)
	  {
	    momitem_t *curitm = mom_load_item_json (ld, jval);
	    jres = (momval_t) curitm;
	  }
	else if (jtypv.pitem == mom_named__json_array)
	  {
	    jres = mom_jsonob_get (jval, (momval_t) mom_named__json_array);
	  }
	else if (jtypv.pitem == mom_named__json_object)
	  {
	    jres = mom_jsonob_get (jval, (momval_t) mom_named__json_object);
	  }
	else if (jtypv.pitem == mom_named__set)
	  {
	    momval_t jset = mom_jsonob_get (jval, (momval_t) mom_named__set);
	    if (mom_type (jset) == momty_jsonarray)
	      jres =
		(momval_t) load_seqitem_json_mom (momty_set, ld,
						  jset.pjsonarr);
	  }
	else if (jtypv.pitem == mom_named__tuple)
	  {
	    momval_t jtup =
	      mom_jsonob_get (jval, (momval_t) mom_named__tuple);
	    if (mom_type (jtup) == momty_jsonarray)
	      jres =
		(momval_t) load_seqitem_json_mom (momty_tuple, ld,
						  jtup.pjsonarr);
	  }
	else if (jtypv.pitem == mom_named__node)
	  {
	    momval_t jnode =
	      mom_jsonob_get (jval, (momval_t) mom_named__node);
	    momval_t jsons =
	      mom_jsonob_get (jval, (momval_t) mom_named__sons);
	    const momitem_t *connitm = mom_load_item_json (ld, jnode);
	    if (connitm != NULL)
	      {
		unsigned nbsons = mom_json_array_size (jsons);
		momval_t tinysons[MOM_TINY_MAX] = { MOM_NULLV };
		momval_t *sons = (nbsons < MOM_TINY_MAX) ? tinysons
		  : MOM_GC_ALLOC ("sons in node", nbsons * sizeof (momval_t));
		for (unsigned ix = 0; ix < nbsons; ix++)
		  sons[ix] =
		    mom_load_value_json (ld, mom_json_array_nth (jsons, ix));
		jres = (momval_t) mom_make_node_sized (connitm, nbsons, sons);
		if (sons != tinysons)
		  MOM_GC_FREE (sons);
	      }
	  }
      }
    }
  return jres;
}


static const char *
fetch_param_mom (struct mom_loader_st *ld, const char *parname)
{
  const char *res = NULL;
  assert (ld && ld->ldr_magic == LOADER_MAGIC);
  // parname at index 1
  if (sqlite3_bind_text
      (ld->ldr_sqlstmt_param_fetch, 1, parname, -1, SQLITE_STATIC))
    MOM_FATAL ("failed to bind parrame: %s", sqlite3_errmsg (ld->ldr_sqlite));
  int stepres = sqlite3_step (ld->ldr_sqlstmt_param_fetch);
  if (stepres == SQLITE_ROW)
    {
      const char *colparam = (const char *)
	sqlite3_column_text (ld->ldr_sqlstmt_param_fetch, 0);
      assert (colparam != NULL);
      res = MOM_GC_STRDUP ("load paramvalue", colparam);
    }
  sqlite3_reset (ld->ldr_sqlstmt_param_fetch);
  return res;
}


static int
compare_predefined_mom (const void *p1, const void *p2)
{
  const momitem_t *itm1 = *(const momitem_t **) p1;
  const momitem_t *itm2 = *(const momitem_t **) p2;
  assert (itm1 && itm1->i_typnum == momty_item
	  && itm1->i_magic == MOM_ITEM_MAGIC);
  assert (itm1->i_space == momspa_predefined);
  assert (itm2 && itm2->i_typnum == momty_item
	  && itm2->i_magic == MOM_ITEM_MAGIC);
  assert (itm2->i_space == momspa_predefined);
  const char *s1 =
    mom_string_cstr ((momval_t)
		     mom_item_get_name_or_idstr ((momitem_t *) itm1));
  const char *s2 =
    mom_string_cstr ((momval_t)
		     mom_item_get_name_or_idstr ((momitem_t *) itm2));
  assert (s1 != NULL && *s1);
  assert (s2 != NULL && *s2);
  return strcmp (s1, s2);
}



////////////////////////////////////////////////////////////////
void
mom_initial_load (const char *ldirnam)
{
  char filpath[MOM_PATH_MAX];
  memset (filpath, 0, sizeof (filpath));
  double startrealtime = mom_clock_time (CLOCK_REALTIME);
  double startcputime = mom_clock_time (CLOCK_PROCESS_CPUTIME_ID);
  if (!ldirnam || !ldirnam[0])
    {
      char dirpath[MOM_PATH_MAX];
      memset (dirpath, 0, sizeof (dirpath));
      MOM_WARNPRINTF ("forcing load directory to current directory: %s",
		      getcwd (dirpath, sizeof (dirpath)));
      ldirnam = ".";
    };
  snprintf (filpath, sizeof (filpath), "%s/%s.dbsqlite", ldirnam,
	    MOM_STATE_FILE_BASENAME);
  if (access (filpath, R_OK))
    MOM_FATAPRINTF ("cannot access initial state file %s", filpath);
  struct mom_loader_st ldr;
  memset (&ldr, 0, sizeof (ldr));
  ldr.ldr_magic = LOADER_MAGIC;
  const int inisiz = 64;
  ldr.ldr_hsize = inisiz;
  ldr.ldr_htable =
    MOM_GC_ALLOC ("initial loader hashtable", inisiz * sizeof (momitem_t *));
  ldr.ldr_hsize = inisiz;
  ldr.ldr_hcount = 0;
  ldr.ldr_dirpath = MOM_GC_STRDUP ("initial loader dirpath", ldirnam);
  /// open the database
  int errcod =
    sqlite3_open_v2 (filpath, &ldr.ldr_sqlite, SQLITE_OPEN_READONLY, NULL);
  if (errcod)
    MOM_FATAPRINTF ("failed to open loaded state sqlite3 file %s: %s",
		    filpath, sqlite3_errmsg (ldr.ldr_sqlite));
  ldr.ldr_sqlpath = MOM_GC_STRDUP ("initial loader sqldb", filpath);
  /// prepare some statements
  if (sqlite3_prepare_v2 (ldr.ldr_sqlite,
			  "SELECT parvalue FROM t_params WHERE parname = ?1",
			  -1, &ldr.ldr_sqlstmt_param_fetch, NULL))
    MOM_FATAPRINTF ("failed to prepare fetchparam query: %s",
		    sqlite3_errmsg (ldr.ldr_sqlite));
  if (sqlite3_prepare_v2 (ldr.ldr_sqlite,
			  "SELECT name, n_idstr, n_spacename FROM t_names ORDER BY name",
			  -1, &ldr.ldr_sqlstmt_named_fetch, NULL))
    MOM_FATAPRINTF ("failed to prepare fetchparam query: %s",
		    sqlite3_errmsg (ldr.ldr_sqlite));
  if (sqlite3_prepare_v2 (ldr.ldr_sqlite,
			  "SELECT itm_jdata FROM t_items WHERE itm_idstr = ?1",
			  -1, &ldr.ldr_sqlstmt_item_fetch, NULL))
    MOM_FATAPRINTF ("failed to prepare fetchitem query: %s",
		    sqlite3_errmsg (ldr.ldr_sqlite));
  /// check the version
  const char *vers = fetch_param_mom (&ldr, MOM_VERSION_PARAM);
  MOM_DEBUGPRINTF (load, "vers=%s", vers);
  if (MOM_UNLIKELY (!vers || strcmp (vers, MOM_DUMP_VERSION)))
    MOM_FATAPRINTF ("incompatible version in %s, expected %s got %s",
		    ldr.ldr_sqlpath, MOM_DUMP_VERSION,
		    vers ? vers : "*nothing*");
  /// load the named items
  int namedrowcount = 0;
  for (;;)
    {
      int stepres = sqlite3_step (ldr.ldr_sqlstmt_named_fetch);
      if (stepres == SQLITE_DONE)
	break;
      else if (stepres == SQLITE_ROW)
	{
	  namedrowcount++;
	  const unsigned char *rowname =
	    sqlite3_column_text (ldr.ldr_sqlstmt_named_fetch, 0);
	  const unsigned char *rowidstr =
	    sqlite3_column_text (ldr.ldr_sqlstmt_named_fetch, 1);
	  const unsigned char *rowspacename =
	    sqlite3_column_text (ldr.ldr_sqlstmt_named_fetch, 2);
	  MOM_DEBUGPRINTF (load,
			   "namedrowcount#%d rowname:%s rowidstr:%s rowspacename:%s",
			   namedrowcount, rowname, rowidstr, rowspacename);
	  if (!mom_looks_like_random_id_cstr ((const char *) rowidstr, NULL))
	    {
	      MOM_WARNPRINTF ("loading: strange idstr %s for name %s",
			      rowidstr, rowname);
	      continue;
	    }
	  momitem_t *curnameditm =
	    mom_make_item_of_identcstr ((const char *) rowidstr);
	  mom_register_item_named_cstr (curnameditm, (const char *) rowname);
	  if (rowspacename && rowspacename[0])
	    {
	      struct mom_spacedescr_st *spad = NULL;
	      for (unsigned spix = momspa_root; spix <= momspa__last; spix++)
		if ((spad = mom_spacedescr_array[spix]) != NULL)
		  {
		    assert (spad->space_magic == MOM_SPACE_MAGIC);
		    assert (spad->space_index == spix);
		    assert (spad->space_name != NULL);
		    if (!strcmp
			(spad->space_name, (const char *) rowspacename))
		      {
			curnameditm->i_space = spix;
			break;
		      }
		  };
	    }
	  else
	    MOM_WARNPRINTF ("loading: item of id %s named %s without space",
			    rowidstr, rowname);
	  if (!add_loaded_item_mom (&ldr, curnameditm))
	    MOM_FATAPRINTF ("failed to add named item %s of id %s", rowname,
			    rowidstr);
	  struct mom_itqueue_st *elq =
	    MOM_GC_ALLOC ("load queue named element",
			  sizeof (struct mom_itqueue_st));
	  elq->iq_item = (momitem_t *) curnameditm;
	  if (MOM_UNLIKELY (!ldr.ldr_qfirst))
	    {
	      ldr.ldr_qfirst = ldr.ldr_qlast = elq;
	    }
	  else
	    {
	      assert (ldr.ldr_qlast != NULL);
	      ldr.ldr_qlast->iq_next = elq;
	      ldr.ldr_qlast = elq;
	    }
	}
      else
	MOM_FATAPRINTF ("failed to step on names: %s",
			sqlite3_errmsg (ldr.ldr_sqlite));
    }
  sqlite3_reset (ldr.ldr_sqlstmt_named_fetch);
  bool spaceinited[momspa__last + 1] = { false };
  memset (spaceinited, 0, sizeof (spaceinited));
  /// loop on the queue of items
  long loadloopcount = 0;
  while (ldr.ldr_qfirst != NULL)
    {
      momitem_t *curlitm = ldr.ldr_qfirst->iq_item;
      assert (curlitm && curlitm->i_typnum == momty_item);
      if (ldr.ldr_qfirst == ldr.ldr_qlast)
	ldr.ldr_qfirst = ldr.ldr_qlast = NULL;
      else
	ldr.ldr_qfirst = ldr.ldr_qfirst->iq_next;
      loadloopcount++;
      unsigned ixspace = curlitm->i_space;
      MOM_DEBUG (load, MOMOUT_LITERAL ("load loop#"),
		 MOMOUT_DEC_INT ((int) loadloopcount),
		 MOMOUT_LITERAL ("; current loaded item:"),
		 MOMOUT_ITEM ((const momitem_t *) curlitm),
		 MOMOUT_LITERAL (" ixspace#"),
		 MOMOUT_DEC_INT ((int) ixspace));
      assert (curlitm != NULL && curlitm->i_typnum == momty_item);
      assert (ixspace < momspa__last);
      struct mom_spacedescr_st *spad = mom_spacedescr_array[ixspace];
      assert (spad && spad->space_magic == MOM_SPACE_MAGIC);
      if (!spaceinited[ixspace] && spad->space_init_load_fun)
	{
	  spaceinited[ixspace] = true;
	  MOM_DEBUGPRINTF (load, "initialize load space %s #%d",
			   spad->space_name, ixspace);
	  spad->space_init_load_fun (&ldr, ixspace);
	};
      if (spad->space_fetch_load_item_fun != NULL)
	{
	  const char *datastr =
	    spad->space_fetch_load_item_fun (&ldr, curlitm);
	  if (datastr && datastr[0])
	    {
	      MOM_DEBUG (load, MOMOUT_LITERAL ("load item:"),
			 MOMOUT_ITEM ((const momitem_t *) curlitm),
			 MOMOUT_LITERAL (" datastr:"),
			 MOMOUT_LITERALV (datastr),
			 //MOMOUT_LITERAL (" backtrace:"), MOMOUT_NEWLINE (), MOMOUT_BACKTRACE (5), 
			 MOMOUT_NEWLINE ());
	      FILE *fdata =
		fmemopen ((void *) datastr, strlen (datastr), "r");
	      if (!fdata)
		MOM_FATAPRINTF ("failed to fmemopen datastr %s", datastr);
	      struct jsonparser_st jparser = { 0 };
	      memset (&jparser, 0, sizeof (jparser));
	      mom_initialize_json_parser (&jparser, fdata, NULL);
	      char *errmsgj = NULL;
	      momval_t jdata = mom_parse_json (&jparser, &errmsgj);
	      if (errmsgj)
		MOM_WARNPRINTF
		  ("failed to parse item %s JSON data in space %s, got %s",
		   mom_string_cstr ((momval_t) mom_item_get_idstr (curlitm)),
		   spad->space_name, errmsgj);
	      else
		{
		  MOM_DEBUG (load, MOMOUT_LITERAL ("load inside item:"),
			     MOMOUT_ITEM ((const momitem_t *) curlitm),
			     MOMOUT_LITERAL (" jdata="),
			     MOMOUT_VALUE (jdata));
		  load_data_inside_item_mom (&ldr, curlitm, jdata);
		}
	      mom_close_json_parser (&jparser);
	    }
	}
    };
  MOM_INFORMPRINTF ("loaded %ld items from dir %s", loadloopcount,
		    ldr.ldr_dirpath);
  // finalize the spaces if needed
  for (unsigned spix = 1; spix < momspa__last; spix++)
    {
      struct mom_spacedescr_st *spad = mom_spacedescr_array[spix];
      if (!spad)
	continue;
      assert (spad && spad->space_magic == MOM_SPACE_MAGIC);
      if (spaceinited[spix] && spad->space_fini_load_fun)
	{
	  MOM_DEBUGPRINTF (load, "finalize load space %s #%d",
			   spad->space_name, spix);
	  spad->space_fini_load_fun (&ldr, spix);
	}
    }
  // finalize the sqlite3 statements
  sqlite3_finalize (ldr.ldr_sqlstmt_param_fetch),
    ldr.ldr_sqlstmt_param_fetch = NULL;
  sqlite3_finalize (ldr.ldr_sqlstmt_named_fetch),
    ldr.ldr_sqlstmt_named_fetch = NULL;
  sqlite3_finalize (ldr.ldr_sqlstmt_item_fetch), ldr.ldr_sqlstmt_item_fetch =
    NULL;
  int errclo = sqlite3_close_v2 (ldr.ldr_sqlite);
  if (errclo != SQLITE_OK)
    MOM_FATAPRINTF ("failed to close loaded sqlite3 %s: %s", ldr.ldr_sqlpath,
		    sqlite3_errstr (errclo));
  double endrealtime = mom_clock_time (CLOCK_REALTIME);
  double endcputime = mom_clock_time (CLOCK_PROCESS_CPUTIME_ID);
  MOM_INFORMPRINTF
    ("end of load of %ld items, %d named from sqlite3 %s in %.3f real, %.3f cpu sec.",
     loadloopcount, namedrowcount, ldr.ldr_sqlpath,
     endrealtime - startrealtime, endcputime - startcputime);
}



static void
set_dump_param_mom (struct mom_dumper_st *du, const char *parname,
		    const char *parval)
{
  assert (du && du->dmp_magic == DUMPER_MAGIC);
  if (!parname || !parval || !parname[0] || !parval[0])
    return;
  // parname at index 1
  if (sqlite3_bind_text
      (du->dmp_sqlstmt_param_insert, 1, parname, -1, SQLITE_STATIC))
    MOM_FATAPRINTF ("failed to bind dump parrame: %s",
		    sqlite3_errmsg (du->dmp_sqlite));
  // parval at index 2
  if (sqlite3_bind_text
      (du->dmp_sqlstmt_param_insert, 2, parval, -1, SQLITE_STATIC))
    MOM_FATAPRINTF ("failed to bind dump parval: %s",
		    sqlite3_errmsg (du->dmp_sqlite));
  int stepres = sqlite3_step (du->dmp_sqlstmt_param_insert);
  if (stepres != SQLITE_DONE)
    MOM_FATAPRINTF ("failed to insert dump parameter %s: %s", parname,
		    sqlite3_errmsg (du->dmp_sqlite));
  sqlite3_reset (du->dmp_sqlstmt_param_insert);
}



void
mom_full_dump (const char *reason, const char *dumpdir,
	       struct mom_dumpoutcome_st *outd)
{
  FILE *filpredefh = NULL;
  char filpath[MOM_PATH_MAX];
  char tempsuffix[32];
  memset (filpath, 0, sizeof (filpath));
  memset (tempsuffix, 0, sizeof (tempsuffix));
  double startrealtime = mom_clock_time (CLOCK_REALTIME);
  double startcputime = mom_clock_time (CLOCK_PROCESS_CPUTIME_ID);
  MOM_DEBUGPRINTF (dump,
		   "mom_full_dump reason=%s startrealtime=%.3f startcputime=%.3f",
		   reason, startrealtime, startcputime);
  snprintf (tempsuffix, sizeof (tempsuffix), "p%dr%x%%", (int) getpid (),
	    mom_random_32 () & 0xfffff);
  MOM_DEBUGPRINTF (dump,
		   "start mom_full_dump reason=%s dumpdir=%s tempsuffix=%s",
		   reason, dumpdir, tempsuffix);
  if (outd)
    memset (outd, 0, sizeof (struct mom_dumpoutcome_st));
  if (!dumpdir || !dumpdir[0])
    {
      MOM_WARNPRINTF ("setting dump directory to current: %s",
		      getcwd (filpath, sizeof (filpath)));
      dumpdir = ".";
    }
  else if (strlen (dumpdir) > MOM_PATH_MAX - 48)
    MOM_FATAPRINTF ("too long dump directory path %s", dumpdir);
  memset (filpath, 0, sizeof (filpath));
  /// lock the mutex
  pthread_mutex_lock (&dump_mtx_mom);
  MOM_INFORMPRINTF ("start full dump reason=%s dumpdir=%s", reason, dumpdir);
  /// try to make the dump directory if it does not exist
  if (access (dumpdir, F_OK) && dumpdir[0] != '.' && dumpdir[0] != '_')
    {
      if (mkdir (dumpdir, 0750))
	MOM_FATAPRINTF ("failed to make dump directory %s", dumpdir);
      else
	MOM_INFORMPRINTF ("made dump directory %s", dumpdir);
    }
  /// open the database with the tempsuffix
  snprintf (filpath, sizeof (filpath), "%s/%s.dbsqlite", dumpdir,
	    MOM_STATE_FILE_BASENAME);
  const char *sqlite3path = MOM_GC_STRDUP ("sqlite3 path", filpath);
  memset (filpath, 0, sizeof (filpath));
  snprintf (filpath, sizeof (filpath), "%s+%s", sqlite3path, tempsuffix);
  sqlite3 *sqlite3dump = NULL;
  if (sqlite3_open_v2 (filpath, &sqlite3dump,
		       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
    MOM_FATAPRINTF ("failed to open dump state sqlite3 file %s: %s",
		    filpath, sqlite3_errmsg (sqlite3dump));
  memset (filpath, 0, sizeof (filpath));
  /// open the predefined header with the tempsuffix
  snprintf (filpath, sizeof (filpath), "%s/%s", dumpdir,
	    MOM_PREDEFINED_HEADER_FILENAME);
  char *predefhpath =
    (char *) MOM_GC_STRDUP ("predefined header path", (char *) filpath);
  snprintf (filpath, sizeof (filpath), "%s+%s", predefhpath, tempsuffix);
  filpredefh = fopen (filpath, "w");
  if (!filpredefh)
    MOM_FATAPRINTF ("failed to open predefined header file %s", filpath);
  memset (filpath, 0, sizeof (filpath));
  /// create & initialize the dumper
  struct mom_dumper_st dmp = { 0 };
  memset (&dmp, 0, sizeof (struct mom_dumper_st));
  const unsigned siz = 512;
  const momitem_t **arr
    = MOM_GC_ALLOC ("dumper array", siz * sizeof (momitem_t *));
  dmp.dmp_array = arr;
  dmp.dmp_size = siz;
  dmp.dmp_count = 0;
  const unsigned predefsiz = 32;
  dmp.dmp_predefarray =
    MOM_GC_ALLOC ("dumper predefined array",
		  predefsiz * sizeof (momitem_t *));
  dmp.dmp_predefsize = predefsiz;
  dmp.dmp_dirpath = MOM_GC_STRDUP ("dumped directory", dumpdir);
  dmp.dmp_sqlpath = sqlite3path;
  dmp.dmp_sqlite = sqlite3dump;
  dmp.dmp_qfirst = dmp.dmp_qlast = NULL;
  dmp.dmp_magic = DUMPER_MAGIC;
  dmp.dmp_state = dus_scan;
  char *errmsg = NULL;
  /// create the initial tables
  if (sqlite3_exec
      (dmp.dmp_sqlite,
       "CREATE TABLE IF NOT EXISTS t_params (parname VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE,"
       " parvalue TEXT NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("in dumped db %s failed to create t_params %s",
		    dmp.dmp_sqlpath, errmsg);
  if (sqlite3_exec
      (dmp.dmp_sqlite,
       "CREATE TABLE IF NOT EXISTS t_items (itm_idstr VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,"
       " itm_jdata TEXT NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("in dumped db %s failed to create t_items %s",
		    dmp.dmp_sqlpath, errmsg);
  if (sqlite3_exec
      (dmp.dmp_sqlite,
       "CREATE TABLE IF NOT EXISTS t_names (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE,"
       // the id can be external so does not always REFERENCES
       // t_items(itm_idstr)
       " n_idstr VARCHAR(30) UNIQUE NOT NULL,"
       " n_spacename VARCHAR(20) NOT NULL)", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("failed to create t_names: %s", errmsg);
  if (sqlite3_exec
      (dmp.dmp_sqlite,
       "CREATE TABLE IF NOT EXISTS t_modules (modname VARCHAR(100) PRIMARY KEY ASC NOT NULL UNIQUE)",
       NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("failed to create t_modules: %s", errmsg);
  // it is important that the entire dump is a single transaction,
  // otherwise it is much more slow...
  if (sqlite3_exec (dmp.dmp_sqlite, "BEGIN TRANSACTION", NULL, NULL, &errmsg))
    MOM_FATAPRINTF ("failed to BEGIN TRANSACTION: %s", errmsg);
  /// prepare some statements
  if (sqlite3_prepare_v2 (dmp.dmp_sqlite,
			  "INSERT INTO t_items (itm_idstr, itm_jdata) VALUES (?1, ?2)",
			  -1, &dmp.dmp_sqlstmt_item_insert, NULL))
    MOM_FATAPRINTF ("failed to prepare item insert query: %s",
		    sqlite3_errmsg (dmp.dmp_sqlite));
  if (sqlite3_prepare_v2 (dmp.dmp_sqlite,
			  "INSERT INTO t_names (name, n_idstr, n_spacename) VALUES (?1, ?2, ?3)",
			  -1, &dmp.dmp_sqlstmt_name_insert, NULL))
    MOM_FATAPRINTF ("failed to prepare name insert query: %s",
		    sqlite3_errmsg (dmp.dmp_sqlite));
  if (sqlite3_prepare_v2 (dmp.dmp_sqlite,
			  "INSERT INTO t_params (parname, parvalue) VALUES (?1, ?2)",
			  -1, &dmp.dmp_sqlstmt_param_insert, NULL))
    MOM_FATAPRINTF ("failed to prepare param insert query: %s",
		    sqlite3_errmsg (dmp.dmp_sqlite));
  if (sqlite3_prepare_v2 (dmp.dmp_sqlite,
			  "INSERT OR IGNORE INTO t_modules (modname) VALUES (?1)",
			  -1, &dmp.dmp_sqlstmt_module_insert, NULL))
    MOM_FATAPRINTF ("failed to prepare module insert query: %s",
		    sqlite3_errmsg (dmp.dmp_sqlite));
  ///
  momval_t tupnameditems = MOM_NULLV, jarrnam = MOM_NULLV;
  tupnameditems =
    (momval_t) mom_alpha_ordered_tuple_of_named_items (&jarrnam);
  MOM_DEBUG (dump, MOMOUT_LITERAL ("tuple of named:"),
	     MOMOUT_VALUE (tupnameditems));
  unsigned nbnameditems = mom_tuple_length (tupnameditems);
  assert (nbnameditems > 0 && nbnameditems == mom_json_array_size (jarrnam));
  mom_dump_scan_value (&dmp, tupnameditems);
  /// scanning loop
  memset (filpath, 0, sizeof (filpath));
  long scancount = 0;
  while (dmp.dmp_qfirst != NULL)
    {
      struct mom_itqueue_st *qel = dmp.dmp_qfirst;
      if (MOM_UNLIKELY (qel == dmp.dmp_qlast))
	{
	  dmp.dmp_qfirst = dmp.dmp_qlast = NULL;
	}
      else
	dmp.dmp_qfirst = qel->iq_next;
      momitem_t *curitm = qel->iq_item;
      assert (curitm != NULL && curitm->i_typnum == momty_item);
      MOM_DEBUG (dump, MOMOUT_LITERAL ("scanning item:"),
		 MOMOUT_ITEM ((const momitem_t *) curitm));
      mom_dump_scan_inside_item (&dmp, curitm);
      scancount++;
    }
  MOM_DEBUGPRINTF (dump, "final scancount=%ld", scancount);
  /// emit loop
  {
    dmp.dmp_state = dus_emit;
    bool spaceinited[momspa__last] = { false };
    memset (spaceinited, 0, sizeof (spaceinited));
    unsigned dmpsize = dmp.dmp_size;
    for (unsigned dix = 0; dix < dmpsize; dix++)
      {
	momitem_t *curitm = (momitem_t *) dmp.dmp_array[dix];
	if (!curitm || curitm == MOM_EMPTY)
	  continue;
	MOM_DEBUG (load, MOMOUT_LITERAL ("current dumped item:"),
		   MOMOUT_ITEM ((const momitem_t *) curitm));
	assert (curitm->i_typnum == momty_item
		&& curitm->i_magic == MOM_ITEM_MAGIC);
	pthread_mutex_lock (&curitm->i_mtx);
	unsigned ispa = curitm->i_space;
	assert (ispa < momspa__last);
	struct mom_spacedescr_st *spad = mom_spacedescr_array[ispa];
	if (!spad)
	  goto done_item;
	assert (spad != NULL && spad->space_magic == MOM_SPACE_MAGIC);
	if (!spaceinited[ispa])
	  {
	    spaceinited[ispa] = true;
	    if (spad->space_init_dump_fun)
	      spad->space_init_dump_fun (&dmp, ispa);
	  }
	if (spad->space_store_item_fun)
	  {
	    char *datastr = NULL;
	    size_t datasiz = 0;
	    struct momout_st out = { 0 };
	    // build the data string
	    momval_t datav = mom_dump_data_inside_item (&dmp, curitm);
	    FILE *fdata = open_memstream (&datastr, &datasiz);
	    if (MOM_UNLIKELY
		(!mom_initialize_output (&out, fdata, outf_jsonhalfindent)))
	      MOM_FATAPRINTF ("failed to initialize data output");
	    MOM_OUT (&out, MOMOUT_JSON_VALUE (datav), MOMOUT_NEWLINE (),
		     MOMOUT_FLUSH ());
	    fclose (fdata);
	    memset (&out, 0, sizeof (out));
	    MOM_DEBUGPRINTF (dump, "datastr=%s", datastr);
	    if (datastr)
	      {
		spad->space_store_item_fun (&dmp, curitm, datastr);
		free (datastr), datastr = NULL;
	      }
	  }
      done_item:
	pthread_mutex_unlock (&curitm->i_mtx);
      }
    /// finish all the initialized spaces
    for (unsigned six = momspa_root; six < momspa__last; six++)
      if (spaceinited[six])
	{
	  struct mom_spacedescr_st *spad = mom_spacedescr_array[six];
	  assert (spad && spad->space_magic == MOM_SPACE_MAGIC
		  && spad->space_index == six);
	  if (spad->space_fini_dump_fun)
	    spad->space_fini_dump_fun (&dmp, six);
	}
  }				// end of emit loop
  //// dump the named
  memset (filpath, 0, sizeof (filpath));
  for (unsigned nix = 0; nix < nbnameditems; nix++)
    {
      momitem_t *curnamitm = mom_tuple_nth_item (tupnameditems, nix);
      momval_t curnamstr = mom_json_array_nth (jarrnam, nix);
      MOM_DEBUG (dump, MOMOUT_LITERAL ("dumping named:"),
		 MOMOUT_ITEM ((const momitem_t *) curnamitm));
      assert (curnamitm && curnamitm->i_typnum == momty_item);
      assert (curnamitm->i_space < momspa__last);
      assert (mom_is_string (curnamstr));
      struct mom_spacedescr_st *spad =
	mom_spacedescr_array[curnamitm->i_space];
      if (!spad)
	continue;
      assert (spad->space_magic == MOM_SPACE_MAGIC);
      int errn = 0;
      // name is field 1
      errn = sqlite3_bind_text
	(dmp.dmp_sqlstmt_name_insert, 1, mom_string_cstr (curnamstr),
	 -1, SQLITE_STATIC);
      if (errn)
	MOM_FATAPRINTF ("failed to bind named item name: %s, err#%d",
			sqlite3_errmsg (dmp.dmp_sqlite), errn);
      // n_idstr is field 2
      errn = sqlite3_bind_text
	(dmp.dmp_sqlstmt_name_insert, 2,
	 mom_string_cstr ((momval_t) mom_item_get_idstr (curnamitm)), -1,
	 SQLITE_STATIC);
      if (errn)
	MOM_FATAPRINTF ("failed to bind named item idstr: %s, err#%d",
			sqlite3_errmsg (dmp.dmp_sqlite), errn);
      // n_spacename is field 3
      errn = sqlite3_bind_text
	(dmp.dmp_sqlstmt_name_insert, 3, spad->space_name, -1, SQLITE_STATIC);
      if (errn)
	MOM_FATAPRINTF ("failed to bind named item spacename: %s, err#%d",
			sqlite3_errmsg (dmp.dmp_sqlite), errn);
      errn = sqlite3_step (dmp.dmp_sqlstmt_name_insert);
      if (errn != SQLITE_DONE)
	MOM_FATAPRINTF ("failed to insert name: %s, err#%d",
			sqlite3_errmsg (dmp.dmp_sqlite), errn);
      (void) sqlite3_reset (dmp.dmp_sqlstmt_name_insert);
    }
  /// generate the predef-monimelt.h file
  memset (filpath, 0, sizeof (filpath));
  {
    struct momout_st outs = { 0 };
    memset (&outs, 0, sizeof (outs));
    MOM_DEBUGPRINTF (dump, "generating for %d predefined", dmp.dmp_predefnb);
    qsort (dmp.dmp_predefarray, dmp.dmp_predefnb, sizeof (momitem_t *),
	   compare_predefined_mom);
    mom_initialize_output (&outs, filpredefh, 0);
    MOM_OUT (&outs,
	     MOMOUT_GPLV3P_NOTICE ((const char *)
				   MOM_PREDEFINED_HEADER_FILENAME),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#ifndef MOM_PREDEFINED_NAMED"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#error missing MOM_PREDEFINED_NAMED"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#endif /*MOM_PREDEFINED_NAMED*/"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#ifndef MOM_PREDEFINED_ANONYMOUS"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#error missing MOM_PREDEFINED_ANONYMOUS"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#endif /*MOM_PREDEFINED_ANONYMOUS*/"),
	     MOMOUT_NEWLINE (), MOMOUT_NEWLINE ());
    for (unsigned pix = 0; pix < dmp.dmp_predefnb; pix++)
      {
	momitem_t *predefitm = (momitem_t *) dmp.dmp_predefarray[pix];
	assert (predefitm && predefitm->i_typnum == momty_item
		&& predefitm->i_space == momspa_predefined);
	pthread_mutex_lock (&predefitm->i_mtx);
	if (predefitm->i_name)
	  MOM_OUT (&outs, MOMOUT_LITERAL ("MOM_PREDEFINED_NAMED("),
		   MOMOUT_LITERALV (mom_string_cstr
				    ((momval_t) predefitm->i_name)),
		   MOMOUT_LITERAL (","),
		   MOMOUT_LITERALV (mom_string_cstr
				    ((momval_t) predefitm->i_idstr)),
		   MOMOUT_LITERAL (")"), MOMOUT_NEWLINE ());
	else
	  MOM_OUT (&outs, MOMOUT_LITERAL ("MOM_PREDEFINED_ANONYMOUS("),
		   MOMOUT_LITERALV (mom_string_cstr
				    ((momval_t) predefitm->i_idstr)),
		   MOMOUT_LITERAL (")"), MOMOUT_NEWLINE ());
	pthread_mutex_unlock (&predefitm->i_mtx);
      }
    MOM_OUT (&outs, MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#undef MOM_PREDEFINED_NAMED"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("#undef MOM_PREDEFINED_ANONYMOUS"),
	     MOMOUT_NEWLINE (),
	     MOMOUT_LITERAL ("// eof " MOM_PREDEFINED_HEADER_FILENAME),
	     MOMOUT_NEWLINE (), MOMOUT_FLUSH ());
    if (fclose (filpredefh))
      MOM_FATAPRINTF ("failed to close predefined header file %s",
		      dmp.dmp_predefhpath);
    filpredefh = NULL;
    {
      snprintf (filpath, sizeof (filpath), "%s%%", dmp.dmp_predefhpath);
      if (rename (filpath, dmp.dmp_predefhpath))
	MOM_FATAPRINTF ("failed to rename %s to %s", filpath,
			dmp.dmp_predefhpath);
    }
    MOM_INFORMPRINTF ("generated predefine header file %s",
		      dmp.dmp_predefhpath);
  };
  set_dump_param_mom (&dmp, MOM_VERSION_PARAM, MOM_DUMP_VERSION);
  set_dump_param_mom (&dmp, "dump_reason", reason);
  memset (filpath, 0, sizeof (filpath));
  /// at last
  goto end;
end:
  //////// finalize and close the database
  sqlite3_finalize (dmp.dmp_sqlstmt_param_insert),
    dmp.dmp_sqlstmt_param_insert = NULL;
  sqlite3_finalize (dmp.dmp_sqlstmt_item_insert),
    dmp.dmp_sqlstmt_item_insert = NULL;
  sqlite3_finalize (dmp.dmp_sqlstmt_name_insert),
    dmp.dmp_sqlstmt_name_insert = NULL;
  sqlite3_finalize (dmp.dmp_sqlstmt_module_insert),
    dmp.dmp_sqlstmt_module_insert = NULL;
  if (sqlite3_exec (dmp.dmp_sqlite, "END TRANSACTION", NULL, NULL, &errmsg))
    MOM_FATAL ("failed to END TRANSACTION: %s", errmsg);
  int errclo = sqlite3_close_v2 (dmp.dmp_sqlite);
  if (errclo != SQLITE_OK)
    MOM_FATAPRINTF ("failed to close sqlite3 %s: %s", dmp.dmp_sqlpath,
		    sqlite3_errstr (errclo));
  dmp.dmp_sqlite = NULL;
  //////// backup the old database file and rename the new
  {
    memset (filpath, 0, sizeof (filpath));
    MOM_DEBUGPRINTF (dump, "backup database %s and rename tempsuffix=%s",
		     sqlite3path, tempsuffix);
    if (!access (sqlite3path, F_OK))
      {
	snprintf (filpath, sizeof (filpath), "%s~", sqlite3path);
	if (rename (sqlite3path, filpath))
	  MOM_WARNPRINTF ("failed to backup Sqlite db %s as %s", sqlite3path,
			  filpath);
	else
	  MOM_INFORMPRINTF ("moved for backup Sqlite db %s to %s",
			    sqlite3path, filpath);
      }
    memset (filpath, 0, sizeof (filpath));
    snprintf (filpath, sizeof (filpath), "%s+%s", sqlite3path, tempsuffix);
    if (rename (filpath, sqlite3path))
      MOM_FATAPRINTF ("failed to rename database %s as %s",
		      filpath, sqlite3path);
  }
  //////// backup the old header file and rename the new
  {
    memset (filpath, 0, sizeof (filpath));
    MOM_DEBUGPRINTF (dump, "backup predefheader %s and rename tempsuffix=%s",
		     predefhpath, tempsuffix);
    if (!access (predefhpath, F_OK))
      {
	snprintf (filpath, sizeof (filpath), "%s~", predefhpath);
	if (rename (predefhpath, filpath))
	  MOM_WARNPRINTF ("failed to backup predefined headers %s as %s",
			  predefhpath, filpath);
	else
	  MOM_INFORMPRINTF
	    ("moved for backup old predefined headers %s as %s", predefhpath,
	     filpath);
      };
    memset (filpath, 0, sizeof (filpath));
    snprintf (filpath, sizeof (filpath), "%s+%s", predefhpath, tempsuffix);
    if (rename (filpath, predefhpath))
      MOM_FATAPRINTF ("failed to rename predefined headers %s as %s",
		      filpath, predefhpath);
  }
  // perhaps fork the dump command
  bool shouldump = strlen (dmp.dmp_sqlpath) < MOM_PATH_MAX - 32;
  for (const char *pc = dmp.dmp_sqlpath; *pc && shouldump; pc++)
    if (!isalnum (*pc) && *pc != '/' && *pc != '.' && *pc != '_'
	&& !(*pc == '-' && pc > dmp.dmp_sqlpath && isalnum (pc[-1])))
      shouldump = false;
  if (shouldump)
    {
      char *dumpargtab[4] = { NULL };
      dumpargtab[0] = MOM_DUMP_SCRIPT;
      dumpargtab[1] = basename (dmp.dmp_sqlpath);
      dumpargtab[2] = NULL;
      dumpargtab[3] = NULL;
      fflush (NULL);
      pid_t dumpid = fork ();
      if (dumpid < 0)
	MOM_FATAPRINTF ("failed to fork %s", MOM_DUMP_SCRIPT);
      else if (dumpid == 0)
	{			// child process
	  for (int i = 3; i < 64; i++)
	    (void) close (i);
	  close (STDIN_FILENO);
	  int nulldev = open ("/dev/null", O_RDONLY);
	  if (nulldev > 0)
	    dup2 (nulldev, STDIN_FILENO);
	  usleep (5000);
	  if (chdir (dmp.dmp_dirpath))
	    {
	      fprintf (stderr, "Failed to chdir %s: %s\n", dmp.dmp_dirpath,
		       strerror (errno));
	      exit (EXIT_FAILURE);
	    };
	  usleep (5000);
	  nice (1);
	  execvp (MOM_DUMP_SCRIPT, dumpargtab);
	  execvp (MOM_DUMP_SCRIPT2, dumpargtab);
	  perror ("exec " MOM_DUMP_SCRIPT " or " MOM_DUMP_SCRIPT2);
	  fflush (NULL);
	  _exit (127);
	};
      fflush (NULL);
      MOM_INFORMPRINTF ("started dump process #%d", (int) dumpid);
      int dumpstat = 0;
      while (waitpid (dumpid, &dumpstat, 0) < 0)
	{
	  MOM_WARNPRINTF ("failed to wait Sqlite dump process pid#%d",
			  (int) dumpid);
	  fflush (NULL);
	  sleep (2);
	}
      if (WIFEXITED (dumpstat) && WEXITSTATUS (dumpstat) > 0)
	MOM_WARNPRINTF ("dump command '%s %s' failed, exited %d",
			MOM_DUMP_SCRIPT, dumpargtab[1],
			WEXITSTATUS (dumpstat));
      else if (WIFSIGNALED (dumpstat))
	{
	  int dusig = WTERMSIG (dumpstat);
	  MOM_FATAPRINTF
	    ("dump command '%s %s' failed, terminated with signal #%d: %s",
	     MOM_DUMP_SCRIPT, dumpargtab[1], dusig, strsignal (dusig));
	}
      else
	{
	  assert (WIFEXITED (dumpstat) && WEXITSTATUS (dumpstat) == 0);
	  MOM_INFORMPRINTF ("completed successfully dump '%s %s' process #%d",
			    MOM_DUMP_SCRIPT, dumpargtab[1], (int) dumpid);
	}
    }
  else
    MOM_WARNPRINTF
      ("dont dump the database %s in SQL form because of strange path",
       dmp.dmp_sqlpath);
  pthread_mutex_unlock (&dump_mtx_mom);
  double endrealtime = mom_clock_time (CLOCK_REALTIME);
  double endcputime = mom_clock_time (CLOCK_PROCESS_CPUTIME_ID);
  MOM_INFORMPRINTF
    ("end of dump into directory %s in %.3f cpu, %.3f real seconds", dumpdir,
     endcputime - startcputime, endrealtime - startrealtime);
}


void
mom_dump_require_module (struct mom_dumper_st *du, const char *modname)
{
  if (!du || !modname || !modname[0])
    return;
  MOM_DEBUGPRINTF (dump, "mom_dump_require_module modname=%s", modname);
  for (const char *pc = modname; *pc; pc++)
    if (!isalnum (*pc) && *pc != '_')
      {
	MOM_WARNPRINTF ("invalid dumped module %s requirement", modname);
	return;
      };
  assert (du->dmp_magic == DUMPER_MAGIC);
  assert (du->dmp_sqlstmt_module_insert != NULL);
  /// modname at rank 1
  int err = sqlite3_bind_text (du->dmp_sqlstmt_module_insert, 1, modname, -1,
			       SQLITE_STATIC);
  if (err)
    MOM_FATAPRINTF ("failed to bind dumped module %s: %s, err#%d", modname,
		    sqlite3_errmsg (du->dmp_sqlite), err);
  err = sqlite3_step (du->dmp_sqlstmt_item_insert);
  if (err != SQLITE_DONE)
    MOM_WARNPRINTF ("failed to require dumped module %s: %s, err#%d", modname,
		    sqlite3_errmsg (du->dmp_sqlite), err);
}

static void
spaceroot_storeitem_mom (struct mom_dumper_st *du, momitem_t *itm,
			 const char *datastr)
{
  int err = 0;
  assert (du && du->dmp_magic == DUMPER_MAGIC);
  MOM_DEBUG (dump, MOMOUT_LITERAL ("spaceroot_storeitem_mom itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (" datastr="), MOMOUT_LITERALV (datastr));
  assert (du->dmp_sqlstmt_item_insert != NULL);
  // idstr at rank 1
  const char *idstr = mom_string_cstr ((momval_t) mom_item_get_idstr (itm));
  err = sqlite3_bind_text
    (du->dmp_sqlstmt_item_insert, 1, idstr, -1, SQLITE_STATIC);
  if (err)
    MOM_FATAPRINTF ("failed to bind dumped item idstr: %s, err#%d",
		    sqlite3_errmsg (du->dmp_sqlite), err);
  // datastr at rank 2
  err = sqlite3_bind_text
    (du->dmp_sqlstmt_item_insert, 2, datastr, -1, SQLITE_STATIC);
  if (err)
    MOM_FATAPRINTF ("failed to bind dumped item datastr: %s, err#%d",
		    sqlite3_errmsg (du->dmp_sqlite), err);
  err = sqlite3_step (du->dmp_sqlstmt_item_insert);
  if (err != SQLITE_DONE)
    MOM_FATAPRINTF ("failed to insert name: %s, err#%d",
		    sqlite3_errmsg (du->dmp_sqlite), err);
  (void) sqlite3_reset (du->dmp_sqlstmt_item_insert);
}

static void
spacepredef_storeitem_mom (struct mom_dumper_st *du, momitem_t *itm,
			   const char *datastr)
{
  assert (du && du->dmp_magic == DUMPER_MAGIC);
  MOM_DEBUG (dump, MOMOUT_LITERAL ("spacepredef_storeitem_mom itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm),
	     MOMOUT_LITERAL (" datastr="), MOMOUT_LITERALV (datastr));
  spaceroot_storeitem_mom (du, itm, datastr);
  if (MOM_UNLIKELY (du->dmp_predefnb + 1 >= du->dmp_predefsize))
    {
      const momitem_t **oldarr = du->dmp_predefarray;
      unsigned oldsiz = du->dmp_predefsize;
      unsigned newsiz = ((3 * du->dmp_predefnb / 2 + 10) | 0xf) + 1;
      const momitem_t **newarr = MOM_GC_ALLOC ("growing predef array",
					       newsiz * sizeof (momitem_t *));
      memcpy (newarr, oldarr, du->dmp_predefnb * sizeof (momitem_t *));
      memset (oldarr, 0, oldsiz * sizeof (momitem_t *));
      du->dmp_predefarray = newarr;
      du->dmp_predefsize = newsiz;
      MOM_GC_FREE (oldarr);
    }
  du->dmp_predefarray[du->dmp_predefnb++] = itm;
}


static const char *
spacerootpredef_fetch_item_mom (struct mom_loader_st *ld, momitem_t *itm)
{
  char *res = NULL;
  assert (ld && ld->ldr_magic == LOADER_MAGIC);
  assert (itm && itm->i_typnum == momty_item
	  && itm->i_magic == MOM_ITEM_MAGIC);
  assert (itm->i_space == momspa_root || itm->i_space == momspa_predefined);
  assert (ld->ldr_sqlstmt_item_fetch != NULL);
  momval_t idstrv = (momval_t) mom_item_get_idstr ((momitem_t *) itm);
  const char *ids = mom_string_cstr (idstrv);
  assert (ids != NULL && ids[0] == '_');
  MOM_DEBUG (load, MOMOUT_LITERAL ("fetch loaded item:"),
	     MOMOUT_ITEM ((const momitem_t *) itm));
  // idstr at index 1
  if (sqlite3_bind_text
      (ld->ldr_sqlstmt_item_fetch, 1, ids, -1, SQLITE_STATIC))
    MOM_FATAL ("failed to bind idstr: %s", sqlite3_errmsg (ld->ldr_sqlite));
  int stepres = sqlite3_step (ld->ldr_sqlstmt_item_fetch);
  if (stepres == SQLITE_ROW)
    {
      const char *coljdata = (const char *)
	sqlite3_column_text (ld->ldr_sqlstmt_item_fetch, 0);
      MOM_DEBUGPRINTF (load, "ids=%s coljdata=%s", ids, coljdata);
      assert (coljdata != NULL);
      res = (char *) MOM_GC_STRDUP ("load item data", coljdata);
      MOM_DEBUG (load, MOMOUT_LITERAL ("fetched data:"),
		 MOMOUT_LITERALV ((const char *) res), MOMOUT_NEWLINE ());
    }
  else
    MOM_WARNPRINTF ("failed to load item %s data (%s) - stepres#%d", ids,
		    sqlite3_errmsg (ld->ldr_sqlite), stepres);
  sqlite3_reset (ld->ldr_sqlstmt_item_fetch);
  return res;
}



static struct mom_spacedescr_st spacepredefdescr_mom =
  {.space_magic = MOM_SPACE_MAGIC,
  .space_index = momspa_predefined,
  .space_name = ".predef",
  .space_namestr = NULL,
  .space_data = NULL,
  .space_init_dump_fun = NULL,
  .space_store_item_fun = spacepredef_storeitem_mom,
  .space_fini_dump_fun = NULL,
  .space_fetch_load_item_fun = spacerootpredef_fetch_item_mom
};

static struct mom_spacedescr_st spacerootdescr_mom =
  {.space_magic = MOM_SPACE_MAGIC,
  .space_index = momspa_root,
  .space_name = ".root",
  .space_namestr = NULL,
  .space_data = NULL,
  .space_init_dump_fun = NULL,
  .space_store_item_fun = spaceroot_storeitem_mom,
  .space_fini_dump_fun = NULL,
  .space_fetch_load_item_fun = spacerootpredef_fetch_item_mom
};

struct mom_spacedescr_st *mom_spacedescr_array[momspa__last + 1] =
  {[momspa_predefined] = &spacepredefdescr_mom,
  [momspa_root] = &spacerootdescr_mom,
  NULL
};
