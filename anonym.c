// file anonym.c - handle anonymous items

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

#define HASH_ANON_MOD_MOM 53
static pthread_mutex_t mtx_anon_mom[HASH_ANON_MOD_MOM];
struct anon_htable_mom_st
{
  unsigned anh_size;
  unsigned anh_count;
  momitem_t *anh_arritm[];
};
static struct anon_htable_mom_st *anh_arr_mom[HASH_ANON_MOD_MOM];


// we choose base 48, because with a 0-9 decimal digit then 8 extended
// digits in base 48 we can express a 48-bit number.  Notice that
// log(2**48/10)/log(48) is 7.9997
#define ID_DIGITS_MOM "0123456789abcdefhijkmnpqrstuvwxyzABCDEFHIJKLMPRU"
#define ID_BASE_MOM 48

static pthread_mutexattr_t anomtxattr_mom;
#define ANH_INITIAL_SIZE_MOM 32
// called from mom_initialize_items in items.c
void
mom_initialize_anonymous_items (void)
{
  static_assert (sizeof (ID_DIGITS_MOM) - 1 == ID_BASE_MOM,
		 "invalid number of id digits");
  pthread_mutexattr_init (&anomtxattr_mom);
  pthread_mutexattr_settype (&anomtxattr_mom, PTHREAD_MUTEX_RECURSIVE);
  for (int ix = 0; ix < HASH_ANON_MOD_MOM; ix++)
    {
      pthread_mutex_init (&mtx_anon_mom[ix], &anomtxattr_mom);
      unsigned siz = ANH_INITIAL_SIZE_MOM;
      struct anon_htable_mom_st *an =	//
	MOM_GC_SCALAR_ALLOC ("anh",
			     sizeof (struct anon_htable_mom_st) +
			     siz * sizeof (momitem_t *));
      an->anh_size = siz;
      anh_arr_mom[ix] = an;
    }
  MOM_DEBUGPRINTF (item, "end mom_initialize_anonymous_items");
}				/* end mom_initialize_anonymous_items */


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


static momitem_t *
find_anonymous_of_id_mom (const char *idstr, momhash_t h)
{
  momitem_t *itm = NULL;
  if (!h)
    h = mom_cstring_hash (idstr);
  unsigned hix = h % HASH_ANON_MOD_MOM;
  pthread_mutex_lock (&mtx_anon_mom[hix]);
  struct anon_htable_mom_st *anh = anh_arr_mom[hix];
  assert (anh && anh->anh_size > 0 && anh->anh_count < anh->anh_size);
  unsigned hsiz = anh->anh_size;
  unsigned startix = h % hsiz;
  for (unsigned ix = startix; ix < hsiz && !itm; ix++)
    {
      momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm)
	break;
      if (curitm == MOM_EMPTY)
	continue;
      assert (curitm->itm_anonymous && curitm->itm_id);
      if (!strcmp (curitm->itm_id->cstr, idstr))
	itm = curitm;
    }
  for (unsigned ix = 0; ix < startix && !itm; ix++)
    {
      momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm)
	break;
      if (curitm == MOM_EMPTY)
	continue;
      assert (curitm->itm_anonymous && curitm->itm_id);
      if (!strcmp (curitm->itm_id->cstr, idstr))
	itm = curitm;
    }
  pthread_mutex_unlock (&mtx_anon_mom[hix]);
  return itm;
}				/* end of find_anonymous_of_id_mom */

static inline void
raw_add_anon_item_mom (momitem_t *itm, unsigned hrk)
{
  assert (itm && itm->itm_id && itm->itm_anonymous);
  assert (hrk < HASH_ANON_MOD_MOM);
  unsigned h = itm->itm_id->shash;
  assert (h % HASH_ANON_MOD_MOM == hrk);
  struct anon_htable_mom_st *anh = anh_arr_mom[hrk];
  assert (anh);
  unsigned hsiz = anh->anh_size;
  assert (hsiz > 2 && anh->anh_count < hsiz);
  unsigned startix = h % hsiz;
  int pos = -1;
  for (unsigned ix = startix; ix < hsiz; ix++)
    {
      momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = (int) ix;
	  break;
	}
      else if (curitm == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = (int) ix;
	}
      else if (curitm == itm)
	return;
    }
  for (unsigned ix = 0; ix < startix; ix++)
    {
      momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm)
	{
	  if (pos < 0)
	    pos = (int) ix;
	  break;
	}
      else if (curitm == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = (int) ix;
	}
      else if (curitm == itm)
	return;
    }
  assert (pos >= 0);
  anh->anh_arritm[pos] = itm;
  anh->anh_count++;
}

static void
reorganize_anon_bucket_mom (unsigned hrk)
{
  assert (hrk < HASH_ANON_MOD_MOM);
  struct anon_htable_mom_st *anh = anh_arr_mom[hrk];
  assert (anh);
  unsigned hsiz = anh->anh_size;
  unsigned bcnt = anh->anh_count;
  unsigned newsiz = ((6 * bcnt / 5 + 7) | 0x1f) + 1;
  if (newsiz == hsiz)
    return;
  struct anon_htable_mom_st *newbuck =	//
    MOM_GC_SCALAR_ALLOC ("newanbuck", sizeof (struct anon_htable_mom_st)
			 + newsiz * sizeof (momitem_t *));
  newbuck->anh_size = newsiz;
  anh_arr_mom[hrk] = newbuck;
  for (unsigned ix = 0; ix < hsiz; ix++)
    {
      const momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      raw_add_anon_item_mom ((momitem_t *) curitm, hrk);
    }
  MOM_GC_FREE (anh, sizeof (struct anon_htable_mom_st)
	       + hsiz * sizeof (momitem_t *));
  assert (newbuck->anh_count == bcnt);
}				/* end of reorganize_anon_bucket_mom */


const momitem_t *
mom_find_anonymous_item (const char *idstr)
{
  return find_anonymous_of_id_mom (idstr, 0);
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
      if (hi == 0 || lo == 0)
	continue;
      char buf1[16], buf2[16], bufstr[32];
#define NUM48LEN_MOM 10
      memset (buf1, 0, sizeof (buf1));
      memset (buf2, 0, sizeof (buf2));
      memset (bufstr, 0, sizeof (bufstr));
      const char *p1 = num48_to_char10_mom (hi, buf1);
      const char *p2 = num48_to_char10_mom (lo, buf2);
      if (MOM_UNLIKELY (!p1 || strlen (p1) != NUM48LEN_MOM))
	MOM_FATAPRINTF ("bad high number %lld", (long long) hi);
      if (MOM_UNLIKELY (!p2 || strlen (p2) != NUM48LEN_MOM))
	MOM_FATAPRINTF ("bad low number %lld", (long long) lo);
      memcpy (bufstr, p1, NUM48LEN_MOM);
      memcpy (bufstr + NUM48LEN_MOM, p2, NUM48LEN_MOM);
      momhash_t hs = mom_cstring_hash (bufstr);
      if (MOM_UNLIKELY (NULL != find_anonymous_of_id_mom (bufstr, hs)))
	continue;
      str = mom_make_string_cstr (bufstr);
      if (protoitem)
	{
	  unsigned hrk = hs % HASH_ANON_MOD_MOM;
	  pthread_mutex_lock (&mtx_anon_mom[hrk]);
	  protoitem->itm_id = str;
	  protoitem->itm_anonymous = true;
	  struct anon_htable_mom_st *anh = anh_arr_mom[hrk];
	  assert (anh != NULL);
	  if (anh->anh_count + anh->anh_count / 8 + 2 >= anh->anh_size)
	    reorganize_anon_bucket_mom (hrk);
	  raw_add_anon_item_mom (protoitem, hrk);
	  pthread_mutex_unlock (&mtx_anon_mom[hrk]);
	}
    }
  while (!str);
  return str;
}				/* end mom_make_random_idstr  */



momitem_t *
mom_make_anonymous_item_salt (unsigned salt)
{
  momitem_t *newitm = MOM_GC_ALLOC ("new anonymous item", sizeof (momitem_t));
  mom_initialize_protoitem (newitm);
  newitm->itm_anonymous = true;
  newitm->itm_space = momspa_transient;
  const momstring_t *ids = mom_make_random_idstr (salt, newitm);
  newitm->itm_id = ids;
  GC_REGISTER_FINALIZER (newitm, mom_gc_finalize_item, NULL, NULL, NULL);
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
  unsigned hrk = hstr % HASH_ANON_MOD_MOM;
  MOM_DEBUGPRINTF (item, "make_anonymous_item_by_id ids=%s hstr=%u hrk=%u",
		   ids, (unsigned) hstr, hrk);
  pthread_mutex_lock (&mtx_anon_mom[hrk]);
  itm = find_anonymous_of_id_mom (ids, hrk);
  if (!itm)
    {
      momitem_t *newitm =
	MOM_GC_ALLOC ("new anonymous item", sizeof (momitem_t));
      mom_initialize_protoitem (newitm);
      newitm->itm_anonymous = true;
      newitm->itm_space = momspa_transient;
      newitm->itm_id = mom_make_string_cstr (ids);
      itm = newitm;
      struct anon_htable_mom_st *anh = anh_arr_mom[hrk];
      assert (anh != NULL);
      if (anh->anh_count + anh->anh_count / 8 + 2 >= anh->anh_size)
	reorganize_anon_bucket_mom (hrk);
      raw_add_anon_item_mom (itm, hrk);
      GC_REGISTER_FINALIZER (newitm, mom_gc_finalize_item, NULL, NULL, NULL);
    }
  pthread_mutex_unlock (&mtx_anon_mom[hrk]);
  MOM_DEBUGPRINTF (item, "make_anonymous_item_by_id ids=%s itm@%p", ids,
		   (void *) itm);
  return (momitem_t *) itm;
}

void
mom_unregister_anonymous_finalized_item (momitem_t *finitm)
{
  assert (finitm && finitm->itm_anonymous && finitm->itm_id);
  momhash_t h = finitm->itm_id->shash;
  unsigned hrk = h % HASH_ANON_MOD_MOM;
  MOM_DEBUGPRINTF (item,
		   "unregister_anonymous_finalized_item finitm %s h=%u hrk=%u",
		   mom_item_cstring (finitm), (unsigned) h, (unsigned) hrk);
  int pos = -1;
  pthread_mutex_lock (&mtx_anon_mom[hrk]);
  struct anon_htable_mom_st *anh = anh_arr_mom[hrk];
  assert (anh != NULL);
  assert (anh && anh->anh_size > 0 && anh->anh_count < anh->anh_size);
  unsigned hsiz = anh->anh_size;
  unsigned startix = h % hsiz;
  for (unsigned ix = startix; ix < hsiz && pos < 0; ix++)
    {
      momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm)
	break;
      if (curitm == MOM_EMPTY)
	continue;
      if (curitm == finitm)
	pos = (int) ix;
    };
  for (unsigned ix = 0; ix < startix && pos < 0; ix++)
    {
      momitem_t *curitm = anh->anh_arritm[ix];
      if (!curitm)
	break;
      if (curitm == MOM_EMPTY)
	continue;
      if (curitm == finitm)
	pos = (int) ix;
    };
  assert (pos >= 0 && pos < (int) hsiz && anh->anh_arritm[pos] == finitm);
  anh->anh_arritm[pos] = MOM_EMPTY;
  anh->anh_count--;
  if (anh->anh_count < hsiz / 4 && hsiz > 2 * ANH_INITIAL_SIZE_MOM)
    reorganize_anon_bucket_mom (hrk);
  pthread_mutex_unlock (&mtx_anon_mom[hrk]);
}				/* end mom_unregister_anonymous_finalized_item */
