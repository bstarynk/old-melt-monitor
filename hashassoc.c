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
    assert (pos >= 0 && pos < hlen);
    struct momhassocent_st *ent = ha->hass_arr + pos;
    ent->ha_key = keyv;
    ent->ha_val = valv;
    ha->hass_cnt = hcnt + 1;
  }
}                               /* end of raw_add_hassoc_mom */

struct momhashassoc_st *
mom_hassoc_really_reserve (struct momhashassoc_st *ha, unsigned gap)
{
  if (gap > MOM_MAX_NODE_LENGTH)
    MOM_FATAPRINTF ("too large gap %u for mom_hassoc_really_reserve", gap);
  uint32_t hsiz = 0;
  uint32_t hcnt = 0;
  uint32_t newsiz = 0;
  if (!ha || ha == MOM_EMPTY)
    newsiz = mom_prime_above (gap + 6);
  else
    {
      hsiz = ha->hass_len;
      hcnt = ha->hass_cnt;
    }
#warning mom_hassoc_really_reserve incomplete
  MOM_FATAPRINTF ("mom_hassoc_really_reserve incomplete gap=%u", gap);
}
