// file output.c

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

static pthread_mutex_t diag_mtx_mom = PTHREAD_MUTEX_INITIALIZER;



void
mom_out_at (const char *sfil, int lin, FILE * out, ...)
{
  va_list alist;
  va_start (alist, out);
  mom_outva_at (sfil, lin, out, alist);
  va_end (alist);
}

void
mom_outva_at (const char *sfil, int lin, FILE * out, va_list alist)
{
#warning mom_outva_at unimplemented
  MOM_FATAPRINTF ("unimplemented mom_outva_at from %s:%d", sfil, lin);
}
