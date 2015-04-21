// file monimelt.h

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
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <execinfo.h>

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

// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];

#define MOM_MAX_WORKERS 10
#define MOM_MIN_WORKERS 2
int mom_nb_workers;
const char *mom_web_host;
const char *mom_user_data;
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
#define MOM_SHARED_MODULE_PREFIX "momg_"
// plugins path start with
#define MOM_PLUGIN_PREFIX "momplug_"

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
  Dbg(json)					\
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
struct momloader_st;		/* private to state.c */
struct momdumper_st;		/* private to state.c */

typedef struct momvalue_st momvalue_t;

#define MOM_GLOBAL_DATA_PATH "global.mom"
#define MOM_USER_DATA_PATH "user.mom"
#define MOM_PREDEFINED_PATH "predef-monimelt.h"

void mom_load_state (void);
void mom_dump_state (const char *prefix);
bool mom_token_load (struct momloader_st *ld, momvalue_t *pval);
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
extern void mom_plugin_init (const char *pluginarg, int *pargc, char ***pargv);	// the plugin initializer
/// they may also define a function to be called after load
extern void momplugin_after_load (void);

// every monimelt value starts with a non-zero unsigned typenum
typedef int8_t momtypenum_t;
typedef enum momvaltype_en
{
  momty_delim = -3,
  momty_double = -2,
  momty_int = -1,
  momty_null = 0,
  momty_item,
  momty_node,
  momty_tuple,
  momty_set,
  momty_string,
} momvaltype_t;

typedef enum momspace_en
{
  momspa_none,
  momspa_transient,
  momspa_user,
  momspa_global,
  momspa_predefined
} momspace_t;

// every hashcode is a non-zero 32 bits unsigned
typedef uint32_t momhash_t;

typedef struct momstring_st momstring_t;
typedef struct momdelim_st momdelim_t;
typedef struct momitem_st momitem_t;
typedef struct momnode_st momnode_t;
typedef struct momseq_st momseq_t;
typedef struct momvalue_st momvalue_t;

struct momdelim_st
{
  char delim[4];
};

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
    momnode_t *vnode;
    momseq_t *vsequ;
    momseq_t *vset;
    momseq_t *vtuple;
  };
};
#define MOM_NONEV ((momvalue_t){momty_null,false,{NULL}})

momhash_t mom_valueptr_hash (momvalue_t *pval);

static inline momhash_t
mom_value_hash (momvalue_t val)
{
  return mom_valueptr_hash (&val);
}

bool mom_value_equal (momvalue_t v1, momvalue_t v2);
int mom_value_compare (momvalue_t v1, momvalue_t v2);

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
void mom_attributes_scan_dump (struct momattributes_st *attrs,
			       struct momdumper_st *du);



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

void
mom_components_put_nth (struct momcomponents_st *csq, int rk,
			const momvalue_t val);

void
mom_components_scan_dump (struct momcomponents_st *csq,
			  struct momdumper_st *du);


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
						   const momitem_t **itmarr);
const momseq_t *mom_hashset_elements_set_meta (struct momhashset_st *hset,
					       momvalue_t metav);
static inline const momseq_t *
mom_hashset_elements_set (struct momhashset_st *hset)
{
  return mom_hashset_elements_set_meta (hset, MOM_NONEV);
};

void mom_hashset_scan_dump (struct momhashset_st *hset,
			    struct momdumper_st *du);

struct momqueuechunk_st;
struct momqueueitems_st
{
  unsigned long que_size;
  struct momqueuechunk_st *que_front;
  struct momqueuechunk_st *que_back;
};
#define MOM_QUEUECHUNK_LEN 6
struct momqueuechunk_st
{
  struct momqueuechunk_st *quech_next;
  struct momqueuechunk_st *quech_prev;
  const momitem_t *quech_items[MOM_QUEUECHUNK_LEN];
};

void mom_queue_push_back (struct momqueueitems_st *qu, const momitem_t *itm);

static inline const momitem_t *
mom_queue_peek_front (struct momqueueitems_st *qu)
{
  if (!qu)
    return NULL;
  struct momqueuechunk_st *fr = qu->que_front;
  if (!fr)
    return NULL;
  for (unsigned ix = 0; ix < MOM_QUEUECHUNK_LEN; ix++)
    {
      const momitem_t *itm = fr->quech_items[ix];
      if (itm && itm != MOM_EMPTY)
	return itm;
    };
  return NULL;
}

static inline unsigned long
mom_queue_size (struct momqueueitems_st *qu)
{
  if (!qu)
    return 0;
  return qu->que_size;
}

const momitem_t *mom_queue_pop_front (struct momqueueitems_st *qu);
const momseq_t *mom_queue_tuple (struct momqueueitems_st *qu,
				 momvalue_t metav);

void mom_queue_scan_dump (struct momqueueitems_st *qu,
			  struct momdumper_st *du);

struct momitem_st
{
  pthread_mutex_t itm_mtx;
  bool itm_anonymous;
  uint8_t itm_space;
  union
  {
    const momstring_t *itm_str;
    const momstring_t *itm_id;	/* when itm_anonymous */
    const momstring_t *itm_name;	/* when !itm_anonymous */
  };
  struct momattributes_st *itm_attrs;
  struct momcomponents_st *itm_comps;
  const momstring_t *itm_comment;
  momitem_t *itm_kind;
  void *itm_data1;
  void *itm_data2;
};

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
  return strcmp (itm1->itm_str->cstr, itm2->itm_str->cstr);
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

const momstring_t *mom_make_string (const char *str);
const momstring_t *mom_string_sprintf (const char *fmt, ...)
  __attribute__ ((format (printf, 1, 2)));

// make a tuple from given items. NULL and MOM_EMPTY item pointers are skipped.
const momseq_t *mom_make_meta_tuple (momvalue_t metav, unsigned nbitems, ...);
#define mom_make_tuple(NbItems,...) mom_make_meta_tuple(MOM_NONEV, (NbItems), __VA_ARGS__)
const momseq_t *mom_make_sized_meta_tuple (momvalue_t metav, unsigned nbitems,
					   const momitem_t **itmarr);
static inline const momseq_t *
mom_make_sized_tuple (unsigned nbitems, const momitem_t **itmarr)
{
  return mom_make_sized_meta_tuple (MOM_NONEV, nbitems, itmarr);
};

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

// find some existing item by its id or its name
momitem_t *mom_find_item (const char *str);


momitem_t *mom_make_named_item (const char *namstr);

momitem_t *mom_make_predefined_named_item (const char *namstr);
const momseq_t *mom_predefined_items_set (void);

momitem_t *mom_make_anonymous_item_by_id (const char *ids);

momitem_t *mom_make_anonymous_item_salt (unsigned salt);
static inline momitem_t *
mom_make_anonymous_item_at (unsigned lin)
{
  static _Thread_local unsigned count;
  count++;
  return mom_make_anonymous_item_salt (count + lin);
}

// mom_scan_dumped_item returns true for an item to be scanned (non
// null, non transient)
bool mom_scan_dumped_item (struct momdumper_st * du, const momitem_t *itm);
void mom_scan_dumped_value (struct momdumper_st *du, const momvalue_t val);
void mom_scan_dumped_module_item (struct momdumper_st *du,
				  const momitem_t *moditm);
void mom_output_gplv3_notice (FILE *out, const char *prefix,
			      const char *suffix, const char *filename);

bool mom_dumpable_value (struct momdumper_st *du, const momvalue_t val);
bool mom_dumpable_item (struct momdumper_st *du, const momitem_t *itm);
void mom_emit_dumped_newline (struct momdumper_st *du);
void mom_emit_dumped_space (struct momdumper_st *du);
void mom_emit_dump_indent (struct momdumper_st *du);
void mom_emit_dump_outdent (struct momdumper_st *du);
bool mom_emit_dumped_itemref (struct momdumper_st *du, const momitem_t *itm);
void mom_emit_dumped_value (struct momdumper_st *du, const momvalue_t val);
#define mom_make_anonymous_item() mom_make_anonymous_item_at(__LINE__)

void mom_output_utf8cstr_cencoded (FILE *fil, const char *str, int len);

#define MOM_HAS_PREDEFINED_NAMED(Nam,Hash) extern momitem_t*mompi_##Nam;
#define MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) extern momitem_t*mompi_##Id;
//
#include "predef-monimelt.h"


#define MOM_PREDEFINED_NAMED(Nam) mompi_##Nam
#define MOM_PREDEFINED_ANONYMOUS(Id) mompi_##Id


#endif /*MONIMELT_INCLUDED_ */
