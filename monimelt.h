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
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>
#include <inttypes.h>
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

// libunistring: https://www.gnu.org/software/libunistring/
#include <unistr.h>

// eventfd(2) & signalfd(2) & timerfd_create(2) are Linux specific
#include <sys/eventfd.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
///
#include <sqlite3.h>
/// Boehm GC from http://www.hboehm.info/gc/
/// ...perhaps we should consider MPS from
///            http://www.ravenbrook.com/project/mps/
#include <gc/gc.h>
#include <glib.h>
// libonion from http://www.coralbits.com/libonion/ &
// https://github.com/davidmoreno/onion
#include <onion/onion.h>
#include <onion/low.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/block.h>
#include <onion/handler.h>
#include <onion/dict.h>
#include <onion/log.h>
#include <onion/shortcuts.h>
#include <onion/exportlocal.h>
#include <onion/internal_status.h>




// libjit, from http://www.gnu.org/software/libjit/
// but use a recent GIT of it from http://savannah.gnu.org/git/?group=libjit
#include <jit/jit.h>


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

// elapsed real time since start of process
double mom_elapsed_real_time (void);

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

typedef char *momcstr_t;
typedef uint8_t momtynum_t;
typedef uint8_t momvflags_t;
typedef uint16_t momspaceid_t;
typedef uint32_t momhash_t;
typedef uint32_t momusize_t;


typedef enum mom_flags_en
{
  momflag_transient = (1 << 0)
} momflags_t;

pthread_mutexattr_t mom_normal_mutex_attr;
pthread_mutexattr_t mom_recursive_mutex_attr;
typedef union momvalueptr_un momval_t;


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
struct momout_st
{
  unsigned mout_magic;		/* always MOM_MOUT_MAGIC */
  int mout_indent;
  FILE *mout_file;
  void *mout_data;
  size_t mout_size;
  long mout_lastnl;		/* offset at last newline with MOMOUT_NEWLINE or MOMOUT_SPACE */
  unsigned mout_flags;
};

typedef struct momout_st momout_t;
extern struct momout_st mom_stdout_data;
extern struct momout_st mom_stderr_data;
#define mom_stdout &mom_stdout_data
#define mom_stderr &mom_stderr_data

#define MOM_MOUT_MAGIC 0x41f67aa5	/* mom_out_magic 1106672293 */
typedef enum momoutflags_en
{
  outf__none = 0,
  outf_cname = 1 << 0,
  outf_jsonhalfindent = 1 << 1,
  outf_jsonindent = 1 << 2,
  outf_shortfloat = 1 << 3,	/* print double numbers with 6 digits at most */
  outf_isbuffer = 1 << 15,	/* internal flag */
} momoutflags_t;

void mom_out_at (const char *sfil, int lin, momout_t *pout, ...)
  __attribute__ ((sentinel));
void mom_outva_at (const char *sfil, int lin, momout_t *pout, va_list alist);
/// output into a string value
momval_t mom_outstring_at (const char *sfil, int lin, unsigned flags, ...)
  __attribute__ ((sentinel));
#define MOM_OUTSTRING_AT_BIS(Fil,Lin,Flag,...) mom_outstring_at(Fil,Lin,Flag,##__VA_ARGS__)
#define MOM_OUTSTRING_AT(Fil,Lin,Flag,...) MOM_OUTSTRING_AT_BIS(Fil,Lin,(Flag),##__VA_ARGS__,NULL)
#define MOM_OUTSTRING(Flag,...) MOM_OUTSTRING_AT(__FILE__,__LINE__,Flag,##__VA_ARGS__)
#define MOM_OUTVA(Out,Alist) mom_outva_at(__FILE__,__LINE__,Out,Alist)
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

// initialize a buffer output, ie using open_memstream with mout_data & mout_size fields
void mom_initialize_buffer_output (struct momout_st *out, unsigned flags);
// finalize a buffer output, so free the data
void mom_finalize_buffer_output (struct momout_st *out);

typedef enum momoutdir_en
{
  MOMOUTDO__END = 0,
#define MOMOUT_END() ((void*)MOMOUTDO__END)
  ///
  /// literal strings
  MOMOUTDO_LITERAL /*, const char*literalstring */ ,
#define MOMOUT_LITERAL(S) MOMOUTDO_LITERAL, MOM_REQUIRES_TYPE(S,const char[],mombad_literal)
#define MOMOUT_LITERALV(S) MOMOUTDO_LITERAL, MOM_REQUIRES_TYPE(S,const char*,mombad_literal)
  // Javascript encoded literal raw newline
#define MOMOUT_JS_RAW_NEWLINE() MOMOUT_LITERAL("\\n")
  ///
  ///
  /// HTML encoded strings
  MOMOUTDO_HTML /*, const char*htmlstring */ ,
#define MOMOUT_HTML(S) MOMOUTDO_HTML, MOM_REQUIRES_TYPE(S,const char*,mombad_html)
  ///
  ///
  /// HTML + JS encoded strings, so newlines are \n...
  MOMOUTDO_JS_HTML /*, const char*htmlstring */ ,
#define MOMOUT_JS_HTML(S) MOMOUTDO_JS_HTML, MOM_REQUIRES_TYPE(S,const char*,mombad_html)
  ///
  /// Javascript encoded strings
  MOMOUTDO_JS_STRING /*, const char*jsstring */ ,
#define MOMOUT_JS_STRING(S) MOMOUTDO_JS_STRING, MOM_REQUIRES_TYPE(S,const char*,mombad_js)
#define MOMOUT_JS_LITERAL(S) MOMOUTDO_JS_STRING, MOM_REQUIRES_TYPE(S,const char[],mombad_literal)
#define MOMOUT_JS_LITERALV(S) MOMOUT_JS_STRING(S)
  ///
  ///
  ///  comment encoded strings
  /// start with //! and replace every newline with //+ then a newline
  /// do nothing if NULL string
  MOMOUTDO_SLASHCOMMENT_STRING /*, const char*jsstring */ ,
#define MOMOUT_SLASHCOMMENT_STRING(S) MOMOUTDO_SLASHCOMMENT_STRING, MOM_REQUIRES_TYPE(S,const char*,mombad_js)
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
  /// any item attributes
  MOMOUTDO_ITEM_ATTRIBUTES /*, momitem_t* itm */ ,
#define MOMOUT_ITEM_ATTRIBUTES(S) MOMOUTDO_ITEM_ATTRIBUTES, \
  MOM_REQUIRES_TYPE(S,const momitem_t*,mombad_item)
  ///
  /// decimal int
  MOMOUTDO_DEC_INT /*, int num */ ,
#define MOMOUT_DEC_INT(N) MOMOUTDO_DEC_INT, MOM_REQUIRES_TYPE(N,int,mombad_int)
  //
  /// hex int
  MOMOUTDO_HEX_INT /*, int num */ ,
#define MOMOUT_HEX_INT(N) MOMOUTDO_HEX_INT, MOM_REQUIRES_TYPE(N,int,mombad_int)
  ///
  /// decimal long
  MOMOUTDO_DEC_LONG /*, long num */ ,
#define MOMOUT_DEC_LONG(N) MOMOUTDO_DEC_LONG, MOM_REQUIRES_TYPE(N,long,mombad_long)
  //
  /// hex long
  MOMOUTDO_HEX_LONG /*, int num */ ,
#define MOMOUT_HEX_LONG(N) MOMOUTDO_HEX_LONG, MOM_REQUIRES_TYPE(N,long,mombad_long)
  ///
  /// decimal intptr_t
  MOMOUTDO_DEC_INTPTR_T /*, intptr_t num */ ,
#define MOMOUT_DEC_INTPTR_T(N) MOMOUTDO_DEC_INTPTR_T, MOM_REQUIRES_TYPE(N,intptr_t,mombad_intptr_t)
  //
  /// hex long
  MOMOUTDO_HEX_INTPTR_T /*, intptr_t num */ ,
#define MOMOUT_HEX_INTPTR_T(N) MOMOUTDO_HEX_INTPTR_T, MOM_REQUIRES_TYPE(N,intptr_t,mombad_intptr_t)
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
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt),		\
  MOM_REQUIRES_TYPE(D,double,mombad_double)
  ///
  ///
  /// format giving a format a long
  MOMOUTDO_FMT_LONG /*, const char*fmt, long l */ ,
#define MOMOUT_FMT_LONG(F,L) MOMOUTDO_FMT_LONG,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt),	\
  MOM_REQUIRES_TYPE(L,long,mombad_long)
  ///
  /// format giving a format a long
  MOMOUTDO_FMT_LONG_LONG /*, const char*fmt, long long l */ ,
#define MOMOUT_FMT_LONG_LONG(F,L) MOMOUTDO_FMT_LONG_LONG,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt),			\
  MOM_REQUIRES_TYPE(L,long long,mombad_longlong)
  ///
  /// format giving a format an int
  MOMOUTDO_FMT_INT /*, const char*fmt, long l */ ,
#define MOMOUT_FMT_INT(F,L) MOMOUTDO_FMT_INT,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt),	\
  MOM_REQUIRES_TYPE(L,int,mombad_int)
  ///
  ///
  /// format giving a format an unsigned
  MOMOUTDO_FMT_UNSIGNED /*, const char*fmt, unsigned l */ ,
#define MOMOUT_FMT_UNSIGNED(F,L) MOMOUTDO_FMT_UNSIGNED,	\
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt),		\
  MOM_REQUIRES_TYPE(L,unsigned,mombad_unsigned)
  ///
  ///
  /// format a double as a time using mom_strftime_centi
  MOMOUTDO_DOUBLE_TIME /*, const char*fmt, double time */ ,
#define MOMOUT_DOUBLE_TIME(F,D) MOMOUTDO_DOUBLE_TIME, \
  MOM_REQUIRES_TYPE(F,const char*,mombad_fmt), \
  MOM_REQUIRES_TYPE(D,double,mombad_double)
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
  ///
} momoutdir_t;			/* end enum momoutdir_en */


////////////////////////////////////////////////////////////////
//////////////// TYPES AND VALUES
////////////////////////////////////////////////////////////////
typedef enum momvaltype_en
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
} momvaltype_t;


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

struct momhead_st
{
  const momtynum_t htype;
  momvflags_t hflags;
};

union momvalueptr_un
{
  void *ptr;
  const momtynum_t *ptype;
  struct momhead_st *phead;
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

// compare function for qsort
int mom_valqsort_cmp (const void *l, const void *r);

// compare, in a JSON friendly way (so a named item compare
// alphanumerically to a string)
int mom_json_cmp (const momval_t l, const momval_t r);

static inline momtynum_t
mom_type (const momval_t v)
{
  if (v.ptr == NULL)
    return momty_null;
  else
    return *v.ptype;
}

const char *mom_type_cstring (momtynum_t ty);

/*************************** boxed integers ***************************/
struct momint_st
{
  momtynum_t typnum;
  momvflags_t flags;
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
  momvflags_t flags;
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
  momvflags_t flags;
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
  // 31 different chars; notice that 31 is prime
#define MOM_IDRANDCHARS "acdefhijkmpqrstuvwxyz0123456789"
#define MOM_IDSTRING_FMT "%24["MOM_IDRANDCHARS"_]"
static inline bool
mom_is_string (momval_t v)
{
  return (v.ptr && v.pstring->typnum == momty_string);
}

static inline const momstring_t *
mom_to_string (momval_t v)
{
  return (v.ptr && v.pstring->typnum == momty_string) ? v.pstring : NULL;
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
  momvflags_t flags;
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
  momvflags_t flags;
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
struct mom_jsonparser_st
{
  uint32_t jsonp_magic;		/* always MOMJSONP_MAGIC */
  int16_t jsonp_cc;		/* read ahead character */
  uint16_t jsonp_valid;		/* jsonp_cc is valid */
  pthread_mutex_t jsonp_mtx;
  FILE *jsonp_file;
  void *jsonp_data;
  char *jsonp_error;
  jmp_buf jsonp_jmpbuf;
};

// initialize a JSON parser
void mom_initialize_json_parser (struct mom_jsonparser_st *jp, FILE * file,
				 void *data);
// get its data
void *mom_json_parser_data (const struct mom_jsonparser_st *jp);
// end the parsing without closing the file
void mom_end_json_parser (struct mom_jsonparser_st *jp);
// end the parsing and close the file
void mom_close_json_parser (struct mom_jsonparser_st *jp);
// parse a JSON value, or else set the error message to *perrmsg
momval_t mom_parse_json (struct mom_jsonparser_st *jp, char **perrmsg);

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

static inline bool
mom_is_json_object (momval_t jsobv)
{
  if (!jsobv.ptr || *jsobv.ptype != momty_jsonobject)
    return false;
  return true;
}

#define mom_is_jsonob(Jo) mom_is_json_object(Jo)

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
#define MOMJSON_END ((void*)MOMJSON__END)
  //
  MOMJSONDIR__ENTRY,		/* momval_t nameval, momval_t attrval */
#define MOMJSOB_ENTRY(N,V) MOMJSONDIR__ENTRY,	\
    MOM_REQUIRES_TYPE(N,momval_t,mombad_value), \
    MOM_REQUIRES_TYPE(V,momval_t,mombad_value)
  //
  MOMJSONDIR__STRING,		/* const char*namestr, momval_t attval */
#define MOMJSOB_STRING(S,V) MOMJSONDIR__STRING,		\
    MOM_REQUIRES_TYPE(S,const char*,mombad_string),	\
    MOM_REQUIRES_TYPE(V,momval_t,mombad_value)
  //
  MOMJSONDIR__COUNTED_ENTRIES,	/* unsigned count, struct mom_jsonentry_st* */
#define MOMJSOB_COUNTED_ENTRIES(C,E)  MOMJSONDIR__COUNTED_ENTRIES,	\
    MOM_REQUIRES_TYPE(C,unsigned,mombad_unsigned),			\
    MOM_REQUIRES_TYPE(E,struct mom_jsonentry_st*,mombad_entries)
  //
};



// make a JSON array of given count
const momjsonarray_t *mom_make_json_array (unsigned nbelem, ...);
const momjsonarray_t *mom_make_json_array_count (unsigned count,
						 const momval_t *arr);
const momjsonarray_t *mom_make_json_array_til_nil (momval_t, ...)
  __attribute__ ((sentinel));

static inline bool
mom_is_json_array (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_jsonarray)
    return false;
  return true;
}

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
  momvflags_t i_flags;
  momspaceid_t i_space;
  const momhash_t i_hash;	/* same as i_idstr->hash */
  const unsigned i_magic;	/* always MOM_ITEM_MAGIC */
  atomic_ulong i_counter;	/* counter for atomic nodes */
  pthread_mutex_t i_mtx;
  const momstring_t *i_idstr;	/* id string */
  const momstring_t *i_name;	/* name, or NULL */
  struct mom_itemattributes_st *i_attrs;
  momval_t i_content;
  uint16_t i_paylkind;		// the kind of payload, see enum mom_kindpayload_en
  uint16_t i_paylxtra;
  void *i_payload;		// the payload data, should be non null if kind is set
};

static inline bool
mom_is_item (momval_t v)
{
  return (v.ptr && v.pitem->i_typnum == momty_item
	  && v.pitem->i_magic == MOM_ITEM_MAGIC);
}

// lock an item & return true if successful
static inline bool
mom_lock_item (momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return false;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  return (pthread_mutex_lock (&itm->i_mtx) == 0);
}

static inline momhash_t
mom_item_hash (const momitem_t *itm)
{
  if (MOM_UNLIKELY (!itm))
    return 0;
  else
    return itm->i_hash;
}

bool mom_lock_item_at (const char *fil, int lin, momitem_t *itm);
#ifndef NDEBUG
#define mom_lock_item(Itm) mom_lock_item_at(__FILE__,__LINE__,Itm)
#endif

#define mom_should_lock_item_at(Lin,Itm) do {			\
  momitem_t* _itm_##Lin = (Itm);				\
  if (!mom_lock_item(_itm_##Lin))				\
    MOM_FATAL(MOMOUT_LITERAL("failed to lock item"),		\
	      MOMOUT_ITEM((const momitem_t*)_itm_##Lin),	\
	      NULL);						\
  } while(0)
#define mom_should_lock_item(Itm) mom_should_lock_item_at(__LINE__,(Itm))

// unlock an item
static inline void
mom_unlock_item (momitem_t *itm)
{
  assert (itm && itm->i_typnum == momty_item
	  && itm->i_magic == MOM_ITEM_MAGIC);
  pthread_mutex_unlock (&itm->i_mtx);
}

void mom_unlock_item_at (const char *fil, int lin, momitem_t *itm);

#ifndef NDEBUG
#define mom_unlock_item(Itm) mom_unlock_item_at(__FILE__,__LINE__,Itm)
#endif



static inline momitem_t *
mom_value_to_item (momval_t v)
{
  return (v.ptr && v.pitem->i_typnum == momty_item) ? v.pitem : NULL;
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

// rarely useful and expensive. Gives the set of items whose ident
// string has a given prefix starting with _ then at least one
// alphanum.
const momset_t *mom_set_of_items_of_ident_prefixed (const char *prefix);

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

const momset_t *mom_set_attributes (const struct mom_itemattributes_st
				    *attrs);

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

/// raw access/modification of item attributes should be done with the
/// item locked!
static inline momval_t
mom_item_get_attribute (const momitem_t *itm, const momitem_t *atitm)
{
  if (!itm || !atitm || itm->i_typnum != momty_item)
    return MOM_NULLV;
  return mom_get_attribute (itm->i_attrs, atitm);
}

static inline void
mom_item_put_attribute (momitem_t *itm, const momitem_t *atitm,
			const momval_t val)
{
  if (!itm || !atitm || itm->i_typnum != momty_item)
    return;
  itm->i_attrs = mom_put_attribute (itm->i_attrs, atitm, val);
}

static inline void
mom_item_remove_attribute (momitem_t *itm, const momitem_t *atitm)
{
  if (!itm || !atitm || itm->i_typnum != momty_item)
    return;
  itm->i_attrs = mom_remove_attribute (itm->i_attrs, atitm);
}

static inline void
mom_item_reserve_attribute (momitem_t *itm, unsigned gap)
{
  if (!itm || itm->i_typnum != momty_item)
    return;
  itm->i_attrs = mom_reserve_attribute (itm->i_attrs, gap);
}

static inline const momset_t *
mom_item_set_attributes (momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  return mom_set_attributes (itm->i_attrs);
}

static inline momval_t
mom_item_content (momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return MOM_NULLV;
  return itm->i_content;
}

static inline void
mom_item_put_content (momitem_t *itm, momval_t val)
{
  if (!itm || itm->i_typnum != momty_item)
    return;
  itm->i_content = val;
}

/// clear the payload of an item - should be called under the item's
/// lock. Can call the payload finalizer
void mom_item_clear_payload (momitem_t *itm);
/// get the set of named items, ordered by item ids
const momset_t *mom_set_of_named_items (void);
// get the tuple of named items prefixed by a given prefix, and 
/// if parrname is not-null, set it to the jsonarray of names
const momtuple_t *mom_alpha_ordered_tuple_of_named_prefixed_items (const char
								   *prefix,
								   momval_t
								   *parrname);
/// get the tuple of named items, alphabetically ordered by name
/// if parrname is not-null, set it to the jsonarray of names
static inline const momtuple_t *
mom_alpha_ordered_tuple_of_named_items (momval_t *parrname)
{
  return mom_alpha_ordered_tuple_of_named_prefixed_items ("", parrname);
}

const momstring_t *mom_item_get_name (momitem_t *itm);
const momstring_t *mom_item_get_idstr (momitem_t *itm);
const momstring_t *mom_item_get_name_or_idstr (momitem_t *itm);
// get an item of given name
momitem_t *mom_get_item_of_name_hash (const char *s, momhash_t h);
static inline momitem_t *
mom_get_item_of_name (const char *s)
{
  return mom_get_item_of_name_hash (s, 0);
};

static inline momitem_t *
mom_get_item_of_name_string (momval_t s)
{
  if (s.ptr && s.pstring->typnum == momty_string)
    return mom_get_item_of_name_hash (s.pstring->cstr, s.pstring->hash);
  return NULL;
}

static inline const char *
mom_ident_cstr_of_item (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  return mom_string_cstr ((momval_t) itm->i_idstr);
}


static inline momval_t
mom_identv_of_item (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return MOM_NULLV;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  return ((momval_t) itm->i_idstr);
}


static inline unsigned
mom_item_payload_kind (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return 0;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  return itm->i_paylkind;
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
  if (!itm)
    return;
  assert (itm->i_typnum == momty_item);
  const momstring_t *nams = mom_item_get_name (itm);
  if (!nams)
    return;
  assert (nams->typnum == momty_string);
  mom_forget_name (nams->cstr);
};

void mom_item_status (int64_t * pnbcreation, int64_t * pnbdestruct,
		      int64_t * pnbitems, int64_t * pnamed);

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
  // the payload finalizer
  void (*dpayl_finalizefun) (momitem_t *ditm, void *payloadata);
  intptr_t dpayl_spare1, dpayl_spare2;
};

momitem_t *mom_load_item_json (struct mom_loader_st *ld, const momval_t jval);
momval_t mom_load_value_json (struct mom_loader_st *ld, const momval_t jval);

enum mom_kindpayload_en
{
  mompayk_none = 0,
  mompayk_queue,		// queue of values
  mompayk_routine,		// low-level routine
  mompayk_closure,		// closure, with closed values and routine
  mompayk_procedure,		// procedure, with closed values
  mompayk_tasklet,		// tasklet with its call stack
  mompayk_buffer,		// character buffer
  mompayk_vector,		// vector of values
  mompayk_hset,			// hashed set of non-nil values
  mompayk_assoc,		// association of items to values
  mompayk_process,		// forked process and buffer for its output pipe
  mompayk_webexchange,		// HTTP interaction
  mompayk_jsonrpcexchange,	// JSONRPC interaction

  mompayk__last = 64
};
struct mom_payload_descr_st *mom_payloadescr[mompayk__last + 1];

/************* queue item *********/
///// the payload is a GC_MALLOC-ed struct mom_valuequeue_st
// start a queue payload, under the item's lock
void mom_item_start_queue (momitem_t *itm);
// add under the item's lock a value at the back of the queue
void mom_item_queue_add_back (momitem_t *itm, momval_t val);
// add under the item's lock a value at the front of the queue
void mom_item_queue_add_front (momitem_t *itm, momval_t val);
// under the item's lock test the queue emptiness
bool mom_item_queue_is_empty (momitem_t *itm);
// under the item's lock compute the queue length
unsigned mom_item_queue_length (momitem_t *itm);
// under the item's lock peek its front value
momval_t mom_item_queue_peek_front (momitem_t *itm);
// under the item's lock peek its back value
momval_t mom_item_queue_peek_back (momitem_t *itm);
// under the item's lock pop its front value
momval_t mom_item_queue_pop_front (momitem_t *itm);

/**************** routine item ****************/

/// the Makefile should define that for momg_* modules!
#define MOM_EMPTY_MODULE "."
#ifndef MONIMELT_CURRENT_MODULE
#define MONIMELT_CURRENT_MODULE MOM_EMPTY_MODULE
#endif

// routine descriptor is read-only
#define MOM_ROUTINE_MAGIC 0x6b9c644d	/* routine magic 1805411405 */
#define MOM_ROUTINE_NAME_PREFIX "momrout_"
#define MOM_SYMBNAME_LEN 128
#define MOM_ROUTINE_NAME_FMT  MOM_ROUTINE_NAME_PREFIX "%s"

#define MOM_MODULE_INIT_PREFIX "mominitmodule_"
typedef void mom_module_init_fun_t (void);
// the routine item FOO has descriptor momrout_FOO
// the routine's code returns a positive state, or 
enum mom_routres_en
{
  momroutres_steady = 0,	/* don't change the current state or tasklet */
  momroutres_pop = -1,		/* pop the current frame */
};
typedef int mom_routine_sig_t (int state, momitem_t *tasklet,
			       momval_t closv, momval_t *locvals,
			       intptr_t * locnums, double *locdbls);
struct momroutinedescr_st
{
  unsigned rout_magic;		/* always MOM_ROUTINE_MAGIC */
  unsigned rout_minclosize;	/* minimal closure size */
  unsigned rout_nbconstants;	/* number of constant items */
  unsigned rout_frame_nbval;	/* number of values in its frame */
  unsigned rout_frame_nbnum;	/* number of intptr_t numbers in its frame */
  unsigned rout_frame_nbdbl;	/* number of double numbers in its frame */
  const char **rout_constantids;
  const momitem_t *const *rout_constantitems;
  const char *rout_ident;	/* the cidentifier of FOO; starts with a dot '.' for Jit-ed routine */
  const char *rout_module;	/* always macro MONIMELT_CURRENT_MODULE or NULL for JIT-ed routine */
  const momval_t rout_jitcode;	/* for a JIT-ed routine, the node of its code */
  mom_routine_sig_t *rout_codefun;
  const char *rout_timestamp;	/* generally __DATE__ "@" __TIME__ */
};

// start a routine.
void mom_item_start_routine (momitem_t *itm);
// initialize, ie generate the machine code and some name, for a JIT
// routine. Return NULL on success or some GC_strduped error message
// on failure.
const char *mom_item_generate_jit_routine (momitem_t *itm,
					   const momval_t jitnode);

// generate a C module, returns 0 if ok, else fill *perrmsg
int mom_generate_c_module (momitem_t *moditm, const char *dirname,
			   char **perrmsg);
/************* closure item *********/
#define MOM_CLOSURE_MAGIC 830382377	/* closure magic  0x317ea129 */
struct momclosure_st
{				/* the payload of closures */
  unsigned clos_magic;		/* always MOM_CLOSURE_MAGIC */
  unsigned clos_len;
  const struct momroutinedescr_st *clos_rout;
  momval_t clos_valtab[];
};

static inline const struct momroutinedescr_st *
mom_item_routinedescr (const momitem_t *itm)
{
  const struct momroutinedescr_st *rdescr = NULL;
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  if (itm->i_paylkind == mompayk_routine)
    rdescr = (struct momroutinedescr_st *) itm->i_payload;
  else if (itm->i_paylkind == mompayk_closure)
    {
      struct momclosure_st *clos = itm->i_payload;
      if (clos && clos->clos_magic == MOM_CLOSURE_MAGIC)
	rdescr = clos->clos_rout;
    };
  if (rdescr && rdescr->rout_magic == MOM_ROUTINE_MAGIC)
    return rdescr;
  return NULL;
}

void mom_item_start_closure_of_routine (momitem_t *itm,
					const struct momroutinedescr_st *rout,
					unsigned len);
void mom_item_start_closure (momitem_t *itm, unsigned len);

void mom_item_closure_set_nth (momitem_t *itm, int rk, momval_t cval);

static inline momval_t
mom_item_closure_nth (const momitem_t *itm, int rk)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_closure)
    return MOM_NULLV;
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  unsigned clen = clos->clos_len;
  if (rk < 0)
    rk += (int) clen;
  if (rk >= 0 && rk < (int) clen)
    return clos->clos_valtab[rk];
  return MOM_NULLV;
}

static inline momval_t *
mom_item_closure_values (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_closure)
    return NULL;
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  return clos->clos_valtab;
}

static inline const momitem_t *const *
mom_item_closure_constants (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_closure)
    return NULL;
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  const struct momroutinedescr_st *crout = clos->clos_rout;
  assert (crout && crout->rout_magic == MOM_ROUTINE_MAGIC);
  return crout->rout_constantitems;
}

static inline unsigned
mom_item_closure_length (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_closure)
    return 0;
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  return clos->clos_len;
}

static inline const char *
mom_item_closure_routine_cident (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item
      || itm->i_paylkind != mompayk_closure)
    return 0;
  struct momclosure_st *clos = itm->i_payload;
  assert (clos && clos->clos_magic == MOM_CLOSURE_MAGIC);
  const struct momroutinedescr_st *rdescr = clos->clos_rout;
  assert (rdescr && rdescr->rout_magic == MOM_ROUTINE_MAGIC);
  return rdescr->rout_ident;
}

/************* procedure item *********/
enum momtypenc_st
{
  momtypenc__none,		/* for void result */
  momtypenc_int = 'i',		/* intptr_t */
  momtypenc_val = 'v',		/* momval_t */
  momtypenc_string = 's',	/* const char* string literal */
  momtypenc_double = 'd',	/* double */
};
typedef enum momtypenc_st momtypenc_t;
#define MOM_PROCEDURE_MAGIC 1038420085	/* procedure magic 0x3de50875 */
struct momprocedure_st
{				/* payload of procedures */
  unsigned proc_magic;		/* always MOM_PROCEDURE_MAGIC */
  const struct momprocrout_st *proc_rout;
  momval_t proc_valtab[];
};
#define MOM_PROCROUT_MAGIC 407208731	/* procrout magic 0x1845831b */

// for a given ID the momprocrout_st is named momprocdescr_ID
// and the C function is named momprocfun_ID
#define MOM_PROCROUTDESCR_PREFIX "momprocdescr_"
#define MOM_PROCROUTFUN_PREFIX "momprocfun_"
struct momprocrout_st
{
  const unsigned prout_magic;	/* always MOM_PROCROUT_MAGIC */
  const momtypenc_t prout_resty;	/* type result */
  unsigned prout_len;
  const char *prout_id;
  const char *prout_module;
  const char **prout_constantids;
  const void *prout_addr;
  const char *prout_argsig;	/* signature, in momtypenc_t */
  const char *prout_timestamp;
};

void mom_item_start_procedure (momitem_t *itm);
static inline momval_t
mom_item_procedure_nth (const momitem_t *itm, int rk)
{
  if (!itm || itm->i_typnum != momty_item)
    return MOM_NULLV;
  if (itm->i_paylkind != mompayk_procedure)
    return MOM_NULLV;
  struct momprocedure_st *proc = itm->i_payload;
  assert (proc && proc->proc_magic == MOM_PROCEDURE_MAGIC);
  const struct momprocrout_st *prout = proc->proc_rout;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  unsigned plen = prout->prout_len;
  if (rk < 0)
    rk += plen;
  if (rk < 0 || rk >= (int) plen)
    return MOM_NULLV;
  return proc->proc_valtab[rk];
}


// this is called in generated procedure prologue
static inline momitem_t *mom_procedure_item_of_id (const char *id);


static inline const char *
mom_item_procedure_module (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  if (itm->i_paylkind != mompayk_procedure)
    return NULL;
  struct momprocedure_st *proc = itm->i_payload;
  assert (proc && proc->proc_magic == MOM_PROCEDURE_MAGIC);
  const struct momprocrout_st *prout = proc->proc_rout;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  return prout->prout_module;
}

static inline const char *
mom_item_procedure_argsig (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  if (itm->i_paylkind != mompayk_procedure)
    return NULL;
  struct momprocedure_st *proc = itm->i_payload;
  assert (proc && proc->proc_magic == MOM_PROCEDURE_MAGIC);
  const struct momprocrout_st *prout = proc->proc_rout;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  return prout->prout_argsig;
}

static inline momtypenc_t
mom_item_procedure_restype (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return 0;
  if (itm->i_paylkind != mompayk_procedure)
    return 0;
  struct momprocedure_st *proc = itm->i_payload;
  assert (proc && proc->proc_magic == MOM_PROCEDURE_MAGIC);
  const struct momprocrout_st *prout = proc->proc_rout;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  return prout->prout_resty;
}

void mom_item_procedure_set_nth (momitem_t *itm, int rk, momval_t cval);

/************* tasklet item *********/

struct momframe_st
{
  uint32_t fr_state;		/* current state */
  uint32_t fr_intoff;		/* offset of integer locals in itk_scalars */
  uint32_t fr_dbloff;		/* offset of double locals in itk_scalars, should be after fr_intoff */
  uint32_t fr_valoff;		/* offset of value locals in its_values */
};

// the payload is a GC_MALLOC-ed struct  mom_tasklet_data_st 
struct mom_taskletdata_st
{
  intptr_t *dtk_ints;		/* space for intptr_t  */
  double *dtk_doubles;		/* space for doubles */
  momval_t *dtk_values;		/* space for value data */
  momval_t *dtk_closurevals;	/* stack of closure values */
  struct momframe_st *dtk_frames;	/* stack of scalar frames */
  momval_t dtk_res1, dtk_res2, dtk_res3;	/* three results max */
  pthread_t dtk_thread;		/* the thread executing this, or else 0 */
  uint32_t dtk_intsize;		/* size of dtk_ints */
  uint32_t dtk_inttop;		/* top of stack offset on dtk_ints */
  uint32_t dtk_dblsize;		/* size of dtk_doubles */
  uint32_t dtk_dbltop;		/* top of stack offset on dtk_doubles */
  uint32_t dtk_valsize;		/* size of dtk_values */
  uint32_t dtk_valtop;		/* top of stack offset on its_values */
  uint32_t dtk_frasize;		/* size of dtk_closures & dtk_frames */
  uint32_t dtk_fratop;		/* top of stack offset on dtk_closures & dtk_frames */
};

// start a tasklet payload, under the item's lock
void mom_item_start_tasklet (momitem_t *itm);

// reserve in a tasklet item, under the item's lock
void
mom_item_tasklet_reserve (momitem_t *itm, unsigned nbint, unsigned nbdbl,
			  unsigned nbval, unsigned nbfram);


/***** push directives for frames *****/
enum mom_pushframedirective_en
{
  MOMPFRDO__END = 0,
#define MOMPFR_END() ((void*)MOMPFRDO__END)
  //
  MOMPFRDO_STATE /*, int state  */ ,
#define MOMPFR_STATE(S) MOMPFRDO_STATE, \
  MOM_REQUIRES_TYPE(S,int,mombad_int)
  //
  MOMPFRDO_VALUE /*, momval_t val */ ,
#define MOMPFR_VALUE(V) MOMPFRDO_VALUE, \
  MOM_REQUIRES_TYPE(V,momval_t,mombad_value)
  //
  MOMPFRDO_TWO_VALUES /*, momval_t val1, val2 */ ,
#define MOMPFR_TWO_VALUES(V1,V2) MOMPFRDO_TWO_VALUES,	\
  MOM_REQUIRES_TYPE(V1,momval_t,mombad_value),		\
  MOM_REQUIRES_TYPE(V2,momval_t,mombad_value)
  //
  MOMPFRDO_THREE_VALUES /*, momval_t val1, val2, val3 */ ,
#define MOMPFR_THREE_VALUES(V1,V2,V3) MOMPFRDO_THREE_VALUES,	\
  MOM_REQUIRES_TYPE(V1,momval_t,mombad_value),			\
  MOM_REQUIRES_TYPE(V2,momval_t,mombad_value),			\
  MOM_REQUIRES_TYPE(V3,momval_t,mombad_value)
  //
  MOMPFRDO_FOUR_VALUES /*, momval_t val1, val2, val3, val4 */ ,
#define MOMPFR_FOUR_VALUES(V1,V2,V3,V4) MOMPFRDO_FOUR_VALUES,	\
  MOM_REQUIRES_TYPE(V1,momval_t,mombad_value),			\
  MOM_REQUIRES_TYPE(V2,momval_t,mombad_value),			\
  MOM_REQUIRES_TYPE(V3,momval_t,mombad_value),			\
  MOM_REQUIRES_TYPE(V4,momval_t,mombad_value)
  //
  MOMPFRDO_FIVE_VALUES /*, momval_t val1, val2, val3, val4, val5 */ ,
#define MOMPFR_FIVE_VALUES(V1,V2,V3,V4,V5) MOMPFRDO_FIVE_VALUES,	\
  MOM_REQUIRES_TYPE(V1,momval_t,mombad_value),				\
  MOM_REQUIRES_TYPE(V2,momval_t,mombad_value),				\
  MOM_REQUIRES_TYPE(V3,momval_t,mombad_value),				\
  MOM_REQUIRES_TYPE(V4,momval_t,mombad_value),				\
  MOM_REQUIRES_TYPE(V5,momval_t,mombad_value)
  //
  MOMPFRDO_SIX_VALUES /*, momval_t val1, val2, val3, val4, val5, val6 */ ,
#define MOMPFR_SIX_VALUES(V1,V2,V3,V4,V5,V6) MOMPFRDO_SIX_VALUES, \
  MOM_REQUIRES_TYPE(V1,momval_t,mombad_value),			  \
  MOM_REQUIRES_TYPE(V2,momval_t,mombad_value),			  \
  MOM_REQUIRES_TYPE(V3,momval_t,mombad_value),			  \
  MOM_REQUIRES_TYPE(V4,momval_t,mombad_value),			  \
  MOM_REQUIRES_TYPE(V4,momval_t,mombad_value),			  \
  MOM_REQUIRES_TYPE(V6,momval_t,mombad_value)
  //
  MOMPFRDO_ARRAY_VALUES /* unsigned count, momval_t valarr[count] */ ,
#define MOMPFR_ARRAY_VALUES(Cnt,Arr) MOMPFRDO_ARRAY_VALUES,	\
  MOM_REQUIRES_TYPE(Cnt,unsigned,mombad_unsigned),		\
  MOM_REQUIRES_TYPE(Arr,momval_t*,mombad_array)
  //
  MOMPFRDO_NODE_VALUES /* momvalue node, -- to push the sons of a node */ ,
#define MOMPFR_NODE_VALUES(Nod) MOMPFRDO_NODE_VALUES,	\
  MOM_REQUIRES_TYPE(Nod,momval_t,mombad_value)
  //
  MOMPFRDO_SEQ_ITEMS
    /* momvalue seqitem, -- to push the items of a set or tuple */ ,
#define MOMPFR_SEQITEM_ITEMS(Seq) MOMPFRDO_SEQ_ITEMS,	\
  MOM_REQUIRES_TYPE(Seq,momval_t,mombad_value)
  //
  MOMPFRDO_INT /*, intptr_t num */ ,
#define MOMPFR_INT(I) MOMPFRDO_INT,	\
  MOM_REQUIRES_TYPE(I,intptr_t,mombad_int)
  //
  MOMPFRDO_TWO_INTS /*, intptr_t num1, num2 */ ,
#define MOMPFR_TWO_INTS(I1,I2) MOMPFRDO_TWO_INTS,	\
  MOM_REQUIRES_TYPE(I1,intptr_t,mombad_int),		\
  MOM_REQUIRES_TYPE(I2,intptr_t,mombad_int)
  //
  MOMPFRDO_THREE_INTS /*, intptr_t num1, num2, num3 */ ,
#define MOMPFR_THREE_INTS(I1,I2,I3) MOMPFRDO_THREE_INTS,	\
  MOM_REQUIRES_TYPE(I1,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I2,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I3,intptr_t,mombad_int)
  //
  MOMPFRDO_FOUR_INTS /*, intptr_t num1, num2, num3, num4 */ ,
#define MOMPFR_FOUR_INTS(I1,I2,I3,I4) MOMPFRDO_FOUR_INTS,	\
  MOM_REQUIRES_TYPE(I1,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I2,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I3,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I4,intptr_t,mombad_int)
  //
  MOMPFRDO_FIVE_INTS /*, intptr_t num1, num2, num3, num4, num5 */ ,
#define MOMPFR_FIVE_INTS(I1,I2,I3,I4,I5) MOMPFRDO_FIVE_INTS,	\
  MOM_REQUIRES_TYPE(I1,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I2,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I3,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I4,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I5,intptr_t,mombad_int)
  //
  MOMPFRDO_SIX_INTS /*, intptr_t num1, num2, num3, num4, num5 */ ,
#define MOMPFR_SIX_INTS(I1,I2,I3,I4,I5,I6) MOMPFRDO_SIX_INTS,	\
  MOM_REQUIRES_TYPE(I1,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I2,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I3,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I4,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I5,intptr_t,mombad_int),			\
  MOM_REQUIRES_TYPE(I6,intptr_t,mombad_int)
  //
  MOMPFRDO_ARRAY_INTS /* unsigned count, intptr_t numarr[count] */ ,
#define MOMPFR_ARRAY_INTS(Cnt,Arr) MOMPFRDO_ARRAY_INTS,	\
  MOM_REQUIRES_TYPE(Cnt,unsigned,mombad_unsigned),	\
  MOM_REQUIRES_TYPE(Arr,intptr_t*,mombad_array)
  //
  MOMPFRDO_DOUBLE /*, double d */ ,
#define MOMPFR_DOUBLE(D) MOMPFRDO_DOUBLE,	\
  MOM_REQUIRES_TYPE(D,double,mombad_double)
  //
  MOMPFRDO_TWO_DOUBLES /*, double d1, d2 */ ,
#define MOMPFR_TWO_DOUBLES(D1,D2) MOMPFRDO_TWO_DOUBLES,	\
  MOM_REQUIRES_TYPE(D1,double,mombad_double),		\
  MOM_REQUIRES_TYPE(D2,double,mombad_double)
  //
  MOMPFRDO_THREE_DOUBLES /*, double d1, d2, d3 */ ,
#define MOMPFR_THREE_DOUBLES(D1,D2,D3) MOMPFRDO_THREE_DOUBLES,	\
  MOM_REQUIRES_TYPE(D1,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D2,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D3,double,mombad_double)
  //
  MOMPFRDO_FOUR_DOUBLES /*, double d1, d2, d3, d4 */ ,
#define MOMPFR_FOUR_DOUBLES(D1,D2,D3,D4) MOMPFRDO_FOUR_DOUBLES,	\
  MOM_REQUIRES_TYPE(D1,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D2,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D3,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D4,double,mombad_double)
  //
  MOMPFRDO_FIVE_DOUBLES /*, double d1, d2, d3, d4, d5 */ ,
#define MOMPFR_FIVE_DOUBLES(D1,D2,D3,D4,D5) MOMPFRDO_FIVE_DOUBLES,	\
  MOM_REQUIRES_TYPE(D1,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D2,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D3,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D4,double,mombad_double),			\
  MOM_REQUIRES_TYPE(D5,double,mombad_double)
  //
  MOMPFRDO_SIX_DOUBLES /*, double d1, d2, d3, d4, d5, d6 */ ,
#define MOMPFR_SIX_DOUBLES(D1,D2,D3,D4,D5,D6) MOMPFRDO_SIX_DOUBLES,	\
  MOM_REQUIRES_TYPE(D1,double,mombad_double),				\
  MOM_REQUIRES_TYPE(D2,double,mombad_double),				\
  MOM_REQUIRES_TYPE(D3,double,mombad_double),				\
  MOM_REQUIRES_TYPE(D4,double,mombad_double),				\
  MOM_REQUIRES_TYPE(D5,double,mombad_double),				\
  MOM_REQUIRES_TYPE(D6,double,mombad_double)
  //
  MOMPFRDO_ARRAY_DOUBLES /* unsigned count, double dblarr[count] */ ,
#define MOMPFR_ARRAY_DOUBLES(Cnt,Arr) MOMPFRDO_ARRAY_DOUBLES,	\
  MOM_REQUIRES_TYPE(Cnt,unsigned,mombad_unsigned),		\
  MOM_REQUIRES_TYPE(Arr,double*,mombad_array)
  //
  MOMPFRDO__LAST
};

void mom_item_tasklet_push_frame (momitem_t *itm, momval_t clo,
				  enum mom_pushframedirective_en, ...)
  __attribute__ ((sentinel));
void mom_item_tasklet_replace_top_frame (momitem_t *itm, momval_t clo,
					 enum mom_pushframedirective_en, ...)
  __attribute__ ((sentinel));
void mom_item_tasklet_pop_frame (momitem_t *itm);

// tasklet inspectors
unsigned mom_item_tasklet_depth (momitem_t *itm);
/// frk is a frame rank with -1 meaning last
/// vrk, nrk, drk are ranks within the frame
int mom_item_tasklet_frame_state (momitem_t *itm, int frk);
momval_t mom_item_tasklet_frame_closure (momitem_t *itm, int frk);
unsigned mom_item_tasklet_frame_nb_values (momitem_t *itm, int frk);
unsigned mom_item_tasklet_frame_nb_ints (momitem_t *itm, int frk);
unsigned mom_item_tasklet_frame_nb_doubles (momitem_t *itm, int frk);
momval_t *mom_item_tasklet_frame_values_pointer (momitem_t *itm, int frk);
momval_t mom_item_tasklet_frame_nth_value (momitem_t *itm, int frk, int vtk);
intptr_t *mom_item_tasklet_frame_ints_pointer (momitem_t *itm, int frk);
intptr_t mom_item_tasklet_frame_nth_int (momitem_t *itm, int frk, int nrk);
double *mom_item_tasklet_frame_doubles_pointer (momitem_t *itm, int frk);
double mom_item_tasklet_frame_nth_double (momitem_t *itm, int frk, int drk);

static inline momval_t
mom_item_tasklet_res1 (momitem_t *itm)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    return itd->dtk_res1;
  return MOM_NULLV;
}

static inline momval_t
mom_item_tasklet_res2 (momitem_t *itm)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    return itd->dtk_res2;
  return MOM_NULLV;
}

static inline momval_t
mom_item_tasklet_res3 (momitem_t *itm)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    return itd->dtk_res3;
  return MOM_NULLV;
}

static inline void
mom_item_tasklet_clear_res (momitem_t *itm)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    {
      itd->dtk_res1 = itd->dtk_res2 = itd->dtk_res3 = MOM_NULLV;
    }
}

static inline void
mom_item_tasklet_set_1res (momitem_t *itm, momval_t r1)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    {
      itd->dtk_res1 = r1;
      itd->dtk_res2 = itd->dtk_res3 = MOM_NULLV;
    }
}

static inline void
mom_item_tasklet_set_2res (momitem_t *itm, momval_t r1, momval_t r2)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    {
      itd->dtk_res1 = r1;
      itd->dtk_res2 = r2;
      itd->dtk_res3 = MOM_NULLV;
    }
}

static inline void
mom_item_tasklet_set_3res (momitem_t *itm, momval_t r1, momval_t r2,
			   momval_t r3)
{
  struct mom_taskletdata_st *itd = NULL;
  if (itm && itm->i_typnum == momty_item && itm->i_paylkind == mompayk_tasklet
      && (itd = (struct mom_taskletdata_st *) itm->i_payload) != NULL)
    {
      itd->dtk_res1 = r1;
      itd->dtk_res2 = r2;
      itd->dtk_res3 = r3;
    }
}

/*********** buffer items ****************/
void mom_item_start_buffer (momitem_t *itm);
void mom_item_buffer_out (momitem_t *itm, ...) __attribute__ ((sentinel));

/************* process item *********/
#define MOM_PROCESS_MAGIC 0x229ec02d	/* process magic 580829229 */
// their payload is some
struct mom_process_data_st
{
  unsigned iproc_magic;		/* always MOM_PROCESS_MAGIC */
  unsigned iproc_jobnum;	/* the internal job number */
  int iproc_outfd;		/* the process output, to be read by
				   the monitor */
  pid_t iproc_pid;
  unsigned iproc_outsize;	/* size of iproc_outbuf */
  unsigned iproc_outpos;	/* last written position */

  unsigned iproc_argcount;
  const momstring_t *iproc_progname;
  const momstring_t **iproc_argv;	/* of iproc_argcout size */
  const momnode_t *iproc_closure;	/* the closure handing process outcome */
  char *iproc_outbuf;		// buffer for output

};


/**************** web exchange items ****************/

#define MOM_WEBX_MAGIC 0x11b63c9b	/* webx magic 297155739 */
struct mom_webexchange_data_st
{
  unsigned webx_magic;		/* always MOM_WEBX_MAGIC */
  int webx_num;
  double webx_time;
  onion_request *webx_requ;
  onion_response *webx_resp;
  char *webx_obuf;		/* malloc-ed, not GC_malloced! */
  size_t webx_osize;
  char *webx_mime;
  momval_t webx_jobpost;	/* jobject for POST arguments */
  momval_t webx_jobquery;	/* jobject for query arguments */
  struct momout_st webx_out;
  pthread_cond_t webx_cond;
  int webx_httpcode;
};

void mom_paylwebx_finalize (momitem_t *witm, void *wdata);	// in web-onion.c

/// caller should have locked the webitm
void mom_webx_out_at (const char *sfil, int lin, momitem_t *webitm, ...)
  __attribute__ ((sentinel));
#define MOM_WEBX_OUT_AT_BIS(Fil,Lin,Witm,...) mom_webx_out_at(Fil,Lin,Witm,##__VA_ARGS__,NULL)
#define MOM_WEBX_OUT_AT(Fil,Lin,Witm,...) MOM_WEBX_OUT_AT_BIS(Fil,Lin,Witm,##__VA_ARGS__)
#define MOM_WEBX_OUT(Witm,...) MOM_WEBX_OUT_AT(__FILE__,__LINE__,(Witm),##__VA_ARGS__)

/// caller should have locked webitm
void mom_webx_reply (momitem_t *webitm, const char *mime, int httpcode);
// gives the JSON object of POST arguments
momval_t mom_webx_jsob_post (momitem_t *webitm);
// give the value of a given POST argument
momval_t mom_webx_post_arg (momitem_t *webitm, const char *argname);
// gives the JSON object of query arguments
momval_t mom_webx_jsob_query (momitem_t *webitm);
// give the value of a given query argument
momval_t mom_webx_query_arg (momitem_t *webitm, const char *argname);
// get the fullpath and the method -as string values- of a webitm
momval_t mom_webx_fullpath (momitem_t *webitm);
momval_t mom_webx_method (momitem_t *webitm);



#define MOM_WEB_DIRECTORY "webdir"
#define MOM_WEB_ROOT_PAGE "mom-root-page.html"

// called from main
void mom_start_web (const char *webhost);

/**************** jsonrpcexchange items ***********/
#define MOM_JSONRPCX_MAGIC 0x148092dd	/* jsonrpcx magic 343970525 */
enum mom_jsonrpcversion_en
{
  momjsonrpc__none,
  momjsonrpc_v1,		// JSONRPC v1
  momjsonrpc_v2z,		// JSONRPC v2.0
  momjsonrpc__last
};

struct mom_jsonrpcexchange_data_st	/// only created by jsonrpc incoming requests
{
  unsigned jrpx_magic;		/* always MOM_JSONRPCX_MAGIC */
  enum mom_jsonrpcversion_en jrpx_version;
  double jrpx_startime;
  long jrpx_rank;		/* unique number rank for the connection */
  momval_t jrpx_jsid;		/* the "id" for JSONRPC */
  momval_t jrpx_result;		/* the result to transmit */
  int jrpx_error;		/* the error code */
  unsigned jrpx_outflags;	/* the output flags */
  bool jrpx_replied;
  const char *jrpx_errmsg;	/* the error message */
  struct jsonrpc_conn_mom_st *jrpx_conn;
};

void mom_payljsonrpc_finalize (momitem_t *jritm, void *jrdata);	// in run.c

void mom_jsonrpc_reply (momitem_t *jritm, momval_t jresult,
			unsigned outflags);

void mom_jsonrpc_error (momitem_t *jritm, int errcode, const char *errmsg);

/**************** vector items ****************/

/** the vector payload data is a GC_MALLOC-ed struct mom_valuevector_st */

struct mom_valuevector_st
{
  unsigned vvec_size;
  unsigned vvec_count;
  momval_t *vvec_array;
};

static inline unsigned
mom_item_vector_count (const momitem_t *itm)
{
  if (!itm || itm->i_typnum != momty_item)
    return 0;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  if (itm->i_paylkind != mompayk_vector)
    return 0;
  struct mom_valuevector_st *vvec = itm->i_payload;
  assert (vvec != NULL);
  return vvec->vvec_count;
}

static inline momval_t
mom_item_vector_nth (const momitem_t *itm, int rk)
{
  if (!itm || itm->i_typnum != momty_item)
    return MOM_NULLV;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  if (itm->i_paylkind != mompayk_vector)
    return MOM_NULLV;
  struct mom_valuevector_st *vvec = itm->i_payload;
  assert (vvec != NULL);
  unsigned cnt = vvec->vvec_count;
  if (!cnt)
    return MOM_NULLV;
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    return vvec->vvec_array[rk];
  return MOM_NULLV;
}

static inline momval_t *
mom_item_vector_ptr_nth (const momitem_t *itm, int rk)
{
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  if (itm->i_paylkind != mompayk_vector)
    return NULL;
  struct mom_valuevector_st *vvec = itm->i_payload;
  assert (vvec != NULL);
  unsigned cnt = vvec->vvec_count;
  if (!cnt)
    return NULL;
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    return vvec->vvec_array + rk;
  return NULL;
}


static inline void
mom_item_vector_put_nth (const momitem_t *itm, int rk, momval_t val)
{
  if (!itm || itm->i_typnum != momty_item)
    return;
  assert (itm->i_magic == MOM_ITEM_MAGIC);
  if (itm->i_paylkind != mompayk_vector)
    return;
  struct mom_valuevector_st *vvec = itm->i_payload;
  assert (vvec != NULL);
  unsigned cnt = vvec->vvec_count;
  if (!cnt)
    return;
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    vvec->vvec_array[rk] = val;
}


void mom_item_start_vector (momitem_t *itm);
void mom_item_vector_reserve (momitem_t *itm, unsigned gap);
void mom_item_vector_append1 (momitem_t *itm, momval_t val);
void mom_item_vector_append_sized (momitem_t *itm, unsigned cnt, ...);
void mom_item_vector_append_til_nil (momitem_t *itm, ...)
  __attribute__ ((sentinel));
void mom_item_vector_append_from_array (momitem_t *itm, unsigned count,
					const momval_t *arr);
void mom_item_vector_append_from_node (momitem_t *itm, momval_t nodv);
momval_t mom_make_node_from_item_vector (const momitem_t *connitm,
					 momitem_t *vectitm);
momval_t mom_make_node_from_item_vector_slice (const momitem_t *connitm,
					       momitem_t *vectitm,
					       int firstix, int afterix);
momval_t mom_make_set_from_item_vector (momitem_t *vectitm);
momval_t mom_make_set_from_item_vector_slice (momitem_t *vectitm, int firstix,
					      int afterix);
momval_t mom_make_tuple_from_item_vector (momitem_t *vectitm);
momval_t mom_make_tuple_from_item_vector_slice (momitem_t *vectitm,
						int firstix, int afterix);

/**************** assoc items ****************/

/** the assoc payload data is a GC_MALLOC-ed struct mom_itemattributes_st */

void mom_item_start_assoc (momitem_t *itm);
void mom_item_assoc_reserve (momitem_t *itm, unsigned gap);
momval_t mom_item_assoc_get (momitem_t *itm, const momitem_t *atitm);
momval_t mom_item_assoc_set_attrs (momitem_t *itm);
void mom_item_assoc_put (momitem_t *itm, const momitem_t *atitm,
			 const momval_t val);
void mom_item_assoc_remove (momitem_t *itm, const momitem_t *atitm);
/************* misc items *********/
// convert a boolean to a predefined item json_true or json_false
const momitem_t *mom_get_item_bool (bool v);


/*************** hset items, hashed set of non-nil values ***********/


#define MOM_HSET_MAGIC 0x1aa83675	/* hset magic 447231605 */
struct momhset_st
{
  unsigned hset_magic;		/* always MOM_HSET_MAGIC */
  unsigned hset_count;		/* used count */
  unsigned hset_size;		/* allocated size */
  momval_t *hset_arr;		/* array */
};

void mom_item_start_hset (momitem_t *itm);
void mom_item_hset_reserve (momitem_t *itm, unsigned gap);
bool mom_item_hset_contains (momitem_t *itm, momval_t elem);
bool mom_item_hset_add (momitem_t *itm, momval_t elem);
bool mom_item_hset_remove (momitem_t *itm, momval_t elem);
momval_t mom_item_hset_items_set (momitem_t *itm);
momval_t mom_item_hset_sorted_values_node (momitem_t *hsetitm,
					   momitem_t *connitm);

////////////////////////////////////////////////////////////////
/////////// SEQUENCE OF ITEMS, SETS & TUPLES
////////////////////////////////////////////////////////////////

// for sets and tuples of item. In sets, itemseq is sorted by
// increasing idstr. In tuples, some itemseq components may be nil.
struct momseqitem_st
{
  momtynum_t typnum;
  momvflags_t flags;
  momusize_t slen;
  momhash_t hash;
  const momitem_t *itemseq[];
};

// make a set until a NULL value, argument can be tuples, sets, items.
const momset_t *mom_make_set_til_nil (momval_t first, ...)
  __attribute__ ((sentinel));
// make a set from a variadic number of arguments, each being a tuple,
// a set or an item
const momset_t *mom_make_set_variadic (unsigned nbargs, ...);
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
// make a tuple from a variadic number of arguments, each being a tuple,
// a set or an item
const momtuple_t *mom_make_tuple_variadic (unsigned nbargs, ...);
const momtuple_t *mom_make_tuple_from_array (unsigned siz,
					     const momitem_t **itemarr);
const momtuple_t *mom_make_tuple_from_slice (const momval_t srcseq,
					     int startix, int endix);

const momtuple_t *mom_make_tuple_insertion (const momval_t srcseq,
					    int ix, const momval_t insv);


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
/////////// NODES
////////////////////////////////////////////////////////////////

// for nodes
struct momnode_st
{
  momtynum_t typnum;
  momvflags_t flags;
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


static inline bool
mom_is_node (momval_t nodv)
{
  return (nodv.ptr && *nodv.ptype == momty_node);
}

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

static inline const momval_t *
mom_closed_values (momval_t clov)
{
  if (clov.ptr == NULL)
    return NULL;
  switch (*clov.ptype)
    {
    case momty_item:
      return mom_item_closure_values (clov.pitem);
    case momty_node:
      return clov.pnode->sontab;
    default:
      return NULL;
    }
}


static inline const struct momroutinedescr_st *
mom_closed_routdescr (momval_t clov)
{
  if (clov.ptr == NULL)
    return NULL;
  switch (*clov.ptype)
    {
    case momty_item:
      return mom_item_routinedescr (clov.pitem);
    case momty_node:
      return mom_item_routinedescr (clov.pnode->connitm);
    default:
      return NULL;
    }
}

////////////////////////////////////////////////////////////////
///// FLAGS
////////////////////////////////////////////////////////////////
static inline bool
mom_with_flags (const momval_t v)
{
  if (v.ptr == NULL)
    return false;
  switch ((enum momvaltype_en) (*v.ptype))
    {
    case momty_null:
    case momty_int:
    case momty_double:
    case momty_string:
    case momty_jsonarray:
    case momty_jsonobject:
      return false;
    case momty_set:
    case momty_tuple:
      return v.pseqitems->slen > 0;
    case momty_node:
      return true;
    case momty_item:
      return false;
    }
  return false;
}


static inline momvflags_t
mom_flags (const momval_t v)
{
  if (!mom_with_flags (v))
    return 0;
  else
    return __atomic_load_n (&v.phead->hflags, __ATOMIC_SEQ_CST);
}

static inline bool
mom_has_flags (const momval_t v, unsigned flags)
{
  if (!mom_with_flags (v))
    return false;
  return (__atomic_load_n (&v.phead->hflags, __ATOMIC_SEQ_CST) & flags) != 0;
}


// return the previous values of the flags
static inline momvflags_t
mom_set_flags (momval_t v, unsigned flags)
{
  if (!mom_with_flags (v))
    return 0;
  return __atomic_fetch_or (&v.phead->hflags, (momvflags_t) flags,
			    __ATOMIC_SEQ_CST);
}

static inline momvflags_t
mom_clear_flags (momval_t v, unsigned flags)
{
  if (!mom_with_flags (v))
    return 0;
  __atomic_fetch_and (&v.phead->hflags, (momvflags_t) ~flags,
		      __ATOMIC_SEQ_CST);
}


static inline momvflags_t
mom_put_flags (momval_t v, unsigned flags)
{
  if (!mom_with_flags (v))
    return 0;
  momvflags_t f = flags;
  return __atomic_exchange_n (&v.phead->hflags, &f, __ATOMIC_SEQ_CST);
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

static inline void
mom_item_set_space (momitem_t *itm, unsigned space)
{
  if (itm && itm->i_typnum == momty_item)
    {
      assert (itm->i_magic == MOM_ITEM_MAGIC);
      if (space <= momspa__last)
	itm->i_space = space;
    }
}

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
  Dbg(gencod)					\
  Dbg(cmd)					\
  Dbg(web)

#define MOM_DEBUG_DEFINE_OPT(Nam) momdbg_##Nam,
enum mom_debug_en
{
  momdbg__none,
  MOM_DEBUG_LIST_OPTIONS (MOM_DEBUG_DEFINE_OPT) momdbg__last
};

unsigned mom_debugflags;

#define MOM_IS_DEBUGGING(Dbg) (mom_debugflags & (1<<momdbg_##Dbg))

void mom_set_debugging (const char *dbgopt);

void
mom_debug_at (const char *fil, int lin, enum mom_debug_en dbg, ...)
__attribute__ ((sentinel));

#define MOM_DEBUG_AT(Fil,Lin,Dbg,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))			\
      mom_debug_at (Fil,Lin,momdbg_##Dbg,	\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_DEBUG_AT_BIS(Fil,Lin,Dbg,...)	\
  MOM_DEBUG_AT(Fil,Lin,Dbg,			\
		    ##__VA_ARGS__)

#define MOM_DEBUG(Dbg,...)			\
  MOM_DEBUG_AT_BIS(__FILE__,__LINE__,Dbg,	\
			##__VA_ARGS__)



void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
		    const char *fmt, ...)
__attribute__ ((format (printf, 4, 5)));

#define MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))				\
      mom_debugprintf_at (Fil,Lin,momdbg_##Dbg,Fmt,	\
		    ##__VA_ARGS__);			\
  } while(0)

#define MOM_DEBUGPRINTF_AT_BIS(Fil,Lin,Dbg,Fmt,...)	\
  MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_DEBUGPRINTF(Dbg,Fmt,...)			\
  MOM_DEBUGPRINTF_AT_BIS(__FILE__,__LINE__,Dbg,Fmt,	\
			##__VA_ARGS__)


/************************* inform *************************/
void mom_inform_at (const char *fil, int lin, ...) __attribute__ ((sentinel));

#define MOM_INFORM_AT(Fil,Lin,...) do {	\
      mom_inform_at (Fil,Lin,	\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_INFORM_AT_BIS(Fil,Lin,...)	\
  MOM_INFORM_AT(Fil,Lin,			\
		    ##__VA_ARGS__)

#define MOM_INFORM(...)		\
  MOM_INFORM_AT_BIS(__FILE__,__LINE__,##__VA_ARGS__)



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

#define MOM_WARNING_AT(Fil,Lin,...) do {	\
      mom_warning_at (Fil,Lin,	\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_WARNING_AT_BIS(Fil,Lin,...)	\
  MOM_WARNING_AT(Fil,Lin,			\
		    ##__VA_ARGS__)

#define MOM_WARNING(...)		\
  MOM_WARNING_AT_BIS(__FILE__,__LINE__,	\
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
void mom_fatal_at (const char *fil, int lin, ...)
  __attribute__ ((sentinel, noreturn));

#define MOM_FATAL_AT(Fil,Lin,...) do {	\
      mom_fatal_at (Fil,Lin,		\
		    ##__VA_ARGS__, NULL);	\
  } while(0)

#define MOM_FATAL_AT_BIS(Fil,Lin,...)	\
  MOM_FATAL_AT(Fil,Lin,			\
		    ##__VA_ARGS__)

#define MOM_FATAL(...)		\
  MOM_FATAL_AT_BIS(__FILE__,__LINE__,	\
			##__VA_ARGS__)



void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4), noreturn));

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

// declare but don't define them. Linker should complain if
// referenced... which happens only with wrong MOMOUT_... macros
// above.
extern const char *mombad_literal;
extern const char *mombad_string;
extern const char *mombad_html;
extern const char *mombad_int;
extern const char *mombad_long;
extern const char *mombad_intptr_t;
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
extern const char *mombad_array;
extern const char *mombad_unsigned;




////////////////////////////////////////////////////////////////
///// QUEUES OF NON-NIL VALUES
////////////////////////////////////////////////////////////////

// we pack together several values in queue elements to achieve better
// cache locality...
#define MOM_QUEUEPACK_LEN 7
struct mom_vaqelem_st
{
  struct mom_vaqelem_st *vqe_next;
  momval_t vqe_valtab[MOM_QUEUEPACK_LEN];
};

struct mom_valuequeue_st
{
  struct mom_vaqelem_st *vaq_first;
  struct mom_vaqelem_st *vaq_last;
};

static inline bool
mom_queue_is_empty (struct mom_valuequeue_st *vq)
{
  assert (vq != NULL);
  return vq->vaq_first == NULL;
}

static inline unsigned
mom_queue_length (struct mom_valuequeue_st *vq)
{
  assert (vq != NULL);
  unsigned cnt = 0;
  for (struct mom_vaqelem_st * qel = vq->vaq_first;
       qel != NULL; qel = qel->vqe_next)
    {
      if (qel == vq->vaq_first || qel == vq->vaq_last)
	{
	  for (unsigned ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	    if (qel->vqe_valtab[ix].ptr)
	      cnt++;
	}
      else
	cnt += MOM_QUEUEPACK_LEN;
    }
  return cnt;
}

static inline void
mom_queue_add_value_back (struct mom_valuequeue_st *vq, const momval_t val)
{
  assert (vq != NULL);
  struct mom_vaqelem_st *qel = NULL;
  if (val.ptr == NULL)
    return;
  if (MOM_UNLIKELY (vq->vaq_last == NULL))
    {
      assert (vq->vaq_first == NULL);
      qel =
	MOM_GC_ALLOC ("add back value empty queue",
		      sizeof (struct mom_vaqelem_st));
      qel->vqe_valtab[0] = val;
      vq->vaq_last = vq->vaq_first = qel;
    }
  else
    {
      qel = vq->vaq_last;
      if (qel->vqe_valtab[MOM_QUEUEPACK_LEN - 1].ptr == NULL
	  || qel->vqe_valtab[0].ptr == NULL)
	{
	  momval_t vpack[MOM_QUEUEPACK_LEN] = { MOM_NULLV };
	  int ix = 0, cnt = 0;
	  for (ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	    {
	      momval_t curval = qel->vqe_valtab[ix];
	      if (curval.ptr)
		vpack[cnt++] = curval;
	    }
	  assert (cnt >= 0 && cnt < MOM_QUEUEPACK_LEN);
	  vpack[cnt++] = val;
	  for (ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	    qel->vqe_valtab[ix] = vpack[ix];
	}
      else
	{
	  qel =
	    MOM_GC_ALLOC ("add back value nonempty queue",
			  sizeof (struct mom_vaqelem_st));
	  qel->vqe_valtab[0] = val;
	  vq->vaq_last->vqe_next = qel;
	  vq->vaq_last = qel;
	}
    }
}

static inline void
mom_queue_add_value_front (struct mom_valuequeue_st *vq, const momval_t val)
{
  assert (vq != NULL);
  if (val.ptr == NULL)
    return;
  struct mom_vaqelem_st *qel = NULL;
  if (MOM_UNLIKELY (vq->vaq_first == NULL))
    {
      assert (vq->vaq_last == NULL);
      qel =
	MOM_GC_ALLOC ("add front value empty queue",
		      sizeof (struct mom_vaqelem_st));
      qel->vqe_valtab[0] = val;
      vq->vaq_last = vq->vaq_first = qel;
    }
  else
    {
      qel = vq->vaq_first;
      if (qel->vqe_valtab[MOM_QUEUEPACK_LEN - 1].ptr == NULL
	  || qel->vqe_valtab[0].ptr == NULL)
	{
	  momval_t vpack[MOM_QUEUEPACK_LEN] = { MOM_NULLV };
	  int ix = 0, cnt = 1;
	  vpack[0] = val;
	  for (ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	    {
	      momval_t curval = qel->vqe_valtab[ix];
	      if (curval.ptr)
		vpack[cnt++] = curval;
	    }
	  assert (cnt > 0 && cnt < MOM_QUEUEPACK_LEN);
	  for (ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	    qel->vqe_valtab[ix] = vpack[ix];
	}
      else
	{
	  qel =
	    MOM_GC_ALLOC ("add front value nonempty queue",
			  sizeof (struct mom_vaqelem_st));
	  qel->vqe_valtab[0] = val;
	  vq->vaq_first->vqe_next = qel;
	  vq->vaq_first = qel;
	}
    }
}

static inline momval_t
mom_queue_peek_value_front (struct mom_valuequeue_st *vq)
{
  assert (vq != NULL);
  struct mom_vaqelem_st *qel = vq->vaq_first;
  if (MOM_UNLIKELY (qel == NULL))
    return MOM_NULLV;
  else
    {
      for (int ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	if (qel->vqe_valtab[ix].ptr)
	  return qel->vqe_valtab[ix];
      MOM_FATAPRINTF ("corrupted queue @%p", vq);
    }
}

static inline momval_t
mom_queue_pop_value_front (struct mom_valuequeue_st *vq)
{
  assert (vq != NULL);
  struct mom_vaqelem_st *qel = vq->vaq_first;
  if (MOM_UNLIKELY (qel == NULL))
    return MOM_NULLV;
  else
    {
      momval_t val = MOM_NULLV;
      momval_t vpack[MOM_QUEUEPACK_LEN] = { MOM_NULLV };
      int cnt = 0;
      for (int ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	{
	  if (qel->vqe_valtab[ix].ptr)
	    {
	      if (!val.ptr)
		val = qel->vqe_valtab[ix];
	      else
		vpack[cnt++] = qel->vqe_valtab[ix];
	    };
	};
      assert (val.ptr);
      if (0 == cnt)
	{
	  if (qel == vq->vaq_last)
	    vq->vaq_first = vq->vaq_last = NULL;
	  else
	    vq->vaq_first = qel->vqe_next;
	  MOM_GC_FREE (qel);
	}
      else
	{
	  for (int ix = 0; ix < MOM_QUEUEPACK_LEN; ix++)
	    qel->vqe_valtab[ix] = vpack[ix];
	}
      if (MOM_UNLIKELY (!val.ptr))
	MOM_FATAPRINTF ("corrupted queue @%p", vq);
      return val;
    }
}

static inline momval_t
mom_queue_peek_value_back (struct mom_valuequeue_st *vq)
{
  assert (vq != NULL);
  struct mom_vaqelem_st *qel = vq->vaq_last;
  if (MOM_UNLIKELY (qel == NULL))
    return MOM_NULLV;
  else
    {
      for (int ix = MOM_QUEUEPACK_LEN - 1; ix >= 0; ix--)
	if (qel->vqe_valtab[ix].ptr)
	  return qel->vqe_valtab[ix];
      MOM_FATAPRINTF ("corrupted queue @%p", vq);
    }
}


////////////////////////////////////////////////////////////////
///// MISCELLANOUS
////////////////////////////////////////////////////////////////
// the program handle from GC_dlopen with NULL
void *mom_prog_dlhandle;
//// load a plugin
void mom_load_plugin (const char *plugname, const char *plugarg);
//// load a module, return true on success
bool mom_load_module (const char *dirname, const char *modulename);
/// modules are required to define
extern const char mom_module_GPL_compatible[];	// a string describing the licence

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
  double odmp_cputime;
  double odmp_elapsedtime;
  unsigned odmp_nbdumpeditems;
  momval_t odmp_tuplenamed;
  momval_t odmp_jarrayname;
  momval_t odmp_setpredef;
  momval_t odmp_nodenotice;
  momval_t odmp_nodemodules;
};

// the initial loading
void mom_initial_load (const char *ldir);

// the state dumper
void mom_full_dump (const char *reason, const char *dumpdir,
		    struct mom_dumpoutcome_st *outd);

// can be called from dumping routines
void mom_dump_require_module (struct mom_dumper_st *du, const char *modname);

void mom_dump_notice (struct mom_dumper_st *du, momval_t nval);

void mom_dump_scan_value (struct mom_dumper_st *dmp, const momval_t val);

void mom_dump_add_scanned_item (struct mom_dumper_st *du,
				const momitem_t *itm);

momval_t mom_dump_emit_json (struct mom_dumper_st *dmp, const momval_t val);



// emit a short representation of an item: if it is in the current
// space, just its id string...
momval_t mom_emit_short_item_json (struct mom_dumper_st *dmp,
				   const momitem_t *itm);

// initial load
void mom_initial_load (const char *ldirnam);

/////////////////// agenda and workers and web
#define MOM_MAX_WORKERS 10
#define MOM_MIN_WORKERS 2
int mom_nb_workers;
const char *mom_web_host;
const char *mom_jsonrpc_host;
void mom_add_tasklet_to_agenda_back (momitem_t *tkitm);
void mom_add_tasklet_to_agenda_front (momitem_t *tkitm);
typedef void mom_todoafterstop_fun_t (void *data);
void mom_stop_work_with_todo (mom_todoafterstop_fun_t * todofun, void *data);
void mom_run_workers (void);
void mom_stop_event_loop (void);
void mom_continue_working (void);

/// initialize signal processing, should be done very early
void mom_initialize_signals (void);

// this is called in generated procedure prologue
static inline momitem_t *
mom_procedure_item_of_id (const char *id)
{
  momitem_t *pitm = mom_get_item_of_identcstr (id);
  if (MOM_UNLIKELY (!pitm || pitm->i_typnum != momty_item
		    || pitm->i_paylkind != mompayk_procedure))
    {
      MOM_WARNPRINTF ("failed to find procedure item of id %s", id);
      return NULL;
    }
  return pitm;
}


static inline momval_t *
mom_item_procedure_values (momitem_t *itm, int nbvals)
{
  momval_t *resarr = NULL;
  if (!itm || itm->i_typnum != momty_item)
    return NULL;
  mom_should_lock_item (itm);
  if (itm->i_paylkind != mompayk_procedure)
    {
      mom_unlock_item (itm);
      return NULL;
    };
  struct momprocedure_st *proc = itm->i_payload;
  assert (proc && proc->proc_magic == MOM_PROCEDURE_MAGIC);
  const struct momprocrout_st *prout = proc->proc_rout;
  assert (prout && prout->prout_magic == MOM_PROCROUT_MAGIC);
  unsigned plen = prout->prout_len;
  if ((int) nbvals <= (int) plen)
    resarr = proc->proc_valtab;
  mom_unlock_item (itm);
  return resarr;
}

/// two prefixes known by our Makefile!
// generated modules start with:
#define MOM_SHARED_MODULE_PREFIX "momg_"
// plugins path start with
#define MOM_PLUGIN_PREFIX "momplug_"
#define MOM_SHARED_MODULE_DIRECTORY "modules"
/// plugins are required to define
extern const char mom_plugin_GPL_compatible[];	// a string describing the licence
extern void mom_plugin_init (const char *pluginarg);	// the plugin initializer
/// they may also define a function to be called after load
extern void momplugin_after_load (void);

/// declare the predefined named and anonymous
#define MOM_PREDEFINED_NAMED(Name,Id,H) extern momitem_t* mom_named__##Name;
#define MOM_PREDEFINED_ANONYMOUS(Id,H) extern momitem_t* mom_anonymous_##Id;
#include "predef-monimelt.h"

/// declare the hash of the predefined as an enum

#define MOM_PREDEFINED_NAMED(Name,Id,H) mom_hashname__##Name = H,
#define MOM_PREDEFINED_ANONYMOUS(Id,H) mom_hashanon__##Id = H,
enum
{
#include "predef-monimelt.h"
};

#ifdef MELTMOM
#pragma MONIMELT DISABLE
#endif

#endif /*MONIMELT_INCLUDED_ */
