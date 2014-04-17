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
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <gc/gc.h>
#include <sqlite3.h>
#include <uuid/uuid.h>
#include <glib.h>
#include <gmodule.h>



#define MONIMELT_EMPTY ((void*)(-1L))
enum momvaltype_en
{
  momty_null = 0,
  momty_int,
  momty_float,
  momty_string,
  momty_jsonarray,
  momty_jsonobject,
  momty_itemset,
  momty_itemtuple,
  momty_node,
  momty_closure,
  /// items below
  momty__itemlowtype,
  momty_jsonitem,
  momty_boolitem,
  momty__last = 1000
};


typedef uint16_t momtynum_t;
typedef uint16_t momspaceid_t;
typedef uint32_t momhash_t;
typedef uint32_t momusize_t;
typedef union momvalueptr_un momval_t;
typedef struct momint_st momint_t;
typedef struct momfloat_st momfloat_t;
typedef struct momstring_st momstring_t;
typedef struct momjsonobject_st momjsonobject_t;
typedef struct momjsonarray_st momjsonarray_t;
typedef struct momnode_st momnode_t;
typedef struct momseqitem_st momseqitem_t;
typedef struct momseqitem_st momitemset_t;
typedef struct momseqitem_st momitemtuple_t;
typedef struct momanyitem_st mom_anyitem_t;
typedef struct momjsonitem_st momit_json_name_t;
typedef struct momboolitem_st momit_bool_t;
union momvalueptr_un
{
  void *ptr;
  momtynum_t *ptype;
  const momint_t *pint;
  const momfloat_t *pfloat;
  const momstring_t *pstring;
  mom_anyitem_t *panyitem;
  struct momjsonarray_st *pjsonarr;
  struct momjsonobject_st *pjsonobj;
  struct momjsonitem_st *pjsonitem;
  struct momboolitem_st *pboolitem;
  struct momnode_st *pnode;
  struct momseqitem_st *pseqitm;
  struct momseqitem_st *pitemset;
  struct momseqitem_st *pitemtuple;
};

struct mom_dumper_st;

// function to load and build an item from its building json and uid
typedef mom_anyitem_t mom_item_loader_sig_t (momval_t json, uuid_t uid);
// function to fill an item from its filling json
typedef void mom_item_filler_sig_t (mom_anyitem_t * itm, momval_t json);
// function to scan an item
typedef void mom_item_scan_sig_t (struct mom_dumper_st *dmp,
				  mom_anyitem_t * itm);
// function to get the building json of an item
typedef momval_t mom_item_get_build_sig_t (mom_anyitem_t * itm);
// function to get the filling json of an item
typedef momval_t mom_item_get_fill_sig_t (mom_anyitem_t * itm);


// the item type FOO is described by momitype_FOO of following type:
struct momitemtypedescr_st
{
  const char *ityp_name;
  mom_item_loader_sig_t *ityp_loader;
  mom_item_filler_sig_t *ityp_filler;
  mom_item_scan_sig_t *ityp_scan;
  mom_item_get_build_sig_t *ityp_getbuild;
  mom_item_get_fill_sig_t *ityp_getfill;
};
struct momitemtypedescr_st *mom_typedescr_array[momty__last];

#ifdef __GNUC__
#define MONIMELT_UNLIKELY(P) __builtin_expect((P),0)
#else
#define MONIMELT_UNLIKELY(P) (P)
#endif

#define MONIMELT_NULLV ((union momvalueptr_un)((void*)0))
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


struct momnode_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  mom_anyitem_t *connitm;
  momval_t sontab[];
};


// for sets and tuples of item. In sets, itemseq is sorted by
// increasing uuids. In tuples, some itemseq components may be nil.
struct momseqitem_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  mom_anyitem_t *itemseq[];
};
struct mom_attrentry_st
{
  mom_anyitem_t *aten_itm;
  momval_t aten_val;
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
  const momstring_t *ij_namejson;
};

struct momboolitem_st
{
  struct momanyitem_st ib_item;
  bool ib_bool;
};

static inline mom_anyitem_t *
mom_value_as_item (momval_t val)
{
  if (!val.ptr)
    return NULL;
  if (*val.ptype > momty__itemlowtype)
    return val.panyitem;
  return NULL;
}

static inline momval_t
mom_value_json (momval_t val)
{
  if (!val.ptr)
    return MONIMELT_NULLV;
  switch (*val.ptype)
    {
    case momty_int:
      return val;
    case momty_float:
      return val;
    case momty_string:
      return val;
    case momty_jsonarray:
      return val;
    case momty_jsonobject:
      return val;
    case momty_jsonitem:
      return val;
    case momty_boolitem:
      return val;
    default:
      return MONIMELT_NULLV;
    }
}

//////////////// integer values
static inline const momint_t *
mom_value_as_int (momval_t val)
{
  if (!val.ptr)
    return NULL;
  if (*val.ptype == momty_int)
    return val.pint;
  return NULL;
}

const momint_t *mom_make_int (intptr_t i);

int mom_item_cmp (const mom_anyitem_t * l, const mom_anyitem_t * r);
int mom_value_cmp (const momval_t l, const momval_t r);
momhash_t mom_value_hash (const momval_t v);

static inline intptr_t
mom_int_of_value (momval_t val, intptr_t def)
{
  if (val.ptr && *val.ptype == momty_int)
    return val.pint->intval;
  return def;
}

#define mom_int_of_value_else_0(V) mom_int_of_value((V),0L)


////////////// float values
const momfloat_t *mom_make_double (double x);

static inline const momfloat_t *
mom_value_as_float (momval_t val)
{
  if (!val.ptr)
    return NULL;
  if (*val.ptype == momty_float)
    return val.pfloat;
  return NULL;
}


///////////// string values
static inline const momstring_t *
mom_value_as_string (momval_t val)
{
  if (!val.ptr)
    return NULL;
  if (*val.ptype == momty_string)
    return val.pstring;
  return NULL;
}


void
mom_fatal_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4), noreturn));

#define MONIMELT_FATAL_AT(Fil,Lin,Fmt,...) do {         \
  mom_fatal_at(Fil,Lin,Fmt,##__VA_ARGS__);} while(0)
#define MONIMELT_FATAL_AT_BIS(Fil,Lin,Fmt,...) \
  MONIMELT_FATAL_AT(Fil,Lin,Fmt,##__VA_ARGS__)
#define MONIMELT_FATAL(Fmt,...) \
  MONIMELT_FATAL_AT_BIS(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

momhash_t mom_string_hash (const char *str, int len);
const momstring_t *mom_make_string_len (const char *str, int len);
static inline const momstring_t *
mom_make_string (const char *str)
{
  return mom_make_string_len (str, -1);
};

const momint_t *mom_make_int (intptr_t n);
void mom_initialize (void);
void *mom_allocate_item (unsigned type, size_t itemsize);
void *mom_allocate_item_with_uuid (unsigned type, size_t itemsize,
				   uuid_t uid);
mom_anyitem_t *mom_item_of_uuid (uuid_t);

momit_json_name_t *mom_make_item_json_name_of_uuid (uuid_t, const char *name);
momit_json_name_t *mom_make_item_json_name (const char *name);
#define mom_create__json_name(Name,Uid) \
  mom_make_item_json_name_of_uuid(Uid,#Name)


static inline bool
mom_is_jsonable (const momval_t val)
{
  if (!val.ptr)
    return true;
  else if (val.ptr == MONIMELT_EMPTY)
    return false;
  else
    switch (*val.ptype)
      {
      case momty_int:
      case momty_float:
      case momty_string:
      case momty_jsonarray:
      case momty_jsonobject:
      case momty_jsonitem:
      case momty_boolitem:
	return true;
      default:
	return false;
      }
}

momit_bool_t *mom_create_named_bool (uuid_t uid, const char *name);
#define mom_create__bool(Name,Uid) \
  mom_create_named_bool(Uid,#Name)

static inline momit_bool_t *
mom_get_item_bool (bool b)
{
  // defined and initialized in create-items.c using monimelt-named.h
  extern momit_bool_t *mom_item__true;
  extern momit_bool_t *mom_item__false;
  if (b)
    return mom_item__true;
  else
    return mom_item__false;
}

static inline bool
mom_item_is_true (mom_anyitem_t * itm)
{
  extern momit_bool_t *mom_item__true;
  return itm == (mom_anyitem_t *) mom_item__true;
}

static inline bool
mom_item_is_false (mom_anyitem_t * itm)
{
  extern momit_bool_t *mom_item__false;
  return itm == (mom_anyitem_t *) mom_item__false;
}


/// global data, managed by functions
// register a new name, nop if existing entry
void mom_register_new_name_item (const char *name, mom_anyitem_t * item);
void mom_register_new_name_string (momstring_t * namestr,
				   mom_anyitem_t * item);

// register a name, replacing any previous entries
void mom_replace_named_item (const char *name, mom_anyitem_t * item);
void mom_replace_name_string (momstring_t * namestr, mom_anyitem_t * item);

// get the item of some given name, or else NULL
mom_anyitem_t *mom_item_named (const char *name);
// also retrieve the string
mom_anyitem_t *mom_item_named_with_string (const char *name,
					   const momstring_t ** pstr);

// get the name of some given item, or else NULL
const momstring_t *mom_name_of_item (const mom_anyitem_t * item);

// compare values for JSON
int mom_json_cmp (momval_t l, momval_t r);
const momval_t mom_jsonob_get_def (const momval_t jsobv, const momval_t namev,
				   const momval_t def);
static inline const momval_t
mom_jsonob_get (const momval_t jsobv, const momval_t namev)
{
  return mom_jsonob_get_def (jsobv, namev, MONIMELT_NULLV);
}

momjsonobject_t *mom_make_json_object (int, ...) __attribute__ ((sentinel));
enum momjsondirective_en
{
  MOMJSON_END,
  MOMJSON_ENTRY,		/* momval_t nameval, momval_t attrval */
  MOMJSON_STRING,		/* const char*namestr, momval_t attval */
  MOMJSON_COUNTED_ENTRIES,	/* unsigned count, struct mom_jsonentry_st* */
};

// make a JSON array of given count
const momjsonarray_t *mom_make_json_array (unsigned nbelem, ...);
const momjsonarray_t *mom_make_json_array_count (unsigned count,
						 const momval_t * arr);
const momjsonarray_t *mom_make_json_array_til_nil (momval_t, ...)
  __attribute__ ((sentinel));

// make a set from items, or sets, or tuples
const momitemset_t *mom_make_item_set_til_nil (momval_t, ...)
  __attribute__ ((sentinel));
// make an tuple from items, or sets, or tuples
const momitemtuple_t *mom_make_item_tuple_til_nil (momval_t, ...)
  __attribute__ ((sentinel));
// make a set of given number of items, removing duplicates...
const momitemset_t *mom_make_item_set_sized (unsigned siz, ...);
// make a tuple of given number of items
const momitemtuple_t *mom_make_item_tuple_sized (unsigned siz, ...);

///// JSON parsing:
struct jsonparser_st
{
  uint32_t jsonp_magic;		/* always MOMJSONP_MAGIC */
  int jsonp_c;			/* read ahead character */
  pthread_mutex_t jsonp_mtx;
  FILE *jsonp_file;
  void *jsonp_data;
  char *jsonp_error;
  jmp_buf jsonp_jmpbuf;
};

// initialize a JSON parser
void mom_initialize_json_parser (struct jsonparser_st *jp, FILE * file,
				 void *data);
// get its data
void *mom_json_parser_data (const struct jsonparser_st *jp);
// end the parsing without closing the file
void mom_end_json_parser (struct jsonparser_st *jp);
// end the parsing and close the file
void mom_close_json_parser (struct jsonparser_st *jp);
// parse a JSON value, or else set the error message to *perrmsg
momval_t mom_parse_json (struct jsonparser_st *jp, char **perrmsg);


///// JSON output
struct jsonoutput_st
{
  uint32_t jsono_magic;		/* always MOMJSONO_MAGIC */
  uint32_t jsono_flags;
  pthread_mutex_t jsono_mtx;
  FILE *jsono_file;
  void *jsono_data;
};

enum jsonoutflags_en
{
  jsof_none = 0,
  jsof_indent = 1 << 0,		/* indent the output */
  jsof_flush = 1 << 1,		/* flush the output at end */
  jsof_cname = 1 << 2,		/* output C identifiers at names in JSON objects */
};

// initialize the output
void mom_json_output_initialize (struct jsonoutput_st *jo, FILE * f,
				 void *data, unsigned flags);

// retrieve the client data
void *mom_json_output_data (const struct jsonoutput_st *jo);

// end the output without closing the file
void mom_json_output_end (struct jsonoutput_st *jo);

// end the output and close the file
void mom_json_output_close (struct jsonoutput_st *jo);

// output a JSON value
void mom_output_json (struct jsonoutput_st *jo, const momval_t val);

// every module should have
extern const char monimelt_GPL_friendly_module[];
extern void monimelt_module_init (const char *marg);
// modules may also define
extern GOptionGroup *monimelt_module_option_group (const char *modname);


//// dumper, see file dump.c
struct mom_dumper_st
{
  unsigned dmp_magic;		/* always DUMPER_MAGIC */
  unsigned dmp_count;
  unsigned dmp_size;
  unsigned dmp_state;
  pthread_mutex_t dmp_mtx;
  struct mom_dumperqueue_st *dmp_qfirst;
  struct mom_dumperqueue_st *dmp_qlast;
  mom_anyitem_t **dmp_array;
};

// initialize a dumper
void mom_dump_initialize (struct mom_dumper_st *dmp);

// add a scanned item into a dumper
void mom_dump_add_item (struct mom_dumper_st *dmp, mom_anyitem_t * itm);

// scan a value into a dumper
void mom_dump_scan_value (struct mom_dumper_st *dmp, const momval_t val);
#endif /* MONIMELT_INCLUDED_ */
