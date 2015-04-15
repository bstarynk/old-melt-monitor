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

void
mom_initialize_random (void)
{
#define RANDSTATELEN 64
  static const pthread_mutex_t inimtx = PTHREAD_MUTEX_INITIALIZER;
  for (int ix = 0; ix < NB_RANDOM_GEN_MOM; ix++)
    memcpy (mtx_random_mom + ix, &inimtx, sizeof (pthread_mutex_t));
  FILE *filr = fopen ("/dev/urandom", "r");
  if (!filr)
    MOM_FATAPRINTF ("failed to open /dev/urandom: %m");
  unsigned rseed;
  for (int ix = 0; ix < NB_RANDOM_GEN_MOM; ix++)
    {
      union
      {
	char state[RANDSTATELEN];
	struct
	{
	  pid_t pid;
	  struct timespec ts;
	  char stuff[RANDSTATELEN - sizeof (pid_t) -
		     sizeof (struct timespec)];
	} data;
      } u;
      memset (&u, 0, sizeof (u));
      u.data.pid = getpid ();
      clock_gettime (CLOCK_REALTIME, &u.data.ts);
      if (!rseed)
	{
	  rseed = u.data.pid ^ (u.data.ts.tv_sec ^ u.data.ts.tv_nsec);
	}
      fread (u.data.stuff, sizeof (u.data.stuff), 1, filr);
      fread (&rseed, sizeof (rseed), 1, filr);
      initstate_r (rseed, u.state, RANDSTATELEN, randata_mom + ix);
    }
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
      random_r (randata_mom + rk, &sr);
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
  uint32_t r;
  pthread_mutex_lock (mtx_random_mom + rk);
  int32_t sr;
  random_r (randata_mom + rk, &sr);
  r = (uint32_t) sr;
  pthread_mutex_unlock (mtx_random_mom + rk);
  return r;
}

uint64_t
mom_random_64 (unsigned num)
{
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r1, r2;
  pthread_mutex_lock (mtx_random_mom + rk);
  int32_t sr;
  random_r (randata_mom + rk, &sr);
  r1 = (uint32_t) sr;
  random_r (randata_mom + rk, &sr);
  r2 = (uint32_t) sr;
  pthread_mutex_unlock (mtx_random_mom + rk);
  return ((uint64_t) r1 << 32) | (uint64_t) r2;
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
mom_random_two_nonzero_32 (unsigned num, uint32_t * r1, uint32_t * r2)
{
  assert (r1);
  assert (r2);
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r;
  pthread_mutex_lock (mtx_random_mom + rk);
  do
    {
      int32_t sr;
      random_r (randata_mom + rk, &sr);
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r1 = r;
  do
    {
      int32_t sr;
      random_r (randata_mom + rk, &sr);
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r2 = r;
  pthread_mutex_unlock (mtx_random_mom + rk);
}


void
mom_random_three_nonzero_32 (unsigned num, uint32_t * r1, uint32_t * r2,
			     uint32_t * r3)
{
  assert (r1);
  assert (r2);
  unsigned rk = num % NB_RANDOM_GEN_MOM;
  uint32_t r;
  pthread_mutex_lock (mtx_random_mom + rk);
  do
    {
      int32_t sr;
      random_r (randata_mom + rk, &sr);
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r1 = r;
  do
    {
      int32_t sr;
      random_r (randata_mom + rk, &sr);
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r2 = r;
  do
    {
      int32_t sr;
      random_r (randata_mom + rk, &sr);
      r = (uint32_t) sr;
    }
  while (MOM_UNLIKELY (r == 0));
  *r3 = r;
  pthread_mutex_unlock (mtx_random_mom + rk);
}
