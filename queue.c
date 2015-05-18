// file queue.c - (FIFO) queues of items and values

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

static int
queueitem_chunk_count_pack_mom (struct momqueuechunkitems_st *chk,
				momitem_t **pack)
{
  if (!chk)
    return 0;
  assert (pack);
  int cnt = 0;
  for (int ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
    {
      const momitem_t *itm = chk->quechi_items[ix];
      if (itm)
	pack[cnt++] = (momitem_t *) itm;
    }
  return cnt;
}

void
mom_queueitem_push_back (struct momqueueitems_st *qu, const momitem_t *itm)
{
  if (!qu)
    return;
  if (!itm || itm == MOM_EMPTY)
    return;
  struct momqueuechunkitems_st *bk = qu->que_back;
  if (bk)
    {
      momitem_t *packarr[MOM_QUEUECHUNK_LEN];
      memset (packarr, 0, sizeof (packarr));
      int nch = queueitem_chunk_count_pack_mom (bk, packarr);
      assert (nch > 0);
      if (nch < MOM_QUEUECHUNK_LEN)
	{
	  packarr[nch++] = (momitem_t *) itm;
	  memset (bk->quechi_items, 0, sizeof (bk->quechi_items));
	  memcpy (bk->quechi_items, packarr, nch * sizeof (momitem_t *));
	  qu->que_size++;
	  return;
	}
      else
	{
	  struct momqueuechunkitems_st *newch	//
	    = MOM_GC_ALLOC ("queue chunk",
			    sizeof (struct momqueuechunkitems_st));
	  bk->quechi_next = newch;
	  newch->quechi_prev = bk;
	  newch->quechi_items[0] = itm;
	  qu->que_size++;
	  qu->que_back = newch;
	  return;
	}
    }
  else				/* !bk, empty queue */
    {
      assert (qu->que_size == 0);
      struct momqueuechunkitems_st *newch	//
	= MOM_GC_ALLOC ("queue chunk", sizeof (struct momqueuechunkitems_st));
      newch->quechi_items[0] = itm;
      qu->que_front = qu->que_back = newch;
      qu->que_size = 1;
      return;
    }
}

void
mom_queueitem_push_front (struct momqueueitems_st *qu, const momitem_t *itm)
{
  if (!qu)
    return;
  if (!itm || itm == MOM_EMPTY)
    return;
  struct momqueuechunkitems_st *fr = qu->que_front;
  if (fr)
    {
      momitem_t *packarr[MOM_QUEUECHUNK_LEN + 1];	// extra space for one item
      memset (packarr, 0, sizeof (packarr));
      assert (fr->quechi_prev == NULL);
      int nch = queueitem_chunk_count_pack_mom (fr, packarr + 1);
      assert (nch > 0);		/* cannot be empty chunk */
      if (nch < MOM_QUEUECHUNK_LEN)
	{
	  packarr[0] = (momitem_t *) itm;
	  nch++;
	  memset (fr->quechi_items, 0, sizeof (fr->quechi_items));
	  memcpy (fr->quechi_items, packarr, nch * sizeof (momitem_t *));
	  qu->que_size++;
	  return;
	}
      else
	{
	  struct momqueuechunkitems_st *newch	//
	    = MOM_GC_ALLOC ("queue chunk",
			    sizeof (struct momqueuechunkitems_st));
	  fr->quechi_prev = newch;
	  newch->quechi_next = fr;
	  newch->quechi_items[0] = itm;
	  qu->que_size++;
	  qu->que_front = newch;
	  return;
	}
    }
  else				/* !fr, empty queue */
    {
      assert (qu->que_size == 0);
      struct momqueuechunkitems_st *newch	//
	= MOM_GC_ALLOC ("queue chunk", sizeof (struct momqueuechunkitems_st));
      newch->quechi_items[0] = itm;
      qu->que_front = qu->que_back = newch;
      qu->que_size = 1;
      return;
    }
}

const momitem_t *
mom_queueitem_pop_front (struct momqueueitems_st *qu)
{
  const momitem_t *itm = NULL;
  if (!qu || !qu->que_size)
    return NULL;
  struct momqueuechunkitems_st *fr = qu->que_front;
  assert (fr);
  itm = fr->quechi_items[0];
  assert (itm);
  if (!fr->quechi_items[1])
    {
      // front becomes empty, remove it
      qu->que_front = fr->quechi_next;
      if (!qu->que_front)
	qu->que_back = NULL;
      MOM_GC_FREE (fr, sizeof (*fr));
      qu->que_size--;
      return itm;
    }
  else
    {
      for (unsigned ix = 1; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  fr->quechi_items[ix - 1] = fr->quechi_items[ix];
	  fr->quechi_items[ix] = NULL;
	}
      qu->que_size--;
      return itm;
    }
}


const momitem_t *
mom_queueitem_peek_nth (struct momqueueitems_st *qu, int rk)
{
  if (!qu || !qu->que_size)
    return NULL;
  if (rk < 0)
    rk += qu->que_size;
  if (rk < 0)
    return NULL;
  struct momqueuechunkitems_st *ch = qu->que_front;
  while (rk >= 0 && ch != NULL)
    {
      __builtin_prefetch (ch->quechi_next);
      if (rk >= MOM_QUEUECHUNK_LEN)
	{
	  if (ch->quechi_items[0] && ch->quechi_items[MOM_QUEUECHUNK_LEN - 1])
	    {
	      rk -= MOM_QUEUECHUNK_LEN;
	      ch = ch->quechi_next;
	      continue;
	    }
	  else
	    return NULL;
	}
      else
	{
	  int cnt = 0;
	  for (int ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	    {
	      const momitem_t *itm = ch->quechi_items[ix];
	      if (itm)
		{
		  if (cnt == rk)
		    return itm;
		  cnt++;
		}
	    }
	}
    }
  return NULL;
}

const momseq_t *
mom_queueitem_tuple (struct momqueueitems_st *qu, momvalue_t metav)
{
  if (!qu)
    return NULL;
  if (qu->que_size > MOM_MAX_SEQ_LENGTH)
    MOM_FATAPRINTF ("too huge %ld queue", (long) qu->que_size);
  unsigned siz = qu->que_size;
  const momitem_t **arr =
    MOM_GC_ALLOC ("queue item array", (siz + 1) * sizeof (momitem_t *));
  unsigned cnt = 0;
  for (struct momqueuechunkitems_st * ch = qu->que_front; ch;
       ch = ch->quechi_next)
    {
      for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  const momitem_t *itm = ch->quechi_items[ix];
	  if (itm && itm != MOM_EMPTY)
	    arr[cnt++] = itm;
	}
    };
  assert (cnt == siz);
  const momseq_t *tu = mom_make_sized_meta_tuple (metav, cnt, arr);
  MOM_GC_FREE (arr, siz * sizeof (momitem_t *));
  return tu;
}


void
mom_queueitem_scan_dump (struct momqueueitems_st *qu)
{
  if (!qu || qu == MOM_EMPTY)
    return;

  for (struct momqueuechunkitems_st * ch = qu->que_front; ch;
       ch = ch->quechi_next)
    {
      for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  const momitem_t *itm = ch->quechi_items[ix];
	  if (itm && itm != MOM_EMPTY)
	    mom_scan_dumped_item (itm);
	}
    }
}


////////////////////////////////////////////////////////////////

static int
queuevalue_chunk_count_pack_mom (struct momqueuechunkvalues_st *chk,
				 momvalue_t *pack)
{
  if (!chk)
    return 0;
  assert (pack);
  int cnt = 0;
  for (int ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
    {
      const momvalue_t val = chk->quechv_values[ix];
      if (val.typnum != momty_null)
	pack[cnt++] = val;
    }
  return cnt;
}



momvalue_t
mom_queuevalue_peek_nth (struct momqueuevalues_st *qu, int rk)
{
  if (!qu)
    return MOM_NONEV;
  if (rk < 0)
    rk += qu->que_size;
  if (rk < 0)
    return MOM_NONEV;
  struct momqueuechunkvalues_st *ch = qu->que_front;
  while (rk >= 0 && ch != NULL)
    {
      __builtin_prefetch (ch->quechv_next);
      if (rk >= MOM_QUEUECHUNK_LEN)
	{
	  if (ch->quechv_values[0].typnum != momty_null
	      && ch->quechv_values[MOM_QUEUECHUNK_LEN].typnum != momty_null)
	    {
	      rk -= MOM_QUEUECHUNK_LEN;
	      ch = ch->quechv_next;
	      continue;
	    }
	  else
	    return MOM_NONEV;
	}
      else
	{
	  int cnt = 0;
	  for (int ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	    {
	      if (ch->quechv_values[ix].typnum == momty_null)
		continue;
	      if (cnt == rk)
		return ch->quechv_values[ix];
	      cnt++;
	    }
	}
    }
  return MOM_NONEV;
}

void
mom_queuevalue_push_back (struct momqueuevalues_st *qu, const momvalue_t val)
{
  if (MOM_UNLIKELY (!qu))
    return;
  if (val.typnum == momty_null)
    return;
  struct momqueuechunkvalues_st *bk = qu->que_back;
  if (bk)
    {
      assert (bk->quechv_next == NULL);
      momvalue_t pack[MOM_QUEUECHUNK_LEN];
      memset (pack, 0, sizeof (pack));
      int nbc = queuevalue_chunk_count_pack_mom (bk, pack);
      assert (nbc > 0);
      if (nbc < MOM_QUEUECHUNK_LEN)
	{
	  pack[nbc++] = val;
	  memset (bk->quechv_values, 0, sizeof (bk->quechv_values));
	  memcpy (bk->quechv_values, pack, nbc * sizeof (momvalue_t));
	  qu->que_size++;
	  return;
	}
      else
	{
	  struct momqueuechunkvalues_st *newch	//
	    = MOM_GC_ALLOC ("queue chunk",
			    sizeof (struct momqueuechunkvalues_st));
	  bk->quechv_next = newch;
	  newch->quechv_prev = bk;
	  newch->quechv_values[0] = val;
	  qu->que_size++;
	  qu->que_back = newch;
	  return;
	}
    }
  else				/* !bk, empty queue */
    {
      assert (qu->que_size == 0);
      struct momqueuechunkvalues_st *newch	//
	= MOM_GC_ALLOC ("queue chunk",
			sizeof (struct momqueuechunkvalues_st));
      newch->quechv_values[0] = val;
      qu->que_front = qu->que_back = newch;
      qu->que_size = 1;
      return;
    }
}

void
mom_queuevalue_push_front (struct momqueuevalues_st *qu, const momvalue_t val)
{
  if (MOM_UNLIKELY (!qu))
    return;
  if (val.typnum == momty_null)
    return;
  struct momqueuechunkvalues_st *fr = qu->que_front;
  if (fr)
    {
      assert (fr->quechv_prev == NULL);
      momvalue_t pack[MOM_QUEUECHUNK_LEN + 1];	/* extra space for new value */
      memset (pack, 0, sizeof (pack));
      int nbc = queuevalue_chunk_count_pack_mom (fr, pack + 1);
      pack[0] = val;
      nbc++;
      if (nbc < MOM_QUEUECHUNK_LEN)
	{
	  memset (fr->quechv_values, 0, sizeof (fr->quechv_values));
	  memcpy (fr->quechv_values, pack, nbc * sizeof (momvalue_t));
	  qu->que_size++;
	  return;
	}
      else
	{
	  struct momqueuechunkvalues_st *newch	//
	    = MOM_GC_ALLOC ("queue chunk",
			    sizeof (struct momqueuechunkvalues_st));
	  fr->quechv_prev = newch;
	  newch->quechv_next = fr;
	  newch->quechv_values[0] = val;
	  qu->que_size++;
	  qu->que_front = newch;
	  return;
	}
    }
  else				/* !fr, empty queue */
    {
      assert (qu->que_size == 0);
      struct momqueuechunkvalues_st *newch	//
	= MOM_GC_ALLOC ("queue chunk",
			sizeof (struct momqueuechunkvalues_st));
      newch->quechv_values[0] = val;
      qu->que_front = qu->que_back = newch;
      qu->que_size = 1;
      return;
    }
}

momvalue_t
mom_queuevalue_pop_front (struct momqueuevalues_st *qu)
{
  momvalue_t val = MOM_NONEV;
  if (!qu || !qu->que_size)
    return MOM_NONEV;
  struct momqueuechunkvalues_st *fr = qu->que_front;
  assert (fr);
  val = fr->quechv_values[0];
  assert (val.typnum != momty_null);
  if (fr->quechv_values[1].typnum == momty_null)
    {
      // front becomes empty, remove it
      qu->que_front = fr->quechv_next;
      if (!qu->que_front)
	qu->que_back = NULL;
      MOM_GC_FREE (fr, sizeof (*fr));
      qu->que_size--;
      return val;
    }
  else
    {
      for (unsigned ix = 1; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  fr->quechv_values[ix - 1] = fr->quechv_values[ix];
	  fr->quechv_values[ix] = MOM_NONEV;
	}
      qu->que_size--;
      return val;
    }
}


const momnode_t *
mom_queuevalue_node (struct momqueuevalues_st *qu, const momitem_t *connitm,
		     momvalue_t metav)
{
  if (!qu || qu == MOM_EMPTY || !connitm || connitm == MOM_EMPTY)
    return NULL;
  if (qu->que_size > MOM_MAX_SEQ_LENGTH)
    MOM_FATAPRINTF ("too huge %ld queue", (long) qu->que_size);
  unsigned siz = qu->que_size;
  momvalue_t *arr =
    MOM_GC_ALLOC ("value queue array", (siz + 1) * sizeof (momvalue_t));
  unsigned cnt = 0;
  for (struct momqueuechunkvalues_st * ch = qu->que_front; ch;
       ch = ch->quechv_next)
    {
      for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  const momvalue_t val = ch->quechv_values[ix];
	  if (val.typnum != momty_null)
	    arr[cnt++] = val;
	}
    };
  assert (cnt == siz);
  const momnode_t *nd =
    mom_make_sized_meta_node (metav, (momitem_t *) connitm, cnt, arr);
  MOM_GC_FREE (arr, siz * sizeof (momvalue_t));
  return nd;
}


void
mom_queuevalue_scan_dump (struct momqueuevalues_st *qu)
{
  if (!qu || qu == MOM_EMPTY)
    return;

  for (struct momqueuechunkvalues_st * ch = qu->que_front; ch;
       ch = ch->quechv_next)
    {
      for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
	{
	  const momvalue_t val = ch->quechv_values[ix];
	  if (val.typnum != momty_null)
	    mom_scan_dumped_value (val);
	}
    }
}
