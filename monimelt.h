// file monimelt.h - common header file to be included everywhere.

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
#include <sys/syscall.h>
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
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <fcntl.h>
#include <dlfcn.h>
#if __GLIBC__
#include <execinfo.h>
#endif

// libunistring: https://www.gnu.org/software/libunistring/
#include <unistr.h>

/// Boehm GC from http://www.hboehm.info/gc/
/// ...perhaps we should consider MPS from
///            http://www.ravenbrook.com/project/mps/
#include <gc/gc.h>

// libonion from http://www.coralbits.com/libonion/ &
// https://github.com/davidmoreno/onion
#include <onion/onion.h>
#include <onion/low.h>
#include <onion/codecs.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/block.h>
#include <onion/handler.h>
#include <onion/dict.h>
#include <onion/log.h>
#include <onion/shortcuts.h>
#include <onion/exportlocal.h>
#include <onion/internal_status.h>
#include <onion/websocket.h>


// jansson, a JSON library in C which is Boehm-GC friendly
// see http://www.digip.org/jansson/
#include <jansson.h>


// CURL, a very usually HTTP client library, see http://curl.haxx.se/libcurl/
#include <curl/curl.h>

// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];

// increasing array of primes and its size
extern const int64_t mom_primes_tab[];
extern const unsigned mom_primes_num;
/// give a prime number above or below a given n, or else 0
int64_t mom_prime_above (int64_t n);
int64_t mom_prime_below (int64_t n);

#define MOM_MAX_WORKERS 10
#define MOM_MIN_WORKERS 2
int mom_nb_workers;
extern _Thread_local int mom_worker_num;

const char *mom_web_host;
const char *mom_socket_path;
const char *mom_user_data;
#define MOM_MAX_WEBDOCROOT 8
const char *mom_webdocroot[MOM_MAX_WEBDOCROOT + 1];

#define MOM_WEB_DOC_ROOT_PREFIX "wdoc/"
#define MOM_WEB_SOCKET_FULL_PATH "/websocket"
/** if the webroot/ directory exists, we serve the files inside as web
   document roots web-doc-root/, so file webroot/jquery.min.js would
   be accessible eg thru the URL
   http://localhost:8087/wdoc/jquery.min.js if we started with
   webservice localhost:8087 */
#define MOM_WEBDOCROOT_DIRECTORY "webroot/"
const char *mom_webpasswdfile;
#define MOM_DEFAULT_WEBPASSWD ".mompasswd"
#define MOM_MIN_PASSWD_LEN 7
typedef bool mom_web_authentificator_sig_t (const char *webuser,
					    const char *webpasswd);
mom_web_authentificator_sig_t *mom_web_authentificator;

// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define MOM_UNLIKELY(P) __builtin_expect((P),0)
#define MOM_LIKELY(P) !__builtin_expect(!(P),0)
#define MOM_UNUSED __attribute__((unused))
#else
#define MOM_UNLIKELY(P) (P)
#define MOM_LIKELY(P) (P)
#define MOM_UNUSED
#endif

extern bool mom_should_stop (void);
extern void mom_stop_work (void);
extern void mom_run_workers (void);

// every hashcode is a non-zero 32 bits unsigned
typedef uint32_t momhash_t;

typedef struct momstring_st momstring_t;
typedef struct momdelim_st momdelim_t;
typedef struct momitem_st momitem_t;
typedef struct momitem_st momlockeditem_t;
typedef struct momnode_st momnode_t;
typedef struct momseq_st momseq_t;
typedef struct momvalue_st momvalue_t;

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


/// two prefixes known by our Makefile!
// generated modules start with:
#define MOM_MODULE_DIRECTORY "modules/"
#define MOM_SHARED_MODULE_PREFIX "momg_"
// plugins path start with
#define MOM_PLUGIN_PREFIX "momplug_"
// generated functions start with
#define MOM_FUNCTION_PREFIX "momfunc_"


// the weblogin template is seeked in all the webroots and at last in
// the MOM_WEBDOCROOT_DIRECTORY; it is used when we have no valid cookies.
#define MOM_WEBLOGIN_TEMPLATE_FILE "momlogin.thtml"

// the weblogin template file should contain a single line with one of:
#define MOM_WEBLOGIN_HIDDEN_INPUT_PI "<?mom_web_login_hidden_input?>"
#define MOM_WEBLOGIN_HIDDEN_INPUT_COMM "<!--mom_web_login_hidden_input-->"
// and could contain also lines with
#define MOM_WEBLOGIN_TIMESTAMP_PI "<?mom_web_login_timestamp?>"

#define MOM_WEBLOGIN_ACTION "mom_web_login"


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

// every monimelt value starts with a non-zero unsigned typenum
typedef int8_t momtypenum_t;
typedef enum momvaltype_en
{
  momty_delim = -3,
  momty_double = -2,
  momty_int = -1,
  momty_null = 0,
  momty_string,
  momty_item,
  momty_tuple,
  momty_set,
  momty_node,
} momvaltype_t;

typedef enum momspace_en
{
  momspa_none,
  momspa_transient,
  momspa_user,
  momspa_global,
  momspa_predefined
} momspace_t;


struct momdelim_st
{
  char delim[4];
};

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


// dont bother freeing, per H.Boehm's advice, too small zones
#define MOM_SMALL_FREE_THRESHOLD (256*sizeof(void*))
// free a pointer of known sizeand clear the variable
#define MOM_GC_FREE_AT(VarPtr,Siz,Lin) do {		\
    void* _fp_##Lin = VarPtr;				\
    size_t _siz_##Lin = (Siz);				\
    if (_fp_##Lin &&  _siz_##Lin>0)			\
      memset(_fp_##Lin, 0, _siz_##Lin);			\
    VarPtr = NULL;					\
    if (_fp_##Lin != NULL				\
	&& _siz_##Lin > MOM_SMALL_FREE_THRESHOLD)	\
      GC_FREE(_fp_##Lin); } while(0)
#define MOM_GC_FREE(VarPtr,Siz) MOM_GC_FREE_AT(VarPtr,Siz,__LINE__)

// GCC compiler trick to add some typechecking in variadic functions
#define MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)				\
  (__builtin_choose_expr((__builtin_types_compatible_p(typeof(V),Typ)), \
			 (V), (void)((Else)+Lin)))
#define MOM_REQUIRES_TYPE_AT_BIS(Lin,V,Typ,Else) MOM_REQUIRES_TYPE_AT(Lin,V,Typ,Else)
#define MOM_REQUIRES_TYPE(V,Typ,Else) MOM_REQUIRES_TYPE_AT_BIS(__LINE__,(V),Typ,Else)



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

// for debugging:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)		\
  Dbg(item)					\
  Dbg(dump)					\
  Dbg(load)					\
  Dbg(run)					\
  Dbg(gencod)					\
  Dbg(cmd)					\
  Dbg(low)					\
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



// the program handle from GC_dlopen with NULL
void *mom_prog_dlhandle;


typedef struct momvalue_st momvalue_t;

#define MOM_GLOBAL_DATA_PATH "global.mom"
#define MOM_USER_DATA_PATH "user.mom"
#define MOM_PREDEFINED_PATH "predef-monimelt.h"
#define MOM_FILL_PREDEFINED_PATH "fill-monimelt.c"
#define MOM_APPLY_HEADER_PATH "apply-monimelt.h"


void mom_load_state (void);
void mom_dump_state (const char *prefix);

momvalue_t mom_peek_token_load_at (const char *fil, int lin);
#define mom_peek_token_load() mom_peek_token_load_at(__FILE__,__LINE__)

momvalue_t mom_peek_next_token_load_at (const char *fil, int lin);
#define mom_peek_next_token_load() mom_peek_token_load_at(__FILE__,__LINE__)

void mom_eat_token_load_at (const char *fil, int lin);
#define mom_eat_token_load() mom_eat_token_load_at(__FILE__,__LINE__)

unsigned mom_load_nb_queued_tokens (void);
// return the node of queued tokens, or nil if none
const momnode_t *mom_load_queued_tokens_mode (const momitem_t *connitm,
					      momvalue_t meta);
void mom_load_push_front_token (momvalue_t valtok);
void mom_load_push_back_token (momvalue_t valtok);

static inline const momitem_t *mom_load_itemref_at (const char *fil, int lin);

bool mom_load_value (momvalue_t *pval);



void mom_initialize_random (void);
void mom_initialize_items (void);

// get a random non-zero number, the num is some small index of random number
// generators
uint32_t mom_random_nonzero_32 (unsigned num);
#define mom_random_nonzero_32_here() mom_random_nonzero_32(__LINE__)
// get two random non-zero numbers
void mom_random_two_nonzero_32 (unsigned num, uint32_t *r1, uint32_t *r2);
// get three random non-zero numbers
void mom_random_three_nonzero_32 (unsigned num, uint32_t *r1, uint32_t *r2,
				  uint32_t *r3);
// get a random, possibly zero, 32 bits or 64 bits number
uint32_t mom_random_32 (unsigned num);
uint64_t mom_random_64 (unsigned num);
uintptr_t mom_random_intptr (unsigned num);

/// plugins are required to define
extern const char mom_plugin_GPL_compatible[];	// a string describing the licence
typedef void mom_plugin_init_t (const char *pluginarg, int *pargc,
				char ***pargv);
extern void mom_plugin_init (const char *pluginarg, int *pargc, char ***pargv);	// the plugin initializer
/// they may also define a function to be called after load
typedef void mom_plugin_after_load_t (void);
extern void mom_plugin_after_load (void);

struct momvalue_st
{
  momtypenum_t typnum;
  atomic_bool istransient;
  union
  {
    void *vptr;
    intptr_t vint;
    double vdbl;
    momdelim_t vdelim;
    const momstring_t *vstr;
    momitem_t *vitem;
    const momnode_t *vnode;
    const momseq_t *vsequ;
    const momseq_t *vset;
    const momseq_t *vtuple;
  };
};

static inline void
mom_valueptr_set_transient (momvalue_t *pval, bool transient)
{
  if (pval && pval != MOM_EMPTY)
    atomic_store (&pval->istransient, transient);
}

static inline bool
mom_valueptr_is_transient (const momvalue_t *pval)
{
  if (!pval || pval == MOM_EMPTY)
    return false;
  return atomic_load (&pval->istransient);
}

#define MOM_NONEV ((momvalue_t){momty_null,false,{NULL}})

// return a GC_STRDUP-ed string with some output for a value
const char *mom_output_gcstring (const momvalue_t val);

// return a momstring with some output for a value
const momstring_t *mout_output_string (const momvalue_t val);

static inline momitem_t *
mom_value_to_item (const momvalue_t val)
{
  if (val.typnum == momty_item)
    return val.vitem;
  return NULL;
}

static inline intptr_t
mom_value_to_int (const momvalue_t val, intptr_t def)
{
  if (val.typnum == momty_int)
    return val.vint;
  return def;
}

momhash_t mom_valueptr_hash (momvalue_t *pval);

static inline momhash_t
mom_value_hash (momvalue_t val)
{
  return mom_valueptr_hash (&val);
}

static inline bool
mom_value_is_delim (momvalue_t v, const char *delim)
{
  return v.typnum == momty_delim
    && !strncmp (v.vdelim.delim, delim, sizeof (v.vdelim));
}

bool mom_value_equal (momvalue_t v1, momvalue_t v2);
int mom_value_compare (momvalue_t v1, momvalue_t v2);

static inline momvalue_t
mom_intv (intptr_t i)
{
  momvalue_t val = MOM_NONEV;
  val.typnum = momty_int;
  val.vint = i;
  return val;
}

static inline momvalue_t
mom_doublev (double x)
{
  momvalue_t val = MOM_NONEV;
  val.typnum = momty_double;
  val.vdbl = x;
  return val;
}

static inline momvalue_t
mom_itemv (const momitem_t *itm)
{
  momvalue_t val = MOM_NONEV;
  if (itm && itm != MOM_EMPTY)
    {
      val.typnum = momty_item;
      val.vitem = (momitem_t *) itm;
    }
  return val;
}

static inline momvalue_t
mom_delimv (const char *s)
{
  momvalue_t val = MOM_NONEV;
  if (s)
    {
      val.typnum = momty_delim;
      strncpy (val.vdelim.delim, s, sizeof (val.vdelim.delim));
    }
  return val;
}

#define MOM_MAX_STRING_LENGTH (1<<25)	/* max string length 33554432 */
struct momstring_st
{
  uint32_t slen;
  momhash_t shash;
  char cstr[];			/* length is slen+1 */
};


#define MOM_MAX_SEQ_LENGTH (1<<24)	/* max sequence length 16777216 */
struct momseq_st
{
  uint32_t slen;
  momhash_t shash;
  momvalue_t meta;
  const momitem_t *arritm[];	/* length is slen */
};

static inline unsigned
mom_seq_length (const struct momseq_st *seq)
{
  if (!seq || seq == MOM_EMPTY)
    return 0;
  return seq->slen;
}

static inline momvalue_t
mom_seq_meta (const struct momseq_st *seq)
{
  if (!seq || seq == MOM_EMPTY)
    return MOM_NONEV;
  return seq->meta;
}

static inline const momitem_t *
mom_seq_nth (const struct momseq_st *seq, int rk)
{
  if (!seq || seq == MOM_EMPTY)
    return NULL;
  unsigned ln = seq->slen;
  if (rk < 0)
    rk += (int) ln;
  if (rk >= 0 && rk < (int) ln)
    return seq->arritm[rk];
  return NULL;
}

#define MOM_MAX_NODE_LENGTH (1<<24)	/* max node length 16777216 */
struct momnode_st
{
  uint32_t slen;
  momhash_t shash;
  momitem_t *conn;
  momvalue_t meta;
  momvalue_t arrsons[];		/* length is slen */
};

struct momentry_st
{
  const momitem_t *ent_itm;
  momvalue_t ent_val;
};

static inline const momstring_t *
mom_value_to_string (const momvalue_t val)
{
  if (val.typnum == momty_string)
    return val.vstr;
  else
    return NULL;
}

static inline const momseq_t *
mom_value_to_sequ (const momvalue_t val)
{
  if (val.typnum == momty_set || val.typnum == momty_tuple)
    return val.vsequ;
  else
    return NULL;
}

static inline const momseq_t *
mom_value_to_set (const momvalue_t val)
{
  if (val.typnum == momty_set)
    return val.vset;
  else
    return NULL;
}

static inline const momseq_t *
mom_value_to_tuple (const momvalue_t val)
{
  if (val.typnum == momty_tuple)
    return val.vtuple;
  else
    return NULL;
}

static inline const momnode_t *
mom_value_to_node (const momvalue_t val)
{
  if (val.typnum == momty_node)
    return val.vnode;
  else
    return NULL;
}

struct momattributes_st
{
  uint32_t at_len;		/* allocated length */
  uint32_t at_cnt;		/* used count */
  struct momentry_st at_entries[];	/* length is at_len */
};

struct momentry_st *mom_attributes_find_entry (const struct momattributes_st
					       *attrs, const momitem_t *itma);
struct momattributes_st *mom_attributes_put (struct momattributes_st *attrs,
					     const momitem_t *itma,
					     const momvalue_t *pval);
struct momattributes_st *mom_attributes_remove (struct momattributes_st
						*attrs,
						const momitem_t *itma);
struct momattributes_st *mom_attributes_make_atva (unsigned nbent, ...
						   /* item1, val1, item2, val2, ... */
  );


static inline struct momattributes_st *
mom_attributes_make (unsigned len)
{
  if (len > MOM_MAX_SEQ_LENGTH)
    MOM_FATAPRINTF ("too long %u attributes table", len);
  struct momattributes_st *att =	//
    MOM_GC_ALLOC ("attributes",
		  sizeof (struct momattributes_st) +
		  len * sizeof (struct momentry_st));
  att->at_len = len;
  return att;
}				/* end mom_attributes_make */


void mom_attributes_scan_dump (struct momattributes_st *attrs);

static inline unsigned
mom_attributes_count (struct momattributes_st *attrs)
{
  if (!attrs || attrs == MOM_EMPTY)
    return 0;
  assert (attrs->at_cnt <= attrs->at_len);
  return attrs->at_cnt;
}

const momseq_t *mom_attributes_set (struct momattributes_st *attrs,
				    momvalue_t meta);

////////////////////////////////////////////////////
struct momdictvalent_st
{
  const momstring_t *dicent_str;
  momvalue_t dicent_val;
};

struct momhashdict_st
{
  uint32_t hdic_len;		// allocated length
  uint32_t hdic_cnt;		// used count
  struct momdictvalent_st hdic_ents[];
};				// end struct momhashdict_st

struct momhashdict_st *mom_hashdict_put (struct momhashdict_st *hdict,
					 const momstring_t *str,
					 momvalue_t val);
struct momhashdict_st *mom_hashdict_reserve (struct momhashdict_st *hdict,
					     unsigned gap);
momvalue_t mom_hashdict_get (const struct momhashdict_st *hdict,
			     const momstring_t *str);
momvalue_t mom_hashdict_getcstr (const struct momhashdict_st *hdict,
				 const char *cstr);
struct momhashdict_st *mom_hashdict_remove (struct momhashdict_st *hdict,
					    const momstring_t *str);
const momnode_t *mom_hashdict_sorted_strings_meta (const struct momhashdict_st
						   *hdict,
						   const momitem_t *connitm,
						   const momvalue_t metav);

static inline const momnode_t *mom_hashdict_sorted_strings
  (const struct momhashdict_st *hdict, const momitem_t *connitm)
{
  if (hdict && connitm)
    return mom_hashdict_sorted_strings_meta (hdict, connitm, MOM_NONEV);
  return NULL;
}

void mom_hashdict_scan_dump (struct momhashdict_st *hdict);

////////////////////////////////////////////////////

struct momcomponents_st
{
  uint32_t cp_len;		/* allocated length */
  uint32_t cp_cnt;		/* used count */
  momvalue_t cp_comps[];	/* length is cp_len */
};

static inline momvalue_t
mom_components_nth (const struct momcomponents_st *csq, int rk)
{
  if (!csq || csq == MOM_EMPTY)
    return MOM_NONEV;
  unsigned cnt = csq->cp_cnt;
  if (rk < 0)
    rk += (int) cnt;
  if (rk >= 0 && rk < (int) cnt)
    return csq->cp_comps[rk];
  return MOM_NONEV;
}

struct momcomponents_st *mom_components_append1 (struct momcomponents_st *csq,
						 const momvalue_t val);

struct momcomponents_st *mom_components_append_values (struct momcomponents_st
						       *csq, unsigned nbval,
						       ... /*values */ );

struct momcomponents_st *mom_components_append_sized_array (struct
							    momcomponents_st
							    *csq,
							    unsigned nbval,
							    const momvalue_t
							    *valarr);


struct momcomponents_st *mom_components_reserve (struct momcomponents_st *csq,
						 unsigned nbcomp);

static inline unsigned
mom_components_count (const struct momcomponents_st *csq)
{
  if (!csq || csq == MOM_EMPTY)
    return 0;
  return csq->cp_cnt;
}

void
mom_components_put_nth (struct momcomponents_st *csq, int rk,
			const momvalue_t val);

void mom_components_scan_dump (struct momcomponents_st *csq);


struct momhashset_st
{
  uint32_t hset_len;		/* allocated length */
  uint32_t hset_cnt;		/* used count */
  const momitem_t *hset_elems[];
};

static inline unsigned
mom_hashset_count (const struct momhashset_st *hset)
{
  if (hset && hset != MOM_EMPTY)
    return hset->hset_cnt;
  return 0;
}

bool mom_hashset_contains (const struct momhashset_st * hset,
			   const momitem_t *itm);
struct momhashset_st *mom_hashset_put (struct momhashset_st *hset,
				       const momitem_t *itm);
struct momhashset_st *mom_hashset_remove (struct momhashset_st *hset,
					  const momitem_t *itm);
struct momhashset_st *mom_hashset_add_items (struct momhashset_st *hset,
					     unsigned nbitems,
					     ... /* items */ );
struct momhashset_st *mom_hashset_add_sized_items (struct momhashset_st *hset,
						   unsigned siz,
						   momitem_t *const *itmarr);
const momseq_t *mom_hashset_elements_set_meta (struct momhashset_st *hset,
					       momvalue_t metav);
static inline const momseq_t *
mom_hashset_elements_set (struct momhashset_st *hset)
{
  return mom_hashset_elements_set_meta (hset, MOM_NONEV);
};

static inline momvalue_t mom_unsafe_setv (const momseq_t *seq);

static inline momvalue_t
mom_hashset_elements_value (struct momhashset_st *hset)
{
  return mom_unsafe_setv (mom_hashset_elements_set (hset));
}

void mom_hashset_scan_dump (struct momhashset_st *hset);


////////////////
struct momqueuechunkitems_st;
struct momqueueitems_st
{
  unsigned long que_size;
  struct momqueuechunkitems_st *que_front;
  struct momqueuechunkitems_st *que_back;
};
#define MOM_QUEUECHUNK_LEN 6
struct momqueuechunkitems_st
{
  struct momqueuechunkitems_st *quechi_next;
  struct momqueuechunkitems_st *quechi_prev;
  const momitem_t *quechi_items[MOM_QUEUECHUNK_LEN];
};

void mom_queueitem_push_back (struct momqueueitems_st *qu,
			      const momitem_t *itm);

void mom_queueitem_push_front (struct momqueueitems_st *qu,
			       const momitem_t *itm);

static inline const momitem_t *
mom_queueitem_peek_front (struct momqueueitems_st *qu)
{
  if (!qu)
    return NULL;
  struct momqueuechunkitems_st *fr = qu->que_front;
  if (!fr)
    return NULL;
  for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
    {
      const momitem_t *itm = fr->quechi_items[ix];
      if (itm && itm != MOM_EMPTY)
	return itm;
    };
  return NULL;
}


// if rank is >= 0, count from front; otherwise count from back
momitem_t *mom_queueitem_peek_nth (struct momqueueitems_st *qu, int rk);

static inline unsigned long
mom_queueitem_size (struct momqueueitems_st *qu)
{
  if (!qu)
    return 0;
  return qu->que_size;
}

momitem_t *mom_queueitem_pop_front (struct momqueueitems_st *qu);
const momseq_t *mom_queueitem_tuple (struct momqueueitems_st *qu,
				     momvalue_t metav);

void mom_queueitem_scan_dump (struct momqueueitems_st *qu);

////////////////

struct momqueuechunkvalues_st;
struct momqueuevalues_st
{
  unsigned long que_size;
  struct momqueuechunkvalues_st *que_front;
  struct momqueuechunkvalues_st *que_back;
};
struct momqueuechunkvalues_st
{
  struct momqueuechunkvalues_st *quechv_next;
  struct momqueuechunkvalues_st *quechv_prev;
  momvalue_t quechv_values[MOM_QUEUECHUNK_LEN];
};

void mom_queuevalue_push_back (struct momqueuevalues_st *qu,
			       const momvalue_t val);

void mom_queuevalue_push_front (struct momqueuevalues_st *qu,
				const momvalue_t val);

static inline momvalue_t
mom_queuevalue_peek_front (struct momqueuevalues_st *qu)
{
  if (!qu)
    return MOM_NONEV;
  struct momqueuechunkvalues_st *fr = qu->que_front;
  if (!fr)
    return MOM_NONEV;
  for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
    {
      const momvalue_t val = fr->quechv_values[ix];
      if (val.typnum != momty_null)
	return val;
    };
  return MOM_NONEV;
}

momvalue_t mom_queuevalue_peek_nth (struct momqueuevalues_st *qu, int rk);

static inline unsigned long
mom_queuevalue_size (struct momqueuevalues_st *qu)
{
  if (!qu)
    return 0;
  return qu->que_size;
}

momvalue_t mom_queuevalue_pop_front (struct momqueuevalues_st *qu);
const momnode_t *mom_queuevalue_node (struct momqueuevalues_st *qu,
				      const momitem_t *conn,
				      momvalue_t metav);

void mom_queuevalue_scan_dump (struct momqueuevalues_st *qu);



////////////////////////////////////////////////////////////////
/// code is supposed to lock the item with itm_mtx before using the item
struct momitem_st
{
  pthread_mutex_t itm_mtx;
  bool itm_anonymous;
  atomic_uchar itm_space;
  union
  {
    const momstring_t *itm_str;
    const momstring_t *itm_id;	/* when itm_anonymous */
    const momstring_t *itm_name;	/* when !itm_anonymous */
  };
  struct momattributes_st *itm_attrs;
  struct momcomponents_st *itm_comps;
  momitem_t *itm_kind;
  void *itm_data1;
  void *itm_data2;
};				/* end struct momitem_st */

// initialize an unregistered item
void mom_initialize_protoitem (momitem_t *protoitm);
void mom_gc_finalize_item (void *itmad, void *data);
void mom_unregister_anonymous_finalized_item (momitem_t *finitm);
void mom_unregister_named_finalized_item (momitem_t *finitm);
static inline const char *
mom_item_cstring (const momitem_t *itm)
{
  if (!itm)
    return "~";
  if (itm == MOM_EMPTY)
    return "~/*empty*/ ";
  assert (itm->itm_str);
  return itm->itm_str->cstr;
}

const char *mom_item_space_string (const momitem_t *itm);

static inline void
mom_item_lock (momitem_t *itm)
{
  assert (itm && itm != MOM_EMPTY);
  pthread_mutex_lock (&itm->itm_mtx);
}

static inline void
mom_item_unlock (momitem_t *itm)
{
  assert (itm && itm != MOM_EMPTY);
  pthread_mutex_unlock (&itm->itm_mtx);
}

static inline
  momvalue_t mom_item_unsync_get_attribute (momitem_t *itm, momitem_t *itmat);

static inline momhash_t
mom_item_hash (const momitem_t *itm)
{
  if (!itm)
    return 0;
  assert (itm->itm_str);
  return itm->itm_str->shash;
}

static inline int
mom_item_cmp (const momitem_t *itm1, const momitem_t *itm2)
{
  if (itm1 == itm2)
    return 0;
  if (!itm1)
    return -1;
  if (!itm2)
    return +1;
  assert (itm1->itm_str);
  assert (itm2->itm_str);
  if (itm1->itm_anonymous)
    {
      if (itm2->itm_anonymous)
	{
	  int cmp = strcmp (itm1->itm_id->cstr, itm2->itm_id->cstr);
	  assert (cmp != 0);
	  if (cmp < 0)
	    return -1;
	  else
	    return 1;
	}
      else
	return -1;
    }
  else
    {				// itm1 is named
      if (itm2->itm_anonymous)
	return +1;
      int cmp = strcmp (itm1->itm_name->cstr, itm2->itm_name->cstr);
      assert (cmp != 0);
      if (cmp < 0)
	return -1;
      else
	return 1;
    }
}

// call the function above to sort an array of momitem_t*
int mom_itemptr_cmp (const void *, const void *);

void mom_item_qsort (const momitem_t **arr, unsigned siz);

const momstring_t *mom_make_random_idstr (unsigned salt,
					  struct momitem_st *protoitem);

// check validity of an id string, if good, sets *pend to its end
// an id starts with _ ...
bool mom_valid_item_id_str (const char *id, const char **pend);

// check validity of a name string, if good, sets *pend to its end.  A
// name starts with a letter, contains letters and digits and
// underscores (_). Underscores cannot be doubled and should not end
// the name.
bool mom_valid_item_name_str (const char *id, const char **pend);

// hash of a C-string

momhash_t mom_cstring_hash_len (const char *str, int len);

static inline momhash_t
mom_cstring_hash (const char *str)
{
  return mom_cstring_hash_len (str, -1);
}

const momstring_t *mom_make_string_cstr (const char *str);
const momstring_t *mom_make_string_sprintf (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));
static inline momvalue_t
mom_stringv (const momstring_t *str)
{
  momvalue_t val = MOM_NONEV;
  if (str)
    {
      val.typnum = momty_string;
      val.vstr = str;
    }
  return val;
}

static inline const char *
mom_string_cstr (const momstring_t *str)
{
  if (str && str != MOM_EMPTY)
    return str->cstr;
  return NULL;
}

#define mom_stringv_cstr(S) mom_stringv(mom_make_string_cstr((S)))
#define mom_stringv_sprintf(F,...) mom_stringv(mom_make_string_sprintf((F),__VA_ARGS__))
#define mom_stringv_output(V) mom_stringv(mout_output_string((V)))

static inline const char *
mom_value_cstr (const momvalue_t val)
{
  if (val.typnum == momty_string)
    {
      assert (val.vstr);
      return val.vstr->cstr;
    }
  return NULL;
}

// make a tuple from given items. NULL and MOM_EMPTY item pointers are skipped.
const momseq_t *mom_make_meta_tuple (momvalue_t metav, unsigned nbitems, ...);
#define mom_make_tuple(NbItems,...) mom_make_meta_tuple(MOM_NONEV, (NbItems), __VA_ARGS__)
const momseq_t *mom_make_sized_meta_tuple (momvalue_t metav, unsigned nbitems,
					   momitem_t *const *itmarr);
static inline const momseq_t *
mom_make_sized_tuple (unsigned nbitems, momitem_t *const *itmarr)
{
  return mom_make_sized_meta_tuple (MOM_NONEV, nbitems, itmarr);
};

static inline momvalue_t
mom_tuplev (const momseq_t *seq)
{
  momvalue_t val = MOM_NONEV;
  if (seq)
    {
      val.typnum = momty_tuple;
      val.vtuple = (momseq_t *) seq;
    }
  return val;
}

#define mom_tuplev_meta_tuple(Meta,Nb,...) mom_tuplev(mom_make_meta_tuple((Meta),(Nb),__VA_ARGS__))
#define mom_tuplev_tuple(Nb,...) mom_tuplev(mom_make_tuple((Nb),__VA_ARGS__))
#define mom_tuplev_sized_meta_tuple(Meta,Nb,Arr)  mom_tuplev(mom_make_sized_meta_tuple((Meta),(Nb),(Arr)))
#define mom_tuplev_sized_tuple(Nb,Arr)  mom_tuplev(mom_make_sized_tuple((Nb),(Arr)))

// make a set from given items. NULL and MOM_EMPTY item pointers are
// skipped.  Remaining items are sorted, and duplicates are ignored.
const momseq_t *mom_make_meta_set (momvalue_t metav, unsigned nbitems, ...);
#define mom_make_set(NbItems,...) mom_make_meta_set(MOM_NONEV, (NbItems), __VA_ARGS__)
const momseq_t *mom_make_sized_meta_set (momvalue_t metav, unsigned nbitems,
					 const momitem_t **itmarr);
static inline const momseq_t *
mom_make_sized_set (unsigned nbitems, const momitem_t **itmarr)
{
  return mom_make_sized_meta_set (MOM_NONEV, nbitems, itmarr);
};

static inline momvalue_t
// this is unsafe, e.g. if seq is not sorted
mom_unsafe_setv (const momseq_t *seq)
{
  momvalue_t val = MOM_NONEV;
  if (seq)
    {
      val.typnum = momty_set;
      val.vset = (momseq_t *) seq;
    }
  return val;
}

#define mom_setv_meta_new(Meta,Nb,...) mom_unsafe_setv(mom_make_meta_set((Meta),(Nb),__VA_ARGS__))
#define mom_setv_new(Nb,...) mom_unsafe_setv(mom_make_set((Nb),__VA_ARGS__))
#define mom_setv_sized_meta(Meta,Nb,ItmArr) mom_unsafe_setv(mom_make_sized_meta_set((Meta),(Nb),(ItmArr)))
#define mom_setv_sized(Nb,ItmArr) mom_unsafe_setv(mom_make_sized_set((Nb),(ItmArr)))

bool mom_setv_contains (const momvalue_t vset, const momitem_t *itm);



// make a node from given values.
const momnode_t *mom_make_meta_node (momvalue_t metav, momitem_t *connitm,
				     unsigned nbsons, ...);
#define mom_make_node(ConnItm,NbSons,...) mom_make_meta_node(MOM_NONEV, (ConnItm), (NbSons), __VA_ARGS__)
const momnode_t *mom_make_sized_meta_node (momvalue_t metav,
					   momitem_t *connitm,
					   unsigned nbsons,
					   momvalue_t *sonarr);
static inline const momnode_t *
mom_make_sized_node (momitem_t *connitm, unsigned nbsons, momvalue_t *sonarr)
{
  return mom_make_sized_meta_node (MOM_NONEV, connitm, nbsons, sonarr);
}

static inline momvalue_t
mom_nodev (const momnode_t *nod)
{
  momvalue_t val = MOM_NONEV;
  if (nod && nod != MOM_EMPTY)
    {
      val.typnum = momty_node;
      val.vnode = (momnode_t *) nod;
    }
  return val;
}

#define mom_nodev_meta_new(Meta,Conn,NbSons,...) \
 mom_nodev(mom_make_meta_node((Meta),(Conn),(NbSons),__VA_ARGS__))

#define mom_nodev_new(Conn,NbSons,...) \
 mom_nodev(mom_make_node((Conn),(NbSons),__VA_ARGS__))

#define mom_nodev_sized_meta(Meta,Conn,NbSons,ValArr) \
  mom_nodev(mom_make_sized_meta_node ((Meta),(Conn),(NbSons),(ValArr))

#define mom_nodev_sized(Conn,NbSons,ValArr) \
  mom_nodev(mom_make_sized_node ((Conn),(NbSons),(ValArr)))

static inline momitem_t *
mom_node_conn (const momnode_t *nod)
{
  if (!nod)
    return NULL;
  return nod->conn;
}

static inline unsigned
mom_node_arity (const momnode_t *nod)
{
  if (!nod)
    return 0;
  return nod->slen;
}

static inline momvalue_t
mom_node_meta (const momnode_t *nod)
{
  if (!nod)
    return MOM_NONEV;
  return nod->meta;
}

static inline momvalue_t
mom_node_nth (const momnode_t *nod, int rk)
{
  if (!nod)
    return MOM_NONEV;
  unsigned ln = nod->slen;
  if (rk < 0)
    rk += (int) ln;
  if (rk < (int) ln)
    return nod->arrsons[rk];
  return MOM_NONEV;
}

// find some existing item by its id or its name
momitem_t *mom_find_item (const char *str);

// find some existing named item by its name
momitem_t *mom_find_named_item (const char *name);

// get the set of named items with a common prefix
momvalue_t mom_set_named_items_of_prefix (const char *prefix);

// find or make a named item; if it is new, it is made transient
momitem_t *mom_make_named_item (const char *namstr);

momitem_t *mom_make_predefined_named_item (const char *namstr);
const momseq_t *mom_predefined_items_set (void);

momitem_t *mom_make_anonymous_item_by_id (const char *ids);

// make an anonymous transient item
momitem_t *mom_make_anonymous_item_salt (unsigned salt);
static inline momitem_t *
mom_make_anonymous_item_at (unsigned lin)
{
  static _Thread_local unsigned count;
  count++;
  return mom_make_anonymous_item_salt (count + lin);
}

// get the set of anonymous items with a common prefix; this is a slow
// operation since it has to scan every anonymous item!
momvalue_t mom_set_anonymous_items_of_prefix (const char *prefix);

enum mom_predefhash_en
{
  mompredh__none = 0,
#define MOM_HAS_PREDEFINED_NAMED(Nam,Hash) mom_predhnamed_##Nam = Hash,
#define MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) mom_predhanon_##Id = Hash,
#include "predef-monimelt.h"
};
#define MOM_HASH_PREDEFINED_NAMED(Nam) mom_predhnamed_##Nam
#define MOM_HASH_PREDEFINED_ANONYMOUS(Id)  mom_predhanon_##Id


/***
 weird macros to be able to make a switch on predefined like

   momitem_t*itm = something();
   switch (mom_item_hash(itm)) {
     case MOM_PREDEFINED_NAMED_CASE(association,itm,otherwiselab):
       do_something_for_association(itm);
       break;
     case MOM_PREDEFINED_NAMED_CASE(double,itm,otherwiselab):
       do_something_for_double(itm);
       break;
     otherwiselab;
     default:
       do_something_otherwise(itm);
   }
***/

#define MOM_PREDEFINED_NAMED_CASE_AT(Nam,Itm,Def,Lin)	\
  (momhash_t)MOM_HASH_PREDEFINED_NAMED(Nam):		\
  if ((Itm) != MOM_PREDEFINED_NAMED(Nam)) goto Def;	\
  else goto predef_named_##Nam##_l##Lin;		\
  predef_named_##Nam##_l##Lin

#define MOM_PREDEFINED_NAMED_CASE_BIS(Nam,Pit,Def,Lin) \
  MOM_PREDEFINED_NAMED_CASE_AT(Nam,Pit,Def,Lin)

#define MOM_PREDEFINED_NAMED_CASE(Nam,Pit,Def) \
  MOM_PREDEFINED_NAMED_CASE_BIS(Nam,Pit,Def,__LINE__)


#define MOM_PREDEFINED_ANONYMOUS_CASE_AT(Id,Itm,Def,Lin)	\
  (momhash_t)MOM_HASH_PREDEFINED_ANONYMOUS(Id):			\
  if ((Itm) != MOM_PREDEFINED_ANONYMOUS(Id)) goto Def;		\
  else goto predef_anon_##Id##_l##Lin;				\
  predef_anon_##Id##_l##Lin

#define MOM_PREDEFINED_ANONYMOUS_CASE_BIS(Id,Itm,Def,Lin) \
  MOM_PREDEFINED_ANONYMOUS_CASE_AT(Id,Itm,Def,Lin)

#define MOM_PREDEFINED_ANONYMOUS_CASE(Id,Itm,Def) \
  MOM_PREDEFINED_ANONYMOUS_CASE_BIS(Id,Pit,Def,__LINE__)




momitem_t *mom_predefined_item_of_hash (momhash_t h);
// mom_scan_dumped_item returns true for an item to be scanned (non
// null, non transient)
bool mom_scan_dumped_item (const momitem_t *itm);
void mom_scan_dumped_valueptr (const momvalue_t *pval);
static inline void
mom_scan_dumped_value (const momvalue_t val)
{
  mom_scan_dumped_valueptr (&val);
}

void mom_scan_dumped_module_item (const momitem_t *moditm);
void mom_output_gplv3_notice (FILE *out, const char *prefix,
			      const char *suffix, const char *filename);

bool mom_dumpable_valueptr (const momvalue_t *pval);
static inline bool
mom_dumpable_value (const momvalue_t val)
{
  return mom_dumpable_valueptr (&val);
}

bool mom_dumpable_item (const momitem_t *itm);
void mom_emit_dumped_newline (void);
void mom_emit_dumped_space (void);
void mom_emit_dump_indent (void);
void mom_emit_dump_outdent (void);
bool mom_emit_dumped_itemref (const momitem_t *itm);

void mom_emit_dumped_valueptr (const momvalue_t *pval);
static inline void
mom_emit_dumped_value (const momvalue_t val)
{
  mom_emit_dumped_valueptr (&val);
}

#define mom_make_anonymous_item() mom_make_anonymous_item_at(__LINE__)

// output an UTF-8 string à la C
void mom_output_utf8cstr_cencoded (FILE *fil, const char *str, int len);
// output an UTF-8 string à la HTML, is usebr is set, output <br/> for newlines.
void mom_output_utf8html_cencoded (FILE *fil, const char *str, int len,
				   bool usebr);

#define MOM_HAS_PREDEFINED_NAMED(Nam,Hash) extern momitem_t*mompi_##Nam;
#define MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) extern momitem_t*mompi_##Id;

//
// predef-monimelt.h is generated
#include "predef-monimelt.h"


#define MOM_PREDEFINED_NAMED(Nam) mompi_##Nam
#define MOM_PREDEFINED_ANONYMOUS(Id) mompi_##Id


extern void *mom_dynload_symbol (const char *name);
//
// apply-monimelt.h is generated
#include "apply-monimelt.h"

const momitem_t *mom_load_new_anonymous_item (bool global);

const momitem_t *mom_load_itemref_at (const char *fil, int lin);
#define mom_load_itemref() mom_load_itemref_at(__FILE__,__LINE__)


momvalue_t
mom_item_unsync_get_attribute (momitem_t *itm, momitem_t *itmat)
{
  if (MOM_UNLIKELY (!itm || itm == MOM_EMPTY))
    return MOM_NONEV;
  if (MOM_UNLIKELY (!itmat || itmat == MOM_EMPTY))
    return MOM_NONEV;
  if (MOM_UNLIKELY (itmat == MOM_PREDEFINED_NAMED (kind)))
    return mom_itemv ((momitem_t *) itm->itm_kind);
  if (MOM_UNLIKELY
      ((momitem_t *) itmat->itm_kind ==
       MOM_PREDEFINED_NAMED (magic_attribute)))
    {
      momvalue_t res = MOM_NONEV;
      momnode_t *getnod = (momnode_t *) itmat->itm_data1;
      assert (getnod != NULL);
      if (mom_applval_2itm_to_val (mom_nodev (getnod), itm, itmat, &res))
	return res;
    }
  if (itm->itm_attrs)
    {
      struct momentry_st *ent =
	mom_attributes_find_entry (itm->itm_attrs, itmat);
      if (ent)
	return ent->ent_val;
    }
  return MOM_NONEV;
}


bool mom_unsync_item_set_kind (momitem_t *itm, momitem_t *kinditm);

static inline bool
mom_item_unsync_put_attribute (momitem_t *itm, momitem_t *itmat,
			       const momvalue_t val)
{
  if (MOM_UNLIKELY (!itm || itm == MOM_EMPTY))
    return false;
  if (MOM_UNLIKELY (!itmat || itmat == MOM_EMPTY))
    return false;
  if (MOM_UNLIKELY (itmat == MOM_PREDEFINED_NAMED (kind)))
    {
      if (val.typnum != momty_item || !val.vitem)
	return false;
      return mom_unsync_item_set_kind (itm, (momitem_t *) val.vitem);
    }
  if (MOM_UNLIKELY
      ((momitem_t *) itmat->itm_kind ==
       MOM_PREDEFINED_NAMED (magic_attribute)))
    {
      momnode_t *putnod = (momnode_t *) itmat->itm_data2;
      assert (putnod != NULL);
      return mom_applval_2itm1val_to_void (mom_nodev (putnod), itm, itmat,
					   val);
    }
  //
  if (val.typnum == momty_null)
    {
      // erase the attribute
      itm->itm_attrs = mom_attributes_remove (itm->itm_attrs, itmat);
      return true;
    }
  else
    {
      // put the attribute
      itm->itm_attrs = mom_attributes_put (itm->itm_attrs, itmat, &val);
      return true;
    }

}



static inline momvalue_t
mom_unsync_item_get_nth_component (momitem_t *itm, int rk)
{
  momvalue_t vres = MOM_NONEV;
  if (itm && itm->itm_comps)
    vres = mom_components_nth (itm->itm_comps, rk);
  return vres;
}

static inline momvalue_t
mom_raw_item_get_indexed_component (momitem_t *itm, unsigned ix)
{
  assert (itm);
  struct momcomponents_st *comps = itm->itm_comps;
  assert (comps);
  assert (ix < comps->cp_cnt);
  return comps->cp_comps[ix];
}


static inline const momvalue_t *
mom_raw_item_get_indexed_component_ptr (momitem_t *itm, unsigned ix)
{
  assert (itm);
  struct momcomponents_st *comps = itm->itm_comps;
  assert (comps);
  assert (ix < comps->cp_cnt);
  return &comps->cp_comps[ix];
}


static inline unsigned
mom_unsync_item_components_count (momitem_t *itm)
{
  if (itm && itm->itm_comps)
    return mom_components_count (itm->itm_comps);
  return 0;
}

static inline void
mom_unsync_item_components_reserve (momitem_t *itm, unsigned siz)
{
  if (!itm || !siz)
    return;
  itm->itm_comps = mom_components_reserve (itm->itm_comps, siz);
}

static inline void
mom_unsync_item_put_nth_component (momitem_t *itm, int rk, momvalue_t val)
{
  mom_components_put_nth (itm->itm_comps, rk, val);
}

FILE *mom_unsync_webexitem_file (const momitem_t *wxitm);
int mom_unsync_webexitem_printf (momitem_t *wxitm, const char *fmt, ...)
  __attribute__ ((format (printf, 2, 3)));
int mom_unsync_webexitem_fputs (momitem_t *wxitm, const char *str);
// mom_unsync_webexitem_reply should have valid argument, otherwise it
// is aborting
void mom_unsync_webexitem_reply (momitem_t *wxitm, const char *mimetype,
				 int code);

static inline void
mom_unsync_webexitem_reply_str (momitem_t *wxitm,
				const momstring_t *mimetypestr, int code)
{
  mom_unsync_webexitem_reply (wxitm, mom_string_cstr (mimetypestr), code);
}

onion_request *mom_unsync_webexitem_request (momitem_t *wxitm);
onion_response *mom_unsync_webexitem_response (momitem_t *wxitm);

extern pthread_cond_t mom_agenda_changed_condvar;

void mom_wake_event_loop (void);

/// start a batch process (e.g. to compile the generated C code in
/// some newly generated module). The closure vclos would be called
/// after the process has ended, and is given the output string value
/// & the exit code integer of that process, or the faulty vprocnode
/// and some negative number. The vprocnode is a node of connective
/// `batch_process` and whose sons are strings, passed to execvp (so
/// the first son is the program name)
void mom_start_batch_process (momvalue_t vclos, momvalue_t vprocnode);

#define MOM_AGENDA_WAIT_SEC 2
/****************************************************************
  Informal descriptions of kinds
  ==============================

magic_attribute-s:

  itm_data1 is the node of a getting closure of  signature_2itm_to_val
     get-closure(container-item,attribute-item)

  itm_data2 is the node of a putting closure of signature_2itm1val_to_val
     put-closure(container-item,attribute-item,value)

association-s
  itm_data1 is pointer to a struct mom_attributes_st

hash_set-s
  itm_data1 is pointer to a struct mom_hashset_st

item_queue-s

  itm_data1 is a pointer to a struct  momqueueitems_st

 json-s
   itm_data1 is a pointer to json_t from <jansson.h>
 ***************************************************************/
#endif /*MONIMELT_INCLUDED_ */
