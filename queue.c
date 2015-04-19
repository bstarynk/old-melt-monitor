// file queue.c

/**   Copyright (C)  2015 Free Software Foundation, Inc.
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

void
mom_queue_push_back (struct momqueueitems_st *qu, const momitem_t *itm)
{
  if (!qu)
    return;
  if (!itm || itm == MOM_EMPTY)
    return;
  struct momqueuechunk_st *bk = qu->que_back;
  if (bk)
    {
      assert (bk->quech_items[0]);	/* cannot be empty chunk */
      assert (bk->quech_next == NULL);
      for (unsigned ix = MOM_QUEUECHUNK_LEN - 1; ix > 0; ix--)
	{
	  if (!bk->quech_items[ix] && bk->quech_items[ix - 1])
	    {
	      bk->quech_items[ix] = itm;
	      qu->que_size++;
	      return;
	    }
	}
      struct momqueuechunk_st *newch	//
	= MOM_GC_ALLOC ("queue chunk", sizeof (struct momqueuechunk_st));
      bk->quech_next = newch;
      newch->quech_prev = bk;
      newch->quech_items[0] = itm;
      qu->que_size++;
      qu->que_back = bk;
      return;
    }
  else				/* !bk, empty queue */
    {
      assert (qu->que_size == 0);
      struct momqueuechunk_st *newch	//
	= MOM_GC_ALLOC ("queue chunk", sizeof (struct momqueuechunk_st));
      newch->quech_items[0] = itm;
      qu->que_front = qu->que_back = newch;
      qu->que_size = 1;
      return;
    }
}

const momitem_t *
mom_queue_pop_front (struct momqueueitems_st *qu)
{
  const momitem_t *itm = NULL;
  if (!qu || !qu->que_size)
    return NULL;
  struct momqueuechunk_st *fr = qu->que_front;
  assert (fr);
  itm = fr->quech_items[0];
  assert (itm);
  if (!fr->quech_items[1])
    {
      // front becomes empty, remove it
      memset (fr, 0, sizeof (*fr));
      MOM_GC_FREE (fr);
      qu->que_front = fr->quech_next;
      qu->que_size--;
      return itm;
    }
  else
    {
      for (unsigned ix = 1; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  fr->quech_items[ix - 1] = fr->quech_items[ix];
	  fr->quech_items[ix] = NULL;
	}
      qu->que_size--;
      return itm;
    }
}


const momseq_t *
mom_queue_tuple (struct momqueueitems_st *qu, momvalue_t metav)
{
  if (!qu)
    return NULL;
  if (qu->que_size > MOM_MAX_SEQ_LENGTH)
    MOM_FATAPRINTF ("too huge %ld queue", (long) qu->que_size);
  unsigned siz = qu->que_size;
  const momitem_t **arr =
    MOM_GC_ALLOC ("queue array", (siz + 1) * sizeof (momitem_t *));
  unsigned cnt = 0;
  for (struct momqueuechunk_st * ch = qu->que_front; ch; ch = ch->quech_next)
    {
      for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  const momitem_t *itm = ch->quech_items[ix];
	  if (itm && itm != MOM_EMPTY)
	    arr[cnt++] = itm;
	}
    };
  assert (cnt == siz);
  const momseq_t *tu = mom_make_sized_meta_tuple (metav, cnt, arr);
  memset (arr, 0, siz * sizeof (momitem_t *));
  MOM_GC_FREE (arr);
  return tu;
}


void
mom_queue_scan_dump (struct momqueueitems_st *qu, struct momdumper_st *du)
{
  if (!qu || qu == MOM_EMPTY)
    return;
  assert (du);

  for (struct momqueuechunk_st * ch = qu->que_front; ch; ch = ch->quech_next)
    {
      for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  const momitem_t *itm = ch->quech_items[ix];
	  if (itm && itm != MOM_EMPTY)
	    mom_scan_dumped_item (du, itm);
	}
    }
}
