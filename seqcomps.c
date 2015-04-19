// file seqcomps.c


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
mom_components_put_nth (struct momcomponents_st *csq, int rk,
			const momvalue_t val)
{
  if (!csq || csq == MOM_EMPTY)
    return;
  unsigned cnt = csq->cp_cnt;
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    csq->cp_comps[rk] = val;
}

void
mom_components_scan_dump (struct momcomponents_st *csq,
			  struct momdumper_st *du)
{
  if (!csq || csq == MOM_EMPTY)
    return;
  assert (du);
  unsigned cnt = csq->cp_cnt;
  for (unsigned ix = 0; ix < cnt; ix++)
    mom_scan_dumped_value (du, csq->cp_comps[ix]);
}
