// file random.c

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


#define NB_RANDOM_GEN_MOM 16

static pthread_mutex_t mtx_random_mom[NB_RANDOM_GEN_MOM];
static struct random_data randata_mom[NB_RANDOM_GEN_MOM];
#define RANDSTATELEN_MOM 32
struct random_state_mom_st
{
  char ranstate[RANDSTATELEN_MOM];
};
static struct random_state_mom_st randstate_mom[NB_RANDOM_GEN_MOM];

void
mom_initialize_random (void)
{
  static const pthread_mutex_t inimtx = PTHREAD_MUTEX_INITIALIZER;
  for (int ix = 0; ix < NB_RANDOM_GEN_MOM; ix++)
    memcpy (mtx_random_mom + ix, &inimtx, sizeof (pthread_mutex_t));
  FILE *filr = fopen ("/dev/urandom", "r");
  if (!filr)
    MOM_FATAPRINTF ("failed to open /dev/urandom: %m");
  unsigned rseed;
  pid_t pid = getpid ();
  struct timespec ts = { 0, 0 };
  for (int ix = 0; ix < NB_RANDOM_GEN_MOM; ix++)
    {
      clock_gettime (CLOCK_REALTIME, &ts);
      rseed = ((31 * pid) ^ (17 * ts.tv_sec ^ ts.tv_nsec)) + 50033 * ix;
      if (fread (&rseed, sizeof (rseed), 1, filr) < 1)
	usleep (5 + (ts.tv_sec + ts.tv_nsec) % 50);
      initstate_r (rseed, (char *) randstate_mom + ix,
		   RANDSTATELEN_MOM, randata_mom + ix);
      setstate_r ((char *) randstate_mom + ix, randata_mom + ix);
    }
  fclose (filr);
}

uint32_t
mom_random_nonzero_32 (unsigned num)
{
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r;
  pthread_mutex_lock (mtx_random_mom + rk);
  do
    {
      int32_t sr;
      if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
	MOM_FATAPRINTF ("random_r failure %m");
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  pthread_mutex_unlock (mtx_random_mom + rk);
  return r;
}

uint32_t
mom_random_32 (unsigned num)
{
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r = 0;
  pthread_mutex_lock (mtx_random_mom + rk);
  int32_t sr = 0;
  if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
    MOM_FATAPRINTF ("random_r failure %m");
  r = (uint32_t) sr;
  pthread_mutex_unlock (mtx_random_mom + rk);
  return r;
}

uint64_t
mom_random_64 (unsigned num)
{
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r1 = 0, r2 = 0;
  pthread_mutex_lock (mtx_random_mom + rk);
  int32_t sr1 = 0;
  int32_t sr2 = 0;
  if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr1)))
    MOM_FATAPRINTF ("random_r failure %m");;
  r1 = (uint32_t) sr1;
  if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr2)))
    MOM_FATAPRINTF ("random_r failure %m");;
  r2 = (uint32_t) sr2;
  pthread_mutex_unlock (mtx_random_mom + rk);
  return (((uint64_t) r1) << 32) | (uint64_t) r2;
}

uintptr_t
mom_random_intptr (unsigned num)
{
  if (sizeof (uintptr_t) == sizeof (uint32_t))
    return mom_random_32 (num);
  else if (sizeof (uintptr_t) == sizeof (uint64_t))
    return mom_random_64 (num);
  else
    MOM_FATAPRINTF ("unsupported architecture with strange pointer size %d",
		    (int) sizeof (uintptr_t));
}

void
mom_random_two_nonzero_32 (unsigned num, uint32_t *r1, uint32_t *r2)
{
  assert (r1);
  assert (r2);
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r;
  pthread_mutex_lock (mtx_random_mom + rk);
  do
    {
      int32_t sr = 0;
      if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
	MOM_FATAPRINTF ("random_r failure %m");
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r1 = r;
  do
    {
      int32_t sr = 0;
      if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
	MOM_FATAPRINTF ("random_r failure %m");
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r2 = r;
  pthread_mutex_unlock (mtx_random_mom + rk);
}


void
mom_random_three_nonzero_32 (unsigned num, uint32_t *r1, uint32_t *r2,
			     uint32_t *r3)
{
  assert (r1);
  assert (r2);
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r;
  pthread_mutex_lock (mtx_random_mom + rk);
  do
    {
      int32_t sr = 0;
      if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
	MOM_FATAPRINTF ("random_r failure %m");
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r1 = r;
  do
    {
      int32_t sr = 0;
      if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
	MOM_FATAPRINTF ("random_r failure %m");
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r2 = r;
  do
    {
      int32_t sr = 0;
      if (MOM_UNLIKELY (random_r (randata_mom + rk, &sr)))
	MOM_FATAPRINTF ("random_r failure %m");
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r3 = r;
  pthread_mutex_unlock (mtx_random_mom + rk);
}
