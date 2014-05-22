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

#define GC_THREADS 1
#define HAVE_PTHREADS 1

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
#include <sched.h>
#include <syslog.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <execinfo.h>
// eventfd(2) & signalfd(2) & timerfd_create(2) are Linux specific
#include <sys/eventfd.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
///
#include <sqlite3.h>
#include <gc/gc.h>
#include <glib.h>
// libonion from http://www.coralbits.com/libonion/ &
// https://github.com/davidmoreno/onion
#include <onion/onion.h>
#include <onion/low.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/handler.h>
#include <onion/dict.h>
#include <onion/log.h>
#include <onion/shortcuts.h>
#include <onion/handlers/internal_status.h>
#include <onion/handlers/exportlocal.h>
#include <onion/handlers/path.h>
#include <onion/handlers/static.h>
#include <onion/handlers/auth_pam.h>


// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];

// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define MOM_UNLIKELY(P) __builtin_expect((P),0)
#else
#define MOM_UNLIKELY(P) (P)
#endif

static inline pid_t
mom_gettid (void)
{
  return syscall (SYS_gettid, 0L);
}

// most versions of Boehm garbage collector (see
// http://hboehm.info/gc/ for more) don't have it, so define::
#ifndef GC_CALLOC
#define MOM_NEED_GC_CALLOC 1
extern void *GC_calloc (size_t nbelem, size_t elsiz);
#define GC_CALLOC(NbElem,ElSiz) GC_calloc(NbElem,ElSiz)
#endif /*GC_CALLOC */

// empty placeholder in hashes
#define MOM_EMPTY ((void*)(-1L))

// we handle specially "tiny" data, e.g. by stack-allocating temporories.
#define MOM_TINY_MAX 8

#define MOM_PATH_MAX 256

// query a clock
static inline double
mom_clock_time (clockid_t cid)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (cid, &ts))
    return NAN;
  else
    return (double) ts.tv_sec + 1.0e-9 * ts.tv_nsec;
}


// call strftime on ti, but replace .__ with centiseconds for ti
char *mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
  __attribute__ ((format (strftime, 3, 0)));
#define mom_now_strftime_centi(Buf,Len,Fmt) mom_strftime_centi((Buf),(Len),(Fmt),mom_clock_time(CLOCK_REALTIME))
#define mom_now_strftime_bufcenti(Buf,Fmt) mom_now_strftime_centi(Buf,sizeof(Buf),(Fmt))
// return the elapsed real time since start of process
double mom_elapsed_real_time (void);

static inline struct timespec
mom_timespec (double t)
{
  struct timespec ts = { 0, 0 };
  if (isnan (t) || t < 0.0)
    return ts;
  double fl = floor (t);
  ts.tv_sec = (time_t) fl;
  ts.tv_nsec = (long) ((t - fl) * 1.0e9);
  // this should not happen
  if (MOM_UNLIKELY (ts.tv_nsec < 0))
    ts.tv_nsec = 0;
  while (MOM_UNLIKELY (ts.tv_nsec >= 1000 * 1000 * 1000))
    {
      ts.tv_sec++;
      ts.tv_nsec -= 1000 * 1000 * 1000;
    };
  return ts;
}


typedef uint8_t momtynum_t;
typedef uint16_t momspaceid_t;
typedef uint32_t momhash_t;
typedef uint32_t momusize_t;



pthread_mutexattr_t mom_normal_mutex_attr;
pthread_mutexattr_t mom_recursive_mutex_attr;



//////////////// wrapping Boehm GC allocation
// using GNU extension: statement expression
#define MOM_GC_ALLOC_AT(Msg,Siz,Lin) ({ \
      size_t _sz_##Lin = Siz;					\
      void* _p_##Lin =						\
	(_sz_##Lin>0)?GC_MALLOC(_sz_##Lin):NULL;		\
  if (MOM_UNLIKELY(!_p_##Lin && _sz_##Lin>0))			\
    MOM_FATAPRINTF("failed to allocate %ld bytes:" Msg,		\
	      (long) _sz_##Lin);				\
  if (_sz_##Lin>0)						\
    memset (_p_##Lin, 0, _sz_##Lin);				\
  _p_##Lin; })
#define MOM_GC_ALLOC(Msg,Siz) \
  MOM_GC_ALLOC_AT(Msg,Siz,__LINE__)

#define MOM_GC_SCALAR_ALLOC_AT(Msg,Siz,Lin) ({			\
  size_t _sz_##Lin = Siz;					\
  void* _p_##Lin =						\
	(_sz_##Lin>0)?GC_MALLOC_ATOMIC(_sz_##Lin):NULL;		\
  if (MOM_UNLIKELY(!_p_##Lin))					\
    MOM_FATAPRINTF("failed to allocate %ld scalar bytes:" Msg,	\
		(long) _sz_##Lin);				\
  memset (_p_##Lin, 0, _sz_##Lin);				\
  _p_##Lin; })
#define MOM_GC_SCALAR_ALLOC(Msg,Siz) \
  MOM_GC_SCALAR_ALLOC_AT(Msg,Siz,__LINE__)

#define MOM_GC_STRDUP_AT(Msg,Str,Lin) ({		\
const char* _str_##Lin = (Str);				\
const char* _dup_##Lin = GC_STRDUP(_str_##Lin);		\
  if (MOM_UNLIKELY(!_dup_##Lin && _str_##Lin))		\
    MOM_FATAPRINTF("failed to duplicate string %s",	\
		   _str_##Lin);				\
  _dup_##Lin; })
#define MOM_GC_STRDUP(Msg,Str) MOM_GC_STRDUP_AT((Msg),(Str),__LINE__)

// free a pointer and clear the variable
#define MOM_GC_FREE_AT(VarPtr,Lin) do {				\
    void* _fp_##Lin = VarPtr;					\
    VarPtr = NULL;						\
    if (_fp_##Lin != NULL) GC_FREE(_fp_##Lin); } while(0)
#define MOM_GC_FREE(VarPtr) MOM_GC_FREE_AT(VarPtr,__LINE__)

// GCC compiler trick to add some typechecking in variadic functions
#define MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)				\
  (__builtin_choose_expr((__builtin_types_compatible_p(typeof(V),Typ)), \
			 (V), (void)((Else)+Lin)))
#define MOM_REQUIRES_TYPE_AT_BIS(Lin,V,Typ,Else) MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)
#define MOM_REQUIRES_TYPE(V,Typ,Else) MOM_REQUIRES_TYPE_AT_BIS(__LINE__,(V),Typ,Else)

////////////////////////////////////////////////////////////////
//////////////// TYPES AND VALUES
////////////////////////////////////////////////////////////////
enum momvaltype_en
{
  momty_null = 0,
  momty_int,
  momty_double,
  momty_string,
  momty_jsonarray,
  momty_jsonobject,
  momty_set,
  momty_tuple,
  momty_node,
  momty_item
};

struct momseqitem_st;

typedef struct momint_st momint_t;
typedef struct momdouble_st momdouble_t;
typedef struct momstring_st momstring_t;
typedef struct momjsonobject_st momjsonobject_t;
typedef struct momjsonarray_st momjsonarray_t;
typedef struct momseqitem_st momseqitem_t;
typedef struct momseqitem_st momset_t;
typedef struct momseqitem_st momtuple_t;
typedef struct momnode_st momnode_t;
typedef struct momitem_st momitem_t;
union momvalueptr_un
{
  void *ptr;
  const momtynum_t *ptype;
  const momint_t *pint;
  const momdouble_t *pdouble;
  const momstring_t *pstring;
  const momjsonobject_t *pjsonobj;
  const momjsonarray_t *pjsonarr;
  const momseqitem_t *pseqitems;
  const momset_t *pset;
  const momtuple_t *ptuple;
  const momnode_t *pnode;
  momitem_t *pitem;
  const momitem_t *pitemk;
};
typedef union momvalueptr_un momval_t;
#define MOM_NULLV ((momval_t)NULL)

// nonzero hash, except for null
momhash_t mom_value_hash (const momval_t v);

// compare
int mom_value_cmp (const momval_t l, const momval_t r);

static inline momtynum_t
mom_type (const momval_t v)
{
  if (v.ptr == NULL)
    return momty_null;
  else
    return *v.ptype;
}

/*************************** boxed integers ***************************/
struct momint_st
{
  momtynum_t typnum;
  int64_t intval;
};



static inline bool
mom_is_integer (momval_t v)
{
  return (v.ptr && v.pint->typnum == momty_int);
}

static inline int64_t
mom_integer_val_def (momval_t v, int64_t def)
{
  return (v.ptr && v.pint->typnum == momty_int) ? (v.pint->intval) : def;
}

#define mom_integer_val(V) mom_integer_val_def((V),0)
momval_t mom_make_integer (int64_t c);

/*************************** boxed doubles ***************************/
struct momdouble_st
{
  momtynum_t typnum;
  double dblval;
};
static inline bool
mom_is_double (momval_t v)
{
  return (v.ptr && v.pdouble->typnum == momty_double);
}

static inline double
mom_double_val_def (momval_t v, double def)
{
  return (v.ptr
	  && v.pdouble->typnum == momty_double) ? (v.pdouble->dblval) : def;
}

#define mom_double_val_else_0(V) mom_double_val_def((V),0.0)
#define mom_double_val(V) mom_double_val_def((V),NAN)
momval_t mom_make_double (double d);

/*************************** strings ***************************/
#define MOM_MAX_STRING_LENGTH (1<<25)	/* max string length 33554432 */
struct momstring_st
{
  momtynum_t typnum;
  momusize_t slen;		/* length in bytes of cstr */
  momhash_t hash;
  char cstr[];			/* zero terminated */
};

momhash_t mom_cstring_hash (const char *str);
// make a boxed UTF8 string
const momstring_t *mom_make_string (const char *str);
const momstring_t *mom_make_string_len (const char *str, unsigned len);
// make a random id string, starting with _ then a digit, in total 24 characters
const momstring_t *mom_make_random_idstr ();
// check that s points to a string looking like a random id string;
// the ending character should not be alphanumerical but it might be _
// or a space or delimiter and is stored in *pend if pend non-null.
bool mom_looks_like_random_id_cstr (const char *s, const char **pend);
#define MOM_IDSTRING_LEN 24
static inline bool
mom_is_string (momval_t v)
{
  return (v.ptr && v.pstring->typnum == momty_string);
}

static inline momhash_t
mom_string_hash (momval_t v)
{
  return (v.ptr && v.pstring->typnum == momty_string) ? (v.pstring->hash) : 0;
}

static inline const char *
mom_string_cstr (momval_t v)
{
  return (v.ptr
	  && v.pstring->typnum == momty_string) ? (v.pstring->cstr) : NULL;
}

static inline unsigned
mom_string_slen (momval_t v)
{
  return (v.ptr && v.pstring->typnum == momty_string) ? (v.pstring->slen) : 0;
}

static inline bool
mom_string_same (momval_t v, const char *cstr)
{
  return (v.ptr && v.pstring->typnum == momty_string && cstr
	  && !strcmp (v.pstring->cstr, cstr));
}

////////////////////////////////////////////////////////////////
/////////// JSON VALUES
////////////////////////////////////////////////////////////////

struct momjsonarray_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  momval_t jarrtab[];
};

/// entries are sorted on the ident string for items or on the string
struct mom_jsonentry_st
{
  union
  {
    momval_t je_name;
    const momstring_t *je_namestr;
    const momitem_t *je_nameitm;
  };
  momval_t je_attr;
};

struct momjsonobject_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  struct mom_jsonentry_st jobjtab[];
};

// dynamically cast any value to a JSON value or else null
static inline momval_t
mom_value_json (momval_t val)
{
  if (!val.ptr)
    return MOM_NULLV;
  switch (*val.ptype)
    {
    case momty_int:
    case momty_double:
    case momty_string:
    case momty_jsonobject:
    case momty_jsonarray:
    case momty_item:
      return val;
    default:
      return MOM_NULLV;
    }
}

static inline bool
mom_is_jsonable (momval_t val)
{
  if (!val.ptr)
    return true;
  switch (*val.ptype)
    {
    case momty_int:
    case momty_double:
    case momty_string:
    case momty_jsonobject:
    case momty_jsonarray:
    case momty_item:
      return true;
    default:
      return false;
    }
}

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

// compare values for JSON
int mom_json_cmp (momval_t l, momval_t r);
// compare a JSON value to a non-null string
int mom_json_cstr_cmp (momval_t jv, const char *str);

momval_t mom_jsonob_getstr (const momval_t jsobv, const char *name);

momval_t mom_jsonob_get_def (const momval_t jsobv, const momval_t namev,
			     const momval_t def);
static inline momval_t
mom_jsonob_get (const momval_t jsobv, const momval_t namev)
{
  return mom_jsonob_get_def (jsobv, namev, MOM_NULLV);
}

static inline unsigned
mom_jsonob_size (momval_t jsobv)
{
  if (!jsobv.ptr || *jsobv.ptype != momty_jsonobject)
    return 0;
  return jsobv.pjsonobj->slen;
}

const momjsonobject_t *mom_make_json_object (int, ...)
  __attribute__ ((sentinel));
enum momjsondirective_en
{
  MOMJSON__END,
  MOMJSONDIR__ENTRY,		/* momval_t nameval, momval_t attrval */
  MOMJSONDIR__STRING,		/* const char*namestr, momval_t attval */
  MOMJSONDIR__COUNTED_ENTRIES,	/* unsigned count, struct mom_jsonentry_st* */
};

#define MOMJSON_END ((void*)MOMJSON__END)

#define MOMJSOB_ENTRY(N,V) MOMJSONDIR__ENTRY,	\
    MOM_REQUIRES_TYPE(N,momval_t,mombad_value), \
    MOM_REQUIRES_TYPE(V,momval_t,mombad_value)

#define MOMJSOB_STRING(S,V) MOMJSONDIR__STRING,		\
    MOM_REQUIRES_TYPE(S,const char*,mombad_string),	\
    MOM_REQUIRES_TYPE(V,momval_t,mombad_value)

#define MOMJSOB_COUNTED_ENTRIES(C,E)  MOMJSONDIR__COUNTED_ENTRIES,	\
    MOM_REQUIRES_TYPE(C,unsigned,mombad_unsigned),			\
    MOM_REQUIRES_TYPE(E,struct mom_jsonentry_st*,mombad_entries)


// make a JSON array of given count
const momjsonarray_t *mom_make_json_array (unsigned nbelem, ...);
const momjsonarray_t *mom_make_json_array_count (unsigned count,
						 const momval_t *arr);
const momjsonarray_t *mom_make_json_array_til_nil (momval_t, ...)
  __attribute__ ((sentinel));

static inline unsigned
mom_json_array_size (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_jsonarray)
    return 0;
  return val.pjsonarr->slen;
}

static inline momval_t
mom_json_array_nth (momval_t val, int rk)
{
  if (!val.ptr || *val.ptype != momty_jsonarray)
    return MOM_NULLV;
  unsigned slen = val.pjsonarr->slen;
  if (rk < 0)
    rk += slen;
  if (rk >= 0 && rk < (int) slen)
    return val.pjsonarr->jarrtab[rk];
  return MOM_NULLV;
}


////////////////////////////////////////////////////////////////
/////////// ITEMS
////////////////////////////////////////////////////////////////
struct mom_attrentry_st
{
  momitem_t *aten_itm;
  momval_t aten_val;
};

struct mom_itemattributes_st	// an hash table
{
  momusize_t nbattr;
  momusize_t size;
  struct mom_attrentry_st itattrtab[];	/* of size entries */
};
#define MOM_ITEM_MAGIC 0x5ce6881	/* mom item magic 97413249 */
struct momitem_st
{
  const momtynum_t i_typnum;	/* always momty_item */
  momspaceid_t i_space;
  const momhash_t i_hash;	/* same as i_idstr->hash */
  const unsigned i_magic;	/* always MOM_ITEM_MAGIC */
  pthread_mutex_t i_mtx;
  const momstring_t *i_idstr;	/* id string */
  const momstring_t *i_name;	/* name, or NULL */
  struct mom_itemattributes_st *i_attrs;
  momval_t i_content;
  uint16_t i_paylkind;
  uint16_t i_paylxtra;
  void *i_payload;
};

static inline bool
mom_is_item (momval_t v)
{
  return (v.ptr && v.pitem->i_typnum == momty_item
	  && v.pitem->i_magic == MOM_ITEM_MAGIC);
}

static inline int
mom_item_cmp (const momitem_t *itm1, const momitem_t *itm2)
{
  if (itm1 == itm2)
    return 0;
  if (!mom_is_item ((momval_t) itm1))
    return -1;
  if (!mom_is_item ((momval_t) itm2))
    return 1;
  const momstring_t *ids1 = itm1->i_idstr;
  const momstring_t *ids2 = itm2->i_idstr;
  assert (ids1 && ids1->typnum == momty_string
	  && ids1->slen == MOM_IDSTRING_LEN);
  assert (ids2 && ids2->typnum == momty_string
	  && ids2->slen == MOM_IDSTRING_LEN);
  int cmp = memcmp (ids1->cstr, ids2->cstr, MOM_IDSTRING_LEN);
  assert (cmp != 0);		// only identical items should have equal idstr
  return cmp;
}

// make a new item  -- low-level
momitem_t *mom_make_item (void);

// make or get an item of given idstr or else -when bad idstr- NULL
momitem_t *mom_make_item_of_ident (const momstring_t *idstr);
// make or get an item of given idstr or else -when bad idcstr- NULL
momitem_t *mom_make_item_of_identcstr (const char *idcstr);

// get an item of given idstr or else NULL
momitem_t *mom_get_item_of_ident (const momstring_t *idstr);
// get an item of given idstr or else NULL
momitem_t *mom_get_item_of_identcstr (const char *idcstr);

// register an item with a given name
void mom_register_item_named (momitem_t *itm, const momstring_t *name);
static inline void
mom_register_item_named_cstr (momitem_t *itm, const char *namestr)
{
  if (!itm || itm->i_typnum != momty_item || !namestr || !namestr[0])
    return;
  mom_register_item_named (itm, mom_make_string (namestr));
}


struct mom_itemattributes_st *mom_put_attribute (struct mom_itemattributes_st
						 *attrs,
						 const momitem_t *atitm,
						 const momval_t val);
struct mom_itemattributes_st *mom_remove_attribute (struct
						    mom_itemattributes_st *,
						    const momitem_t *atitm);
struct mom_itemattributes_st *mom_reserve_attribute (struct
						     mom_itemattributes_st *,
						     unsigned gap);
static inline momval_t
mom_get_attribute (const struct mom_itemattributes_st *const attrs,
		   const momitem_t *atitm)
{
  if (!attrs || !atitm)
    return MOM_NULLV;
  unsigned siz = attrs->size;
  assert (siz > 0);
  assert (atitm->i_typnum == momty_item && atitm->i_magic == MOM_ITEM_MAGIC);
  if (siz < MOM_TINY_MAX)
    {
      for (unsigned ix = 0; ix < siz; ix++)
	if (attrs->itattrtab[ix].aten_itm == atitm)
	  return attrs->itattrtab[ix].aten_val;
    }
  else
    {
      unsigned istart = atitm->i_hash % siz;
      for (unsigned ix = istart; ix < siz; ix++)
	{
	  const momitem_t *curatitm = attrs->itattrtab[ix].aten_itm;
	  if (curatitm == atitm)
	    return attrs->itattrtab[ix].aten_val;
	  if (!curatitm)
	    return MOM_NULLV;
	  else if (curatitm == MOM_EMPTY)
	    continue;
	}
      for (unsigned ix = 0; ix < istart; ix++)
	{
	  const momitem_t *curatitm = attrs->itattrtab[ix].aten_itm;
	  if (curatitm == atitm)
	    return attrs->itattrtab[ix].aten_val;
	  if (!curatitm)
	    return MOM_NULLV;
	  else if (curatitm == MOM_EMPTY)
	    continue;
	}
    }
  return MOM_NULLV;
}

/// get the set of named items, ordered by item ids
const momset_t *mom_set_of_named_items (void);
/// get the tuple of named items, alphabetically ordered by name
/// if parrname is not-null, set it to the jsonarray of names
const momtuple_t *mom_alpha_ordered_tuple_of_named_items (momval_t *parrname);

const momstring_t *mom_item_get_name (momitem_t *itm);
const momstring_t *mom_item_get_idstr (momitem_t *itm);
const momstring_t *mom_item_get_name_or_idstr (momitem_t *itm);
// get an item of given name
momitem_t *mom_get_item_of_name_hash (const char *s, momhash_t h);
#define mom_get_item_of_name(S) mom_get_item_of_name_hash((S),0)

static inline const char *
mom_ident_cstr_of_item (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  return mom_string_cstr ((momval_t) itm->i_idstr);
}

// get an item of given name or ident
momitem_t *mom_get_item_of_name_or_ident_cstr_hash (const char *s,
						    momhash_t h);
// get an item of given name or ident
#define mom_get_item_of_name_or_ident_cstr(S) \
  mom_get_item_of_name_or_ident_cstr_hash ((S), 0)
static inline momitem_t *
mom_get_item_of_name_or_ident_string (momval_t s)
{
  if (s.ptr && s.pstring->typnum == momty_string)
    return mom_get_item_of_name_or_ident_cstr_hash (s.pstring->cstr,
						    s.pstring->hash);
  return NULL;
}

void mom_forget_name (const char *namestr);

static inline void
mom_forget_name_string (const momstring_t *namev)
{
  if (namev && namev->typnum == momty_string)
    mom_forget_name (namev->cstr);
}

static inline void
mom_forget_item (momitem_t *itm)
{
  mom_forget_name (mom_string_cstr ((momval_t) mom_item_get_name (itm)));
};


////////////////////////////////////////////////////////////////
struct mom_dumper_st;
struct mom_loader_st;
/************** payload descriptors ********************/
#define MOM_PAYLOAD_MAGIC 0x128ffdcb	/* payload magic 311426507 */
struct mom_payload_descr_st
{
  unsigned dpayl_magic;		/* always MOM_PAYLOAD_MAGIC */
  unsigned dpayl_spare;
  const char *dpayl_name;
  // the payload loader
  void (*dpayl_loadfun) (struct mom_loader_st * ld, momitem_t *litm,
			 momval_t jsob);
  // the payload dump scanner
  void (*dpayl_dumpscanfun) (struct mom_dumper_st * du, momitem_t *ditm);
  // the payload dumper, should return a json object
  momval_t (*dpayl_dumpjsonfun) (struct mom_dumper_st * du, momitem_t *ditm);
  intptr_t dpayl_spare1, dpayl_spare2;
};

momitem_t *mom_load_item_json (struct mom_loader_st *ld, const momval_t jval);
momval_t mom_load_value_json (struct mom_loader_st *ld, const momval_t jval);

enum mom_kindpayload_en
{
  mompayl_none = 0,
  mompayl_queue,
  mompayl_tasklet,

  mompayl__last = 32
};
struct mom_payload_descr_st *mom_payloadescr[mompayl__last + 1];
/************* misc items *********/
// convert a boolean to a predefined item json_true or json_false
const momitem_t *mom_get_item_bool (bool v);


////////////////////////////////////////////////////////////////
/////////// SEQUENCE OF ITEMS, SETS & TUPLES
////////////////////////////////////////////////////////////////

// for sets and tuples of item. In sets, itemseq is sorted by
// increasing idstr. In tuples, some itemseq components may be nil.
struct momseqitem_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  const momitem_t *itemseq[];
};

// make a set until a NULL value, argument can be tuples, sets, items.
const momset_t *mom_make_set_til_nil (momval_t first, ...)
  __attribute__ ((sentinel));
// make a set of siz items
const momset_t *mom_make_set_sized (unsigned siz, ...);
// make a set from an array of items
const momset_t *mom_make_set_from_array (unsigned siz,
					 const momitem_t **itemarr);

// union of two sets values
momval_t mom_make_set_union (momval_t s1, momval_t s2);
// intersection of two sets values
momval_t mom_make_set_intersection (momval_t s1, momval_t s2);
/// in set S1 remove the items from set, tuple V2 or remove the item
/// V2 if it is an item...
momval_t mom_make_set_without (momval_t s1, momval_t v2);

// make a tuple til nil. Arguments which are MOM_EMPTY are replaced by nil.
const momtuple_t *mom_make_tuple_til_nil (momval_t first, ...)
  __attribute__ ((sentinel));
const momtuple_t *mom_make_tuple_sized (unsigned siz, ...);
const momtuple_t *mom_make_tuple_from_array (unsigned siz,
					     const momitem_t **itemarr);


static inline bool
mom_is_set (momval_t setv)
{
  if (!setv.ptr || *setv.ptype != momty_set)
    return false;
  return true;
}

static inline unsigned
mom_set_cardinal (momval_t setv)
{
  if (!setv.ptr || *setv.ptype != momty_set)
    return 0;
  return setv.pset->slen;
}

static inline momitem_t *
mom_set_nth_item (momval_t setv, int rk)
{
  if (!setv.ptr || *setv.ptype != momty_set)
    return NULL;
  unsigned slen = setv.pset->slen;
  if (rk < 0)
    rk += (int) slen;
  if (rk >= 0 && rk < (int) slen)
    return (momitem_t *) (setv.pset->itemseq[rk]);
  return NULL;
}

static inline bool
mom_is_tuple (momval_t tupv)
{
  if (!tupv.ptr || *tupv.ptype != momty_tuple)
    return false;
  return true;
}

static inline unsigned
mom_tuple_length (momval_t tupv)
{
  if (!tupv.ptr || *tupv.ptype != momty_tuple)
    return 0;
  return tupv.ptuple->slen;
}

static inline momitem_t *
mom_tuple_nth_item (momval_t tupv, int rk)
{
  if (!tupv.ptr || *tupv.ptype != momty_tuple)
    return 0;
  unsigned slen = tupv.ptuple->slen;
  if (rk < 0)
    rk += (int) slen;
  if (rk >= 0 && rk < (int) slen)
    return (momitem_t *) (tupv.ptuple->itemseq[rk]);
  return NULL;
}

static inline bool
mom_is_seqitem (momval_t seqv)
{
  if (!seqv.ptr || (*seqv.ptype != momty_tuple && *seqv.ptype != momty_set))
    return false;
  return true;
}

static inline unsigned
mom_seqitem_length (momval_t seqv)
{
  if (!seqv.ptr || (*seqv.ptype != momty_tuple && *seqv.ptype != momty_set))
    return 0;
  return seqv.pseqitems->slen;
}

static inline momitem_t *
mom_seqitem_nth_item (momval_t seqv, int rk)
{
  if (!seqv.ptr || (*seqv.ptype != momty_tuple && *seqv.ptype != momty_set))
    return 0;
  unsigned slen = seqv.pseqitems->slen;
  if (rk < 0)
    rk += (int) slen;
  if (rk >= 0 && rk < (int) slen)
    return (momitem_t *) (seqv.pseqitems->itemseq[rk]);
  return NULL;
}


////////////////////////////////////////////////////////////////
/////////// NODES, notably CLOSURES
////////////////////////////////////////////////////////////////

// for nodes & closures
struct momnode_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  const momitem_t *connitm;
  const momval_t sontab[];
};

// node making functions would return nil if the connective is not an
// item...

// make a node til nil. MOM_EMPTY arguments are replaced with nil.
const momnode_t *mom_make_node_til_nil (const momitem_t *conn, ...)
  __attribute__ ((sentinel));

// make a node of given size. all arguments after siz should be
// genuine values without MOM_EMPTY...
const momnode_t *mom_make_node_sized (const momitem_t *conn,
				      unsigned siz, ...);

// make a node from an array
const momnode_t *mom_make_node_from_array (const momitem_t *conn,
					   unsigned siz, momval_t *arr);

static inline unsigned
mom_node_arity (momval_t nodv)
{
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return 0;
  return nodv.pnode->slen;
}

static inline const momitem_t *
mom_node_conn (momval_t nodv)
{
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return NULL;
  return nodv.pnode->connitm;
}


static inline momval_t
mom_node_nth (momval_t nodv, int rk)
{
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return MOM_NULLV;
  unsigned l = nodv.pnode->slen;
  if (rk < 0)
    rk += (int) l;
  if (rk >= 0 && rk < (int) l)
    return nodv.pnode->sontab[rk];
  return MOM_NULLV;
}

////////////////////////////////////////////////////////////////
/////////// SPACES
////////////////////////////////////////////////////////////////
enum mom_space_en
{
  momspa_none = 0,
  momspa_predefined = 1,
  momspa_root = 2,

  momspa__last = 32,
};
#define MOM_SPACE_MAGIC 0x5eaf0539	/* mom space magic 1588528441 */
extern struct mom_spacedescr_st
{
  unsigned space_magic;		/* always MOM_SPACE_MAGIC */
  unsigned space_index;		/* my index in mom_spacedescr_array */
  const char *space_name;
  const momstring_t *space_namestr;
  void *space_data;
  // initialize the space for dumping
  void (*space_init_dump_fun) (struct mom_dumper_st * dmp, unsigned spacix);
  // store an item in the spac
  void (*space_store_item_fun) (struct mom_dumper_st * dmp, momitem_t *itm,
				const char *datastr);
  // finalize the state for dumping, only done if initialized
  void (*space_fini_dump_fun) (struct mom_dumper_st * dmp, unsigned spacix);
  // initialize the space for loading
  void (*space_init_load_fun) (struct mom_loader_st * ld, unsigned spacix);
  // fetch a GC-strdup-ed data string for a given item
  const char *(*space_fetch_load_item_fun) (struct mom_loader_st * ld,
					    momitem_t *itm);
  // finalize the space for loading, only done if initialized
  void (*space_fini_load_fun) (struct mom_loader_st * ld, unsigned spacix);
} *mom_spacedescr_array[momspa__last + 1];

////////////////////////////////////////////////////////////////
/////////// DIAGNOSTICS
////////////////////////////////////////////////////////////////


/************************* debugging *************************/
// for debugging:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)		\
  Dbg(item)					\
  Dbg(dump)					\
  Dbg(load)					\
  Dbg(json)					\
  Dbg(run)					\
  Dbg(web)

#define MOM_DEBUG_DEFINE_OPT(Nam) momdbg_##Nam,
enum mom_debug_en
{
  momdbg__none,
  MOM_DEBUG_LIST_OPTIONS (MOM_DEBUG_DEFINE_OPT) momdbg__last
};

unsigned mom_debugflags;

#define MOM_IS_DEBUGGING(Dbg) (mom_debugflags & (1<<momdbg_##Dbg))

void
mom_debug_at (enum mom_debug_en dbg, const char *fil, int lin, ...)
__attribute__ ((sentinel));

#define MOM_DEBUG_AT(Dbg,Fil,Lin,Fmt,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))			\
      mom_debug_at (momdbg_##Dbg,Fil,Lin,Fmt,	\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_DEBUG_AT_BIS(Dbg,Fil,Lin,Fmt,...)	\
  MOM_DEBUG_AT(Dbg,Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_DEBUG(Dbg,Fmt,...)			\
  MOM_DEBUG_AT_BIS(Dbg,__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)



void
mom_debugprintf_at (enum mom_debug_en dbg, const char *fil, int lin,
		    const char *fmt, ...)
__attribute__ ((format (printf, 4, 5)));

#define MOM_DEBUGPRINTF_AT(Dbg,Fil,Lin,Fmt,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))				\
      mom_debugprintf_at (momdbg_##Dbg,Fil,Lin,Fmt,	\
		    ##__VA_ARGS__);			\
  } while(0)

#define MOM_DEBUGPRINTF_AT_BIS(Dbg,Fil,Lin,Fmt,...)	\
  MOM_DEBUGPRINTF_AT(Dbg,Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_DEBUGPRINTF(Dbg,Fmt,...)			\
  MOM_DEBUGPRINTF_AT_BIS(Dbg,__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)


/************************* inform *************************/
void mom_inform_at (const char *fil, int lin, ...) __attribute__ ((sentinel));

#define MOM_INFORM_AT(Fil,Lin,Fmt,...) do {	\
      mom_inform_at (Fil,Lin,Fmt,	\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_INFORM_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_INFORM_AT(Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_INFORM(Fmt,...)		\
  MOM_INFORM_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)



void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,...) do {	\
      mom_informprintf_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__);		\
  } while(0)

#define MOM_INFORMPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_INFORMPRINTF(Fmt,...)			\
  MOM_INFORMPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)


/************************* warning *************************/
void
mom_warning_at (const char *fil, int lin, ...) __attribute__ ((sentinel));

#define MOM_WARNING_AT(Fil,Lin,Fmt,...) do {	\
      mom_warning_at (Fil,Lin,Fmt,	\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_WARNING_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_WARNING_AT(Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_WARNING(Fmt,...)		\
  MOM_WARNING_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)



void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_WARNPRINTF_AT(Fil,Lin,Fmt,...) do {	\
      mom_warnprintf_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__);		\
  } while(0)

#define MOM_WARNPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_WARNPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_WARNPRINTF(Fmt,...)			\
  MOM_WARNPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)



/************************* fatal *************************/
void mom_fatal_at (const char *fil, int lin, ...) __attribute__ ((sentinel));

#define MOM_FATAL_AT(Fil,Lin,Fmt,...) do {	\
      mom_fatal_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_FATAL_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_FATAL_AT(Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_FATAL(Dbg,Fmt,...)		\
  MOM_FATAL_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)



void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_FATAPRINTF_AT(Fil,Lin,Fmt,...) do {	\
      mom_fataprintf_at (Fil,Lin,Fmt,		\
		    ##__VA_ARGS__);		\
  } while(0)

#define MOM_FATAPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_FATAPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_FATAPRINTF(Fmt,...)			\
  MOM_FATAPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)




////////////////////////////////////////////////////////////////
/////////// OUTPUT
////////////////////////////////////////////////////////////////

// rename ORIGPATH as DESTPATH -both in same directory or filesystem-
// if the content of ORIGPATH is not the same as the content of
// DESTPATH also backup DESTPATH as DESTPATH~; if both contents are
// same, remove ORIGPATH without touching metadata of DESTPATH
// e.g.
//
/// mom_rename_if_content_changed("dir/foo.c+1234tmp", "dir/foo.c")
void mom_rename_if_content_changed (const char *origpath,
				    const char *destpath);

#define MOM_MOUT_MAGIC 0x41f67aa5	/* mom_out_magic 1106672293 */
enum outflags_en
{
  outf__none = 0,
  outf_cname = 1 << 0,
  outf_jsonhalfindent = 1 << 1,
  outf_jsonindent = 1 << 2,
};

struct momout_st
{
  unsigned mout_magic;		/* always MOM_MOUT_MAGIC */
  int mout_indent;
  FILE *mout_file;
  void *mout_data;
  long mout_lastnl;		/* offset at last newline with MOMOUT_NEWLINE or MOMOUT_SPACE */
  unsigned mout_flags;
};
typedef struct momout_st momout_t;
extern struct momout_st mom_stdout_data;
extern struct momout_st mom_stderr_data;
#define mom_stdout &mom_stdout_data
#define mom_stderr &mom_stderr_data
void mom_out_at (const char *sfil, int lin, momout_t *pout, ...)
  __attribute__ ((sentinel));
void mom_outva_at (const char *sfil, int lin, momout_t *pout, va_list alist);
#define MOM_OUT_AT_BIS(Fil,Lin,Out,...) mom_out_at(Fil,Lin,Out,##__VA_ARGS__,NULL)
#define MOM_OUT_AT(Fil,Lin,Out,...) MOM_OUT_AT_BIS(Fil,Lin,Out,##__VA_ARGS__)
#define MOM_OUT(Out,...) MOM_OUT_AT(__FILE__,__LINE__,Out,##__VA_ARGS__)

static inline bool
mom_initialize_output (struct momout_st *out, FILE * fil, unsigned flags)
{
  if (!out || !fil)
    return false;
  memset (out, 0, sizeof (struct momout_st));
  out->mout_magic = MOM_MOUT_MAGIC;
  out->mout_file = fil;
  out->mout_flags = flags;
  return true;
}

enum momoutdir_en
{
  MOMOUTDO__END = 0,
  ///
  /// literal strings
  MOMOUTDO_LITERAL /*, const char*literalstring */ ,
#define MOMOUT_LITERAL(S) MOMOUTDO_LITERAL, MOM_REQUIRES_TYPE(S,const char[],mombad_literal)
#define MOMOUT_LITERALV(S) MOMOUTDO_LITERAL, MOM_REQUIRES_TYPE(S,const char*,mombad_literal)
  ///
  /// HTML encoded strings
  MOMOUTDO_HTML /*, const char*htmlstring */ ,
#define MOMOUT_HTML(S) MOMOUTDO_HTML, MOM_REQUIRES_TYPE(S,const char*,mombad_html)
  ///
  /// Javascript encoded strings
  MOMOUTDO_JS_STRING /*, const char*jsstring */ ,
#define MOMOUT_JS_STRING(S) MOMOUTDO_JS_STRING, MOM_REQUIRES_TYPE(S,const char*,mombad_js)
  ///
  /// JSON value
  MOMOUTDO_JSON_VALUE /*, momval_t jsval */ ,
#define MOMOUT_JSON_VALUE(S) MOMOUTDO_JSON_VALUE, MOM_REQUIRES_TYPE(S,momval_t,mombad_value)
  ///
  /// any value
  MOMOUTDO_VALUE /*, momval_t val */ ,
#define MOMOUT_VALUE(S) MOMOUTDO_VALUE, MOM_REQUIRES_TYPE(S,momval_t,mombad_value)
  ///
  /// any item
  MOMOUTDO_ITEM /*, momitem_t* itm */ ,
#define MOMOUT_ITEM(S) MOMOUTDO_ITEM, MOM_REQUIRES_TYPE(S,const momitem_t*,mombad_item)
  ///
  /// decimal int
  MOMOUTDO_DEC_INT /*, int num */ ,
#define MOMOUT_DEC_INT(N) MOMOUTDO_DEC_INT, MOM_REQUIRES_TYPE(N,int,mombad_int)
  //
  /// hex int
  MOMOUTDO_HEX_INT /*, int num */ ,
#define MOMOUT_HEX_INT(N) MOMOUTDO_HEX_INT, MOM_REQUIRES_TYPE(N,int,mombad_int)
  ///
  /// format a double with %g 
  MOMOUTDO_DOUBLE_G /*, double time */ ,
#define MOMOUT_DOUBLE_G(D) MOMOUTDO_DOUBLE_G, \
  MOM_REQUIRES_TYPE(D,double,mombad_double)
  ///
  /// format a double with %f 
  MOMOUTDO_DOUBLE_F /*, double time */ ,
#define MOMOUT_DOUBLE_F(D) MOMOUTDO_DOUBLE_F, \
  MOM_REQUIRES_TYPE(D,double,mombad_double)
  ///
  /// format giving a format a double
  MOMOUTDO_FMT_DOUBLE /*, const char*fmt, double x */ ,
#define MOMOUT_FMT_DOUBLE(F,D) MOMOUTDO_FMT_DOUBLE,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt) \
    MOM_REQUIRES_TYPE(D,double,mombad_double)
  ///
  ///
  /// format giving a format a long
  MOMOUTDO_FMT_LONG /*, const char*fmt, long l */ ,
#define MOMOUT_FMT_LONG(F,L) MOMOUTDO_FMT_LONG,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt) \
    MOM_REQUIRES_TYPE(L,long,mombad_long)
  ///
  /// format giving a format a long
  MOMOUTDO_FMT_LONG_LONG /*, const char*fmt, long long l */ ,
#define MOMOUT_FMT_LONG_LONG(F,L) MOMOUTDO_FMT_LONG_LONG,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt) \
    MOM_REQUIRES_TYPE(L,long long,mombad_longlong)
  ///
  /// format giving a format an int
  MOMOUTDO_FMT_INT /*, const char*fmt, long l */ ,
#define MOMOUT_FMT_INT(F,L) MOMOUTDO_FMT_INT,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt) \
    MOM_REQUIRES_TYPE(L,int,mombad_int)
  ///
  ///
  /// format giving a format an unsigned
  MOMOUTDO_FMT_UNSIGNED /*, const char*fmt, unsigned l */ ,
#define MOMOUT_FMT_UNSIGNED(F,L) MOMOUTDO_FMT_UNSIGNED,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt) \
    MOM_REQUIRES_TYPE(L,unsigned,mombad_unsigned)
  ///
  ///
  /// format a double as a time using mom_strftime_centi
  MOMOUTDO_DOUBLE_TIME /*, const char*fmt, double time */ ,
#define MOMOUT_DOUBLE_TIME(F,D) MOMOUTDO_DOUBLE_TIME, \
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt), MOM_REQUIRES_TYPE(D,double,mombad_double)
  ///
  ///
  /// copy verbatim the bytes of an opened FILE*
  MOMOUTDO_VERBATIM_FILE /*, FILE*fil */ ,
#define MOMOUT_VERBATIM_FILE(F) MOMOUTDO_VERBATIM_FILE, \
  MOM_REQUIRES_TYPE(F,FILE*,mombad_file),
  ///
  ///
  /// copy HTML encoded the bytes of an opened FILE*
  MOMOUTDO_HTML_FILE /*, FILE*fil */ ,
#define MOMOUT_HTML_FILE(F) MOMOUTDO_HTML_FILE, \
  MOM_REQUIRES_TYPE(F,FILE*,mombad_file),
  ///
  /// copy JS encoded the bytes of an opened FILE*
  MOMOUTDO_JS_FILE /*, FILE*fil */ ,
#define MOMOUT_JS_FILE(F) MOMOUTDO_JS_FILE, \
  MOM_REQUIRES_TYPE(F,FILE*,mombad_file),
  ///
  ///
  /// increase indentation
  MOMOUTDO_INDENT_MORE /* -no arguments-  */ ,
#define MOMOUT_INDENT_MORE() MOMOUTDO_INDENT_MORE
  ///
  ///
  /// decrease indentation
  MOMOUTDO_INDENT_LESS /* -no arguments- */ ,
#define MOMOUT_INDENT_LESS() MOMOUTDO_INDENT_LESS
  ///
  /// indented newline, at most 16 spaces
  MOMOUTDO_NEWLINE /* -no arguments- */ ,
#define MOMOUT_NEWLINE() MOMOUTDO_NEWLINE
  ///
  /// indented newline, at most 8 spaces
  MOMOUTDO_SMALL_NEWLINE /* -no arguments- */ ,
#define MOMOUT_SMALL_NEWLINE() MOMOUTDO_SMALL_NEWLINE
  ///
  /// output a space or an indented newline if the current line
  /// exceeds a given threshold
  MOMOUTDO_SPACE /*, unsigned threshold */ ,
#define MOMOUT_SPACE(L) MOMOUTDO_SPACE,	\
    MOM_REQUIRES_TYPE(L,int,mombad_space)
  ///
  /// output some backtrace, mostly useful for debugging
  MOMOUTDO_BACKTRACE /*, unsigned maxlevel */ ,
#define MOMOUT_BACKTRACE(L) MOMOUTDO_BACKTRACE,	\
  MOM_REQUIRES_TYPE((L),int,mombad_space)
  ///
  ///
  /// output a space or an indented small newline if the current line
  /// exceeds a given threshold
  MOMOUTDO_SMALL_SPACE /*, unsigned threshold */ ,
#define MOMOUT_SMALL_SPACE(L) MOMOUTDO_SMALL_SPACE,	\
    MOM_REQUIRES_TYPE(L,int,mombad_space)
  /// output a space or an indented small newline if the current line
  /// exceeds a given threshold
  MOMOUTDO_FLUSH /* */ ,
#define MOMOUT_FLUSH() MOMOUTDO_FLUSH
  ///
  /// output a GPLv3+ copyright notice commented à la C++ with two-slashes
  /// mentioning a given generated file
  MOMOUTDO_GPLV3P_NOTICE /*, const char* file */ ,
#define MOMOUT_GPLV3P_NOTICE(F) MOMOUTDO_GPLV3P_NOTICE,	\
  MOM_REQUIRES_TYPE((F),const char*,mombad_string)
  ///

};
// declare but don't define them. Linker should complain if
// referenced... which happens only with wrong MOMOUT_... macros
// above.
extern const char *mombad_literal;
extern const char *mombad_string;
extern const char *mombad_html;
extern const char *mombad_int;
extern const char *mombad_js;
extern const char *mombad_fmt;
extern const char *mombad_double;
extern const char *mombad_long;
extern const char *mombad_longlong;
extern const char *mombad_file;
extern const char *mombad_space;
extern const char *mombad_value;
extern const char *mombad_item;
extern const char *mombad_entries;
extern const char *mombad_unsigned;


struct mom_itqelem_st
{
  struct mom_itqelem_st *iqe_next;
  const momitem_t *iqe_item;
};

struct mom_itemqueue_st
{
  struct mom_itqelem_st *itq_first;
  struct mom_itqelem_st *itq_last;
};

static inline bool
mom_queue_is_empty (struct mom_itemqueue_st *iq)
{
  assert (iq != NULL);
  return iq->itq_first != NULL;
}

static inline void
mom_queue_add_item_back (struct mom_itemqueue_st *iq, const momitem_t *itm)
{
  assert (iq != NULL);
  struct mom_itqelem_st *qel =
    MOM_GC_ALLOC ("add back item queue", sizeof (struct mom_itqelem_st));
  qel->iqe_item = itm;
  if (MOM_UNLIKELY (iq->itq_first == NULL))
    iq->itq_first = iq->itq_last = qel;
  else
    {
      iq->itq_last->iqe_next = qel;
      iq->itq_last = qel;
    }
}

static inline void
mom_queue_add_item_front (struct mom_itemqueue_st *iq, const momitem_t *itm)
{
  assert (iq != NULL);
  struct mom_itqelem_st *qel =
    MOM_GC_ALLOC ("add front item queue", sizeof (struct mom_itqelem_st));
  qel->iqe_item = itm;
  if (MOM_UNLIKELY (iq->itq_first == NULL))
    iq->itq_first = iq->itq_last = qel;
  else
    {
      qel->iqe_next = iq->itq_first;
      iq->itq_first = qel;
    }
}

static inline const momitem_t *
mom_queue_peek_item_front (struct mom_itemqueue_st *iq)
{
  assert (iq != NULL);
  if (MOM_UNLIKELY (iq->itq_first == NULL))
    return NULL;
  else
    return iq->itq_first->iqe_item;
}

static inline const momitem_t *
mom_queue_pop_item_front (struct mom_itemqueue_st *iq)
{
  assert (iq != NULL);
  if (MOM_UNLIKELY (iq->itq_first == NULL))
    return NULL;
  else
    {
      struct mom_itqelem_st *qel = iq->itq_first;
      const momitem_t *itm = qel->iqe_item;
      if (MOM_UNLIKELY (qel == iq->itq_last))
	iq->itq_first = iq->itq_last = NULL;
      else
	iq->itq_first = qel->iqe_next;
      MOM_GC_FREE (qel);
      return itm;
    }
}

static inline const momitem_t *
mom_queue_peek_item_back (struct mom_itemqueue_st *iq)
{
  assert (iq != NULL);
  if (MOM_UNLIKELY (iq->itq_last == NULL))
    return NULL;
  else
    return iq->itq_last->iqe_item;
}



//////// random numbers
uint32_t mom_random_nonzero_32 (void);
uint32_t mom_random_32 (void);
uint64_t mom_random_nonzero_64 (void);
uint64_t mom_random_64 (void);

#define MOM_STATE_FILE_BASENAME "state-monimelt"
#define MOM_PREDEFINED_HEADER_FILENAME "predef-monimelt.h"
#define MOM_DUMP_SCRIPT "monimelt-dump-state"
#define MOM_DUMP_SCRIPT2 "monimelt-dump-state.sh"
// outcome of successful dump
struct mom_dumpoutcome_st
{
  momval_t *odmp_tuplenamed;
};

// the initial loading
void mom_initial_load (const char *ldir);

// the state dumper
void mom_full_dump (const char *reason, const char *dumpdir,
		    struct mom_dumpoutcome_st *outd);

// can be called from dumping routines
void mom_dump_require_module (struct mom_dumper_st *du, const char *modname);


// initial load
void mom_initial_load (const char *ldirnam);

/////////////////// agenda and workers and web
int mom_nb_workers;
const char *mom_web_host;

/// declare the predefined named and anonymous
#define MOM_PREDEFINED_NAMED(Name,Id) extern momitem_t* mom_named__##Name;
#define MOM_PREDEFINED_ANONYMOUS(Id) extern momitem_t* mom_anonymous_##Id;
#include "predef-monimelt.h"
#endif /*MONIMELT_INCLUDED_ */
