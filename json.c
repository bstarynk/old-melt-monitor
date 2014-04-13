// file json.c

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

#define MOMJSON_MAGIC 0x124ba95b	/*json magic 306948443 */
struct jsonstatelevel_st
{
  unsigned je_rank;
  unsigned je_state;
};

struct jsonparser_st
{
  uint32_t json_magic;		/* always MOMJSON_MAGIC */
  unsigned json_top;
  unsigned json_size;
  momval_t *json_valarr;
  struct jsonstatelevel_st *json_levarr;
};

enum jsonstate_en
{
  jse_none,
  jse_startjson,
};

static inline void
push_state (struct jsonparser_st *jp, unsigned state, momval_t val,
	    unsigned rank)
{
  unsigned jtop = jp->json_top;
  if (MONIMELT_UNLIKELY (jtop + 2 >= jp->json_size))
    {
      unsigned newsize = ((3 * jtop / 2 + 50) | 0x3f) + 1;
      if (newsize > jp->json_size)
	{
	  momval_t *newvalarr = GC_MALLOC (sizeof (momval_t) * newsize);
	  if (MONIMELT_UNLIKELY (!newvalarr))
	    MONIMELT_FATAL ("failed to grow json state value stack to %u",
			    newsize);
	  memset (newvalarr, 0, sizeof (momval_t) * newsize);
	  memcpy (newvalarr, jp->json_valarr, sizeof (momval_t) * jtop);
	  struct jsonstatelevel_st *newlevarr =
	    GC_MALLOC_ATOMIC (sizeof (struct jsonstatelevel_st) * newsize);
	  if (MONIMELT_UNLIKELY (!newlevarr))
	    MONIMELT_FATAL ("failed to grow json state level stack to %u",
			    newsize);
	  memset (newlevarr, 0, sizeof (struct jsonstatelevel_st) * newsize);
	  memcpy (newlevarr, jp->json_levarr,
		  sizeof (struct jsonstatelevel_st) * jtop);
	  GC_FREE (jp->json_valarr);
	  jp->json_valarr = newvalarr;
	  GC_FREE (jp->json_levarr);
	  jp->json_levarr = newlevarr;
	}
    }
  jp->json_valarr[jtop] = val;
  jp->json_levarr[jtop].je_rank = rank;
  jp->json_levarr[jtop].je_state = state;
  jp->json_top = jtop + 1;
}

void
mom_json_initialize (struct jsonparser_st *jp)
{
  memset (jp, 0, sizeof (*jp));
  const unsigned inisiz = 64;
  jp->json_top = 0;
  jp->json_size = inisiz;
  jp->json_valarr = GC_MALLOC (sizeof (momval_t) * inisiz);
  if (!jp->json_valarr)
    MONIMELT_FATAL ("failed to initialize json values sized %u", inisiz);
  memset (jp->json_valarr, 0, sizeof (momval_t) * inisiz);
  jp->json_levarr =
    GC_MALLOC_ATOMIC (sizeof (struct jsonstatelevel_st) * inisiz);
  if (!jp->json_levarr)
    MONIMELT_FATAL ("failed to initialize json states sized %u", inisiz);
  memset (jp->json_levarr, 0, sizeof (struct jsonstatelevel_st) * inisiz);
  jp->json_magic = MOMJSON_MAGIC;
  push_state (jp, jse_startjson, MONIMELT_NULLV, 0u);
}

// return the number of consumed bytes
int
mom_json_consume (struct jsonparser_st *jp, const char *buf, int len)
{
  if (!jp || jp->json_magic != MOMJSON_MAGIC || !buf)
    return -1;
  if (len < 0)
    len = strlen (buf);
}
