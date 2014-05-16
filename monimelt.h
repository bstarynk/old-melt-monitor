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

#define MOM_WARNING(Dbg,Fmt,...)		\
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



void mom_out_at (const char *sfil, int lin, FILE * out, ...)
  __attribute__ ((sentinel));
void mom_outva_at (const char *sfil, int lin, FILE * out, va_list alist);
#define MOM_OUT_AT_BIS(Fil,Lin,Out,...) mom_out_at(Fil,Lin,Out,##__VA_ARGS__,NULL)
#define MOM_OUT_AT(Fil,Lin,Out,...) MOM_OUT_AT_BIS(Fil,Lin,Out,...)
#define MOM_OUT(Out,...) MOM_OUT_AT(__FILE__,__LINE__,Out,##__VA_ARGS__)

#define MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)				\
  (__builtin_choose_expr((__builtin_types_compatible_p(typeof(V),Typ)), \
			 (V), ((Else)[Lin])))
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
};

extern const char *mombad_literal;
extern const char *mombad_html;

#endif /*MONIMELT_INCLUDED_ */
