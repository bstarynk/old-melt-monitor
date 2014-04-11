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

#include <features.h>		// GNU things
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <gc/gc.h>
#include <sqlite3.h>
#include <uuid/uuid.h>

enum momvaltype_en
{
  momty_none = 0,
  momty_int,
  momty_float,
  momty_string,
  momty_jsonarray,
  momty_jsonobject,
  momty_jsonitem,
};

typedef uint16_t momtynum_t;
typedef uint16_t momspaceid_t;
typedef uint32_t momhash_t;
typedef uint32_t momusize_t;
typedef union momvalueptr_un momval_t;
typedef struct momint_st momint_t;
typedef struct momfloat_st momfloat_t;
typedef struct momstring_st momstring_t;
typedef struct momanyitem_st mom_anyitem_t;
typedef struct momjsonitem_st momit_json_name_t;
union momvalueptr_un
{
  void *ptr;
  momtynum_t *ptype;
  momint_t *pint;
  momfloat_t *pfloat;
  momstring_t *pstring;
  mom_anyitem_t *panyitem;
};

struct momint_st
{
  momtynum_t typnum;
  intptr_t intval;
};

struct momfloat_st
{
  momtynum_t typnum;
  double floval;
};

struct momstring_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  char cstr[];
};

struct momjsonarray_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  momval_t jarrtab[];
};

struct mom_jsonentry_st
{
  momval_t je_name;
  momval_t je_attr;
};

struct momjsonobject_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  struct mom_jsonentry_st jobjtab[];
};

struct mom_attrentry_st
{
};

struct mom_itemattributes_st
{
  momusize_t nbattr;
  struct mom_attrentry_st itattrtab[];
};

struct momanyitem_st
{
  momtynum_t typnum;
  momspaceid_t i_space;
  momhash_t i_hash;
  uuid_t i_uuid;
  pthread_mutex_t i_mtx;
  struct mom_itemattributes_st *i_attrs;
  momval_t i_content;
};

struct momjsonitem_st
{
  struct momanyitem_st ij_item;
  momstring_t *ij_namejson;
};

#define MONIMELT_FATAL_AT(Fil,Lin,Fmt,...) do {		\
  char thname_##Lin[24];				\
  memset (thname_##Lin, 0, sizeof(thname_##Lin));	\
  pthread_getname_np(pthread_self(),thname_##Lin,	\
		     sizeof(thname_##Lin)-1);		\
  fprintf(stderr, "%s:%d <%s> " Fmt "; %m\n", Fil, Lin,	\
	  thname_##Lin, __VA_ARGS__);			\
  abort(); } while(0)
#define MONIMELT_FATAL_AT_BIS(Fil,Lin,Fmt,...) \
  MONIMELT_FATAL_AT(Fil,Lin,Fmt,__VA_ARGS__)
#define MONIMELT_FATAL(Fmt,...) \
  MONIMELT_FATAL_AT_BIS(__FILE__,__LINE__,Fmt,__VA_ARGS__)

momhash_t mom_string_hash (const char *str, int len);
momstring_t *mom_make_string_len (const char *str, int len);
static inline momstring_t *
mom_make_string (const char *str)
{
  return mom_make_string_len (str, -1);
};

momint_t *mom_make_int (intptr_t n);
void mom_initialize (void);
void *mom_allocate_item (unsigned type, size_t itemsize);
void *mom_allocate_item_with_uuid (unsigned type, size_t itemsize,
				   uuid_t uid);
mom_anyitem_t *mom_item_of_uuid (uuid_t);

momit_json_name_t *mom_make_item_json_name_of_uuid (uuid_t, const char *name);
momit_json_name_t *mom_make_item_json_name (const char *name);
#endif /* MONIMELT_INCLUDED_ */
