// file hashset.c - manage hashed sets

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

// we seggregate small hashsets, scanned linearly, from bigger hashed set
#define SMALL_HASHSET_LEN_MOM 12

const momseq_t *
mom_hashset_elements_set_meta (struct momhashset_st *hset, momvalue_t metav)
{
  if (!hset)
    return NULL;
  return mom_make_sized_meta_set (metav, hset->hset_len, hset->hset_elems);
}

bool
mom_hashset_contains (const struct momhashset_st * hset, const momitem_t *itm)
{
  if (!hset || hset == MOM_EMPTY || !itm || itm == MOM_EMPTY)
    return false;
  unsigned hslen = hset->hset_len;
  if (hslen <= SMALL_HASHSET_LEN_MOM)
    {
      for (unsigned ix = 0; ix < hslen; ix++)
        if (hset->hset_elems[ix] == itm)
          return true;
    }
  else
    {
      unsigned startix = mom_item_hash (itm) % hslen;
      for (unsigned ix = startix; ix < hslen; ix++)
        {
          const momitem_t *curitm = hset->hset_elems[ix];
          if (!curitm)
            return false;
          else if (curitm == itm)
            return true;
        }
      for (unsigned ix = 0; ix < startix; ix++)
        {
          const momitem_t *curitm = hset->hset_elems[ix];
          if (!curitm)
            return false;
          else if (curitm == itm)
            return true;
        }
    }
  return false;
}


static void
hashset_raw_hash_add_mom (struct momhashset_st *hset, const momitem_t *itm)
{
  assert (hset && itm);
  unsigned hslen = hset->hset_len;
  assert (hslen > SMALL_HASHSET_LEN_MOM);
  unsigned hscnt = hset->hset_cnt;
  assert (hscnt < hslen);
  int pos = -1;
  unsigned startix = mom_item_hash (itm) % hslen;
  for (unsigned ix = startix; ix < hslen; ix++)
    {
      const momitem_t *curitm = hset->hset_elems[ix];
      if (!curitm)
        {
          if (pos < 0)
            pos = ix;
          hset->hset_elems[pos] = itm;
          hset->hset_cnt = hscnt + 1;
          return;
        }
      else if (curitm == itm)
        return;
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const momitem_t *curitm = hset->hset_elems[ix];
      if (!curitm)
        {
          if (pos < 0)
            pos = ix;
          hset->hset_elems[pos] = itm;
          hset->hset_cnt = hscnt + 1;
          return;
        }
      else if (curitm == itm)
        return;
    }
  // never reached
  MOM_FATAPRINTF ("corrupted hashset@%p", (void *) hset);
}


struct momhashset_st *
mom_hashset_put (struct momhashset_st *hset, const momitem_t *itm)
{
  if (!itm || itm == MOM_EMPTY)
    return hset;
  if (!hset || hset == MOM_EMPTY)
    {
      unsigned newsiz = SMALL_HASHSET_LEN_MOM / 2;
      struct momhashset_st *newhset     //
        = MOM_GC_ALLOC ("new small hset",
                        sizeof (struct momhashset_st) +
                        newsiz * sizeof (momitem_t *));
      newhset->hset_len = newsiz;
      newhset->hset_cnt = 1;
      newhset->hset_elems[0] = itm;
      return newhset;
    };
  unsigned hslen = hset->hset_len;
  unsigned hscnt = hset->hset_cnt;
  if (hslen <= SMALL_HASHSET_LEN_MOM)
    {
      if (hscnt + 1 < hslen)
      small_nonfull_hset:
        {

          int pos = -1;
          for (unsigned ix = 0; ix < hslen; ix++)
            {
              const momitem_t *curitm = hset->hset_elems[ix];
              if (!curitm)
                {
                  if (pos < 0)
                    pos = ix;
                  break;
                }
              else if (curitm == MOM_EMPTY)
                {
                  if (pos < 0)
                    pos = ix;
                }
              else if (curitm == itm)
                return hset;
            }
          assert (pos >= 0);
          hset->hset_elems[pos] = itm;
          hset->hset_cnt++;
          return hset;
        }
      else
        {
          if (hslen < SMALL_HASHSET_LEN_MOM)
            {
              unsigned siz = SMALL_HASHSET_LEN_MOM;
              struct momhashset_st *newhset     //
                = MOM_GC_ALLOC ("new small hashset",
                                sizeof (struct momhashset_st) +
                                siz * sizeof (momitem_t *));
              newhset->hset_len = siz;
              memcpy (newhset->hset_elems, hset->hset_elems,
                      hslen * sizeof (momitem_t *));
              newhset->hset_cnt = hset->hset_cnt;
              MOM_GC_FREE (hset,
                           sizeof (struct momhashset_st) +
                           hslen * sizeof (momitem_t *));
              hset = newhset;
              hslen = siz;
              goto small_nonfull_hset;
            }
          else
            {
              unsigned siz = ((4 * hscnt / 3 + 10) | 0xf) + 1;
              struct momhashset_st *newhset     //
                = MOM_GC_ALLOC ("new  hashset",
                                sizeof (struct momhashset_st) +
                                siz * sizeof (momitem_t *));
              newhset->hset_len = siz;
              for (unsigned ix = 0; ix < hslen; ix++)
                {
                  const momitem_t *olditm = hset->hset_elems[ix];
                  if (!olditm || olditm == MOM_EMPTY)
                    continue;
                  hashset_raw_hash_add_mom (newhset, olditm);
                }
              hashset_raw_hash_add_mom (newhset, itm);
              MOM_GC_FREE (hset,
                           sizeof (struct momhashset_st) +
                           hslen * sizeof (momitem_t *));
              hset = newhset;
              return hset;
            }
        }
    }
  else
    {
      if (4 * hscnt + 2 > 3 * hslen)
        {
          unsigned siz = ((3 * hscnt / 2 + 10) | 0xf) + 1;
          struct momhashset_st *newhset //
            = MOM_GC_ALLOC ("new hashset",
                            sizeof (struct momhashset_st) +
                            siz * sizeof (momitem_t *));
          newhset->hset_len = siz;
          for (unsigned ix = 0; ix < hslen; ix++)
            {
              const momitem_t *olditm = hset->hset_elems[ix];
              if (!olditm || olditm == MOM_EMPTY)
                continue;
              hashset_raw_hash_add_mom (newhset, olditm);
            }
          hashset_raw_hash_add_mom (newhset, itm);
          MOM_GC_FREE (hset,
                       sizeof (struct momhashset_st) +
                       hslen * sizeof (momitem_t *));
          hset = newhset;
          return hset;
        }
      else
        {
          hashset_raw_hash_add_mom (hset, itm);
          return hset;
        }
    }
}



struct momhashset_st *
mom_hashset_remove (struct momhashset_st *hset, const momitem_t *itm)
{
  if (!hset || hset == MOM_EMPTY)
    return NULL;
  if (!itm || itm == MOM_EMPTY)
    return hset;
  unsigned hslen = hset->hset_len;
  unsigned hscnt = hset->hset_cnt;
  if (hslen <= SMALL_HASHSET_LEN_MOM)
    {
      for (unsigned ix = 0; ix < hslen; ix++)
        if (hset->hset_elems[ix] == itm)
          {
            hset->hset_elems[ix] = MOM_EMPTY;
            hset->hset_cnt = hscnt - 1;
            break;
          }
      return hset;
    }
  else
    {
      if (hscnt < 2 * SMALL_HASHSET_LEN_MOM / 3)
        {
          // shrink the plain hashset to a small one
          unsigned newsiz = SMALL_HASHSET_LEN_MOM;
          struct momhashset_st *newhset //
            = MOM_GC_ALLOC ("new hashset",
                            sizeof (struct momhashset_st) +
                            newsiz * sizeof (momitem_t *));
          newhset->hset_len = newsiz;
          unsigned newcnt = 0;
          for (unsigned ix = 0; ix < hslen; ix++)
            {
              const momitem_t *olditm = hset->hset_elems[ix];
              if (olditm == itm)
                continue;
              if (olditm && olditm != MOM_EMPTY)
                newhset->hset_elems[newcnt++] = olditm;
            }
          newhset->hset_cnt = newcnt;
          MOM_GC_FREE (hset, sizeof (struct momhashset_st)
                       + hslen * sizeof (momitem_t *));
          hset = newhset;
          return hset;
        }
      else if (hscnt + 2 > hslen / 2)
      remove_item_from_plain_hashset:{
          // keep the plain hashset, but find the item to remove it
          unsigned startix = mom_item_hash (itm) % hslen;
          for (unsigned ix = startix; ix < hslen; ix++)
            {
              const momitem_t *olditm = hset->hset_elems[ix];
              if (!olditm)
                return hset;
              else if (olditm == itm)
                {
                  hset->hset_elems[ix] = MOM_EMPTY;
                  hset->hset_cnt = hscnt - 1;
                  return hset;
                }
            };
          for (unsigned ix = 0; ix < startix; ix++)
            {
              const momitem_t *olditm = hset->hset_elems[ix];
              if (!olditm)
                return hset;
              else if (olditm == itm)
                {
                  hset->hset_elems[ix] = MOM_EMPTY;
                  hset->hset_cnt = hscnt - 1;
                  return hset;
                }
            };
          return hset;
        }
      else
        {
          // shrink to a smaller plain hashset
          unsigned newsiz = ((5 * hscnt / 4 + 2) | 0xf) + 1;
          if (newsiz >= hslen)
            goto remove_item_from_plain_hashset;
          struct momhashset_st *newhset //
            = MOM_GC_ALLOC ("new hashset",
                            sizeof (struct momhashset_st) +
                            newsiz * sizeof (momitem_t *));
          newhset->hset_len = newsiz;
          for (unsigned ix = 0; ix < hslen; ix++)
            {
              const momitem_t *olditm = hset->hset_elems[ix];
              if (olditm == itm)
                continue;
              if (olditm && olditm != MOM_EMPTY)
                hashset_raw_hash_add_mom (newhset, olditm);
            }
          return newhset;
        }
    }
}


struct momhashset_st *
mom_hashset_add_items (struct momhashset_st *hset,
                       unsigned nbitems, ... /* items */ )
{
  va_list args;
  if (hset == MOM_EMPTY)
    hset = NULL;
  if (!nbitems)
    return hset;
  unsigned hslen = hset ? hset->hset_len : 0;
  unsigned hscnt = hset ? hset->hset_cnt : 0;
  unsigned newsiz = 0;
  if (hslen <= SMALL_HASHSET_LEN_MOM && hscnt + nbitems <= hslen)
    {
      // small hashset, keep it
      const momitem_t *itmarr[SMALL_HASHSET_LEN_MOM] = { NULL };
      memset (itmarr, 0, sizeof (itmarr));
      unsigned cnt = 0;
      for (unsigned ix = 0; ix < hslen; ix++)
        {
          const momitem_t *olditm = hset->hset_elems[ix];
          if (!olditm || olditm == MOM_EMPTY)
            continue;
          itmarr[cnt++] = olditm;
        };
      assert (cnt + nbitems <= SMALL_HASHSET_LEN_MOM);
      va_start (args, nbitems);
      for (unsigned ix = 0; ix < nbitems; ix++)
        {
          const momitem_t *newitm = va_arg (args, const momitem_t *);
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          itmarr[cnt++] = newitm;
        }
      va_end (args);
      assert (cnt <= hslen);
      memset (hset->hset_elems, 0, hslen * sizeof (momitem_t *));
      if (cnt)
        memcpy (hset->hset_elems, itmarr, cnt * sizeof (momitem_t *));
      if (hset)
        hset->hset_cnt = cnt;
      return hset;
    }
  else if (hscnt + nbitems <= SMALL_HASHSET_LEN_MOM)
    {
      // make a small hashset
      unsigned newsiz =
        ((hscnt + nbitems) <=
         SMALL_HASHSET_LEN_MOM / 2) ? (SMALL_HASHSET_LEN_MOM /
                                       2) : SMALL_HASHSET_LEN_MOM;
      struct momhashset_st *newhset     //
        = MOM_GC_ALLOC ("new hashset",
                        sizeof (struct momhashset_st) +
                        newsiz * sizeof (momitem_t *));
      newhset->hset_len = newsiz;
      unsigned cnt = 0;
      for (unsigned ix = 0; ix < hslen; ix++)
        {
          const momitem_t *olditm = hset->hset_elems[ix];
          if (!olditm || olditm == MOM_EMPTY)
            continue;
          newhset->hset_elems[cnt++] = olditm;
        }
      assert (cnt + nbitems <= newsiz);
      va_start (args, nbitems);
      for (unsigned ix = 0; ix < nbitems; ix++)
        {
          const momitem_t *newitm = va_arg (args, const momitem_t *);
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          newhset->hset_elems[cnt++] = newitm;
        }
      va_end (args);
      newhset->hset_cnt = cnt;
      if (hset)
        {
          MOM_GC_FREE (hset,
                       sizeof (struct momhashset_st) +
                       hslen * sizeof (momitem_t *));
        }
      return newhset;
    }
  newsiz = ((5 * (hscnt + nbitems) / 4 + 2) | 0xf) + 1;
  if (newsiz == hslen)
    {                           // keep the same plain hashset
      va_start (args, nbitems);
      for (unsigned ix = 0; ix < nbitems; ix++)
        {
          const momitem_t *newitm = va_arg (args, const momitem_t *);
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          hashset_raw_hash_add_mom (hset, newitm);
        }
      va_end (args);
      return hset;
    }
  else
    {                           // reallocate a plain hashset
      struct momhashset_st *newhset     //
        = MOM_GC_ALLOC ("new hashset",
                        sizeof (struct momhashset_st) +
                        newsiz * sizeof (momitem_t *));
      newhset->hset_len = newsiz;
      for (unsigned ix = 0; ix < hslen; ix++)
        {
          const momitem_t *olditm = hset->hset_elems[ix];
          if (!olditm || olditm == MOM_EMPTY)
            continue;
          hashset_raw_hash_add_mom (newhset, olditm);
        }
      va_start (args, nbitems);
      for (unsigned ix = 0; ix < nbitems; ix++)
        {
          const momitem_t *newitm = va_arg (args, const momitem_t *);
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          hashset_raw_hash_add_mom (newhset, newitm);
        }
      va_end (args);
      if (hset)
        {
          MOM_GC_FREE (hset,
                       sizeof (struct momhashset_st) +
                       hslen * sizeof (momitem_t *));
        }
      return newhset;
    }
}


struct momhashset_st *
mom_hashset_add_sized_items (struct momhashset_st *hset,
                             unsigned siz, momitem_t *const *itmarr)
{
  if (!itmarr || itmarr == MOM_EMPTY)
    siz = 0;
  if (hset == MOM_EMPTY)
    hset = NULL;
  if (!siz)
    return hset;
  unsigned hslen = hset ? hset->hset_len : 0;
  unsigned hscnt = hset ? hset->hset_cnt : 0;
  unsigned newsiz = 0;
  if (hslen <= SMALL_HASHSET_LEN_MOM && hscnt + siz <= hslen)
    {
      // small hashset, keep it
      const momitem_t *tmpitmarr[SMALL_HASHSET_LEN_MOM] = { NULL };
      memset (tmpitmarr, 0, sizeof (tmpitmarr));
      unsigned cnt = 0;
      for (unsigned ix = 0; ix < hslen; ix++)
        {
          const momitem_t *olditm = hset->hset_elems[ix];
          if (!olditm || olditm == MOM_EMPTY)
            continue;
          tmpitmarr[cnt++] = olditm;
        };
      assert (cnt + siz <= SMALL_HASHSET_LEN_MOM);
      for (unsigned ix = 0; ix < siz; ix++)
        {
          const momitem_t *newitm = itmarr[ix];
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          tmpitmarr[cnt++] = newitm;
        }
      assert (cnt <= hslen);
      memset (hset->hset_elems, 0, hslen * sizeof (momitem_t *));
      if (cnt)
        memcpy (hset->hset_elems, tmpitmarr, cnt * sizeof (momitem_t *));
      if (hset)
        hset->hset_cnt = cnt;
      return hset;
    }
  else if (hscnt + siz <= SMALL_HASHSET_LEN_MOM)
    {
      // make a small hashset
      unsigned newsiz =
        ((hscnt + siz) <=
         SMALL_HASHSET_LEN_MOM / 2) ? (SMALL_HASHSET_LEN_MOM /
                                       2) : SMALL_HASHSET_LEN_MOM;
      struct momhashset_st *newhset     //
        = MOM_GC_ALLOC ("new hashset",
                        sizeof (struct momhashset_st) +
                        newsiz * sizeof (momitem_t *));
      newhset->hset_len = newsiz;
      unsigned cnt = 0;
      for (unsigned ix = 0; ix < hslen; ix++)
        {
          const momitem_t *olditm = hset->hset_elems[ix];
          if (!olditm || olditm == MOM_EMPTY)
            continue;
          newhset->hset_elems[cnt++] = olditm;
        }
      assert (cnt + siz <= newsiz);
      for (unsigned ix = 0; ix < siz; ix++)
        {
          const momitem_t *newitm = itmarr[ix];
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          newhset->hset_elems[cnt++] = newitm;
        }
      assert (cnt <= newsiz);
      newhset->hset_cnt = cnt;
      if (hset)
        {
          MOM_GC_FREE (hset,
                       sizeof (struct momhashset_st) +
                       hslen * sizeof (momitem_t *));
        }
      return newhset;
    }
  newsiz = ((5 * (hscnt + siz) / 4 + 2) | 0xf) + 1;
  if (newsiz == hslen)
    {                           // keep the same plain hashset
      for (unsigned ix = 0; ix < siz; ix++)
        {
          const momitem_t *newitm = itmarr[ix];
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          hashset_raw_hash_add_mom (hset, newitm);
        }
      return hset;
    }
  else
    {                           // reallocate a plain hashset
      struct momhashset_st *newhset     //
        = MOM_GC_ALLOC ("new hashset",
                        sizeof (struct momhashset_st) +
                        newsiz * sizeof (momitem_t *));
      newhset->hset_len = newsiz;
      for (unsigned ix = 0; ix < hslen; ix++)
        {
          const momitem_t *olditm = hset->hset_elems[ix];
          if (!olditm || olditm == MOM_EMPTY)
            continue;
          hashset_raw_hash_add_mom (newhset, olditm);
        }
      for (unsigned ix = 0; ix < siz; ix++)
        {
          const momitem_t *newitm = itmarr[ix];
          if (!newitm || newitm == MOM_EMPTY)
            continue;
          hashset_raw_hash_add_mom (newhset, newitm);
        }
      if (hset)
        {
          MOM_GC_FREE (hset,
                       sizeof (struct momhashset_st) +
                       hslen * sizeof (momitem_t *));
        }
      return newhset;
    }
}



void
mom_hashset_scan_dump (struct momhashset_st *hset)
{
  if (!hset || hset == MOM_EMPTY)
    return;
  unsigned hslen = hset->hset_len;
  for (unsigned ix = 0; ix < hslen; ix++)
    {
      const momitem_t *curitm = hset->hset_elems[ix];
      if (!curitm || curitm == MOM_EMPTY)
        continue;
      mom_scan_dumped_item (curitm);
    }
}
