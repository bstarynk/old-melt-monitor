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

static pthread_mutex_t loadump_mtx_mom =
  PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

enum dumpstate_en
{
  dus_none = 0,
  dus_scan,
  dus_emit
};

#define DUMPER_MAGIC 0x572bb695	/* dumper magic 1462482581 */

#define LOADER_MAGIC 0x169128bb	/* loader magic 378611899 */
void
mom_dumper_initialize (struct mom_dumper_st *dmp)
{
  const unsigned siz = 512;
  memset (dmp, 0, sizeof (struct mom_dumper_st));
  const momitem_t **arr
    = MOM_GC_ALLOC ("dumper array", siz * sizeof (momitem_t *));
  dmp->dmp_array = arr;
  dmp->dmp_size = siz;
  dmp->dmp_count = 0;
  dmp->dmp_qfirst = dmp->dmp_qlast = NULL;
  dmp->dmp_magic = DUMPER_MAGIC;
  dmp->dmp_state = dus_scan;
  MOM_DEBUG (dump, "initialize dmp@%p", (void *) dmp);
}

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

static const momjsonarray_t *
jsonarray_emit_itemseq_mom (struct mom_dumper_st *dmp,
			    const struct momseqitem_st *si)
{
  unsigned slen = si->slen;
  if (slen <= MOM_TINY_MAX)
    {
      momval_t tab[MOM_TINY_MAX] = { MOM_NULLV };
      for (unsigned ix = 0; ix < slen; ix++)
	{
	  momitem_t *curitm = (momitem_t *) (si->itemseq[ix]);
	  if (curitm && curitm->i_space > 0)
	    tab[ix] = (momval_t) mom_item_get_idstr (curitm);
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
	    arr[ix] = raw_dump_emit_json_mom (dmp, (momval_t) curitm);
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
  unsigned slen = nd->slen;
  if (slen <= MOM_TINY_MAX)
    {
      momval_t tab[MOM_TINY_MAX] = { MOM_NULLV };
      for (unsigned ix = 0; ix < slen; ix++)
	tab[ix] = raw_dump_emit_json_mom (dmp, (nd->sontab[ix]));
      return mom_make_json_array_count (slen, tab);
    }
  else
    {
      momval_t *arr =
	MOM_GC_ALLOC ("jsonarray emit node", sizeof (momval_t) * slen);
      for (unsigned ix = 0; ix < slen; ix++)
	arr[ix] = raw_dump_emit_json_mom (dmp, (nd->sontab[ix]));
      const momjsonarray_t *jarr = mom_make_json_array_count (slen, arr);
      MOM_GC_FREE (arr);
      return jarr;
    }
}

static momval_t
raw_dump_emit_json_mom (struct mom_dumper_st *dmp, const momval_t val)
{
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
	const momstring_t *itemids = mom_item_get_idstr (val.pitem);
	assert (mom_is_string ((momval_t) itemids));
	jsval = (momval_t) mom_make_json_object
	  (MOMJSOB_ENTRY
	   ((momval_t) mom_named__jtype, (momval_t) mom_named__item_ref),
	   MOMJSOB_ENTRY ((momval_t) mom_named__item_ref, (momval_t) itemids),
	   MOMJSON_END);
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
	    const momstring_t *connids =
	      mom_item_get_idstr ((momitem_t *) curconn);
	    assert (mom_is_string ((momval_t) connids));
	    jsval = (momval_t) mom_make_json_object
	      (MOMJSOB_ENTRY
	       ((momval_t) mom_named__jtype, (momval_t) mom_named__node),
	       MOMJSOB_ENTRY ((momval_t) mom_named__node, (momval_t) connids),
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

const momitem_t *
mom_load_item_json (struct mom_loader_st *ld, const momval_t jval)
{
  const momitem_t *litm = NULL;
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
    }
  else if (mom_jsonob_get (jval, (momval_t) mom_named__jtype).pitem ==
	   mom_named__item_ref)
    litm = mom_load_item_json (ld,
			       mom_jsonob_get (jval,
					       (momval_t)
					       mom_named__item_ref));
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
	    momval_t jidstr =
	      mom_jsonob_get (jval, (momval_t) mom_named__item_ref);
	    if (mom_is_string (jidstr)
		&& mom_looks_like_random_id_cstr (jidstr.pstring->cstr, NULL))
	      jres = (momval_t) mom_make_item_of_ident (jidstr.pstring);
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
