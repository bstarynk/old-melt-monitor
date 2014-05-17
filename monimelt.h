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
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <fcntl.h>
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



////////////////////////////////////////////////////////////////
//////////////// TYPES AND VALUES
////////////////////////////////////////////////////////////////
enum momvaltype_en
{
  momty_null = 0,
  momty_int,
  momty_float,
  momty_string,
  momty_set,
  momty_tuple,
  momty_node,
  momty_assoc,
  momty_item
};

struct momseqitem_st;

typedef struct momint_st momint_t;
typedef struct momfloat_st momfloat_t;
typedef struct momstring_st momstring_t;
typedef struct momseqitem_st momseqitem_t;
typedef struct momseqitem_st momset_t;
typedef struct momseqitem_st momtuple_t;
typedef struct momnode_st momnode_t;
typedef struct momassoc_st momassoc_t;
typedef struct momitem_st momitem_t;
union momvalueptr_un
{
  void *ptr;
  const momtynum_t *ptype;
  const momint_t *pint;
  const momfloat_t *pfloat;
  const momstring_t *pstring;
  const momset_t *pset;
  const momtuple_t *ptuple;
  const momnode_t *pnode;
  const momassoc_t *passoc;
  momitem_t *pitem;
};
typedef union momvalueptr_un momval_t;


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
// make a random id string
const momstring_t *mom_make_random_idstr ();
// check that s points to a string looking like a random id string;
// the ending character should not be alphanumerical but it might be _
// and is stored in *pend if pend non-null.
bool mom_looks_like_random_id_cstr (const char *s, const char **pend);
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

////////////////////////////////////////////////////////////////
/////////// DIAGNOSTICS
////////////////////////////////////////////////////////////////

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
		    ##__VA_ARGS__, NULL);		\
  } while(0)

#define MOM_DEBUGPRINTF_AT_BIS(Dbg,Fil,Lin,Fmt,...)	\
  MOM_DEBUGPRINTF_AT(Dbg,Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_DEBUGPRINTF(Dbg,Fmt,...)			\
  MOM_DEBUGPRINTF_AT_BIS(Dbg,__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)


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

#define MOM_MOUT_MAGIC 0x41f67aa5	/* mom_out_magic 1106672293 */
struct momout_st
{
  unsigned mout_magic;		/* always MOM_MOUT_MAGIC */
  int mout_indent;
  FILE *mout_file;
  long mout_lastnl;		/* offset at last newline with MOMOUT_NEWLINE or MOMOUT_SPACE */
};
typedef struct momout_st momout_t;
extern struct momout_st mom_stdout_data;
extern struct momout_st mom_stderr_data;
#define mom_stdout &mom_stdout_data
#define mom_stderr &mom_stderr_data
void mom_out_at (const char *sfil, int lin, momout_t * pout, ...)
  __attribute__ ((sentinel));
void mom_outva_at (const char *sfil, int lin, momout_t * pout, va_list alist);
#define MOM_OUT_AT_BIS(Fil,Lin,Out,...) mom_out_at(Fil,Lin,Out,##__VA_ARGS__,NULL)
#define MOM_OUT_AT(Fil,Lin,Out,...) MOM_OUT_AT_BIS(Fil,Lin,Out,##__VA_ARGS__)
#define MOM_OUT(Out,...) MOM_OUT_AT(__FILE__,__LINE__,Out,##__VA_ARGS__)


#define MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)				\
  (__builtin_choose_expr((__builtin_types_compatible_p(typeof(V),Typ)), \
			 (V), (void)((Else)+Lin)))
#define MOM_REQUIRES_TYPE_AT_BIS(Lin,V,Typ,Else) MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)
#define MOM_REQUIRES_TYPE(V,Typ,Else) MOM_REQUIRES_TYPE_AT_BIS(__LINE__,(V),Typ,Else)

enum momoutdir_en
{
  MOMOUTDO__END = 0,
  ///
  /// literal strings
  MOMOUTDO_LITERAL /*, const char*literalstring */ ,
#define MOMOUT_LITERAL(S) MOMOUTDO_LITERAL, MOM_REQUIRES_TYPE(S,const char*,mombad_literal)
  ///
  /// HTML encoded strings
  MOMOUTDO_HTML /*, const char*htmlstring */ ,
#define MOMOUT_HTML(S) MOMOUTDO_HTML, MOM_REQUIRES_TYPE(S,const char*,mombad_html)
  ///
  /// Javascript encoded strings
  MOMOUTDO_JS /*, const char*jsstring */ ,
#define MOMOUT_JS(S) MOMOUTDO_JS, MOM_REQUIRES_TYPE(S,const char*,mombad_js)
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
  ///
  /// output a space or an indented small newline if the current line
  /// exceeds a given threshold
  MOMOUTDO_SMALL_SPACE /*, unsigned threshold */ ,
#define MOMOUT_SMALL_SPACE(L) MOMOUTDO_SMALL_SPACE,	\
    MOM_REQUIRES_TYPE(L,int,mombad_space)
  ///

};
// declare but don't define them. Linker should complain if
// referenced... which happens only with wrong MOMOUT_... macros
// above.
extern const char *mombad_literal;
extern const char *mombad_html;
extern const char *mombad_int;
extern const char *mombad_js;
extern const char *mombad_fmt;
extern const char *mombad_double;
extern const char *mombad_long;
extern const char *mombad_longlong;
extern const char *mombad_file;
extern const char *mombad_space;

#endif /*MONIMELT_INCLUDED_ */
