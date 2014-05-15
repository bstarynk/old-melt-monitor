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

#define MOM_EXPLICIT_GC_THREAD 1
#warning enabling MOM_EXPLICIT_GC_THREAD


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
#include <uuid/uuid.h>
#include <gc/gc.h>
#include <glib.h>
// Gmime from http://spruce.sourceforge.net/gmime/
#include <gmime/gmime.h>
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
// curl from http://curl.haxx.se/libcurl is an HTTP client library
#include <curl/curl.h>


// in generated _timestamp.c
extern const char monimelt_timestamp[];
extern const char monimelt_lastgitcommit[];

#if MOM_EXPLICIT_GC_THREAD
#define MOMGC_REGISTER_MY_THREAD(Sb) GC_register_my_thread(Sb)
#define MOMGC_UNREGISTER_MY_THREAD() GC_unregister_my_thread()
#define MOMGC_CALL_WITH_STACK_BASE(Rout,Base) GC_call_with_stack_base(Rout,Base)
#else
#define MOMGC_REGISTER_MY_THREAD(Sb) do{if (0) (void)(Sb);}while(0)
#define MOMGC_UNREGISTER_MY_THREAD() do{}while(0)
#define MOMGC_CALL_WITH_STACK_BASE(Rout,Base) Rout(NULL,(Base))
#endif

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

// empty placeholder in hashes
#define MOM_EMPTY ((void*)(-1L))

// reasonable path length
#define MOM_PATH_LEN 256

// most versions of Boehm garbage collector (see
// http://hboehm.info/gc/ for more) don't have it, so define::
#ifndef GC_CALLOC
#define MOM_NEED_GC_CALLOC 1
extern void *GC_calloc (size_t nbelem, size_t elsiz);
#define GC_CALLOC(NbElem,ElSiz) GC_calloc(NbElem,ElSiz)
#endif /*GC_CALLOC */

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


enum momvaltype_en
{
  momty_null = 0,
  momty_int,
  momty_float,
  momty_string,
  momty_jsonarray,
  momty_jsonobject,
  momty_set,
  momty_tuple,
  momty_node,
  momty_closure,
  ////
  momty__itemlowtype,		//// types below are for items
  momty_jsonitem,
  momty_booleanitem,
  momty_routineitem,
  momty_taskletitem,
  momty_vectoritem,
  momty_associtem,
  momty_boxitem,
  momty_queueitem,
  momty_bufferitem,
  momty_dictionnaryitem,
  momty_webrequestitem,
  momty_processitem,
  /////
  momty__last = 1000
};


#define UUID_PARSED_LEN 40
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
typedef struct momnode_st momclosure_t;
typedef struct momseqitem_st momseqitem_t;
typedef struct momseqitem_st momset_t;
typedef struct momseqitem_st momitemtuple_t;
typedef struct momanyitem_st mom_anyitem_t;
typedef struct momjsonitem_st momit_json_name_t;
typedef struct mombooleanitem_st momit_boolean_t;
typedef struct momroutineitem_st momit_routine_t;
typedef struct momtaskletitem_st momit_tasklet_t;
typedef struct momvectoritem_st momit_vector_t;
typedef struct momassocitem_st momit_assoc_t;
typedef struct momboxitem_st momit_box_t;
typedef struct momqueueitem_st momit_queue_t;
typedef struct mombufferitem_st momit_buffer_t;
typedef struct momdictionnaryitem_st momit_dictionnary_t;
typedef struct momwebrequestitem_st momit_webrequest_t;
typedef struct momprocessitem_st momit_process_t;
pthread_mutexattr_t mom_normal_mutex_attr;
pthread_mutexattr_t mom_recursive_mutex_attr;
void *mom_prog_handle;

// generated modules start with:
#define MOM_SHARED_MODULE_PREFIX "momg_"
void mom_register_dumped_module (const char *modname);

// below TINY_MAX we try to allocate on stack temporary vectors
#define TINY_MAX 8


union momvalueptr_un
{
  void *ptr;
  momtynum_t *ptype;
  const momint_t *pint;
  const momfloat_t *pfloat;
  const momstring_t *pstring;
  mom_anyitem_t *panyitem;
  const struct momjsonarray_st *pjsonarr;
  const struct momjsonobject_st *pjsonobj;
  const struct momnode_st *pnode;
  const momclosure_t *pclosure;
  const struct momseqitem_st *pseqitm;
  const struct momseqitem_st *pset;
  const struct momseqitem_st *ptuple;
  struct momjsonitem_st *pjsonitem;
  struct mombooleanitem_st *pbooleanitem;
  struct momroutineitem_st *proutitem;
  struct momtaskletitem_st *ptaskitem;
  struct momvectoritem_st *pvectitem;
  struct momassocitem_st *passocitem;
  struct momqueueitem_st *pqueueitem;
  struct momboxitem_st *pboxitem;
  struct mombufferitem_st *pbufferitem;
  struct momdictionnaryitem_st *pdictionnaryitem;
  struct momwebrequestitem_st *pwebrequestitem;
  struct momprocessitem_st *pprocessitem;
};

// for debugging:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)		\
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
mom_debug_at (enum mom_debug_en dbg, const char *fil, int lin,
	      const char *fmt, ...) __attribute__ ((format (printf, 4, 5)));

#define MOM_DEBUG_AT(Dbg,Fil,Lin,Fmt,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))				\
      mom_debug_at (momdbg_##Dbg,Fil,Lin,Fmt,		\
		   ##__VA_ARGS__);			\
  } while(0)

#define MOM_DEBUG_AT_BIS(Dbg,Fil,Lin,Fmt,...)	\
  MOM_DEBUG_AT(Dbg,Fil,Lin,Fmt,			\
		    ##__VA_ARGS__)

#define MOM_DEBUG(Dbg,Fmt,...)			\
  MOM_DEBUG_AT_BIS(Dbg,__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)



void mom_debugprint_item (FILE * fil, const mom_anyitem_t * itm);
void mom_debugprint_value (FILE * fil, momval_t val);
void mom_dbgout_item (const mom_anyitem_t * itm);
void mom_dbgout_value (momval_t val);

void mom_dbg_item_at (enum mom_debug_en dbg, const char *file, int line,
		      const char *msg, const mom_anyitem_t * itm);
#define MOM_DBG_ITEM(Dbg,Msg,Itm)  do {			\
      if (MOM_IS_DEBUGGING(Dbg))			\
       mom_dbg_item_at (momdbg_##Dbg,			\
			 __FILE__,__LINE__,		\
			 (Msg),(Itm));} while(0)

void mom_dbg_value_at (enum mom_debug_en dbg, const char *fil, int lin,
		       const char *msg, const momval_t val);

#define MOM_DBG_VALUE(Dbg,Msg,Val) do {		\
      if (MOM_IS_DEBUGGING(Dbg))		\
      mom_dbg_value_at (momdbg_##Dbg,		\
		       __FILE__,__LINE__,	\
			(Msg),(Val));} while(0)

static inline enum momvaltype_en
mom_type (const momval_t v)
{
  if (!v.ptr)
    return momty_null;
  return (enum momvaltype_en) *v.ptype;
}

struct mom_dumper_st;
struct mom_loader_st;
void mom_dumper_initialize (struct mom_dumper_st *dmp);

// function to load and build an item from its building json and uid
typedef mom_anyitem_t *mom_item_loader_sig_t (struct mom_loader_st *ld,
					      momval_t json, uuid_t uid,
					      unsigned space);
// function to fill an item from its filling json
typedef void mom_item_filler_sig_t (struct mom_loader_st *ld,
				    mom_anyitem_t * itm, momval_t json);
// function to scan an item
typedef void mom_item_scan_sig_t (struct mom_dumper_st *dmp,
				  mom_anyitem_t * itm);

// optional function to mascarade the type of a dumped item
typedef const char *mom_item_mascarade_type_sig_t (struct mom_dumper_st *dmp,
						   mom_anyitem_t * itm);
// function to get the building json of an item
typedef momval_t mom_item_get_build_sig_t (struct mom_dumper_st *dmp,
					   mom_anyitem_t * itm);
// function to get the filling json of an item 
typedef momval_t mom_item_get_fill_sig_t (struct mom_dumper_st *dmp,
					  mom_anyitem_t * itm);
// function to destroy an item
typedef void mom_item_destroy_sig_t (mom_anyitem_t * itm);

// the item type FOO is described by momitype_FOO of following type:
#define ITEMTYPE_MAGIC 0x5237aed3	/* item type magic 1379380947 */
struct momitemtypedescr_st
{
  unsigned ityp_magic;		/* always ITEMTYPE_MAGIC */
  const char *ityp_name;
  mom_item_loader_sig_t *ityp_loader;
  mom_item_filler_sig_t *ityp_filler;
  mom_item_scan_sig_t *ityp_scan;
  mom_item_mascarade_type_sig_t *ityp_mascarade_dump;
  mom_item_get_build_sig_t *ityp_getbuild;
  mom_item_get_fill_sig_t *ityp_getfill;
  mom_item_destroy_sig_t *ityp_destroy;
};
const struct momitemtypedescr_st *mom_typedescr_array[momty__last];


#define SPACE_MAGIC 0x167d68fd	/* space magic 377317629 */


// fetch a GC_STRDUP-ed string to build an item of given uuid string
typedef char *mom_space_fetch_build_sig_t (unsigned spanum,
					   const char *uuidstr);
// fetch a GC_STRDUP-ed string to fill an item of given uuid string
typedef char *mom_space_fetch_fill_sig_t (unsigned spanum,
					  const char *uuidstr);
// store the build & fill GC_STRDUP-ed strings of a given uuid string
typedef void mom_space_store_build_fill_sig_t (struct mom_dumper_st *dmp,
					       mom_anyitem_t * itm,
					       const char *buildstr,
					       const char *fillstr);
// the space FOO is described by momspace_FOO of following type:
struct momspacedescr_st
{
  unsigned spa_magic;		/* always SPACE_MAGIC */
  const char *spa_name;
  void *spa_data;
  mom_space_fetch_build_sig_t *spa_fetch_build;
  mom_space_fetch_fill_sig_t *spa_fetch_fill;
  mom_space_store_build_fill_sig_t *spa_store_build_fill;
};
#define MOM_ROOT_SPACE_NAME "."
#define MOM_SPACE_NONE 0
#define MOM_SPACE_ROOT 1
#define MOM_FIRST_USER_SPACE 2
#define MOM_SPACE_MAX 64
struct momspacedescr_st *mom_spacedescr_array[MOM_SPACE_MAX];
const struct momstring_st *mom_spacename_array[MOM_SPACE_MAX];


#define MOM_NULLV ((union momvalueptr_un)((void*)0))
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


// for nodes & closures
struct momnode_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  const mom_anyitem_t *connitm;
  const momval_t sontab[];
};


// for sets and tuples of item. In sets, itemseq is sorted by
// increasing uuids. In tuples, some itemseq components may be nil.
struct momseqitem_st
{
  momtynum_t typnum;
  momusize_t slen;
  momhash_t hash;
  const mom_anyitem_t *itemseq[];
};
struct mom_attrentry_st
{
  mom_anyitem_t *aten_itm;
  momval_t aten_val;
};

struct mom_itemattributes_st
{
  momusize_t nbattr;
  momusize_t size;
  struct mom_attrentry_st itattrtab[];	/* of size entries */
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

struct mombooleanitem_st
{
  struct momanyitem_st ib_item;
  bool ib_bool;
};

// routine descriptor is read-only
#define ROUTINE_MAGIC 0x6b9c644d	/* routine magic 1805411405 */
#define MOM_ROUTINE_NAME_PREFIX "momrout_"
#define MOM_SYMBNAME_LEN 128
#define MOM_ROUTINE_NAME_FMT  MOM_ROUTINE_NAME_PREFIX "%s"
// the routine item FOO has descriptor momrout_FOO
// the routine returns a positive state 
enum routres_en
{
  routres_steady = 0,		/* don't change the current state or tasklet */
  routres_pop = -1,		/* pop the current frame */
};
typedef int momrout_sig_t (int state, momit_tasklet_t * tasklet,
			   momclosure_t * closure, momval_t * locvals,
			   intptr_t * locnums, double *locdbls);
struct momroutinedescr_st
{
  unsigned rout_magic;		/* always ROUTINE_MAGIC */
  unsigned rout_minclosize;	/* minimal closure size */
  unsigned rout_frame_nbval;	/* number of values in its frame */
  unsigned rout_frame_nbnum;	/* number of intptr_t numbers in its frame */
  unsigned rout_frame_nbdbl;	/* number of double numbers in its frame */
  const char *rout_name;	/* the name FOO */
  const momrout_sig_t *rout_code;	/* the code */
  const char *rout_timestamp;
};

struct momroutineitem_st
{
  struct momanyitem_st irt_item;
  struct momroutinedescr_st *irt_descr;
};

struct momframe_st
{
  uint32_t fr_state;		/* current state */
  uint32_t fr_intoff;		/* offset of integer locals in itk_scalars */
  uint32_t fr_dbloff;		/* offset of double locals in itk_scalars, should be after fr_intoff */
  uint32_t fr_valoff;		/* offset of value locals in its_values */
};

struct momtaskletitem_st
{
  struct momanyitem_st itk_item;	/* common part */
  intptr_t *itk_scalars;	/* space for scalar data, intptr_t or double-s */
  momval_t *itk_values;		/* space for value data */
  momclosure_t **itk_closures;	/* stack of closures */
  struct momframe_st *itk_frames;	/* stack of frames */
  pthread_t itk_thread;		/* the thread executing this, or else 0 */
  uint32_t itk_scalsize;	/* size of itk_scalars */
  uint32_t itk_scaltop;		/* top of stack offset on itk_scalars */
  uint32_t itk_valsize;		/* size of itk_values */
  uint32_t itk_valtop;		/* top of stack offset on its_values */
  uint32_t itk_frasize;		/* size of itk_closures & itk_frames */
  uint32_t itk_fratop;		/* top of stack offset on itk_closures & itk_frames */
};


enum mom_pushframedirective_en
{
  MOMPFR__END = 0,
  MOMPFR_STATE /*, int state  */ ,
  MOMPFR_VALUE /*, momval_t val */ ,
  MOMPFR_TWO_VALUES /*, momval_t val1, val2 */ ,
  MOMPFR_THREE_VALUES /*, momval_t val1, val2, val3 */ ,
  MOMPFR_FOUR_VALUES /*, momval_t val1, val2, val3, val4 */ ,
  MOMPFR_FIVE_VALUES /*, momval_t val1, val2, val3, val4, val5 */ ,
  MOMPFR_ARRAY_VALUES /* unsigned count, momval_t valarr[count] */ ,
  MOMPFR_NODE_VALUES /* momnode_st* node, -- to push the sons of a node */ ,
  MOMPFR_INT /*, intptr_t num */ ,
  MOMPFR_TWO_INTS /*, intptr_t num1, num2 */ ,
  MOMPFR_THREE_INTS /*, intptr_t num1, num2, num3 */ ,
  MOMPFR_FOUR_INTS /*, intptr_t num1, num2, num3, num4 */ ,
  MOMPFR_FIVE_INTS /*, intptr_t num1, num2, num3, num4, num5 */ ,
  MOMPFR_ARRAY_INTS /* unsigned count, intptr_t numarr[count] */ ,
  MOMPFR_DOUBLE /*, double d */ ,
  MOMPFR_TWO_DOUBLES /*, double d1, d2 */ ,
  MOMPFR_THREE_DOUBLES /*, double d1, d2, d3 */ ,
  MOMPFR_FOUR_DOUBLES /*, double d1, d2, d3, d4 */ ,
  MOMPFR_FIVE_DOUBLES /*, double d1, d2, d3, d4, d5 */ ,
  MOMPFR_ARRAY_DOUBLES /* unsigned count, double dblarr[count] */ ,
};
#define MOMPFR_END ((void*)MOMPFR__END)

momit_tasklet_t *mom_make_item_tasklet (unsigned space);
momit_tasklet_t *mom_make_item_tasklet_of_uuid (uuid_t uid, unsigned space);




struct momvectoritem_st
{
  struct momanyitem_st itv_item;	/* common part */
  unsigned itv_count;
  unsigned itv_size;
  momval_t *itv_arr;		/* of size itv_size, with itv_count elements */
};
// make an item vector with a reserved size
momit_vector_t *mom_make_item_vector (unsigned space, unsigned reserve);
momit_vector_t *mom_make_item_vector_of_uuid (uuid_t uid, unsigned space,
					      unsigned size);
unsigned mom_item_vector_count (const momval_t vec);
const momval_t mom_item_vector_nth (const momval_t vec, int rk);
void mom_item_vector_put_nth (const momval_t vec, int rk, const momval_t val);
void mom_item_vector_resize (momval_t vec, unsigned newcount);
void mom_item_vector_reserve (momval_t vec, unsigned more);
void mom_item_vector_append1 (momval_t vec, momval_t val);
void mom_item_vector_append_values (momval_t vec, unsigned nbval, ...);
void mom_item_vector_append_til_nil (momval_t vec, ...)
  __attribute__ ((sentinel));
void mom_item_vector_append_count (momval_t vec, unsigned nbval,
				   momval_t * arr);
const momset_t *mom_make_set_from_item_vector (momval_t vec);
const momitemtuple_t *mom_make_tuple_from_item_vector (momval_t vec);
const momnode_t *mom_make_node_from_item_vector (momval_t conn, momval_t vec);
const momclosure_t *mom_make_closure_from_item_vector (momval_t conn,
						       momval_t vec);

// union and intersection of two sets
momval_t mom_make_set_union (momval_t s1, momval_t s2);
momval_t mom_make_set_intersection (momval_t s1, momval_t s2);

/// in set S1 remove the items from set, tuple V2 or remove the item
/// V2 if it is an item...
momval_t mom_make_set_without (momval_t s1, momval_t v2);

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

static inline mom_anyitem_t *
mom_set_nth_item (momval_t setv, int rk)
{
  if (!setv.ptr || *setv.ptype != momty_set)
    return NULL;
  unsigned slen = setv.pset->slen;
  if (rk < 0)
    rk += (int) slen;
  if (rk >= 0 && rk < (int) slen)
    return (mom_anyitem_t *) (setv.pset->itemseq[rk]);
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

static inline mom_anyitem_t *
mom_tuple_nth_item (momval_t tupv, int rk)
{
  if (!tupv.ptr || *tupv.ptype != momty_tuple)
    return 0;
  unsigned slen = tupv.ptuple->slen;
  if (rk < 0)
    rk += (int) slen;
  if (rk >= 0 && rk < (int) slen)
    return (mom_anyitem_t *) (tupv.ptuple->itemseq[rk]);
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
  return seqv.pseqitm->slen;
}

static inline mom_anyitem_t *
mom_seqitem_nth_item (momval_t seqv, int rk)
{
  if (!seqv.ptr || (*seqv.ptype != momty_tuple && *seqv.ptype != momty_set))
    return 0;
  unsigned slen = seqv.pseqitm->slen;
  if (rk < 0)
    rk += (int) slen;
  if (rk >= 0 && rk < (int) slen)
    return (mom_anyitem_t *) (seqv.pseqitm->itemseq[rk]);
  return NULL;
}


static inline unsigned
mom_node_arity (momval_t nodv)
{
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return 0;
  return nodv.pnode->slen;
}

static inline unsigned
mom_closure_arity (momval_t clov)
{
  if (!clov.ptr || *clov.ptype != momty_closure)
    return 0;
  return clov.pclosure->slen;
}

static inline unsigned
mom_arity_of_node_or_closure (momval_t clonodv)
{
  if (!clonodv.ptr
      || (*clonodv.ptype != momty_closure && *clonodv.ptype != momty_node))
    return 0;
  return clonodv.pclosure->slen;
}

static inline bool
mom_is_node_or_closure (momval_t clonodv)
{
  if (!clonodv.ptr
      || (*clonodv.ptype != momty_closure && *clonodv.ptype != momty_node))
    return false;
  return true;
}

static inline momval_t
mom_node_nth (momval_t nodv, int rk)
{
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return MOM_NULLV;
  unsigned l = nodv.pnode->slen;
  if (rk < 0)
    rk += (int) l;
  if (rk >= 0 && rk < l)
    return nodv.pnode->sontab[rk];
  return MOM_NULLV;
}

static inline const mom_anyitem_t *
mom_node_conn (momval_t nodv)
{
  if (!nodv.ptr || *nodv.ptype != momty_node)
    return NULL;
  return nodv.pnode->connitm;
}

static inline const mom_anyitem_t *
mom_closure_conn (momval_t clov)
{
  if (!clov.ptr || *clov.ptype != momty_node)
    return NULL;
  return clov.pclosure->connitm;
}

static inline const mom_anyitem_t *
mom_conn_of_node_or_closure (momval_t clonodv)
{
  if (!clonodv.ptr
      || (*clonodv.ptype != momty_closure && *clonodv.ptype != momty_node))
    return NULL;
  return clonodv.pclosure->connitm;
}

static inline momval_t
mom_closure_nth (momval_t clov, int rk)
{
  if (!clov.ptr || *clov.ptype != momty_closure)
    return MOM_NULLV;
  unsigned l = clov.pclosure->slen;
  if (rk < 0)
    rk += (int) l;
  if (rk >= 0 && rk < l)
    return clov.pclosure->sontab[rk];
  return MOM_NULLV;
}

static inline momval_t
mom_son_nth_of_node_or_closure (momval_t clonodv, int rk)
{
  if (!clonodv.ptr
      || (*clonodv.ptype != momty_closure && *clonodv.ptype != momty_node))
    return MOM_NULLV;
  unsigned l = clonodv.pclosure->slen;
  if (rk < 0)
    rk += (int) l;
  if (rk >= 0 && rk < l)
    return clonodv.pclosure->sontab[rk];
  return MOM_NULLV;
}

/// unparse the uuid of an item
static inline const char *
mom_unparse_item_uuid (const mom_anyitem_t * itm, char buf[UUID_PARSED_LEN])
{
  if (!buf)
    return NULL;
  memset (buf, 0, UUID_PARSED_LEN);
  if (!itm)
    return buf;
  uuid_unparse (itm->i_uuid, buf);
  return buf;
}

// unparse the uuid of an item, using _ insteead of -

static inline const char *
mom_underscore_item_uuid (const mom_anyitem_t * itm,
			  char buf[UUID_PARSED_LEN])
{
  if (!buf)
    return NULL;
  memset (buf, 0, UUID_PARSED_LEN);
  if (!itm)
    return buf;
  uuid_unparse (itm->i_uuid, buf);
  for (unsigned ix = 0; ix < UUID_PARSED_LEN && buf[ix]; ix++)
    if (buf[ix] == '-')
      buf[ix] = '_';
  return buf;
}

///// assoc items
struct momassocitem_st
{
  struct momanyitem_st ita_item;	/* common part */
  unsigned ita_count;
  unsigned ita_size;
  struct mom_attrentry_st *ita_htab;	/* hash table of entries */
};


momit_assoc_t *mom_make_item_assoc (unsigned space);
momit_assoc_t *mom_make_item_assoc_of_uuid (uuid_t uid, unsigned space);
unsigned mom_item_assoc_count (const momval_t asso);
momval_t mom_item_assoc_get1 (const momval_t asso, const momval_t attr);
void mom_item_assoc_put1 (momval_t asso, const momval_t attr,
			  const momval_t val);
// mom_item_assoc_get_several(assoc, itat1, &val1, itat2, &val2, ... NULL)
// return the number of found attributes
int mom_item_assoc_get_several (momval_t asso, ...)
  __attribute__ ((sentinel));
// mom_item_assoc_put_several(assoc, itat1, val1, itat2, val2, .... NULL);
// if no items are given this may reorganize the assoc
void mom_item_assoc_put_several (momval_t asso, ...)
  __attribute__ ((sentinel));

/// iteration on assoc items:
/**
 int hint=0;
 for (mom_anyitem_t*attr = mom_item_assoc_first_attr(asso, &hint);
      attr != NULL;
      att = mom_item_assoc_next_attr(asso, attr, &hint)) {...}
**/
mom_anyitem_t *mom_item_assoc_first_attr (momval_t assoc, int *phint);
mom_anyitem_t *mom_item_assoc_next_attr (momval_t assoc, mom_anyitem_t * attr,
					 int *phint);


//////////////// box item
struct momboxitem_st
{
  struct momanyitem_st ita_item;	/* common part */
  momval_t itb_boxv;
};

momit_box_t *mom_make_item_box (unsigned space);
momit_box_t *mom_make_item_box_of_uuid (uuid_t uid, unsigned space);
#define mom_create__box(Name,Uid) \
  mom_make_item_box_of_uuid(Uid,MOM_SPACE_ROOT)

// get the boxed value
momval_t mom_item_box_get (momval_t boxv);
// put a new boxed value and return the old one
momval_t mom_item_box_put (momval_t boxv, momval_t val);

//////////////// queue item - contain a queue with items
struct momqueueitem_st
{
  struct momanyitem_st itq_item;
  unsigned itq_len;
  struct mom_itqueue_st *itq_first;
  struct mom_itqueue_st *itq_last;
};
momit_queue_t *mom_make_item_queue (unsigned space);
momit_queue_t *mom_make_item_queue_of_uuid (uuid_t uid, unsigned space);

#define mom_create__queue(Name,Uid) \
  mom_make_item_queue_of_uuid(Uid,MOM_SPACE_ROOT)
void mom_item_queue_push_back (momval_t quev, momval_t itmv);
void mom_item_queue_push_front (momval_t quev, momval_t itmv);
void mom_item_queue_push_many_back (momval_t quev, ...)
  __attribute__ ((sentinel));
void mom_item_queue_push_many_front (momval_t quev, ...)
  __attribute__ ((sentinel));
void mom_item_queue_push_counted_back (momval_t, unsigned count,
				       mom_anyitem_t * arr[]);
void mom_item_queue_push_counted_front (momval_t, unsigned count,
					mom_anyitem_t * arr[]);
unsigned mom_item_queue_length (momval_t quev);
mom_anyitem_t *mom_item_queue_first (momval_t quev);
mom_anyitem_t *mom_item_queue_last (momval_t quev);
mom_anyitem_t *mom_item_queue_pop_front (momval_t quev);
momval_t mom_item_queue_tuple (momval_t quev);


//////////////// buffer item
struct mombufferitem_st
{
  struct momanyitem_st itu_item;	/* common part */
  char *itu_buf;
  unsigned itu_size;		/* allocated size */
  unsigned itu_begin;		/* offset of beginning */
  unsigned itu_end;		/* offset of end */
};

momit_buffer_t *mom_make_item_buffer (unsigned space);
momit_buffer_t *mom_make_item_buffer_of_uuid (uuid_t uid, unsigned space);
// get a duplicate of the buffer string with its length
const char *mom_item_buffer_cstr (momval_t bufv, unsigned *plen);
// output the content of the buffer into a FILE
int mom_item_buffer_output_content_to_file (momval_t bufv, FILE * file);
// get the content of the buffer as a string value
momval_t mom_item_buffer_string_value (momval_t bufv);
// get the length of the buffer
unsigned mom_item_buffer_length (momval_t bufv);
// peek the character at given offset or else EOF ie -1
int mom_item_buffer_peek (momval_t bufv, int off);
// reserve space for at least gap characters
void mom_item_buffer_reserve (momval_t bufv, unsigned gap);
// clear the buffer
void mom_item_buffer_clear (momval_t bufv);
// put a string at end of buffer
void mom_item_buffer_puts (momval_t bufv, const char *str);
// print into a buffer
void mom_item_buffer_printf (momval_t bufv, const char *fmt, ...)
  __attribute__ ((format (printf, 2, 3)));
int mom_item_buffer__scanf_pos (momval_t bufv, int *pos, const char *fmt, ...)
  __attribute__ ((format (scanf, 3, 4)));
#define mom_item_buffer__scanf_at(Lin,Bufv,Fmt,...) ({int pos_##Lin=0;	\
      int res_##Lin							\
	= mom_item_buffer__scanf_pos((Bufv), &pos_##Lin, Fmt "%n",	\
				     ##__VA_ARGS__, &pos_##Lin);       	\
      res_##Lin;})
#define mom_item_buffer__scanf_at_lin(Lin,Bufv,Fmt,...) \
  mom_item_buffer__scanf_at(Lin,Bufv,Fmt,##__VA_ARGS__)

// usable only with a literal Fmt
#define mom_item_buffer_scanf(Bufv,Fmt,...) \
  mom_item_buffer__scanf_at_lin(__LINE__,Bufv,Fmt,##__VA_ARGS__)


//////////////////////////////// dictionnary items
struct mom_name_value_entry_st	// for dictionnary 
{
  const momstring_t *nme_str;
  const momval_t nme_val;
};
struct momdictionnaryitem_st
{
  struct momanyitem_st idi_item;	/* common part */
  unsigned idi_count;
  unsigned idi_size;
  struct mom_name_value_entry_st *idi_dictab;
};

momit_dictionnary_t *mom_make_item_dictionnary (unsigned space);
momit_dictionnary_t *mom_make_item_dictionnary_of_uuid (uuid_t uid,
							unsigned space);
#define mom_create__dictionnary(Name,Uid) \
  mom_make_item_dictionnary_of_uuid(Uid,MOM_SPACE_ROOT)
void mom_item_dictionnary_reserve (momval_t dictv, unsigned more);
void mom_item_dictionnary_put (momval_t dictv, momval_t namev, momval_t valv);
static inline const momstring_t *mom_make_string (const char *str);
static inline void
mom_item_dictionnary_put_cstr (momval_t dictv, const char *name,
			       momval_t valv)
{
  if (dictv.ptr && *dictv.ptype == momty_dictionnaryitem && name && name[0])
    mom_item_dictionnary_put (dictv, (momval_t) mom_make_string (name), valv);
}

momval_t mom_item_dictionnary_get (momval_t dictv, momval_t namev);
momval_t mom_item_dictionnary_get_cstr (momval_t dictv, const char *namestr);
unsigned mom_item_dictionnary_count (momval_t dictv);
// make a node with the sorted names
momval_t mom_item_dictionnary_sorted_name_node (momval_t dictv,
						momval_t connv);



//////////////// web request items, only allocated in web-onion.c
///// mascaraded at dump time into a box item
struct momwebrequestitem_st
{
  struct momanyitem_st iweb_item;	/* common part */
  onion_request *iweb_request;
  onion_response *iweb_response;
  unsigned long iweb_webnum;
  double iweb_time;		/* real time of request */
  pthread_cond_t iweb_cond;
  mom_anyitem_t *iweb_methoditm;
  momval_t iweb_postjsob;	/* JSON object for POST arguments */
  momval_t iweb_queryjsob;	/* JSON object for query arguments */
  momval_t iweb_path;		/* path string */
  // the reply buffer its mimetype, size and current length
  char *iweb_replybuf;
  const char *iweb_replymime;
  unsigned iweb_replysize;
  unsigned iweb_replylength;
  int iweb_replycode;
};

enum mom_webreplydirective_en
{
  MOMWEB__END = 0,
  MOMWEB_LIT_STRING /*, const char* literalstring; for literal  string */ ,
  MOMWEB_HTML_STRING
    /*, const char* htmlstring; for HTML encoded literal string  */ ,
  MOMWEB_JS_STRING		/*, const char*jstring; for JSON encoded literal string */
    ,
  MOMWEB_VALUE
    /*, momval_t val; for boxed numbers, JSON values, named items, buffer items  */
    ,
  MOMWEB_JSON_VALUE /*, momval_t jsonval; for JSON values */ ,
  MOMWEB_HTML_VALUE
    /*, momval_t val; for boxed numbers, JSON values, named items HTML encoded */
    ,
  MOMWEB_DEC_INT /*, int num; for numbers in decimal */ ,
  MOMWEB_DEC_UNSIGNED /*, unsigned num */ ,
  MOMWEB_HEX_INT /*, int num; for numbers in hexadecimal  */ ,
  MOMWEB_DEC_LONG /*, long num; for numbers in decimal */ ,
  MOMWEB_HEX_LONG /*, long num; for numbers in hexadecimal  */ ,
  MOMWEB_DEC_INT64 /*, int64_t num; for numbers in decimal */ ,
  MOMWEB_HEX_INT64 /*, int64_t num; for numbers in hexadecimal  */ ,
  MOMWEB_DOUBLE /*, double x; for double with %g */ ,
  MOMWEB_FIXED_DOUBLE /*, double x; for double with %.15f */ ,
  MOMWEB_FIX2DIG_DOUBLE /*, double x; for double with %.2f */ ,
  MOMWEB_RESERVE /*, unsigned more; to reserve space in the buffer */ ,
  MOMWEB_CLEAR_BUFFER /*; to clear the buffer and restart  */ ,
  MOMWEB_SET_MIME /* const char*mimetype; e.g. "text/html" */ ,
  MOMWEB_STDIO_FILE_CONTENT /* FILE* file  */ ,
  MOMWEB_STDIO_FILE_HTML_CONTENT /* FILE* file; HTML encoded  */ ,
  MOMWEB_REPLY_CODE /* int httpcode; will trigger the send */ ,
  MOMWEB__LAST
};
#define MOMWEB_END ((void*)MOMWEB__END)

void mom_item_webrequest_add (momval_t val, ...) __attribute__ ((sentinel));

// return the string value for a POST argument or else null
momval_t mom_item_webrequest_post_arg (momval_t val, const char *argname);
// return the JSON object for POST arguments
momval_t mom_item_webrequest_jsob_post (momval_t);
// return the string value for a query argument or else null
momval_t mom_item_webrequest_query_arg (momval_t val, const char *argname);
// return the JSON object for query arguments
momval_t mom_item_webrequest_jsob_query (momval_t);
// return the method item
momval_t mom_item_webrequest_method (momval_t val);

static inline unsigned long
mom_item_webrequest_webnum (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_webrequestitem)
    return 0;
  return val.pwebrequestitem->iweb_webnum;
}


///////////////////////////////////////////// process items
///// mascaraded at dump time into a box item
struct momprocessitem_st
{
  struct momanyitem_st iproc_item;	/* common part */
  const momstring_t *iproc_progname;
  const momstring_t **iproc_argv;	/* of iproc_argc size */
  const momclosure_t *iproc_closure;	/* the closure handing process outcome */
  char *iproc_outbuf;		// buffer for output
  unsigned iproc_outsize;	/* its size */
  unsigned iproc_outpos;	/* last written position */

  unsigned iproc_jobnum;	/* the internal job number */
  unsigned iproc_argcount;
  int iproc_outfd;		/* the process output, to be read by
				   the monitor */
  pid_t iproc_pid;
};

// a process item is created with its program and arguments
// it has to be started later with mom_item_process_start

// mom_make_item_process_argvals(progstrv, arg1strv, ...., NULL)
momit_process_t *mom_make_item_process_argvals (momval_t progstr, ...)
  __attribute__ ((sentinel));
momit_process_t *mom_make_item_process_from_array (momval_t progstr,
						   unsigned argc,
						   momval_t * argv);
momit_process_t *mom_make_item_process_from_node (momval_t progstr,
						  momval_t node);

// queue the process to be started, giving the closure which will be
// invoked at completion
void mom_item_process_start (momval_t procv, momval_t clov);

////////////////////////////////
/////// tasklets
void mom_tasklet_push_frame (momval_t tsk, momval_t clo,
			     enum mom_pushframedirective_en, ...)
  __attribute__ ((sentinel));
void mom_tasklet_replace_top_frame (momval_t tsk, momval_t clo,
				    enum mom_pushframedirective_en, ...)
  __attribute__ ((sentinel));
momval_t mom_run_closure (momval_t clo,
			  enum mom_pushframedirective_en, ...)
  __attribute__ ((sentinel));
// reserve or shrink to fit
void mom_tasklet_reserve (momval_t tsk, unsigned nbint, unsigned nbdbl,
			  unsigned nbval, unsigned nbfram);
void mom_tasklet_pop_frame (momval_t tsk);
int mom_tasklet_depth (momval_t tsk);

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
    return MOM_NULLV;
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
    case momty_booleanitem:
      return val;
    default:
      return MOM_NULLV;
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

static inline int
mom_item_cmp (const mom_anyitem_t * l, const mom_anyitem_t * r)
{
  if (l == r)
    return 0;
  if (!l)
    return -1;
  if (!r)
    return 1;
  return memcmp (l->i_uuid, r->i_uuid, sizeof (uuid_t));
}

static inline bool
mom_set_contains (momval_t s1, const mom_anyitem_t * itm)
{
  if (!s1.ptr || *s1.ptype != momty_set
      || !itm || itm->typnum <= momty__itemlowtype)
    return false;
  const momset_t *s1set = s1.pset;
  unsigned s1len = s1set->slen;
  unsigned lo = 0, hi = s1len, md = 0;
  while (lo + 2 < hi)
    {
      md = (lo + hi) / 2;
      int cmp = mom_item_cmp (s1set->itemseq[md], itm);
      if (cmp < 0)
	lo = md;
      else if (cmp > 0)
	hi = md;
      else
	{
	  assert (s1set->itemseq[md] == itm);
	  return true;
	};
    }
  for (md = lo; md < hi; md++)
    if (s1set->itemseq[md] == itm)
      return true;
  return false;
}

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


static inline double
mom_double_of_value (momval_t val, double def)
{
  if (val.ptr && *val.ptype == momty_float)
    return val.pfloat->floval;
  return def;
}

#define mom_double_of_value_else_0(V) mom_double_of_value((V),0.0)


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

#define MOM_FATAL_AT(Fil,Lin,Fmt,...) do {         \
  mom_fatal_at(Fil,Lin,Fmt,##__VA_ARGS__);} while(0)
#define MOM_FATAL_AT_BIS(Fil,Lin,Fmt,...) \
  MOM_FATAL_AT(Fil,Lin,Fmt,##__VA_ARGS__)
#define MOM_FATAL(Fmt,...) \
  MOM_FATAL_AT_BIS(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

void
mom_inform_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_INFORM_AT(Fil,Lin,Fmt,...) do {         \
  mom_inform_at(Fil,Lin,Fmt,##__VA_ARGS__);} while(0)
#define MOM_INFORM_AT_BIS(Fil,Lin,Fmt,...) \
  MOM_INFORM_AT(Fil,Lin,Fmt,##__VA_ARGS__)
#define MOM_INFORM(Fmt,...) \
  MOM_INFORM_AT_BIS(__FILE__,__LINE__,Fmt,##__VA_ARGS__)


void
mom_warning_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_WARNING_AT(Fil,Lin,Fmt,...) do {         \
  mom_warning_at(Fil,Lin,Fmt,##__VA_ARGS__);} while(0)
#define MOM_WARNING_AT_BIS(Fil,Lin,Fmt,...) \
  MOM_WARNING_AT(Fil,Lin,Fmt,##__VA_ARGS__)
#define MOM_WARNING(Fmt,...) \
  MOM_WARNING_AT_BIS(__FILE__,__LINE__,Fmt,##__VA_ARGS__)

//////////////// wrapping Boehm GC allocation
// using GNU extension: statement expression
#define MOM_GC_ALLOC_AT(Msg,Siz,Lin) ({ \
      size_t _sz_##Lin = Siz;					\
      void* _p_##Lin =						\
	(_sz_##Lin>0)?GC_MALLOC(_sz_##Lin):NULL;		\
  if (MOM_UNLIKELY(!_p_##Lin && _sz_##Lin>0))			\
    MOM_FATAL("failed to allocate %ld bytes:" Msg,		\
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
    MOM_FATAL("failed to allocate %ld scalar bytes:" Msg,	\
		(long) _sz_##Lin);				\
  memset (_p_##Lin, 0, _sz_##Lin);				\
  _p_##Lin; })
#define MOM_GC_SCALAR_ALLOC(Msg,Siz) \
  MOM_GC_SCALAR_ALLOC_AT(Msg,Siz,__LINE__)

// free a pointer and clear the variable
#define MOM_GC_FREE_AT(VarPtr,Lin) do {				\
    void* _fp_##Lin = VarPtr;					\
    VarPtr = NULL;						\
    if (_fp_##Lin != NULL) GC_FREE(_fp_##Lin); } while(0)
#define MOM_GC_FREE(VarPtr) MOM_GC_FREE_AT(VarPtr,__LINE__)

momhash_t mom_string_hash (const char *str, int len);
const momstring_t *mom_make_string_len (const char *str, int len);
static inline const momstring_t *
mom_make_string (const char *str)
{
  return mom_make_string_len (str, -1);
};

static inline bool
mom_is_string (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_string)
    return false;
  return true;
}

static inline bool
mom_same_string (momval_t val, const char *cstr)
{
  if (!val.ptr || *val.ptype != momty_string || !cstr)
    return false;
  return !strcmp (val.pstring->cstr, cstr);
}

static inline const char *
mom_string_cstr (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_string)
    return NULL;
  return val.pstring->cstr;
}

static inline unsigned
mom_string_length (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_string)
    return 0;
  return val.pstring->slen;
}

static inline const char *
mom_jsonstring_cstr (momval_t val)
{
  if (!val.ptr)
    return NULL;
  switch (*val.ptype)
    {
    case momty_string:
      return val.pstring->cstr;
    case momty_jsonitem:
      return mom_string_cstr ((momval_t) (val.pjsonitem->ij_namejson));
    default:
      return NULL;
    }
}

#define MOM_DEFAULT_STATE_FILE "state-monimelt.dbsqlite"
#define MOM_WEB_DIRECTORY "webdir"
#define MOM_WEB_ROOT_PAGE "mom-root-page.html"
const momint_t *mom_make_int (intptr_t n);
void mom_initialize (void);
void mom_initial_load (const char *state);
// the full dump should be called without worker thread running
void mom_full_dump_at (const char *fil, int lin, const char *reason,
		       const char *state);
#define mom_full_dump(Reason,State) mom_full_dump_at(__FILE__,__LINE__,(Reason),(State))

void *mom_allocate_item (unsigned type, size_t itemsize, unsigned space);
void *mom_allocate_item_with_uuid (unsigned type, size_t itemsize,
				   unsigned space, uuid_t uid);
mom_anyitem_t *mom_item_of_uuid (uuid_t);
unsigned mom_nb_items (void);

momit_json_name_t *mom_make_item_json_name_of_uuid (uuid_t, const char *name,
						    unsigned space);
momit_json_name_t *mom_make_item_json_name (const char *name, unsigned space);
#define mom_create__json_name(Name,Uid) \
  mom_make_item_json_name_of_uuid(Uid,#Name,MOM_SPACE_ROOT)


// fail if routine not found
momit_routine_t *mom_make_item_routine_of_uuid (uuid_t, const char *name,
						unsigned space);
momit_routine_t *mom_make_item_routine (const char *name, unsigned space);
// return NULL if routine not found
momit_routine_t *mom_try_make_item_routine (const char *name, unsigned space);
// make an embryonic routine 
momit_routine_t *mom_make_item_embryonic_routine (const char *name,
						  unsigned space);
// gives the embryonic name of such an embryonic routine, or else (ie
// non-embryonic or non routine) NULL
const char *mom_embryonic_routine_name (momit_routine_t * itrout);

#define mom_create__routine(Name,Uid) \
  mom_make_item_routine_of_uuid(Uid,#Name,MOM_SPACE_ROOT)

momit_tasklet_t *mom_make_item_tasklet_of_uuid (uuid_t uid, unsigned space);
momit_tasklet_t *mom_make_item_tasklet ();
#define mom_create__tasklet(Name,Uid) \
  mom_make_item_tasklet_of_uuid(Uid,MOM_SPACE_ROOT)
int mom_tasklet_step (momit_tasklet_t *);

static inline bool
mom_is_jsonable (const momval_t val)
{
  if (!val.ptr)
    return true;
  else if (val.ptr == MOM_EMPTY)
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
      case momty_booleanitem:
	return true;
      default:
	return false;
      }
}


// get the content of an item
static inline momval_t
mom_item_get_content (mom_anyitem_t * itm)
{
  momval_t res = MOM_NULLV;
  if (!itm || itm->typnum <= momty__itemlowtype)
    return MOM_NULLV;
  pthread_mutex_lock (&itm->i_mtx);
  res = itm->i_content;
  pthread_mutex_unlock (&itm->i_mtx);
  return res;
}

// put the content of an item
static inline void
mom_item_put_content (mom_anyitem_t * itm, momval_t val)
{
  if (!itm || itm->typnum <= momty__itemlowtype)
    return;
  pthread_mutex_lock (&itm->i_mtx);
  itm->i_content = val;
  pthread_mutex_unlock (&itm->i_mtx);
}

// change the space of an item, return the previous space
static inline unsigned
mom_item_set_space (mom_anyitem_t * itm, unsigned spacenum)
{
  if (!itm || itm->typnum <= momty__itemlowtype
      || spacenum > MOM_SPACE_MAX
      || (spacenum > MOM_SPACE_NONE
	  && mom_spacedescr_array[spacenum] == NULL))
    return MOM_SPACE_NONE;
  unsigned spa = MOM_SPACE_NONE;
  pthread_mutex_lock (&itm->i_mtx);
  spa = itm->i_space;
  itm->i_space = spacenum;
  pthread_mutex_unlock (&itm->i_mtx);
  return spa;
}

static inline unsigned
mom_item_space (mom_anyitem_t * itm)
{
  if (!itm || itm->typnum <= momty__itemlowtype)
    return MOM_SPACE_NONE;
  unsigned spa = MOM_SPACE_NONE;
  pthread_mutex_lock (&itm->i_mtx);
  spa = itm->i_space;
  pthread_mutex_unlock (&itm->i_mtx);
  return spa;
}

// get one attribute
momval_t mom_item_get_attr (mom_anyitem_t * itm, mom_anyitem_t * itat);

// atomically get several attributes : mom_item_get_several_attrs(itm,
// itat1, &val1, itat2, &val2, ... NULL);
int mom_item_get_several_attrs (mom_anyitem_t *, ...)
  __attribute__ ((sentinel));

// put an attribute, if itat or val is nil remove it
void mom_item_put_attr (mom_anyitem_t * itm, mom_anyitem_t * itat,
			momval_t val);

// atomically put several attributes : mom_item_get_several_attrs(itm,
// itat1, &val1, itat2, &val2, ... NULL);
void mom_item_put_several_attrs (mom_anyitem_t *, ...)
  __attribute__ ((sentinel));

momit_boolean_t *mom_create_named_bool (uuid_t uid, const char *name);
#define mom_create__boolean(Name,Uid) \
  mom_create_named_bool(Uid,#Name)

static inline momit_boolean_t *
mom_get_item_bool (bool b)
{
  extern momit_boolean_t *mom_item__true_value;
  extern momit_boolean_t *mom_item__false_value;
  // defined and initialized in create-items.c using monimelt-named.h
  if (b)
    return mom_item__true_value;
  else
    return mom_item__false_value;
}

static inline bool
mom_item_is_true (mom_anyitem_t * itm)
{
  extern momit_boolean_t *mom_item__true_value;
  return itm == (mom_anyitem_t *) mom_item__true_value;
}

static inline bool
mom_item_is_false (mom_anyitem_t * itm)
{
  extern momit_boolean_t *mom_item__false_value;
  return itm == (mom_anyitem_t *) mom_item__false_value;
}

// compare values for JSON
int mom_json_cmp (momval_t l, momval_t r);
// compare a JSON value to a non-null string
int mom_json_cstr_cmp (momval_t jv, const char *str);

const momval_t mom_jsonob_getstr (const momval_t jsobv, const char *name);

const momval_t mom_jsonob_get_def (const momval_t jsobv, const momval_t namev,
				   const momval_t def);
static inline const momval_t
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
  MOMJSON_ENTRY,		/* momval_t nameval, momval_t attrval */
  MOMJSON_STRING,		/* const char*namestr, momval_t attval */
  MOMJSON_COUNTED_ENTRIES,	/* unsigned count, struct mom_jsonentry_st* */
};

#define MOMJSON_END ((void*)MOMJSON__END)
// make a JSON array of given count
const momjsonarray_t *mom_make_json_array (unsigned nbelem, ...);
const momjsonarray_t *mom_make_json_array_count (unsigned count,
						 const momval_t * arr);
const momjsonarray_t *mom_make_json_array_til_nil (momval_t, ...)
  __attribute__ ((sentinel));

static inline unsigned
mom_json_array_size (momval_t val)
{
  if (!val.ptr || *val.ptype != momty_jsonarray)
    return 0;
  return val.pjsonarr->slen;
}

static inline const momval_t
mom_json_array_nth (momval_t val, int rk)
{
  if (!val.ptr || *val.ptype != momty_jsonarray)
    return MOM_NULLV;
  unsigned slen = val.pjsonarr->slen;
  if (rk < 0)
    rk += slen;
  if (rk >= 0 && rk < slen)
    return val.pjsonarr->jarrtab[rk];
  return MOM_NULLV;
}

// make a set from items, or sets, or tuples
const momset_t *mom_make_set_til_nil (momval_t, ...)
  __attribute__ ((sentinel));
// make an tuple from items, or sets, or tuples
const momitemtuple_t *mom_make_tuple_til_nil (momval_t, ...)
  __attribute__ ((sentinel));
// make a set of given number of items, removing duplicates...
const momset_t *mom_make_set_sized (unsigned siz, ...);
// make a set of given number of items, removing duplicates...
const momset_t *mom_make_set_from_array (unsigned siz,
					 const mom_anyitem_t ** arr);
// make a tuple of given number of items
const momitemtuple_t *mom_make_tuple_sized (unsigned siz, ...);

// make a tuple of given number of items
const momitemtuple_t *mom_make_tuple_from_array (unsigned siz,
						 const mom_anyitem_t ** arr);

// make a node from a nil terminated sequence of components
const momnode_t *mom_make_node_til_nil (const mom_anyitem_t * conn, ...)
  __attribute__ ((sentinel));
// make a node of given arity
const momnode_t *mom_make_node_sized (const mom_anyitem_t * conn,
				      unsigned siz, ...);
const momnode_t *mom_make_node_from_array (const mom_anyitem_t * conn,
					   unsigned siz, momval_t * arr);

// make a closure from a nil terminated sequence of components
const momclosure_t *mom_make_closure_til_nil (const mom_anyitem_t * conn, ...)
  __attribute__ ((sentinel));
// make a closure of given arity
const momclosure_t *mom_make_closure_sized (const mom_anyitem_t * conn,
					    unsigned siz, ...);
const momclosure_t *mom_make_closure_from_array (const mom_anyitem_t * conn,
						 unsigned siz,
						 momval_t * arr);

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

struct mom_loader_st;
// load a value from its JSON           
void
mom_load_any_item_data (struct mom_loader_st *ld, mom_anyitem_t * itm,
			momval_t jsob);

momval_t mom_load_value_json (struct mom_loader_st *ld, const momval_t jval);
// load the common part of an item 
///// JSON output
struct jsonoutput_st
{
  uint32_t jsono_magic;		/* always MOMJSONO_MAGIC */
  uint32_t jsono_flags;
  pthread_mutex_t jsono_mtx;
  FILE *jsono_file;
  long jsono_lastnewline;
  void *jsono_data;
};

enum jsonoutflags_en
{
  jsof_none = 0,
  jsof_indent = 1 << 0,		/* indent systematically the output */
  jsof_halfindent = 1 << 1,	/* indent sometimes the output, producing not too long lines */
  jsof_flush = 1 << 2,		/* flush the output at end */
  jsof_cname = 1 << 3,		/* output C identifiers at names in JSON objects */
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
extern const char mom_GPL_friendly_module[];
extern void mom_module_init (const char *marg);
// modules may also define for option processing
extern GOptionGroup *mom_module_option_group (const char *modname);
// and for post load processing
extern void mom_module_post_load (void);

struct mom_itqueue_st
{
  struct mom_itqueue_st *iq_next;
  mom_anyitem_t *iq_item;
};

//// dumper, see file dump-load.c
struct mom_dumper_st
{
  unsigned dmp_magic;		/* always DUMPER_MAGIC */
  unsigned dmp_count;
  unsigned dmp_size;
  unsigned dmp_state;
  const char *dmp_reason;
  const char *dmp_srcfile;
  int dmp_srcline;
  struct mom_itqueue_st *dmp_qfirst;
  struct mom_itqueue_st *dmp_qlast;
  const mom_anyitem_t **dmp_array;
};

// initialize a dumper
void mom_dump_initialize (struct mom_dumper_st *dmp);

typedef void mom_dumpglobal_sig_t (const mom_anyitem_t * itm,
				   const momstring_t * name, void *data);

void mom_dump_globals (struct mom_dumper_st *, mom_dumpglobal_sig_t * globcb,
		       void *data);

// add a scanned item into a dumper
void mom_dump_add_item (struct mom_dumper_st *dmp, const mom_anyitem_t * itm);

// scan a value into a dumper
void mom_dump_scan_value (struct mom_dumper_st *dmp, const momval_t val);

// give the JSON value to dump the value VAL
momval_t mom_dump_emit_json (struct mom_dumper_st *dmp, const momval_t val);

momval_t mom_attributes_emit_json (struct mom_dumper_st *dmp,
				   struct mom_itemattributes_st *iat);

#define LOADER_MAGIC 0x169128bb	/* loader magic 378611899 */
struct mom_loader_st
{
  unsigned ldr_magic;		/* always LOADER_MAGIC */
  struct mom_itqueue_st *ldr_qfirst;
  struct mom_itqueue_st *ldr_qlast;
};

////////////////////////////////////////////////////////////////
/// global data, managed by functions
// register a new name, nop if existing entry
void mom_register_new_name_item (const char *name, mom_anyitem_t * item);
void mom_register_new_name_string (const momstring_t * namestr,
				   mom_anyitem_t * item);

// register a name, replacing any previous entries
void mom_replace_named_item (const char *name, mom_anyitem_t * item);
void mom_replace_name_string (const momstring_t * namestr,
			      mom_anyitem_t * item);

/// forget a name, or a string value, or an item
void mom_forget_name (const char *name);
void mom_forget_string (const momstring_t * namestr);
void mom_forget_item (mom_anyitem_t * item);

// get the item of some given name, or else NULL
mom_anyitem_t *mom_item_named (const char *name);
// use a string value 
mom_anyitem_t *mom_item_of_name_string (momval_t namev);
// also retrieve the string
mom_anyitem_t *mom_item_named_with_string (const char *name,
					   const momstring_t ** pstr);

// get the name of some given item, or else NULL
const momstring_t *mom_name_of_item (const mom_anyitem_t * item);

// get a node with the sorted names starting with a given prefix (or all if prefix is NULL)
momval_t mom_node_sorted_names_prefixed (const mom_anyitem_t * conn,
					 const char *prefix);
// get the set of items starting with a given prefix (or all items if prefix is NULL)
momval_t mom_set_named_items_prefixed (const char *prefix);
////////////////
// run work threads

#define MOM_MIN_WORKERS 2
#define MOM_MAX_WORKERS 16

unsigned mom_nb_workers;
pthread_mutex_t mom_run_mtx;
pthread_cond_t mom_run_changed_cond;

// start the worker threads
void mom_run_at (const char *srcfil, int srclin, const char *reason);
#define mom_run(Reason) mom_run_at(__FILE__,__LINE__,(Reason))

// a post runner function returns >0 to request that running resumes
typedef int mom_post_runner_sig_t (void *data);
// ask workers to stop, then have the main thread invokes a post runner
void mom_request_stop_at (const char *srcfil, int srclin, const char *reason,
			  mom_post_runner_sig_t * postrunner,
			  void *clientdata);
#define mom_request_stop(Reason,Postrunner,Data) mom_request_stop_at(__FILE__,__LINE__,\
								     (Reason),(Postrunner),(Data))

// a generated code module might have a function to be executed after the code module load
extern void mom_after_code_load (const char *modname);
// load a code module, resolve embryonic routines, run again
int mom_load_code_post_runner (const char *modname);

void mom_agenda_add_tasklet_front (momval_t tsk);
void mom_agenda_add_tasklet_back (momval_t tsk);

// number of tasklet steps 
long long mom_agenda_work_counter (void);

#define MOM_NAMED(Name,Type,Uid) extern momit_##Type##_t* mom_item__##Name;
#include "monimelt-names.h"
#endif /* MOM_INCLUDED_ */
