// file monimelt.h

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
#ifndef MONIMELT_INCLUDED_
#define MONIMELT_INCLUDED_ 1
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#include <features.h> // GNU things
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <gc/gc.h>
#include <sqlite3.h>

enum momvaltype_en {
  momty_none=0,
  momty_int,
  momty_float,
  momty_string,
  momty_jsonarray,
  momty_jsonobject
};

typedef uint16_t momtynum_t;
typedef uint32_t momhash_t;
typedef uint32_t momusize_t;
typedef union monvalueptr_un monval_t;
typedef struct momint_st momint_t;
typedef struct momfloat_st momfloat_t;
typedef struct momstring_st momstring_t;

union monvalueptr_un {
  momtynum_t* ptype;
  momint_t* pint;
  momfloat_t* pfloat;
  momstring_t* pstring;
};

struct momint_st {
  momtynum_t typnum;
  intptr_t intval;
};

struct momfloat_st {
  momtynum_t typnum;
  double floval;
};

struct momstring_st {
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  char cstr[];
};

struct momjsonarray_st {
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  monval_t jvaltab[];
};

momhash_t mom_string_hash(const char*str, int len);
momstring_t* mom_make_string_len(const char*str, int len);
static inline momstring_t* mom_make_string(const char*str)
{ return mom_make_string_len(str, -1); };
momint_t* mom_make_int(intptr_t n);

#endif /* MONIMELT_INCLUDED_ */

