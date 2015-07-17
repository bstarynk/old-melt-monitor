// file item.c - handle items

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

#include "monimelt.h"

////////////////
/// define the predefined items
#define MOM_HAS_PREDEFINED_NAMED(Nam,Hash) momitem_t*mompi_##Nam;
#define MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) momitem_t*mompi_##Id;
//
#include "predef-monimelt.h"


static struct momhashset_st *predefined_hashset_mom;
static pthread_mutex_t predefined_mutex_mom = PTHREAD_MUTEX_INITIALIZER;
/// routine to create the predefined
static void create_predefined_items_mom (void);

//////////////// named items Named items are organized in a 2-level
// tree,i.e. a vector of buckets, with O(sqrt(N)) average complexity
// where N is the number of names. Each bucket is sorted
// (alphanumerically).

static pthread_rwlock_t named_lock_mom = PTHREAD_RWLOCK_INITIALIZER;

struct namebucket_mom_st
{                               //// buckets should be scalar zones, the items in them are weak pointers
  unsigned nambuck_size;        /* allocated size of bucket */
  unsigned nambuck_len;         /* used length */
  const momitem_t *nambuck_arr[];
};

static unsigned named_count_mom;        /* total number of named items */
static unsigned named_nbuck_mom;        /* number of buckets */
/// all the buckets are non-nil pointers 
struct namebucket_mom_st **named_buckets_mom;




static pthread_mutexattr_t mutexattr_recur_item_mom;



bool
mom_valid_item_name_str (const char *id, const char **pend)
{
  if (pend)
    *pend = NULL;
  if (!id)
    return false;
  if (!isalpha (id[0]))
    return false;
  const char *pc = NULL;
  for (pc = id + 1;; pc++)
    {
      if (isalnum (*pc))
        continue;
      else if (*pc == '_')
        {
          if (pc[-1] == '_' || !isalnum (pc[1]))
            return false;
          continue;
        }
      else
        {
          if (pend)
            *pend = pc;
          return true;
        }
    }
  // not reached
  return false;
}


const char *
mom_item_space_string (const momitem_t *itm)
{
  if (!itm || itm == MOM_EMPTY)
    return "~";
  switch (itm->itm_space)
    {
    case momspa_none:
      return "none";
    case momspa_transient:
      return "transient";
    case momspa_user:
      return "user";
    case momspa_global:
      return "global";
    case momspa_predefined:
      return "predefined";
    default:
      {
        char buf[32];
        memset (buf, 0, sizeof (buf));
        snprintf (buf, sizeof (buf) - 1, "space#%d?", (int) itm->itm_space);
        return MOM_GC_STRDUP ("strange space", buf);
      }
    }
}

void
mom_initialize_items (void)
{
  extern void mom_initialize_anonymous_items (void);    // in anonym.c
  pthread_mutexattr_init (&mutexattr_recur_item_mom);
  pthread_mutexattr_settype (&mutexattr_recur_item_mom,
                             PTHREAD_MUTEX_RECURSIVE);
  mom_initialize_anonymous_items ();
  /// create the predefined items
  create_predefined_items_mom ();
}







momitem_t *
mom_find_named_item (const char *name)
{
  const momitem_t *itm = NULL;
  const char *endname = NULL;
  int bix = -1;
  if (!name || !mom_valid_item_name_str (name, &endname)
      || !endname || *endname != '\0')
    return NULL;
  pthread_rwlock_rdlock (&named_lock_mom);
  MOM_DEBUGPRINTF (item, "find_named_item name=%s", name);
  if (MOM_UNLIKELY (named_count_mom == 0))
    goto end;
  {
    int lobix = 0, hibix = named_nbuck_mom;
    while (lobix + 2 < hibix)
      {
        int mdbix = 0;
        while (lobix < hibix && named_buckets_mom[lobix]->nambuck_len == 0)
          lobix++;
        while (hibix > lobix
               && named_buckets_mom[hibix - 1]->nambuck_len == 0)
          hibix--;
        if (lobix + 2 >= hibix)
          break;
        for (mdbix = (lobix + hibix) / 2;
             mdbix < hibix && named_buckets_mom[mdbix]->nambuck_len == 0;
             mdbix++);
        if (mdbix == hibix)
          break;
        const struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
        unsigned mdlen = mdbuck->nambuck_len;
        assert (mdlen > 0);
        const momitem_t *firstitm = mdbuck->nambuck_arr[0];
        assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
        const momitem_t *lastitm = mdbuck->nambuck_arr[mdlen - 1];
        assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
        if (strcmp (name, firstitm->itm_name->cstr) < 0)
          {
            hibix = mdbix;
            continue;
          }
        else if (strcmp (name, lastitm->itm_name->cstr) > 0)
          {
            lobix = mdbix;
            continue;
          }
        else
          {
            bix = mdbix;
            break;
          };
      };
    MOM_DEBUGPRINTF (item, "find_named_item lobix=%d hibix=%d bix=%d", lobix,
                     hibix, bix);
    for (int mdbix = lobix; bix < 0 && mdbix < hibix; mdbix++)
      {
        const struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
        unsigned mdlen = mdbuck->nambuck_len;
        if (mdlen == 0)
          continue;
        const momitem_t *firstitm = mdbuck->nambuck_arr[0];
        assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
        const momitem_t *lastitm = mdbuck->nambuck_arr[mdlen - 1];
        assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
        if (strcmp (name, firstitm->itm_name->cstr) >= 0
            && strcmp (name, lastitm->itm_name->cstr) <= 0)
          bix = mdbix;
      };
    if (bix < 0)
      goto end;
    assert (bix >= 0 && bix < (int) named_nbuck_mom);
    const struct namebucket_mom_st *buck = named_buckets_mom[bix];
    unsigned blen = buck->nambuck_len;
    MOM_DEBUGPRINTF (item, "find_named_item bix=%d blen=%u", bix, blen);
    if (blen == 0)
      goto end;
    int lo = 0, hi = (int) blen, md = 0;
    while (lo + 3 < hi)
      {
        md = (lo + hi) / 2;
        const momitem_t *curitm = buck->nambuck_arr[md];
        assert (curitm && !curitm->itm_anonymous && curitm->itm_name);
        int cmp = strcmp (name, curitm->itm_name->cstr);
        if (!cmp)
          {
            itm = curitm;
            goto end;
          }
        else if (cmp < 0)
          hi = md;
        else
          lo = md;
      }
    MOM_DEBUGPRINTF (item, "find_named_item lo=%d hi=%d", lo, hi);
    for (md = lo; md < hi; md++)
      {
        const momitem_t *curitm = buck->nambuck_arr[md];
        assert (curitm && !curitm->itm_anonymous && curitm->itm_name);
        int cmp = strcmp (name, curitm->itm_name->cstr);
        if (!cmp)
          {
            itm = curitm;
            goto end;
          }
      }
  }
end:
  MOM_DEBUGPRINTF (item, "find_named_item name=%s bix=%d itm@%p", name, bix,
                   itm);
  pthread_rwlock_unlock (&named_lock_mom);
  return (momitem_t *) itm;
}



momvalue_t
mom_set_named_items_of_prefix (const char *prefix)
{
  momvalue_t setv = MOM_NONEV;
  int lobix = 0, hibix = 0;
  if (!prefix || prefix == MOM_EMPTY)
    prefix = "";
  if (prefix[0])
    {
      if (!isalpha (prefix[0]))
        return MOM_NONEV;
      for (const char *p = prefix; *p; p++)
        {
          if (!isalnum (*p) && *p != '_')
            return MOM_NONEV;
        }
    }
  int prefixlen = strlen (prefix);
  pthread_rwlock_rdlock (&named_lock_mom);
  MOM_DEBUGPRINTF (item, "set_named_items_of_prefix prefix=%s", prefix);
  if (MOM_UNLIKELY (named_count_mom == 0))
    goto end;
  lobix = 0;
  hibix = named_nbuck_mom;
  {
    while (lobix + 2 < hibix)
      {
        int mdbix = 0;
        while (lobix < hibix && named_buckets_mom[lobix]->nambuck_len == 0)
          lobix++;
        while (hibix > lobix
               && named_buckets_mom[hibix - 1]->nambuck_len == 0)
          hibix--;
        if (lobix + 2 >= hibix)
          break;
        for (mdbix = (lobix + hibix) / 2;
             mdbix < hibix && named_buckets_mom[mdbix]->nambuck_len == 0;
             mdbix++);
        if (mdbix == hibix)
          break;
        const struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
        unsigned mdlen = mdbuck->nambuck_len;
        assert (mdlen > 0);
        const momitem_t *firstitm = mdbuck->nambuck_arr[0];
        assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
        const momitem_t *lastitm = mdbuck->nambuck_arr[mdlen - 1];
        assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
        if (strncmp (prefix, firstitm->itm_name->cstr, prefixlen) < 0)
          {
            hibix = mdbix;
            continue;
          }
        else if (strncmp (prefix, lastitm->itm_name->cstr, prefixlen) > 0)
          {
            lobix = mdbix;
            continue;
          }
        else
          {
            lobix = mdbix;
            break;
          };
      };
    assert (lobix < (int) named_nbuck_mom);
    assert (lobix <= hibix);
    unsigned arrsiz =
      ((5 + (hibix - lobix + 1) * (int) sqrt (named_count_mom + 3)) | 0xf) +
      1;
    unsigned itmcount = 0;
    momitem_t **arritm =
      MOM_GC_ALLOC ("named arritm", sizeof (momitem_t *) * arrsiz);
    MOM_DEBUGPRINTF (item,
                     "set_named_items_of_prefix lobix=%d hibix=%d arrsiz=%u",
                     lobix, hibix, arrsiz);
    for (int bix = lobix; bix < (int) named_nbuck_mom; bix++)
      {
        const struct namebucket_mom_st *curbuck = named_buckets_mom[bix];
        assert (curbuck != NULL);
        unsigned blen = curbuck->nambuck_len;
        if (blen == 0)
          continue;
        const momitem_t *firstitm = curbuck->nambuck_arr[0];
        assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
        const momitem_t *lastitm = curbuck->nambuck_arr[blen - 1];
        assert (lastitm && !firstitm->itm_anonymous && lastitm->itm_name);
        int firstcmp = strncmp (firstitm->itm_name->cstr, prefix, prefixlen);
        int lastcmp = strncmp (prefix, lastitm->itm_name->cstr, prefixlen);
        MOM_DEBUGPRINTF (item,
                         "set_named_items_of_prefix prefix=%s bix=%d blen=%u firstitm=%s lastitm=%s",
                         prefix, bix, blen, mom_item_cstring (firstitm),
                         mom_item_cstring (lastitm));
        if (firstcmp > 0)
          break;
        if (lastcmp < 0)
          continue;
        int lo = 0, hi = (int) blen, md = 0;
        while (lo + 8 < hi)
          {
            md = (lo + hi) / 2;
            const momitem_t *miditm = curbuck->nambuck_arr[md];
            assert (miditm && !miditm->itm_anonymous && miditm->itm_name);
            int mdcmp = strncmp (miditm->itm_name->cstr, prefix, prefixlen);
            if (!mdcmp)
              break;
            if (mdcmp < 0)
              lo = md;
            else
              hi = md;
          };
        for (md = lo; md < (int) blen; md++)
          {
            const momitem_t *miditm = curbuck->nambuck_arr[md];
            assert (miditm && !miditm->itm_anonymous && miditm->itm_name);
            int mdcmp = strncmp (miditm->itm_name->cstr, prefix, prefixlen);
            if (!mdcmp)
              {
                if (MOM_UNLIKELY (itmcount + 1 >= arrsiz))
                  {
                    unsigned newsiz = ((10 + 4 * arrsiz / 3) | 0xf) + 1;
                    unsigned oldsiz = arrsiz;
                    assert (newsiz > oldsiz);
                    momitem_t **oldarritm = arritm;
                    arritm =
                      MOM_GC_ALLOC ("grown named arritm",
                                    sizeof (momitem_t *) * newsiz);
                    arrsiz = newsiz;
                    memcpy (arritm, oldarritm,
                            itmcount * sizeof (momitem_t *));
                    MOM_GC_FREE (oldarritm, oldsiz * sizeof (momitem_t *));
                  };
                arritm[itmcount] = (momitem_t *) miditm;
                itmcount++;
              }
            else if (mdcmp > 0)
              {
                bix = (int) named_nbuck_mom;
                break;
              }
          }
      };
    setv = mom_unsafe_setv (mom_make_sized_tuple (itmcount, arritm));
    MOM_GC_FREE (arritm, arrsiz * sizeof (momitem_t *));
  }
end:
  MOM_DEBUGPRINTF (item,
                   "set_named_items_of_prefix prefix=%s lobix=%d, hibix=%d, setv=%s",
                   prefix, lobix, hibix, mom_output_gcstring (setv));
  pthread_rwlock_unlock (&named_lock_mom);
  return setv;
}                               /* end of mom_set_named_items_of_prefix */



// reorganize all the named items, should be called under the write lock
static void
reorganize_named_items_mom (void)
{
  const momitem_t **allnamedarr = NULL;
  unsigned oldnamecount = named_count_mom;
  static long reorgcounter;
  reorgcounter++;
  MOM_DEBUGPRINTF (item,
                   "reorganize_named_items reorgcounter#%ld oldnamecount=%u",
                   reorgcounter, oldnamecount);
  allnamedarr =
    MOM_GC_ALLOC ("allnamedarr", sizeof (momitem_t *) * (oldnamecount + 2));
  unsigned namecount = 0;
  // first loop to fill name allnamedarr
  for (unsigned bix = 0; bix < named_nbuck_mom; bix++)
    {
      struct namebucket_mom_st *curbuck = named_buckets_mom[bix];
      assert (curbuck != NULL);
      unsigned blen = curbuck->nambuck_len;
      unsigned bsiz = curbuck->nambuck_size;
      assert (bsiz >= blen);
      for (unsigned itmix = 0; itmix < blen; itmix++)
        {
          const momitem_t *itm = curbuck->nambuck_arr[itmix];
          assert (itm && !itm->itm_anonymous && itm->itm_name);
          if (MOM_UNLIKELY (namecount > named_count_mom))
            MOM_FATAPRINTF ("more named items than expected %d", namecount);
          allnamedarr[namecount] = itm;
          if (MOM_LIKELY (namecount > 0))
            {
              const momitem_t *previtm = allnamedarr[namecount - 1];
              if (MOM_UNLIKELY
                  (strcmp (previtm->itm_name->cstr, itm->itm_name->cstr) >=
                   0))
                MOM_FATAPRINTF ("corrupted named item element #%d",
                                namecount);
            };
          namecount++;
        }
      named_buckets_mom[bix] = NULL;
      MOM_GC_FREE (curbuck,
                   sizeof (struct namebucket_mom_st) +
                   bsiz * sizeof (momitem_t *));
    }
  MOM_DEBUGPRINTF (item,
                   "reorganize_named_items namecount=%u named_count_mom=%u",
                   namecount, named_count_mom);
  assert (namecount == named_count_mom);
  MOM_GC_FREE (named_buckets_mom,
               sizeof (struct namebucket_mom_st *) * named_nbuck_mom);
  named_buckets_mom = NULL;
  named_nbuck_mom = 0;
  named_count_mom = 0;
  unsigned newnbuck = (int) (1.2 * sqrt (namecount)) + 11;
  unsigned newblen = (namecount + newnbuck / 2) / newnbuck + 1;
  if (MOM_UNLIKELY (newblen < 2))
    newblen++;
  unsigned newbsiz = ((3 * newblen / 2 + 5) | 0xf) + 1;
  MOM_DEBUGPRINTF (item,
                   "reorganize_named_items newnbuck=%u newblen=%u newbsiz=%u",
                   newnbuck, newblen, newbsiz);
  named_buckets_mom =
    MOM_GC_ALLOC ("named buckets",
                  newnbuck * sizeof (struct namebucket_mom_st *));
  named_nbuck_mom = newnbuck;
  // second loop to fill the buckets
  unsigned namix = 0;
  for (unsigned bix = 0; bix < newnbuck; bix++)
    {
      struct namebucket_mom_st *curbuck =
        MOM_GC_SCALAR_ALLOC ("bucket", sizeof (struct namebucket_mom_st)
                             + newbsiz * sizeof (momitem_t *));
      curbuck->nambuck_size = newbsiz;
      named_buckets_mom[bix] = curbuck;
      unsigned itmix = 0;
      for (itmix = 0; itmix < newblen && namix < namecount; itmix++)
        {
          const momitem_t *curitm = allnamedarr[namix];
          curbuck->nambuck_arr[itmix] = curitm;
          namix++;
        };
      curbuck->nambuck_len = itmix;
    };
  MOM_DEBUGPRINTF (item,
                   "reorganize_named_items ending namix=%u namecount=%u newnbuck=%u",
                   namix, namecount, newnbuck);
  assert (namix >= namecount);
  named_count_mom = namecount;
  assert (oldnamecount == namecount);
}


static int
compare_names_mom (const void *p1, const void *p2)
{
  return strcmp (*(const char **) p1, *(const char **) p2);
}

void mom_initialize_protoitem (momitem_t *protoitm);

static void
create_predefined_items_mom (void)
{
  int nbnamed = 0, nbanon = 0;
  char **arrnames =             //
    MOM_GC_ALLOC ("predefined names",
                  (MOM_NB_PREDEFINED_NAMED + 3) * sizeof (char *));
  // arritems is needed to avoid premature GC
  momitem_t **arritems =        //
    MOM_GC_ALLOC ("predefined items",
                  (MOM_NB_PREDEFINED_NAMED + 3) * sizeof (momitem_t *));

#define  MOM_HAS_PREDEFINED_NAMED(Nam,Hash) do {	\
    arrnames[nbnamed] = #Nam;                           \
    assert (mom_cstring_hash(#Nam) == Hash);		\
    nbnamed++;						\
  } while(0);
  //
#define  MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) do {	\
    mompi_##Id = mom_make_anonymous_item_by_id(#Id);	\
    assert (mompi_##Id->itm_id->shash == Hash);         \
    mompi_##Id->itm_space = momspa_predefined;		\
    predefined_hashset_mom =				\
      mom_hashset_put(predefined_hashset_mom,		\
		  mompi_##Nam);				\
    nbanon++;						\
  } while(0);
  //
  //
#include "predef-monimelt.h"
  //
  {
    pthread_rwlock_wrlock (&named_lock_mom);
    assert (nbnamed > 0);
    qsort (arrnames, nbnamed, sizeof (char *), compare_names_mom);
    assert (named_count_mom == 0);
    assert (named_nbuck_mom == 0);
    assert (named_buckets_mom == NULL);
    unsigned nbucks = ((int) sqrt ((double) nbnamed)) + 3;
    unsigned bucklen = (nbnamed + 1) / nbucks + 1;
    if (bucklen < 2)
      bucklen = 2;
    unsigned bucksiz = ((5 * bucklen / 4 + 3) | 0xf) + 1;
    MOM_DEBUGPRINTF (item, "nbnamed=%d nbanon=%d bucklen=%d nbucks=%d",
                     nbnamed, nbanon, bucklen, nbucks);
    assert (nbnamed == MOM_NB_PREDEFINED_NAMED);
    assert (nbanon == MOM_NB_PREDEFINED_ANONYMOUS);
    named_buckets_mom =         //
      MOM_GC_ALLOC ("initial named buckets", nbucks * sizeof (void *));
    named_nbuck_mom = nbucks;
    int namecount = 0;
    for (unsigned bix = 0; bix < nbucks; bix++)
      {
        struct namebucket_mom_st *curbuck =     //
          MOM_GC_SCALAR_ALLOC ("initial named bucket",
                               sizeof (struct namebucket_mom_st)
                               + bucksiz * sizeof (momitem_t *));
        curbuck->nambuck_size = bucksiz;
        named_buckets_mom[bix] = curbuck;
        MOM_DEBUGPRINTF (item, "created bucket#%d @%p of size %u", bix,
                         curbuck, bucksiz);
        unsigned blen = 0;
        for (int ixitm = 0; ixitm < (int) bucklen && namecount < nbnamed;
             ixitm++)
          {
            const char *curname = arrnames[namecount];
            assert (curname && isalpha (curname[0]));
            momitem_t *itm =    //
              MOM_GC_ALLOC ("initial named item",
                            sizeof (momitem_t));
            mom_initialize_protoitem (itm);
            itm->itm_anonymous = false;
            itm->itm_space = momspa_predefined;
            itm->itm_name = mom_make_string_cstr (curname);
            curbuck->nambuck_arr[ixitm] = itm;
            arritems[namecount] = itm;
            blen = ixitm + 1;
            namecount++;
            MOM_DEBUGPRINTF (item,
                             "create predefined named item @%p: %s #%d (bix#%d, ixitm=%d)",
                             itm, curname, namecount, bix, ixitm);
          }
        curbuck->nambuck_len = blen;
        assert (named_buckets_mom[bix] != NULL);
      }
    named_count_mom = namecount;
    pthread_rwlock_unlock (&named_lock_mom);
  }
#define  MOM_HAS_PREDEFINED_NAMED(Nam,Hash) do {		\
    mompi_##Nam = mom_find_named_item (#Nam);			\
    assert (mompi_##Nam && !mompi_##Nam->itm_anonymous);	\
    assert (mompi_##Nam->itm_name				\
	   && mompi_##Nam->itm_name->shash == Hash);		\
    predefined_hashset_mom =					\
      mom_hashset_put(predefined_hashset_mom,			\
		  mompi_##Nam);					\
  } while(0);
  //
#define  MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) do {	\
  } while(0)
#include "predef-monimelt.h"

  MOM_INFORMPRINTF ("created %d predefined named & %d predefined anonymous",
                    nbnamed, nbanon);
}                               /* end create_predefined_items_mom */


momitem_t *
find_named_item_mom (const char *str)
{
  static unsigned long count;
  count++;
  MOM_DEBUGPRINTF (item, "find_named_item_mom str=%s count#%ld", str, count);
  assert (mom_valid_item_name_str (str, NULL));
  assert (named_nbuck_mom > 0);
  assert (named_count_mom > 0);
  int lobix = 0, hibix = named_nbuck_mom, mdbix = 0;
  int bix = -1;
  while (lobix + 3 < hibix)
    {
      while (lobix < hibix && named_buckets_mom[lobix]->nambuck_len == 0)
        lobix++;
      while (hibix > lobix && named_buckets_mom[hibix - 1]->nambuck_len == 0)
        hibix--;
      if (lobix + 2 >= hibix)
        break;
      mdbix = (lobix + hibix) / 2;
      while (mdbix < hibix && named_buckets_mom[mdbix]->nambuck_len == 0)
        mdbix++;
      struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
      unsigned mdlen = mdbuck->nambuck_len;
      assert (mdlen > 0);
      const momitem_t *mditm = mdbuck->nambuck_arr[mdlen / 2];
      assert (mditm && !mditm->itm_anonymous && mditm->itm_name);
      MOM_DEBUGPRINTF (item,
                       "find_named_item_mom str=%s mdbix=%d lobix=%d hibix=%d mditm=%s",
                       str, mdbix, lobix, hibix, mditm->itm_name->cstr);
      if (strcmp (str, mditm->itm_name->cstr) < 0)
        hibix = mdbix + 1;
      else
        lobix = mdbix;
    }
  // try to find the appropriate bucket index bix
  MOM_DEBUGPRINTF (item,
                   "find_named_item_mom str=%s lobix=%d hibix=%d", str,
                   lobix, hibix);
  for (mdbix = lobix; mdbix < hibix && bix < 0; mdbix++)
    {
      struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
      unsigned mdlen = mdbuck->nambuck_len;
      if (mdlen == 0)
        continue;
      const momitem_t *firstitm = mdbuck->nambuck_arr[0];
      assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
      const momitem_t *lastitm = mdbuck->nambuck_arr[mdlen - 1];
      assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
      MOM_DEBUGPRINTF (item,
                       "find_named_item_mom str=%s mdbix=%d mdlen=%d firstitm=%s lastitm=%s",
                       str, mdbix, mdlen, firstitm->itm_name->cstr,
                       lastitm->itm_name->cstr);
      if (strcmp (firstitm->itm_name->cstr, str) <= 0
          && strcmp (str, lastitm->itm_name->cstr) <= 0)
        bix = mdbix;
    }
  if (bix < 0)
    return NULL;
  assert (bix < (int) named_nbuck_mom);
  {
    struct namebucket_mom_st *buck = named_buckets_mom[bix];
    assert (buck != NULL);
    unsigned blen = buck->nambuck_len;
    assert (blen > 0);
    int lo = 0, hi = blen, md = 0;
    while (lo + 3 < hi)
      {
        md = (lo + hi) / 2;
        const momitem_t *itm = buck->nambuck_arr[md];
        assert (itm && !itm->itm_anonymous && itm->itm_name);
        int cmp = strcmp (str, itm->itm_name->cstr);
        if (cmp == 0)
          return (momitem_t *) itm;
        if (cmp < 0)
          hi = md;
        else
          lo = md;
      };
    MOM_DEBUGPRINTF (item,
                     "find_named_item_mom str=%s lo=%d hi=%d bix=%d", str,
                     lo, hi, bix);
    for (md = lo; md < hi; md++)
      {
        const momitem_t *itm = buck->nambuck_arr[md];
        assert (itm && !itm->itm_anonymous && itm->itm_name);
        int cmp = strcmp (str, itm->itm_name->cstr);
        if (cmp == 0)
          {
            MOM_DEBUGPRINTF (item,
                             "find_named_item_mom str=%s found itm@%p", str,
                             itm);
            return (momitem_t *) itm;
          }
      }
  }
  MOM_DEBUGPRINTF (item, "find_named_item_mom str=%s not-found", str);
  return NULL;
}                               // end find_named_item_mom




momitem_t *
mom_find_item (const char *str)
{
  const momitem_t *itm = NULL;
  const char *end = NULL;
  if (!str || !str[0])
    return NULL;
  if (isalpha (str[0]) && mom_valid_item_name_str (str, &end) && end && !*end)
    {
      pthread_rwlock_rdlock (&named_lock_mom);
      itm = find_named_item_mom (str);
      pthread_rwlock_unlock (&named_lock_mom);
      return (momitem_t *) itm;
    }
  else if (str[0] == '_' && mom_valid_item_id_str (str, &end) && end && !*end)
    {
      // see anonym.c
      extern const momitem_t *mom_find_anonymous_item (const char *idstr);
      return (momitem_t *) mom_find_anonymous_item (str);
    }
  return NULL;
}

void
mom_initialize_protoitem (momitem_t *protoitm)
{
  pthread_mutex_init (&protoitm->itm_mtx, &mutexattr_recur_item_mom);
}

void
mom_unregister_named_finalized_item (momitem_t *finitm)
{
  assert (finitm && !finitm->itm_anonymous && finitm->itm_name);
  const momstring_t *itmnam = finitm->itm_name;
  pthread_rwlock_wrlock (&named_lock_mom);
  assert (named_count_mom > 0); /* we have some predefined names! */
  if (MOM_UNLIKELY
      (named_count_mom > 0
       && named_nbuck_mom * (named_nbuck_mom + 1) < named_count_mom / 3))
    reorganize_named_items_mom ();
  MOM_WARNPRINTF
    ("unregister_named_finalized_item unimplemented finitm@%p %s",
     (void *) finitm, mom_item_cstring (finitm));
  int lobix = 0, hibix = named_nbuck_mom, mdbix = 0;
  int bix = -1;
  while (lobix + 3 < hibix)
    {
      while (lobix < hibix && named_buckets_mom[lobix]->nambuck_len == 0)
        lobix++;
      while (hibix > lobix && named_buckets_mom[hibix - 1]->nambuck_len == 0)
        hibix--;
      if (lobix + 2 >= hibix)
        break;
      mdbix = (lobix + hibix) / 2;
      while (mdbix < hibix && named_buckets_mom[mdbix]->nambuck_len == 0)
        mdbix++;
      struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
      unsigned mdlen = mdbuck->nambuck_len;
      assert (mdlen > 0);
      const momitem_t *mditm = mdbuck->nambuck_arr[mdlen / 2];
      assert (mditm && !mditm->itm_anonymous && mditm->itm_name);
      MOM_DEBUGPRINTF (item,
                       "unregister_named_finalized_item %s mdbix %d (lobix=%d hibix=%d), mditm %s",
                       itmnam->cstr, mdbix, lobix, hibix,
                       mditm->itm_name->cstr);
      if (strcmp (itmnam->cstr, mditm->itm_name->cstr) < 0)
        hibix = mdbix + 1;
      else
        lobix = mdbix;
    }
  // try to find the appropriate bucket index bix
  for (mdbix = lobix; mdbix < hibix && bix < 0; mdbix++)
    {
      struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
      unsigned mdlen = mdbuck->nambuck_len;
      MOM_DEBUGPRINTF (item,
                       "unregister_named_finalized_item %s mdbix=%d mdlen=%d",
                       itmnam->cstr, mdbix, mdlen);
      if (mdlen == 0)
        continue;
      const momitem_t *firstitm = mdbuck->nambuck_arr[0];
      assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
      const momitem_t *lastitm = mdbuck->nambuck_arr[mdlen - 1];
      assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
      MOM_DEBUGPRINTF (item,
                       "unregister_named_finalized_item %s mdbix=%d firstitm %s lastitm %s",
                       itmnam->cstr, mdbix, firstitm->itm_name->cstr,
                       lastitm->itm_name->cstr);
      if (strcmp (firstitm->itm_name->cstr, itmnam->cstr) <= 0
          && strcmp (itmnam->cstr, lastitm->itm_name->cstr) <= 0)
        bix = mdbix;
    };
  assert (bix >= 0);
  {
    struct namebucket_mom_st *buck = named_buckets_mom[bix];
    int pos = -1;
    unsigned blen = buck->nambuck_len;
    MOM_DEBUGPRINTF (item,
                     "unregister_named_finalized_item %s blen=%d bix=%d",
                     itmnam->cstr, blen, bix);
    assert (blen > 0);
    int lo = 0, hi = (int) blen, md = 0;
    while (lo + 3 < hi && pos < 0)
      {
        int md = (lo + hi) / 2;
        const momitem_t *curitm = buck->nambuck_arr[md];
        assert (curitm && !curitm->itm_anonymous && curitm->itm_name);
        MOM_DEBUGPRINTF (item,
                         "unregister_named_finalized_item %s lo=%d hi=%d md=%d curitm %s",
                         itmnam->cstr, lo, hi, md, curitm->itm_name->cstr);
        int cmp = strcmp (curitm->itm_name->cstr, itmnam->cstr);
        if (!cmp)
          pos = md;
        else if (cmp < 0)
          lo = md;
        else
          hi = md;
      }
    for (md = lo; md <= hi && pos < 0; md++)
      {
        const momitem_t *curitm = buck->nambuck_arr[md];
        assert (curitm && !curitm->itm_anonymous && curitm->itm_name);
        if (curitm == finitm)
          pos = md;
      };
    if (pos < 0)
      MOM_FATAPRINTF ("failed to find finalized named item %s in bucket#%d",
                      itmnam->cstr, bix);
    for (int ix = pos; ix + 1 < (int) blen; ix++)
      buck->nambuck_arr[ix] = buck->nambuck_arr[ix + 1];
    buck->nambuck_arr[blen - 1] = NULL;
    buck->nambuck_len = blen - 1;
  }
  pthread_rwlock_unlock (&named_lock_mom);
}                               /* end mom_unregister_named_finalized_item */



void
mom_gc_finalize_item (void *itmad, void *data __attribute__ ((unused)))
{
  momitem_t *finitm = itmad;
#warning mom_finalize_item unimplemented
  MOM_DEBUGPRINTF (item, "mom_finalize_item finitm@%p = %s (kind %s)",
                   (void *) finitm, mom_item_cstring (finitm),
                   mom_item_cstring (finitm->itm_kind));
  if (finitm->itm_anonymous)
    mom_unregister_anonymous_finalized_item (finitm);
  else
    mom_unregister_named_finalized_item (finitm);
  MOM_WARNPRINTF ("unimplemented mom_finalize_item %s",
                  mom_item_cstring (finitm));
  pthread_mutex_destroy (&finitm->itm_mtx);
  memset (finitm, 0, sizeof (momitem_t));
}




momitem_t *
mom_make_named_item (const char *namstr)
{
  momitem_t *itm = NULL;
  const char *end = NULL;
  if (!namstr || !mom_valid_item_name_str (namstr, &end) || !end || *end)
    return NULL;
  MOM_DEBUGPRINTF (item, "make_named_item %s", namstr);
  pthread_rwlock_wrlock (&named_lock_mom);
  assert (named_count_mom > 0); /* we have some predefined names! */
  if (MOM_UNLIKELY
      (named_count_mom > 0
       && (5 * named_nbuck_mom / 4 + 1) * (named_nbuck_mom + 1) <
       named_count_mom))
    reorganize_named_items_mom ();
  else if (MOM_UNLIKELY (named_count_mom > 0
                         && (2 * named_nbuck_mom + 1) * (named_nbuck_mom +
                                                         2) >=
                         3 * named_count_mom))
    reorganize_named_items_mom ();
  int lobix = 0, hibix = named_nbuck_mom, mdbix = 0;
  int bix = -1;
  while (lobix + 3 < hibix)
    {
      while (lobix < hibix && named_buckets_mom[lobix]->nambuck_len == 0)
        lobix++;
      while (hibix > lobix && named_buckets_mom[hibix - 1]->nambuck_len == 0)
        hibix--;
      if (lobix + 2 >= hibix)
        break;
      mdbix = (lobix + hibix) / 2;
      while (mdbix < hibix && named_buckets_mom[mdbix]->nambuck_len == 0)
        mdbix++;
      struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
      unsigned mdlen = mdbuck->nambuck_len;
      assert (mdlen > 0);
      const momitem_t *mditm = mdbuck->nambuck_arr[mdlen / 2];
      assert (mditm && !mditm->itm_anonymous && mditm->itm_name);
      MOM_DEBUGPRINTF (item,
                       "make_named_item %s mdbix %d (lobix=%d hibix=%d), mditm %s",
                       namstr, mdbix, lobix, hibix, mditm->itm_name->cstr);
      if (strcmp (namstr, mditm->itm_name->cstr) < 0)
        hibix = mdbix + 1;
      else
        lobix = mdbix;
    }
  MOM_DEBUGPRINTF (item,
                   "make_named_item %s lobix=%d (bucklen %d) hibix=%d (bucklen %d)",
                   namstr, lobix,
                   named_buckets_mom[lobix]
                   ? (named_buckets_mom[lobix]->nambuck_len) : 0, hibix,
                   named_buckets_mom[hibix]
                   ? (named_buckets_mom[hibix]->nambuck_len) : 0);
  // try to find the appropriate bucket index bix
  for (mdbix = lobix; mdbix < hibix && bix < 0; mdbix++)
    {
      struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
      unsigned mdlen = mdbuck->nambuck_len;
      MOM_DEBUGPRINTF (item, "make_named_item %s mdbix=%d mdlen=%d", namstr,
                       mdbix, mdlen);
      if (mdlen == 0)
        continue;
      const momitem_t *firstitm = mdbuck->nambuck_arr[0];
      assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
      const momitem_t *lastitm = mdbuck->nambuck_arr[mdlen - 1];
      assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
      MOM_DEBUGPRINTF (item,
                       "make_named_item %s mdbix=%d firstitm %s lastitm %s",
                       namstr, mdbix, firstitm->itm_name->cstr,
                       lastitm->itm_name->cstr);
      if (strcmp (firstitm->itm_name->cstr, namstr) <= 0
          && strcmp (namstr, lastitm->itm_name->cstr) <= 0)
        bix = mdbix;
      else if (strcmp (namstr, firstitm->itm_name->cstr) < 0)
        {
          struct namebucket_mom_st *prevbuck = NULL;
          if (mdbix > 0)
            prevbuck = named_buckets_mom[mdbix - 1];
          if (prevbuck && prevbuck->nambuck_len < mdlen)
            bix = mdbix - 1;
          else
            bix = mdbix;
        }
    }
  MOM_DEBUGPRINTF (item, "make_named_item %s bix=%d lobix=%d hibix=%d",
                   namstr, bix, lobix, hibix);
  if (bix < 0 && hibix > 0)
    {
      unsigned prevbix = hibix - 1;
      struct namebucket_mom_st *prevbuck = named_buckets_mom[prevbix];
      const momitem_t *prevlastitm = NULL;
      unsigned prevlen = prevbuck->nambuck_len;
      assert (prevlen == 0
              || ((prevlastitm = prevbuck->nambuck_arr[prevlen - 1]) != NULL
                  && !prevlastitm->itm_anonymous
                  && prevlastitm->itm_name
                  && strcmp (prevlastitm->itm_name->cstr, namstr) < 0));
      bix = prevbix;
    }
  int pos = -1;
  MOM_DEBUGPRINTF (item, "make_named_item %s bix=%d", namstr, bix);
  if (bix >= 0)
    {
      struct namebucket_mom_st *buck = named_buckets_mom[bix];
      unsigned blen = buck->nambuck_len;
      MOM_DEBUGPRINTF (item, "make_named_item %s blen=%d", namstr, blen);
      if (blen > 0)
        {
          int lo = 0, hi = (int) blen, md = 0;
          while (lo + 3 < hi)
            {
              int md = (lo + hi) / 2;
              const momitem_t *curitm = buck->nambuck_arr[md];
              assert (curitm && !curitm->itm_anonymous && curitm->itm_name);
              MOM_DEBUGPRINTF (item,
                               "make_named_item %s lo=%d hi=%d md=%d curitm %s",
                               namstr, lo, hi, md, curitm->itm_name->cstr);
              int cmp = strcmp (curitm->itm_name->cstr, namstr);
              if (!cmp)
                {
                  itm = (momitem_t *) curitm;
                  MOM_DEBUGPRINTF (item, "make_named_item %s found @%p",
                                   namstr, (void *) itm);
                  goto end;
                };
              if (cmp < 0)
                lo = md;
              else
                hi = md;
            };
          if (hi >= (int) blen)
            hi = blen - 1;
          MOM_DEBUGPRINTF (item, "make_named_item %s lo=%d hi=%d",
                           namstr, lo, hi);
          for (md = lo; md <= hi; md++)
            {
              const momitem_t *curitm = buck->nambuck_arr[md];
              assert (curitm && !curitm->itm_anonymous && curitm->itm_name);
              MOM_DEBUGPRINTF (item, "make_named_item %s md=%d curitm %s",
                               namstr, md, curitm->itm_name->cstr);
              int cmp = strcmp (curitm->itm_name->cstr, namstr);
              if (!cmp)
                {
                  itm = (momitem_t *) curitm;
                  pos = md;
                  MOM_DEBUGPRINTF (item,
                                   "make_named_item %s found @%p bix=%d pos=%d",
                                   namstr, (void *) itm, bix, pos);
                  goto end;
                };
              if (cmp > 0)
                {
                  pos = md;
                  break;
                }
            };
          if (pos < 0 && blen > 0)
            {
              const momitem_t *lastitm = buck->nambuck_arr[blen - 1];
              assert (lastitm && !lastitm->itm_anonymous
                      && lastitm->itm_name);
              if (strcmp (lastitm->itm_name->cstr, namstr) < 0)
                pos = blen;
            }
        }
      else                      // blen==0
        {
          pos = 0;
        }
    }
  else
    {                           // bix<0
      MOM_DEBUGPRINTF (item, "make_named_item %s bix=%d negative", namstr,
                       bix);
      for (mdbix = lobix; mdbix < (int) named_nbuck_mom - 1 && bix < 0;
           mdbix++)
        {
          MOM_DEBUGPRINTF (item, "make_named_item %s mdbix=%d", namstr,
                           mdbix);
          struct namebucket_mom_st *mdbuck = named_buckets_mom[mdbix];
          struct namebucket_mom_st *nxbuck = named_buckets_mom[mdbix + 1];
          if (mdbuck->nambuck_len == 0 && nxbuck->nambuck_len > 0
              && strcmp (namstr, nxbuck->nambuck_arr[0]->itm_name->cstr) < 0)
            {
              bix = mdbix;
              pos = 0;
              MOM_DEBUGPRINTF (item, "make_named_item %s bix=%d pos=%d",
                               namstr, bix, pos);
            }
          else if (mdbuck->nambuck_len > 0 && nxbuck->nambuck_len == 0
                   && strcmp (mdbuck->nambuck_arr[mdbuck->nambuck_len -
                                                  1]->itm_name->cstr,
                              namstr) < 0)
            {
              bix = mdbix + 1;
              pos = 0;
              MOM_DEBUGPRINTF (item, "make_named_item %s bix=%d pos=%d",
                               namstr, bix, pos);
            }
        }
    };
  MOM_DEBUGPRINTF (item, "make_named_item %s bix=%d pos=%d itm@%p",
                   namstr, bix, pos, itm);
  assert (bix >= 0);
  assert (pos >= 0);
  struct namebucket_mom_st *curbuck = named_buckets_mom[bix];
  unsigned bucklen = curbuck->nambuck_len;
  if (MOM_UNLIKELY
      (bucklen > 3 && (3 + bucklen) * (bucklen + 1) > 3 * named_count_mom))
    reorganize_named_items_mom ();
  if (!itm)
    {
      momitem_t *newitm = MOM_GC_ALLOC ("new named item", sizeof (momitem_t));
      mom_initialize_protoitem (newitm);
      newitm->itm_name = mom_make_string_cstr (namstr);
      newitm->itm_space = momspa_transient;
      newitm->itm_anonymous = false;
      if (pos < (int) bucklen)
        {
          memmove (curbuck->nambuck_arr + pos + 1,
                   curbuck->nambuck_arr + pos,
                   (bucklen - pos) * sizeof (momitem_t *));
        }
      itm = newitm;
      curbuck->nambuck_arr[pos] = itm;
      MOM_DEBUGPRINTF (item, "make_named_item %s pos=%d bix=%d newitm@%p",
                       namstr, pos, bix, newitm);
      bucklen = ++curbuck->nambuck_len;
      named_count_mom++;
      GC_REGISTER_FINALIZER (newitm, mom_gc_finalize_item, NULL, NULL, NULL);
    }
  if (MOM_UNLIKELY
      (bucklen > 2 && (2 + bucklen) * bucklen > 3 * named_count_mom))
    reorganize_named_items_mom ();
end:
  pthread_rwlock_unlock (&named_lock_mom);
  assert (itm != NULL);
  return itm;
}                               /* end mom_make_named_item */



momitem_t *
mom_make_predefined_named_item (const char *namstr)
{
  if (!mom_valid_item_name_str (namstr, NULL))
    MOM_FATAPRINTF ("invalid predefined name %s", namstr);
  momitem_t *pritm = mom_make_named_item (namstr);
  assert (pritm);
  pritm->itm_space = momspa_predefined;
  pthread_mutex_lock (&predefined_mutex_mom);
  predefined_hashset_mom = mom_hashset_put (predefined_hashset_mom, pritm);
  pthread_mutex_unlock (&predefined_mutex_mom);
  return pritm;
}

const momseq_t *
mom_predefined_items_set (void)
{
  const momseq_t *set = NULL;
  pthread_mutex_lock (&predefined_mutex_mom);
  set = mom_hashset_elements_set (predefined_hashset_mom);
  pthread_mutex_unlock (&predefined_mutex_mom);
  return set;
}


int
mom_itemptr_cmp (const void *p1, const void *p2)
{
  const momitem_t *itm1 = *(momitem_t **) p1;
  const momitem_t *itm2 = *(momitem_t **) p2;
  int cmp = mom_item_cmp (itm1, itm2);
  return cmp;
}

void
mom_item_qsort (const momitem_t **arr, unsigned siz)
{
  MOM_DEBUGPRINTF (item, "item_qsort siz=%u", siz);
  if (MOM_UNLIKELY (!siz || siz == 1))
    return;
  if (MOM_UNLIKELY (siz == 2))
    {
      if (mom_item_cmp (arr[0], arr[1]) > 0)
        {
          const momitem_t *itmtmp = arr[0];
          arr[0] = arr[1];
          arr[1] = itmtmp;
        }
      return;
    }
  if (MOM_IS_DEBUGGING (item))
    {
      for (unsigned ix = 0; ix < siz; ix++)
        MOM_DEBUGPRINTF (item, "item_qsort unsorted arr[%d] = %s", ix,
                         mom_item_cstring (arr[ix]));
    }
  MOM_DEBUGPRINTF (item, "item_qsort before qsort siz %u", siz);
  qsort (arr, (size_t) siz, sizeof (momitem_t *), mom_itemptr_cmp);
  MOM_DEBUGPRINTF (item, "item_qsort after qsort siz %u", siz);
  if (MOM_IS_DEBUGGING (item))
    {
      for (unsigned ix = 0; ix < siz; ix++)
        MOM_DEBUGPRINTF (item, "item_qsort qsorted arr[%d] = %s", ix,
                         mom_item_cstring (arr[ix]));
    }
}


bool
mom_unsync_item_set_kind (momitem_t *itm, momitem_t *kinditm)
{
  bool res = false;
  if (!itm || itm == MOM_EMPTY)
    return false;
  if (!kinditm || kinditm == MOM_EMPTY)
    return false;
  if (MOM_UNLIKELY (itm->itm_kind == kinditm))
    return true;
  MOM_DEBUGPRINTF (item, "item_set_kind itm=%s kinditm=%s oldkind=%s",
                   mom_item_cstring (itm), mom_item_cstring (kinditm),
                   mom_item_cstring (itm->itm_kind));
  itm->itm_data1 = NULL;
  itm->itm_data2 = NULL;
  itm->itm_kind = NULL;
  momvalue_t vcons = MOM_NONEV;
  {
    mom_item_lock (kinditm);
    if (kinditm->itm_kind == MOM_PREDEFINED_NAMED (signature))
      {
        momvalue_t vfillclos =  //
          mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_function), 1,
                         kinditm);
        res = mom_applval_1itm_to_void (vfillclos, itm);
        goto endunlocklab;
      }
    vcons =                     //
      mom_item_unsync_get_attribute     //
      (kinditm, MOM_PREDEFINED_NAMED (kind_constructor));
    if (vcons.typnum == momty_node)
      {
        res = mom_applval_1itm_to_void (vcons, itm);
        goto endunlocklab;
      };
    if (kinditm->itm_kind == MOM_PREDEFINED_NAMED (kind))
      {
        itm->itm_kind = kinditm;
        res = true;
        goto endunlocklab;
      };
  endunlocklab:
    mom_item_unlock (kinditm);
  }
  return res;
}                               /* end of mom_unsync_item_set_kind */


// if this function compiles correctly, it means that all predefined items have distinct hashcode.
momitem_t *
mom_predefined_item_of_hash (momhash_t h)
{
  switch (h)
    {
    case 0:
      return NULL;
#define MOM_HAS_PREDEFINED_NAMED(Nam,Hash) case Hash: return MOM_PREDEFINED_NAMED(Nam);
#define MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) case Hash: return MOM_PREDEFINED_ANONYMOUS(Id);
#include "predef-monimelt.h"
    default:
      return NULL;
    }
}
