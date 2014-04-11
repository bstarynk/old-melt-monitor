// file values.c

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



momhash_t
mom_string_hash (const char *str, int len)
{
  if (!str)
    return 0;
  if (len < 0)
    len = strlen (str);
  momhash_t h1 = 0, h2 = len, h;
  int l = len;
  while (l > 4)
    {
      h1 =
	(509 * h2 +
	 307 * ((signed char *) str)[0]) ^ (1319 * ((signed char *) str)[1]);
      h2 =
	(17 * l + 5 + 5309 * h2) ^ ((3313 * ((signed char *) str)[2]) +
				    9337 * ((signed char *) str)[3] + 517);
      l -= 4;
      str += 4;
    }
  if (l > 0)
    {
      h1 = (h1 * 7703) ^ (503 * ((signed char *) str)[0]);
      if (l > 1)
	{
	  h2 = (h2 * 7717) ^ (509 * ((signed char *) str)[1]);
	  if (l > 2)
	    {
	      h1 = (h1 * 9323) ^ (11 + 523 * ((signed char *) str)[2]);
	      if (l > 3)
		{
		  h2 =
		    (h2 * 7727 + 127) ^ (313 +
					 547 * ((signed char *) str)[3]);
		}
	    }
	}
    }
  h = (h1 * 29311 + 59) ^ (h2 * 7321 + 120501);
  if (!h)
    {
      h = h1;
      if (!h)
	{
	  h = h2;
	  if (!h)
	    h = (len & 0xffffff) + 11;
	}
    }
  return h;
}

momstring_t *
mom_make_string_len (const char *str, int len)
{
  if (!str)
    return NULL;
  if (len < 0)
    len = strlen (str);
  momstring_t *sv = GC_MALLOC_ATOMIC (sizeof (momstring_t) + ((len | 3) + 1));
  memset (sv, 0, sizeof (momstring_t) + ((len | 3) + 1));
  sv->hash = mom_string_hash (str, len);
  sv->slen = len;
  memcpy (sv->cstr, str, len);
  sv->typnum = momty_string;
  return sv;
}

momint_t *
mom_make_int (intptr_t n)
{
  momint_t *iv = GC_MALLOC_ATOMIC (sizeof (momint_t));
  memset (iv, 0, sizeof (momint_t));
  iv->intval = n;
  iv->typnum = momty_int;
  return iv;
}
