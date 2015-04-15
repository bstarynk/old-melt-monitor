// file item.c

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

#define ITEM_NUM_SALT_MOM 16


// we choose base 48, because with a 0-9 digit then 8 digits in base 48
// we can express a 48-bit number
// notice that log(2**48/10)/log(48) is 7.9997
#define ID_DIGITS_MOM "0123456789abcdefhijkmnpqrstuvwxyzABCDEFHIJKLMPRU"
#define ID_BASE_MOM 48
static pthread_mutex_t itemtx_random_mom[ITEM_NUM_SALT_MOM];

// convert a 48 bits number to a 10 char string starting with _ then a
// 0-9 digit then 8 extended digits
static const char *
num48_to_char10_mom (uint64_t num, char *buf)
{
  for (int ix = 8; ix > 0; ix--)
    {
      unsigned dig = num % ID_BASE_MOM;
      num = num / ID_BASE_MOM;
      buf[ix + 1] = ID_DIGITS_MOM[dig];
    }
  assert (num <= 9);
  buf[1] = '0' + num;
  buf[0] = '_';
  return buf;
}

uint64_t
char10_to_num48_mom (const char *buf)
{
  uint64_t num = 0;
  if (buf[0] != '_')
    return 0;
  if (buf[1] < '0' || buf[1] > '9')
    return 0;
  for (int ix = 1; ix <= 9; ix++)
    {
      char c = buf[ix];
      char *p = strchr (ID_DIGITS_MOM, c);
      if (!p)
	return 0;
      num = (num * 48 + p - ID_DIGITS_MOM);
    }
  return num;
}

void
mom_initialize_items (void)
{
  static const pthread_mutex_t inimtx = PTHREAD_MUTEX_INITIALIZER;
  for (int ix = 0; ix < ITEM_NUM_SALT_MOM; ix++)
    memcpy (itemtx_random_mom + ix, &inimtx, sizeof (pthread_mutex_t));
  static_assert (sizeof (ID_DIGITS_MOM) - 1 == ID_BASE_MOM,
		 "invalid number of id digits");
  for (int i = 0; i < 4; i++)
    {
      char buf[48];
      memset (buf, 0, sizeof (buf));
      uint64_t n = mom_random_64 (__LINE__) & 0xffffffffffffLL;
      num48_to_char10_mom (n, buf);
      uint64_t u = char10_to_num48_mom (buf);
      printf ("i=%d n=%lld=%#llx buf=%s u=%lld=%#llx\n",
	      i, (long long) n, (long long) n, buf, (long long) u,
	      (long long) u);
      assert (buf[0] && strlen (buf) == 10);
    }
}

momstring_t *
mom_make_random_idstr (unsigned salt, struct momitem_st *protoitem)
{
  momstring_t *str = NULL;
  assert (!protoitem || protoitem->itm_id == NULL);
  do
    {
      uint32_t r1 = 0, r2 = 0, r3 = 0;
      uint64_t hi = 0, lo = 0;	/* actually 48 bits unsigned each */
      mom_random_three_nonzero_32 (salt, &r1, &r2, &r3);
      hi = ((uint64_t) r1) << 32 | (uint64_t) (r2 >> 16);
      lo = (((uint64_t) (r2 & 0xffff)) << 32) | ((uint64_t) r3);
      // replace the four highest bits of 48 bits number hi with salt&0xf
      hi &= 0xffffffffff;
      hi |= ((uint64_t) (salt & 0xf) << 40);
      if (hi == 0 || lo == 0)
	continue;
    }
  while (!str);
  return str;
}
