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

/***************** integers ****************/
static const momint_t int0_mom = {.typnum = momty_int,.intval = 0 };
static const momint_t int1_mom = {.typnum = momty_int,.intval = 1 };
static const momint_t int2_mom = {.typnum = momty_int,.intval = 2 };
static const momint_t int3_mom = {.typnum = momty_int,.intval = 3 };
static const momint_t int4_mom = {.typnum = momty_int,.intval = 4 };
static const momint_t int5_mom = {.typnum = momty_int,.intval = 5 };
static const momint_t int6_mom = {.typnum = momty_int,.intval = 6 };
static const momint_t int7_mom = {.typnum = momty_int,.intval = 7 };
static const momint_t int8_mom = {.typnum = momty_int,.intval = 8 };
static const momint_t int9_mom = {.typnum = momty_int,.intval = 9 };
static const momint_t int10_mom = {.typnum = momty_int,.intval = 10 };
static const momint_t int11_mom = {.typnum = momty_int,.intval = 11 };
static const momint_t int12_mom = {.typnum = momty_int,.intval = 12 };
static const momint_t int13_mom = {.typnum = momty_int,.intval = 13 };
static const momint_t int14_mom = {.typnum = momty_int,.intval = 14 };
static const momint_t int15_mom = {.typnum = momty_int,.intval = 15 };
static const momint_t int16_mom = {.typnum = momty_int,.intval = 16 };
static const momint_t intm1_mom = {.typnum = momty_int,.intval = -1 };
static const momint_t intm2_mom = {.typnum = momty_int,.intval = -2 };
static const momint_t intm3_mom = {.typnum = momty_int,.intval = -3 };
static const momint_t intm4_mom = {.typnum = momty_int,.intval = -4 };
static const momint_t intm5_mom = {.typnum = momty_int,.intval = -5 };
static const momint_t intm6_mom = {.typnum = momty_int,.intval = -6 };
static const momint_t intm7_mom = {.typnum = momty_int,.intval = -7 };
static const momint_t intm8_mom = {.typnum = momty_int,.intval = -8 };
static const momint_t intm9_mom = {.typnum = momty_int,.intval = -9 };

momval_t
mom_make_integer (int64_t c)
{
  if (c > -127L && c <= 128L)
    switch ((int) c)
      {
      case 0:
	return (momval_t) & int0_mom;
      case 1:
	return (momval_t) & int1_mom;
      case 2:
	return (momval_t) & int2_mom;
      case 3:
	return (momval_t) & int3_mom;
      case 4:
	return (momval_t) & int4_mom;
      case 5:
	return (momval_t) & int5_mom;
      case 6:
	return (momval_t) & int6_mom;
      case 7:
	return (momval_t) & int7_mom;
      case 8:
	return (momval_t) & int8_mom;
      case 9:
	return (momval_t) & int9_mom;
      case 10:
	return (momval_t) & int10_mom;
      case 11:
	return (momval_t) & int11_mom;
      case 12:
	return (momval_t) & int12_mom;
      case 13:
	return (momval_t) & int13_mom;
      case 14:
	return (momval_t) & int14_mom;
      case 15:
	return (momval_t) & int15_mom;
      case 16:
	return (momval_t) & int16_mom;
      case -1:
	return (momval_t) & intm1_mom;
      case -2:
	return (momval_t) & intm2_mom;
      case -3:
	return (momval_t) & intm3_mom;
      case -4:
	return (momval_t) & intm4_mom;
      case -5:
	return (momval_t) & intm5_mom;
      case -6:
	return (momval_t) & intm6_mom;
      case -7:
	return (momval_t) & intm7_mom;
      case -8:
	return (momval_t) & intm8_mom;
      case -9:
	return (momval_t) & intm9_mom;
      default:
	break;
      };
  momint_t *v = GC_MALLOC_ATOMIC (sizeof (momint_t));
  if (MOM_UNLIKELY (!v))
    MOM_FATAPRINTF ("cannot allocate boxed integer");
  memset (v, 0, sizeof (momint_t));
  v->typnum = momty_int;
  v->intval = c;
  return (momval_t) (const momint_t *) v;
}


/*************************** boxed doubles ***************************/
const momdouble_t dbl0_mom = {.typnum = momty_double,.dblval = 0.0 };
const momdouble_t dbl1_mom = {.typnum = momty_double,.dblval = 1.0 };
const momdouble_t dblm1_mom = {.typnum = momty_double,.dblval = -1.0 };

momval_t
mom_make_double (double d)
{
  if (d == 0.0)
    return (momval_t) & dbl0_mom;
  else if (d == 1.0)
    return (momval_t) & dbl1_mom;
  else if (d == -1.0)
    return (momval_t) & dblm1_mom;
  momdouble_t *vd = GC_MALLOC_ATOMIC (sizeof (momdouble_t));
  if (MOM_UNLIKELY (!vd))
    MOM_FATAPRINTF ("failed to make double");
  memset (vd, 0, sizeof (momdouble_t));
  vd->typnum = momty_double;
  vd->dblval = d;
  return (momval_t) (const momdouble_t *) vd;
}



/********************* strings ********************/
momhash_t
mom_cstring_hash (const char *str)
{
  if (!str)
    return 0;
  int len = strlen (str);
  int l = len;
  momhash_t h1 = 0, h2 = len, h;
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


const momstring_t *
mom_make_string (const char *str)
{
  if (!str)
    return NULL;
  const gchar *end = NULL;
  unsigned slen = strlen (str);
  if (MOM_UNLIKELY (slen > MOM_MAX_STRING_LENGTH))
    MOM_FATAPRINTF ("too long %d string to make %.50s", slen, str);
  if (MOM_UNLIKELY (!g_utf8_validate ((const gchar *) str, slen, &end)))
    MOM_FATAPRINTF ("invalid UTF8 in %d-sized string %.50s", slen, str);
  momstring_t *res = GC_MALLOC_ATOMIC (sizeof (momstring_t) + slen + 1);
  if (MOM_UNLIKELY (!res))
    MOM_FATAPRINTF ("failed to allocate string of %d bytes", slen);
  memset (res, 0, sizeof (momstring_t) + slen + 1);
  res->slen = slen;
  res->hash = mom_cstring_hash (str);
  memcpy (res->cstr, str, slen);
  res->typnum = momty_string;
  return res;
}
