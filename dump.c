// file dump.c

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
#warning missing cases for node etc in mom_dump_scan_value
    default:
      if (typ > momty__itemlowtype)
	mom_dump_add_item (dmp, val.panyitem);
      return;
    }
}
