// file item.c

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
#define MOM_HAS_PREDEFINED_DELIM(Nam,Str)
//
#include "predef-monimelt.h"


/// routine to create the predefined
static void create_predefined_items_mom (void);

//////////////// named items Named items are organized in a 2-level
// tree,i.e. a vector of buckets, with O(sqrt(N)) average complexity
// where N is the number of names. Each bucket is sorted
// (alphanumerically).

static pthread_rwlock_t named_lock_mom = PTHREAD_RWLOCK_INITIALIZER;

struct namebucket_mom_st
{				//// buckets should be scalar zones, the items in them are weak pointers
  unsigned nambuck_size;	/* allocated size of bucket */
  unsigned nambuck_len;		/* used length */
  const momitem_t *nambuck_arr[];
};

static unsigned named_count_mom;	/* total number of named items */
static unsigned named_nbuck_mom;	/* number of buckets */
/// all the buckets are non-nil pointers 
struct namebucket_mom_st **named_buckets_mom;


//////////////// anonymous items
#define ITEM_NUM_SALT_MOM 16

// we choose base 48, because with a 0-9 decimal digit then 8 extended
// digits in base 48 we can express a 48-bit number.  Notice that
// log(2**48/10)/log(48) is 7.9997
#define ID_DIGITS_MOM "0123456789abcdefhijkmnpqrstuvwxyzABCDEFHIJKLMPRU"
#define ID_BASE_MOM 48
// mutex for locking access to item of given salthash
static pthread_mutex_t item_mutex_mom[ITEM_NUM_SALT_MOM];
// cardinal of items of given salthash
static unsigned item_card_mom[ITEM_NUM_SALT_MOM];
// size of nonscanned itemarray of given salthash
static unsigned item_size_mom[ITEM_NUM_SALT_MOM];
// nonscanned array of anonymous item pointers of given salthash
static momitem_t **item_anonarr_mom[ITEM_NUM_SALT_MOM];



static pthread_mutexattr_t mutexattr_recur_item_mom;



////////////////////////////////////////////////////////////////
// convert a 48 bits number to a 10 char string starting with _ then a
// 0-9 digit then 8 extended digits
static const char *
num48_to_char10_mom (uint64_t num, char *buf)
{
  for (int ix = 8; ix > 0; ix--)
    {
      unsigned dig = num % ID_BASE_MOM;
      num = num / ID_BASE_MOM;
      buf[ix + 1] = ID_DIGITS_MOM[dig];
    }
  assert (num <= 9);
  buf[1] = '0' + num;
  buf[0] = '_';
  return buf;
}

uint64_t
char10_to_num48_mom (const char *buf)
{
  uint64_t num = 0;
  if (buf[0] != '_')
    return 0;
  if (buf[1] < '0' || buf[1] > '9')
    return 0;
  for (int ix = 1; ix <= 9; ix++)
    {
      char c = buf[ix];
      char *p = strchr (ID_DIGITS_MOM, c);
      if (!p)
	return 0;
      num = (num * 48 + p - ID_DIGITS_MOM);
    }
  return num;
}

bool
mom_valid_item_id_str (const char *id, const char **pend)
{
  if (pend)
    *pend = NULL;
  if (!id)
    return false;
  for (int ns = 0; ns < 2; ns++)
    {
      if (id[ns * 10 + 0] != '_')
	return false;
      if (id[ns * 10 + 1] < '0' || id[ns * 10 + 1] > '9')
	return false;
      for (int i = 1; i <= 9; i++)
	if (!strchr (ID_DIGITS_MOM, id[ns * 10 + i]))
	  return false;
    }
  if (isalnum (id[20]) || id[20] == '_')
    return false;
  if (pend)
    *pend = id + 20;
  return true;
}


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

#if 0
static void
useless_test_mom (void)
{
  for (int i = 0; i < 4; i++)
    {
      char buf[48];
      memset (buf, 0, sizeof (buf));
      uint64_t n = mom_random_64 (__LINE__) & 0xffffffffffffLL;
      num48_to_char10_mom (n, buf);
      uint64_t u = char10_to_num48_mom (buf);
      printf ("i=%d n=%lld=%#llx buf=%s u=%lld=%#llx\n",
	      i, (long long) n, (long long) n, buf, (long long) u,
	      (long long) u);
      assert (buf[0] && strlen (buf) == 10);
    }
  {
    unsigned salt = getpid () % 8;
    printf ("salt %u\n", salt);
    for (int i = 0; i < 8; i++)
      {
	const momstring_t *str = mom_make_random_idstr (salt, NULL);
	assert (str != NULL);
	printf ("i=%d str:%s hash=%u\n", i, str->cstr, str->shash);
	assert (mom_valid_item_id_str (str->cstr, NULL));
      }
  }
  {
    momitem_t *anitm1 = mom_make_anonymous_item ();
    momitem_t *anitm2 = mom_make_anonymous_item ();
    printf ("anitm1@%p id %s h %u=%#x\n",
	    anitm1, anitm1->itm_id->cstr, anitm1->itm_id->shash,
	    anitm1->itm_id->shash);
    printf ("anitm2@%p id %s h %u=%#x\n", anitm2, anitm2->itm_id->cstr,
	    anitm2->itm_id->shash, anitm2->itm_id->shash);
  }
}
#endif /* 0 */

void
mom_initialize_items (void)
{
  pthread_mutexattr_init (&mutexattr_recur_item_mom);
  pthread_mutexattr_settype (&mutexattr_recur_item_mom,
			     PTHREAD_MUTEX_RECURSIVE);
  static const pthread_mutex_t inimtx = PTHREAD_MUTEX_INITIALIZER;
  for (int ix = 0; ix < ITEM_NUM_SALT_MOM; ix++)
    memcpy (item_mutex_mom + ix, &inimtx, sizeof (pthread_mutex_t));
  static_assert (sizeof (ID_DIGITS_MOM) - 1 == ID_BASE_MOM,
		 "invalid number of id digits");
  /// create the predefined items
  create_predefined_items_mom ();
}

// if a ppos is given, we want to add a new item
static int
find_anonitem_position_mom (unsigned hash, const char *idbuf, int *ptrpos)
{
  if (MOM_UNLIKELY (!hash))
    hash = mom_cstring_hash (idbuf);
  unsigned hrk = hash % ITEM_NUM_SALT_MOM;
  int foundix = -1;
  int pos = -1;
  if (ptrpos)
    *ptrpos = -1;
  momitem_t **arr = item_anonarr_mom[hrk];
  if (arr)
    {
      unsigned sz = item_size_mom[hrk];
      assert (sz > 0);
      unsigned startix = hash % sz;
      for (unsigned ix = startix; ix < sz && foundix < 0; ix++)
	{
	  momitem_t *curitm = arr[ix];
	  if (!curitm)
	    {
	      if (ptrpos && pos < 0)
		pos = ix;
	      break;
	    }
	  else if (curitm == MOM_EMPTY)
	    {
	      if (ptrpos && pos < 0)
		pos = ix;
	      continue;
	    }
	  const momstring_t *curid = curitm->itm_id;
	  if (curid->shash == hash && !strcmp (curid->cstr, idbuf))
	    foundix = (int) ix;
	}
      for (unsigned ix = 0; ix < startix && foundix < 0; ix++)
	{
	  momitem_t *curitm = arr[ix];
	  if (!curitm)
	    {
	      if (ptrpos && pos < 0)
		pos = ix;
	      break;
	    }
	  else if (curitm == MOM_EMPTY)
	    {
	      if (ptrpos && pos < 0)
		pos = ix;
	      continue;
	    }
	  const momstring_t *curid = curitm->itm_id;
	  if (curid->shash == hash && !strcmp (curid->cstr, idbuf))
	    foundix = (int) ix;
	}
    }
  if (ptrpos && pos >= 0)
    *ptrpos = pos;
  return foundix;
}

static int
add_anonitem_mom (momitem_t *itm, unsigned salthash)
{
  int pos = -1;
  int foundix = -1;
  assert (itm != NULL);
  assert (salthash < ITEM_NUM_SALT_MOM);
  momhash_t ih = itm->itm_id->shash;
  assert (ih % ITEM_NUM_SALT_MOM == salthash);
  unsigned sz = item_size_mom[salthash];
  assert (sz > 0);
  momitem_t **arr = item_anonarr_mom[salthash];
  assert (arr != NULL);
  unsigned startix = ih % sz;
  for (unsigned ix = startix; ix < sz && foundix < 0; ix++)
    {
      momitem_t *curitm = arr[ix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = ix;
	  foundix = pos;
	  break;
	}
      else if (curitm == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = ix;
	  continue;
	}
      if (curitm == itm)
	{
	  foundix = ix;
	  pos = -1;
	};
      const momstring_t *curid = curitm->itm_id;
      assert (curid->shash != ih || strcmp (curid->cstr, itm->itm_id->cstr));
    }
  for (unsigned ix = 0; ix < startix && foundix < 0; ix++)
    {
      momitem_t *curitm = arr[ix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = ix;
	  foundix = pos;
	  break;
	}
      else if (curitm == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = ix;
	  continue;
	}
      if (curitm == itm)
	{
	  foundix = ix;
	  pos = -1;
	};
      const momstring_t *curid = curitm->itm_id;
      assert (curid->shash != ih || strcmp (curid->cstr, itm->itm_id->cstr));
    }
  if (pos > 0)
    {
      arr[pos] = itm;
      item_card_mom[salthash]++;
    };
  return foundix;
}

static void
reorganize_item_bucket_mom (unsigned salthash)
{
  assert (salthash < ITEM_NUM_SALT_MOM);
  momitem_t **oldarr = item_anonarr_mom[salthash];
  unsigned oldsiz = item_size_mom[salthash];
  unsigned oldcard = item_card_mom[salthash];
  unsigned newsiz = ((3 * oldcard / 2 + 20) | 0x1f) + 1;
  momitem_t **newarr =
    MOM_GC_SCALAR_ALLOC ("item bucket", newsiz * sizeof (momitem_t *));
  item_anonarr_mom[salthash] = newarr;
  item_size_mom[salthash] = newsiz;
  item_card_mom[salthash] = 0;
  for (unsigned ix = 0; ix < oldsiz; ix++)
    {
      momitem_t *curitm = oldarr[ix];
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      int ix = add_anonitem_mom (curitm, salthash);
      if (MOM_UNLIKELY (ix < 0))
	MOM_FATAPRINTF ("corrupted item bucket salthash=%d", salthash);
    }
  assert (item_card_mom[salthash] == oldcard);
  if (MOM_LIKELY (oldarr && oldsiz > 0))
    {
      memset (oldarr, 0, oldsiz * sizeof (momitem_t *));
      MOM_GC_FREE (oldarr);
    }
}

const momstring_t *
mom_make_random_idstr (unsigned salt, struct momitem_st *protoitem)
{
  const momstring_t *str = NULL;
  assert (!protoitem || protoitem->itm_id == NULL);
  do
    {
      uint32_t r1 = 0, r2 = 0, r3 = 0;
      uint64_t hi = 0, lo = 0;	/* actually 48 bits unsigned each */
      mom_random_three_nonzero_32 (salt, &r1, &r2, &r3);
      hi = ((uint64_t) r1) << 32 | (uint64_t) (r2 >> 16);
      lo = (((uint64_t) (r2 & 0xffff)) << 32) | ((uint64_t) r3);
      // replace the four highest bits of 48 bits number hi with salt&0xf
      hi &= (((uint64_t) 1ULL << 44) - 1);
      hi |= ((uint64_t) (salt & 0xf) << 44);
      if (hi == 0 || lo == 0)
	continue;
      char buf1[16], buf2[16], bufstr[32];
#define NUM48LEN_MOM 10
      memset (buf1, 0, sizeof (buf1));
      memset (buf2, 0, sizeof (buf2));
      const char *p1 = num48_to_char10_mom (hi, buf1);
      const char *p2 = num48_to_char10_mom (lo, buf2);
      if (MOM_UNLIKELY (!p1 || strlen (p1) != NUM48LEN_MOM))
	MOM_FATAPRINTF ("bad high number %lld", (long long) hi);
      if (MOM_UNLIKELY (!p2 || strlen (p2) != NUM48LEN_MOM))
	MOM_FATAPRINTF ("bad low number %lld", (long long) lo);
      memset (bufstr, 0, sizeof (bufstr));
      memcpy (bufstr, p1, NUM48LEN_MOM);
      memcpy (bufstr + NUM48LEN_MOM, p2, NUM48LEN_MOM);
      momhash_t hs = mom_cstring_hash (bufstr);
      unsigned hrk = hs % ITEM_NUM_SALT_MOM;
      int foundix = -1;
      int pos = -1;
      pthread_mutex_lock (item_mutex_mom + hrk);
      foundix =
	find_anonitem_position_mom (hs, bufstr, protoitem ? (&pos) : NULL);
      if (MOM_LIKELY (protoitem))
	{			// add the new item
	  int newix = -1;
	  if (pos >= 0)
	    {
	      item_anonarr_mom[hrk][pos] = protoitem;
	      newix = pos;
	    }
	  str = mom_make_string (bufstr);
	  assert (!protoitem->itm_id);
	  protoitem->itm_id = str;
	  unsigned bsiz = item_size_mom[hrk];
	  unsigned bcard = item_card_mom[hrk];
	  assert (bsiz <= bcard);
	  if (!bsiz || 9 * bcard / 8 + 2 <= bsiz)
	    reorganize_item_bucket_mom (hrk);
	  if (newix < 0)
	    newix = add_anonitem_mom (protoitem, hrk);
	  if (MOM_UNLIKELY (newix < 0))
	    MOM_FATAPRINTF ("failed to add item in salthash bucket %u", hrk);
	}
      if (foundix < 0 && !protoitem)
	str = mom_make_string (bufstr);
      pthread_mutex_unlock (item_mutex_mom + hrk);
      if (foundix >= 0)
	continue;
    }
  while (!str);
  return str;
}

// if insertpix is given, we want some insertion and we get the bucket index inside it
static struct namebucket_mom_st *
find_named_bucket_mom (const char *name, int *insertpix)
{
  int lo = 0, hi = (int) named_nbuck_mom, md = 0;
  if (insertpix)
    *insertpix = -1;
  while (lo + 4 < hi)
    {
      bool hit = false;
      for (md = (lo + hi) / 2; md < hi; md++)
	{
	  struct namebucket_mom_st *curbuck = named_buckets_mom[md];
	  assert (curbuck);
	  unsigned blen = curbuck->nambuck_len;
	  if (blen == 0)
	    continue;
	  const momitem_t *firstitm = curbuck->nambuck_arr[0];
	  assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
	  int cmpfirstname = strcmp (name, firstitm->itm_name->cstr);
	  if (cmpfirstname < 0)
	    {
	      hi = md;
	      hit = true;
	      break;
	    }
	  else if (cmpfirstname == 0)
	    return curbuck;
	  else
	    {
	      const momitem_t *lastitm = curbuck->nambuck_arr[blen - 1];
	      assert (lastitm && !lastitm->itm_anonymous
		      && lastitm->itm_name);
	      int cmplastname = strcmp (name, lastitm->itm_name->cstr);
	      if (cmplastname < 0)
		{
		  lo = md;
		  hit = true;
		  break;
		}
	      else
		return curbuck;
	    }
	};
      if (!hit)
	break;
    };
  for (md = lo; md < hi; md++)
    {
      struct namebucket_mom_st *curbuck = named_buckets_mom[md];
      assert (curbuck);
      unsigned blen = curbuck->nambuck_len;
      if (blen == 0)
	{
	  if (md == hi - 1 && insertpix)
	    {
	      *insertpix = md;
	      return curbuck;
	    }
	  else
	    continue;
	};
      const momitem_t *firstitm = curbuck->nambuck_arr[0];
      assert (firstitm && !firstitm->itm_anonymous && firstitm->itm_name);
      int cmpfirstname = strcmp (name, firstitm->itm_name->cstr);
      if (cmpfirstname < 0)
	{
	  if (!insertpix)
	    return NULL;
	  if (md > 0)
	    {
	      struct namebucket_mom_st *prevbuck = named_buckets_mom[md - 1];
	      if (prevbuck->nambuck_len < curbuck->nambuck_len)
		{
		  *insertpix = md - 1;
		  return prevbuck;
		}
	      else
		{
		  *insertpix = md;
		  return curbuck;
		}
	    }
	  else
	    {
	      *insertpix = md;
	      return curbuck;
	    }
	}
      else if (cmpfirstname == 0)
	return curbuck;
      const momitem_t *lastitm = curbuck->nambuck_arr[blen - 1];
      assert (lastitm && !lastitm->itm_anonymous && lastitm->itm_name);
      int cmplastname = strcmp (name, lastitm->itm_name->cstr);
      if (cmplastname <= 0)
	return curbuck;
    }
  return NULL;
}				/* end find_named_bucket_mom */

static int
index_in_bucket_mom (const struct namebucket_mom_st *buck, const char *name,
		     bool insert)
{
  assert (buck != NULL);
  unsigned blen = buck->nambuck_len;
  int lo = 0, hi = (int) blen, md = 0;
  while (lo + 4 < hi)
    {
      md = (lo + hi) / 2;
      const momitem_t *miditm = buck->nambuck_arr[md];
      assert (miditm && !miditm->itm_anonymous && miditm->itm_name);
      int cmpname = strcmp (name, miditm->itm_name->cstr);
      if (!cmpname)
	return md;
      else if (cmpname < 0)
	hi = md;
      else
	lo = md;
    }
  for (md = lo; md < hi; md++)
    {
      const momitem_t *miditm = buck->nambuck_arr[md];
      assert (miditm && !miditm->itm_anonymous && miditm->itm_name);
      int cmpname = strcmp (name, miditm->itm_name->cstr);
      if (0 == cmpname)
	return md;
      else if (cmpname < 0)
	continue;
      else if (insert)
	return md;
    }
  if (insert)
    return blen;
  return -1;
}




momitem_t *
mom_find_named_item (const char *name)
{
  const momitem_t *itm = NULL;
  const char *endname = NULL;
  if (!name || !mom_valid_item_name_str (name, &endname)
      || !endname || *endname != '\0')
    return NULL;
  pthread_rwlock_rdlock (&named_lock_mom);
  struct namebucket_mom_st *curbuck = find_named_bucket_mom (name, NULL);
  if (curbuck)
    {
      int bix = index_in_bucket_mom (curbuck, name, false);
      if (bix >= 0 && bix < (int) curbuck->nambuck_len)
	itm = curbuck->nambuck_arr[bix];
    }
  pthread_rwlock_unlock (&named_lock_mom);
  return (momitem_t *) itm;
}

// reorganize all the named items, should be called under the write lock
static void
reorganize_named_items_mom (void)
{
  const momitem_t **allnamedarr = NULL;
  allnamedarr =
    MOM_GC_ALLOC ("allnamedarr",
		  sizeof (momitem_t *) * (named_count_mom + 2));
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
	    }
	}
      memset (curbuck, 0,
	      sizeof (struct namebucket_mom_st) +
	      bsiz * sizeof (momitem_t *));
      named_buckets_mom[bix] = NULL;
      MOM_GC_FREE (curbuck);
    }
  MOM_GC_FREE (named_buckets_mom);
  named_buckets_mom = NULL;
  named_nbuck_mom = 0;
  unsigned newnbuck = (int) (1.2 * sqrt (namecount)) + 10;
  unsigned newblen = namecount / newnbuck;
  if (MOM_UNLIKELY (newblen < 2))
    newblen++;
  unsigned newbsiz = ((3 * newblen / 2 + 5) | 0xf) + 1;
  unsigned namix = 0;
  named_buckets_mom =
    MOM_GC_ALLOC ("named buckets",
		  newnbuck * sizeof (struct namebucket_mom_st *));
  // second loop to fill the buckets
  for (unsigned bix = 0; bix < newnbuck; bix++)
    {
      struct namebucket_mom_st *curbuck =
	MOM_GC_SCALAR_ALLOC ("bucket", sizeof (struct namebucket_mom_st)
			     + newbsiz * sizeof (momitem_t *));
      curbuck->nambuck_size = newbsiz;
      unsigned itmix = 0;
      if (MOM_LIKELY (namix < namecount))
	for (itmix = 0; itmix < newblen; itmix++)
	  {
	    if (MOM_UNLIKELY (namix >= namecount))
	      break;
	    curbuck->nambuck_arr[itmix] = allnamedarr[namecount++];
	  };
      curbuck->nambuck_len = itmix;
      named_buckets_mom[bix] = curbuck;
    }
  assert (namix >= namecount);
  named_nbuck_mom = newnbuck;
  named_count_mom = namecount;
}


static void
create_predefined_items_mom (void)
{
  int nbnamed = 0, nbanon = 0;
  {
    pthread_rwlock_wrlock (&named_lock_mom);
    reorganize_named_items_mom ();	// to initialize the buckets
    assert (named_nbuck_mom > 0);
    pthread_rwlock_unlock (&named_lock_mom);
  }
#define  MOM_HAS_PREDEFINED_NAMED(Nam,Hash) do {	\
    mompi_##Nam = mom_make_named_item(#Nam);		\
    assert (mompi_##Nam->itm_name->shash == Hash);      \
    mompi_##Nam->itm_space = momspa_predefined;		\
    nbnamed++;						\
  } while(0);
  //
#define  MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) do {	\
    mompi_##Id = mom_make_anonymous_item_by_id(#Id);	\
    assert (mompi_##Id->itm_id->shash == Hash);         \
    mompi_##Id->itm_space = momspa_predefined;		\
    nbanon++;						\
  } while(0);
  //
#define MOM_HAS_PREDEFINED_DELIM(Nam,Str)
  //
#include "predef-monimelt.h"
  {
    pthread_rwlock_wrlock (&named_lock_mom);
    reorganize_named_items_mom ();
    pthread_rwlock_unlock (&named_lock_mom);
  }
  MOM_INFORMPRINTF ("created %d predefined named & %d predefined anonymous",
		    nbnamed, nbanon);
}



momitem_t *
mom_find_item (const char *str)
{
  const momitem_t *itm = NULL;
  const char *end = NULL;
  if (!str || !str[0])
    return NULL;
  if (isalpha (str[0]) && mom_valid_item_name_str (str, &end) && end && *end)
    {
      pthread_rwlock_rdlock (&named_lock_mom);
      struct namebucket_mom_st *curbuck = find_named_bucket_mom (str, NULL);
      if (curbuck)
	{
	  int bix = index_in_bucket_mom (curbuck, str, false);
	  if (bix >= 0 && bix < (int) curbuck->nambuck_len)
	    itm = curbuck->nambuck_arr[bix];
	}
      pthread_rwlock_unlock (&named_lock_mom);
      return (momitem_t *) itm;
    }
  else if (str[0] == '_' && mom_valid_item_id_str (str, &end) && end && !*end)
    {
      momhash_t hstr = mom_cstring_hash (str);
      unsigned hrk = hstr % ITEM_NUM_SALT_MOM;
      int foundix = -1;
      pthread_mutex_lock (item_mutex_mom + hrk);
      foundix = find_anonitem_position_mom (hstr, str, NULL);
      if (foundix >= 0)
	itm = item_anonarr_mom[hrk][foundix];
      pthread_mutex_unlock (item_mutex_mom + hrk);
      return (momitem_t *) itm;
    }
  return NULL;
}

static void
initialize_protoitem_mom (momitem_t *protoitm)
{
  pthread_mutex_init (&protoitm->itm_mtx, &mutexattr_recur_item_mom);
}

static void
finalize_item_mom (void *itmad, void *data __attribute__ ((unused)))
{
  momitem_t *itm = (momitem_t *) itmad;
  MOM_INFORMPRINTF ("finalizing item@%p %s", itm, itm->itm_str->cstr);
#warning finalize_item_mom incomplete
}

momitem_t *
mom_make_anonymous_item_salt (unsigned salt)
{
  momitem_t *newitm = MOM_GC_ALLOC ("new anonymous item", sizeof (momitem_t));
  initialize_protoitem_mom (newitm);
  newitm->itm_anonymous = true;
  const momstring_t *ids = mom_make_random_idstr (salt, newitm);
  newitm->itm_id = ids;
  GC_REGISTER_FINALIZER (newitm, finalize_item_mom, NULL, NULL, NULL);
  return newitm;
}

momitem_t *
mom_make_anonymous_item_by_id (const char *ids)
{
  momitem_t *itm = NULL;
  const char *end = NULL;
  if (!ids || !mom_valid_item_id_str (ids, &end) || !end || *end)
    return NULL;
  momhash_t hstr = mom_cstring_hash (ids);
  unsigned hrk = hstr % ITEM_NUM_SALT_MOM;
  int foundix = -1;
  pthread_mutex_lock (item_mutex_mom + hrk);
  foundix = find_anonitem_position_mom (hstr, ids, NULL);
  if (foundix >= 0)
    itm = item_anonarr_mom[hrk][foundix];
  else
    {
      unsigned bsiz = item_size_mom[hrk];
      unsigned bcard = item_card_mom[hrk];
      assert (bsiz <= bcard);
      if (!bsiz || 9 * bcard / 8 + 2 <= bsiz)
	reorganize_item_bucket_mom (hrk);
      const momstring_t *idstr = mom_make_string (ids);
      momitem_t *protoitm =
	MOM_GC_ALLOC ("new anonymous item", sizeof (momitem_t));
      initialize_protoitem_mom (protoitm);
      protoitm->itm_id = idstr;
      protoitm->itm_anonymous = true;
      int newix = add_anonitem_mom (protoitm, hrk);
      if (MOM_UNLIKELY (newix < 0))
	MOM_FATAPRINTF ("failed to add anonitem %s in salthash bucket %u",
			ids, hrk);
      GC_REGISTER_FINALIZER (protoitm, finalize_item_mom, NULL, NULL, NULL);
      itm = protoitm;
    }
  pthread_mutex_unlock (item_mutex_mom + hrk);
  return (momitem_t *) itm;
}

momitem_t *
mom_make_named_item (const char *namstr)
{
  momitem_t *itm = NULL;
  const char *end = NULL;
  if (!namstr || !mom_valid_item_name_str (namstr, &end) || !end || *end)
    return NULL;
  pthread_rwlock_wrlock (&named_lock_mom);
  if (MOM_UNLIKELY
      (named_count_mom > 0
       && (5 * named_nbuck_mom / 4 + 2) * (named_nbuck_mom + 3) >
       named_count_mom))
    reorganize_named_items_mom ();
  int bix = -1;
  struct namebucket_mom_st *curbuck = find_named_bucket_mom (namstr, &bix);
  assert (curbuck != NULL);
  assert (bix >= 0 && bix < (int) named_nbuck_mom
	  && named_buckets_mom[bix] == curbuck);
  unsigned bucklen = curbuck->nambuck_len;
  if (bucklen + 3 >= curbuck->nambuck_size)
    {
      unsigned newbucksize = ((3 * bucklen / 2 + 10) | 0xf) + 1;
      struct namebucket_mom_st *newbuck	//
	= MOM_GC_ALLOC ("new name bucket",
			sizeof (struct namebucket_mom_st) +
			newbucksize * sizeof (momitem_t *));
      newbuck->nambuck_size = newbucksize;
      memcpy (newbuck->nambuck_arr, curbuck->nambuck_arr,
	      bucklen * sizeof (momitem_t *));
      newbuck->nambuck_len = bucklen;
      memset (curbuck, 0,
	      sizeof (struct namebucket_mom_st) +
	      bucklen * sizeof (momitem_t *));
      MOM_GC_FREE (curbuck);
      curbuck = named_buckets_mom[bix] = newbuck;
    }
  int nix = index_in_bucket_mom (curbuck, namstr, true);
  assert (nix >= 0);
  {
    const momitem_t *olditm = curbuck->nambuck_arr[nix];
    if (olditm && !strcmp (olditm->itm_name->cstr, namstr))
      itm = (momitem_t *) olditm;
  }
  if (!itm)
    {
      momitem_t *newitm = MOM_GC_ALLOC ("new named item", sizeof (momitem_t));
      initialize_protoitem_mom (newitm);
      newitm->itm_name = mom_make_string (namstr);
      newitm->itm_anonymous = false;
      if (nix < (int) bucklen)
	{
	  memmove (curbuck->nambuck_arr + nix + 1, curbuck->nambuck_arr + nix,
		   (bucklen - nix) * sizeof (momitem_t *));
	  bucklen = ++curbuck->nambuck_len;
	}
      itm = newitm;
      GC_REGISTER_FINALIZER (newitm, finalize_item_mom, NULL, NULL, NULL);
    }
  if (MOM_UNLIKELY
      (bucklen > 2 && (2 + bucklen) * bucklen > 3 * named_count_mom))
    reorganize_named_items_mom ();
  pthread_rwlock_unlock (&named_lock_mom);
  assert (itm != NULL);
  return itm;
}

int
mom_itemptr_cmp (const void *p1, const void *p2)
{
  return mom_item_cmp (*(momitem_t **) p1, *(momitem_t **) p2);
}

void
mom_item_qsort (const momitem_t **arr, unsigned siz)
{
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
  qsort (arr, siz, sizeof (momitem_t *), mom_itemptr_cmp);
}
