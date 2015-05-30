// file hashdict.c - manage hashed dictionnaries

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

static void
hashdict_raw_put_mom (struct momhashdict_st *hdict, const momstring_t *str,
		      const momvalue_t val)
{
  assert (hdict != NULL && hdict != MOM_EMPTY);
  assert (str != NULL && str != MOM_EMPTY);
  assert (val.typnum != momty_null);
  unsigned len = hdict->hdic_len;
  unsigned cnt = hdict->hdic_cnt;
  assert (cnt + cnt / 16 + 2 < len);
  assert (len > 2);
  momhash_t hs = str->shash;
  unsigned startix = hs % len;
  for (unsigned ix = startix; ix < len; ix++)
    {
      const momstring_t *curstr = hdict->hdic_ents[ix].dicent_str;
      if (curstr == MOM_EMPTY)
	continue;
      if (!curstr)
	{
	  hdict->hdic_ents[ix].dicent_str = str;
	  hdict->hdic_ents[ix].dicent_val = val;
	  hdict->hdic_cnt++;
	  return;
	}
      if (curstr->shash == hs && !strcmp (curstr->cstr, str->cstr))
	{
	  hdict->hdic_ents[ix].dicent_val = val;
	  return;
	}
    };
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const momstring_t *curstr = hdict->hdic_ents[ix].dicent_str;
      if (curstr == MOM_EMPTY)
	continue;
      if (!curstr)
	{
	  hdict->hdic_ents[ix].dicent_str = str;
	  hdict->hdic_ents[ix].dicent_val = val;
	  hdict->hdic_cnt++;
	  return;
	}
      if (curstr->shash == hs && !strcmp (curstr->cstr, str->cstr))
	{
	  hdict->hdic_ents[ix].dicent_val = val;
	  return;
	}
    };
  // should never happen
  MOM_FATAPRINTF ("corrupted dictval @%p", hdict);
}				/* end of hashdict_raw_put_mom */


static void
fill_hashdict_from_old_mom (struct momhashdict_st *newhdict,
			    const struct momhashdict_st *oldhdict)
{
  assert (newhdict != NULL);
  assert (oldhdict != NULL);
  unsigned oldlen = oldhdict->hdic_len;
  for (unsigned oix = 0; oix < oldlen; oix++)
    {
      const momstring_t *oldcurstr = oldhdict->hdic_ents[oix].dicent_str;
      if (!oldcurstr || oldcurstr == MOM_EMPTY)
	continue;
      hashdict_raw_put_mom (newhdict, oldcurstr,
			    oldhdict->hdic_ents[oix].dicent_val);
    }
}				/* end fill_hashdict_from_old_mom */

struct momhashdict_st *
mom_hashdict_put (struct momhashdict_st *hdict,
		  momstring_t *str, momvalue_t val)
{
  if (hdict == MOM_EMPTY)
    hdict = NULL;
  if (!str || str == MOM_EMPTY)
    return hdict;
  if (val.typnum == momty_null)
    return mom_hashdict_remove (hdict, str);
  if (!hdict)
    {
      unsigned siz = 11;
      hdict =			//
	MOM_GC_ALLOC ("new hashdict",
		      sizeof (struct momhashdict_st) +
		      siz * sizeof (struct momdictvalent_st));
      hdict->hdic_len = siz;
      hdict->hdic_cnt = 0;
      hashdict_raw_put_mom (hdict, str, val);
      assert (hdict->hdic_cnt == 1);
      return hdict;
    };
  unsigned oldcnt = hdict->hdic_cnt;
  unsigned oldlen = hdict->hdic_len;
  assert (oldlen > 2);
  if (MOM_UNLIKELY (5 * oldcnt > 4 * oldlen))
    {
      unsigned newsiz = mom_prime_above (4 * oldcnt / 3 + oldcnt / 32 + 10);
      assert (newsiz > oldlen);
    }
}				/* end of mom_hashdict_put */



struct momhashdict_st *
mom_hashdict_remove (struct momhashdict_st *hdict, const momstring_t *str)
{
  if (hdict == MOM_EMPTY || hdict == NULL)
    return NULL;
  if (!str || str == MOM_EMPTY)
    return hdict;
#warning mom_hashdict_remove incomplete
}				/* end mom_hashdict_remove */

#warning missing other hashdict functions
