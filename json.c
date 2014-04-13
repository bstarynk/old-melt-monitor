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
struct jsonparser_st
{
  uint32_t json_magic;		/* always MOMJSON_MAGIC */
  unsigned json_top;
  unsigned json_size;
  momval_t *json_valarr;
  struct
  {
    unsigned je_rank;
    unsigned je_state;
  } *json_statearr;
};


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
  jp->json_statearr =
    GC_MALLOC_ATOMIC (sizeof (jp->json_statearr[0]) * inisiz);
  if (!jp->json_statearr)
    MONIMELT_FATAL ("failed to initialize json states sized %u", inisiz);
  memset (jp->json_statearr, 0, sizeof (jp->json_statearr[0]) * inisiz);
  jp->json_magic = MOMJSON_MAGIC;
}
