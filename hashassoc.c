// file hashassoc.c - manage hash tables associating non-null values to non-null values

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

static inline void
raw_add_hassoc_mom (struct momhashassoc_st *ha, momvalue_t keyv,
                    momvalue_t valv)
{
  assert (ha != NULL);
  uint32_t hlen = ha->hass_len;
  uint32_t hcnt = ha->hass_cnt;
  assert (hlen > 2);
  assert (hcnt < MOM_MAX_NODE_LENGTH);
  assert (hcnt < hlen);
  assert (keyv.typnum != momty_null);
  assert (valv.typnum != momty_null);
  momhash_t h = mom_value_hash (keyv);
  assert (h != 0);
  unsigned startix = h % hlen;
  int pos = -1;
  for (unsigned ix = startix; ix < hlen; ix++)
    {
      struct momhassocent_st *curent = ha->hass_arr + ix;
      if (curent->ha_key.typnum == momty_null)
        {
          if (pos < 0)
            pos = (int) ix;
          if (curent->ha_key.vptr == MOM_EMPTY)
            continue;
          else
            goto add_entry;
        }
      else if (mom_value_equal (curent->ha_key, keyv))
        {
          curent->ha_val = valv;
          return;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      struct momhassocent_st *curent = ha->hass_arr + ix;
      if (curent->ha_key.typnum == momty_null)
        {
          if (pos < 0)
            pos = (int) ix;
          if (curent->ha_key.vptr == MOM_EMPTY)
            continue;
          else
            goto add_entry;
        }
      else if (mom_value_equal (curent->ha_key, keyv))
        {
          curent->ha_val = valv;
          return;
        }
    }
  /// should never happen that we reach this
  MOM_FATAPRINTF ("corrupted hash association @%p", (void *) ha);
add_entry:
  {
    assert (pos >= 0 && pos < (int) hlen);
    struct momhassocent_st *ent = ha->hass_arr + pos;
    ent->ha_key = keyv;
    ent->ha_val = valv;
    ha->hass_cnt = hcnt + 1;
  }
}                               /* end of raw_add_hassoc_mom */


// return the index of a key, or negative if not found
static inline int
raw_find_index_hassoc_mom (const struct momhashassoc_st *ha, momvalue_t keyv)
{
  assert (ha != NULL);
  uint32_t hlen = ha->hass_len;
  uint32_t hcnt = ha->hass_cnt;
  assert (hlen > 2);
  assert (hcnt < MOM_MAX_NODE_LENGTH);
  assert (hcnt < hlen);
  assert (keyv.typnum != momty_null);
  momhash_t h = mom_value_hash (keyv);
  assert (h != 0);
  unsigned startix = h % hlen;
  for (unsigned ix = startix; ix < hlen; ix++)
    {
      const struct momhassocent_st *curent = ha->hass_arr + ix;
      if (curent->ha_key.typnum == momty_null)
        {
          if (curent->ha_key.vptr == MOM_EMPTY)
            continue;
          else
            return -1;
        }
      else if (mom_value_equal (curent->ha_key, keyv))
        {
          return (int) ix;
        }
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const struct momhassocent_st *curent = ha->hass_arr + ix;
      if (curent->ha_key.typnum == momty_null)
        {
          if (curent->ha_key.vptr == MOM_EMPTY)
            continue;
          else
            return -1;
        }
      else if (mom_value_equal (curent->ha_key, keyv))
        {
          return (int) ix;
        }
    }
  return -1;
}                               /* end raw_find_index_hassoc_mom  */


struct momhashassoc_st *
mom_hassoc_really_reserve (struct momhashassoc_st *ha, unsigned gap)
{
  if (gap > MOM_MAX_NODE_LENGTH)
    MOM_FATAPRINTF ("too large gap %u for mom_hassoc_really_reserve", gap);
  uint32_t hsiz = 0;
  uint32_t hcnt = 0;
  uint32_t newsiz = 0;
  if (!ha || ha == MOM_EMPTY)
    {
      newsiz = mom_prime_above (gap + 6);
      ha = NULL;
    }
  else
    {
      hsiz = ha->hass_len;
      hcnt = ha->hass_cnt;
      newsiz = mom_prime_above (4 * hcnt / 3 + gap + hcnt / 8 + 5);
    }
  if (newsiz == hsiz)
    return ha;
  struct momhashassoc_st *newha =       //
    MOM_GC_ALLOC ("newha",
                  sizeof (struct momhashassoc_st) +
                  newsiz * sizeof (struct momhassocent_st));
  newha->hass_len = newsiz;
  for (unsigned ix = 0; ix < hsiz; ix++)
    {
      struct momhassocent_st *curent = ha->hass_arr + ix;
      if (curent->ha_key.typnum == momty_null)
        continue;
      raw_add_hassoc_mom (newha, curent->ha_key, curent->ha_val);
    }
  if (ha)
    MOM_GC_FREE (ha,
                 sizeof (struct momhashassoc_st) +
                 hsiz * sizeof (struct momhassocent_st));
  assert (newha->hass_cnt == hcnt);
  return newha;
}                               /* end of mom_hassoc_really_reserve */


momvalue_t
mom_hassoc_get (const struct momhashassoc_st *ha, momvalue_t keyv)
{
  if (ha == MOM_EMPTY)
    ha = NULL;
  if (!ha || keyv.typnum == momty_null)
    return MOM_NONEV;
  int ix = raw_find_index_hassoc_mom (ha, keyv);
  if (ix < 0)
    return MOM_NONEV;
  assert (ix < (int) ha->hass_len);
  return ha->hass_arr[ix].ha_val;
}                               /* end of mom_hassoc_get */


struct momhashassoc_st *
mom_hassoc_put (struct momhashassoc_st *ha, momvalue_t keyv, momvalue_t valv)
{
  if (ha == MOM_EMPTY)
    ha = NULL;
  if (keyv.typnum == momty_null)
    return ha;
  if (valv.typnum == momty_null)
    return mom_hassoc_remove (ha, keyv);
  uint32_t hsiz = ha->hass_len;
  uint32_t hcnt = ha->hass_cnt;
  if (5 * hcnt + 1 >= 4 * hsiz)
    ha = mom_hassoc_reserve (ha, hcnt / 64 + 3);
  raw_add_hassoc_mom (ha, keyv, valv);
  return ha;
}                               /* end of mom_hassoc_put */


struct momhashassoc_st *
mom_hassoc_remove (struct momhashassoc_st *ha, momvalue_t keyv)
{
  if (ha == MOM_EMPTY)
    ha = NULL;
  if (keyv.typnum == momty_null)
    return ha;
  if (!ha)
    return NULL;
  uint32_t hsiz = ha->hass_len;
  uint32_t hcnt = ha->hass_cnt;
  if (hsiz > 64 && hcnt < hsiz / 3)
    {
      struct momhashassoc_st *newha =
        mom_hassoc_really_reserve (NULL, hcnt + hcnt / 3 + 1);
      for (unsigned ix = 0; ix < hsiz; ix++)
        {
          struct momhassocent_st *curent = ha->hass_arr + ix;
          if (curent->ha_key.typnum == momty_null)
            continue;
          raw_add_hassoc_mom (newha, curent->ha_key, curent->ha_val);
        }
      MOM_GC_FREE (ha,
                   sizeof (struct momhashassoc_st) +
                   hsiz * sizeof (struct momhassocent_st));
      hsiz = newha->hass_len;
      assert (hcnt == newha->hass_cnt);
      ha = newha;
    };
  int ix = raw_find_index_hassoc_mom (ha, keyv);
  if (ix < 0)
    return ha;
  assert (hcnt > 0);
  memset (ha->hass_arr + ix, 0, sizeof (struct momhassocent_st));
  ha->hass_arr[ix].ha_key.typnum = 0;
  ha->hass_arr[ix].ha_key.vptr = MOM_EMPTY;
  ha->hass_cnt = hcnt - 1;
  return ha;
}                               /* end of mom_hassoc_remove */
