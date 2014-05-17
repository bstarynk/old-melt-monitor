// file random.c

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

#include "monimelt.h"


static pthread_mutex_t mtx_random_mom = PTHREAD_MUTEX_INITIALIZER;

static struct random_data randata_mom;

static unsigned randcount_mom;

static unsigned rand_reseed_period_mom;

static void
seed_random_mom (void)
{
#define RANDSTATELEN 64
  static union
  {
    char state[RANDSTATELEN];
    struct
    {
      pid_t pid;
      struct timespec ts;
      char stuff[RANDSTATELEN - sizeof (pid_t) - sizeof (struct timespec)];
    } data;
  } u;
  static unsigned rseed;
  FILE *filr = fopen ("/dev/urandom", "r");
  u.data.pid = getpid ();
  clock_gettime (CLOCK_REALTIME, &u.data.ts);
  if (!rseed)
    {
      rseed = u.data.pid ^ (u.data.ts.tv_sec ^ u.data.ts.tv_nsec);
    }
  if (filr)
    {
      fread (u.data.stuff, sizeof (u.data.stuff), 1, filr);
      fread (&rseed, sizeof (rseed), 1, filr);
      fclose (filr);
    }
  else
    MOM_WARNPRINTF ("failed to open /dev/urandom");
  initstate_r (rseed, u.state, RANDSTATELEN, &randata_mom);
  int32_t res;
  random_r (&randata_mom, &res);
  rand_reseed_period_mom = (((res % 283) & 0xff) + 32);
}

static uint32_t
random32_unlocked_mom (void)
{
  int32_t r;
  if (MOM_UNLIKELY (randcount_mom >= rand_reseed_period_mom))
    {
      randcount_mom = 0;
      seed_random_mom ();
    };
  random_r (&randata_mom, &r);
  return (uint32_t) r;
}

uint32_t
mom_random_nonzero_32 (void)
{
  uint32_t r;
  pthread_mutex_lock (&mtx_random_mom);
  do
    {
      r = random32_unlocked_mom ();
    }
  while (MOM_UNLIKELY (r == 0));
  pthread_mutex_unlock (&mtx_random_mom);
  return r;
}


uint32_t
mom_random_32 (void)
{
  uint32_t r;
  pthread_mutex_lock (&mtx_random_mom);
  r = random32_unlocked_mom ();
  pthread_mutex_unlock (&mtx_random_mom);
  return r;
}

uint64_t
mom_random_nonzero_64 (void)
{
  uint64_t r;
  pthread_mutex_lock (&mtx_random_mom);
  do
    {
      uint32_t h = random32_unlocked_mom ();
      uint32_t l = random32_unlocked_mom ();
      r = ((uint64_t) h << 32) + (uint64_t) l;
    }
  while (MOM_UNLIKELY (r == 0));
  pthread_mutex_unlock (&mtx_random_mom);
  return r;
}

uint64_t
mom_random_64 (void)
{
  uint64_t r;
  pthread_mutex_lock (&mtx_random_mom);
  uint32_t h = random32_unlocked_mom ();
  uint32_t l = random32_unlocked_mom ();
  r = ((uint64_t) h << 32) + (uint64_t) l;
  pthread_mutex_unlock (&mtx_random_mom);
  return r;
}

  // 31 different chars; notice that 31 is prime
#define RANDCHARS_MOM "acdefhijkmpqrstuvwxyz0123456789"
#define NBRANDCHARS_MOM (sizeof(RANDCHARS_MOM)-1)
const momstring_t *
mom_make_random_idstr (void)
{
  char resbuf[32];
  assert (NBRANDCHARS_MOM == 31);
  uint32_t r0, r1, r2, r3;
  memset (resbuf, 0, sizeof (resbuf));
  resbuf[0] = '_';
  pthread_mutex_lock (&mtx_random_mom);
  do
    {
      r0 = random32_unlocked_mom ();
    }
  while (MOM_UNLIKELY ((r0 < 4096)));
  do
    {
      r1 = random32_unlocked_mom ();
    }
  while (MOM_UNLIKELY ((r1 < 4096)));
  r2 = random32_unlocked_mom ();
  r3 = random32_unlocked_mom ();
  (resbuf[1] = '0' + (r0 % 10)), r0 = r0 / 10;
  (resbuf[2] = RANDCHARS_MOM[r0 % NBRANDCHARS_MOM]), r0 =
    r0 / NBRANDCHARS_MOM;
  (resbuf[3] = RANDCHARS_MOM[r0 % NBRANDCHARS_MOM]), r0 =
    r0 / NBRANDCHARS_MOM;
  (resbuf[4] = RANDCHARS_MOM[r0 % NBRANDCHARS_MOM]), r0 =
    r0 / NBRANDCHARS_MOM;
  (resbuf[5] = RANDCHARS_MOM[r0 % NBRANDCHARS_MOM]), r0 =
    r0 / NBRANDCHARS_MOM;
  (resbuf[6] = RANDCHARS_MOM[r1 % NBRANDCHARS_MOM]), r1 =
    r1 / NBRANDCHARS_MOM;
  (resbuf[7] = RANDCHARS_MOM[r1 % NBRANDCHARS_MOM]), r1 =
    r1 / NBRANDCHARS_MOM;
  (resbuf[8] = RANDCHARS_MOM[r1 % NBRANDCHARS_MOM]), r1 =
    r1 / NBRANDCHARS_MOM;
  (resbuf[9] = RANDCHARS_MOM[r1 % NBRANDCHARS_MOM]), r1 =
    r1 / NBRANDCHARS_MOM;
  (resbuf[10] = RANDCHARS_MOM[r1 % NBRANDCHARS_MOM]), r1 =
    r1 / NBRANDCHARS_MOM;
  (resbuf[11] = RANDCHARS_MOM[r1 % NBRANDCHARS_MOM]), r1 =
    r1 / NBRANDCHARS_MOM;
  resbuf[12] = '_';
  (resbuf[13] = RANDCHARS_MOM[r2 % NBRANDCHARS_MOM]), r2 =
    r2 / NBRANDCHARS_MOM;
  (resbuf[14] = RANDCHARS_MOM[r2 % NBRANDCHARS_MOM]), r2 =
    r2 / NBRANDCHARS_MOM;
  (resbuf[15] = RANDCHARS_MOM[r2 % NBRANDCHARS_MOM]), r2 =
    r2 / NBRANDCHARS_MOM;
  (resbuf[16] = RANDCHARS_MOM[r2 % NBRANDCHARS_MOM]), r2 =
    r2 / NBRANDCHARS_MOM;
  (resbuf[17] = RANDCHARS_MOM[r2 % NBRANDCHARS_MOM]), r2 =
    r2 / NBRANDCHARS_MOM;
  (resbuf[18] = RANDCHARS_MOM[r2 % NBRANDCHARS_MOM]), r2 =
    r2 / NBRANDCHARS_MOM;
  (resbuf[19] = RANDCHARS_MOM[r3 % NBRANDCHARS_MOM]), r3 =
    r3 / NBRANDCHARS_MOM;
  (resbuf[20] = RANDCHARS_MOM[r3 % NBRANDCHARS_MOM]), r3 =
    r3 / NBRANDCHARS_MOM;
  (resbuf[21] = RANDCHARS_MOM[r3 % NBRANDCHARS_MOM]), r3 =
    r3 / NBRANDCHARS_MOM;
  (resbuf[22] = RANDCHARS_MOM[r3 % NBRANDCHARS_MOM]), r3 =
    r3 / NBRANDCHARS_MOM;
  (resbuf[23] = RANDCHARS_MOM[r3 % NBRANDCHARS_MOM]), r3 =
    r3 / NBRANDCHARS_MOM;
  pthread_mutex_unlock (&mtx_random_mom);
  return mom_make_string (resbuf);
}


bool
mom_looks_like_random_id_cstr (const char *s, const char **pend)
{
  if (!s)
    return false;
  if (pend)
    *pend = NULL;
  if (s[0] != '_')
    return false;
  if (!isdigit (s[1]))
    return false;
#define CHECKRANDCHAR(C) ((C) && strchr(RANDCHARS_MOM, (C)))
  if (!CHECKRANDCHAR (s[2]))
    return false;
  if (!CHECKRANDCHAR (s[3]))
    return false;
  if (!CHECKRANDCHAR (s[4]))
    return false;
  if (!CHECKRANDCHAR (s[5]))
    return false;
  if (!CHECKRANDCHAR (s[6]))
    return false;
  if (!CHECKRANDCHAR (s[7]))
    return false;
  if (!CHECKRANDCHAR (s[8]))
    return false;
  if (!CHECKRANDCHAR (s[9]))
    return false;
  if (!CHECKRANDCHAR (s[10]))
    return false;
  if (!CHECKRANDCHAR (s[11]))
    return false;
  if (s[12] != '_')
    return false;
  if (!CHECKRANDCHAR (s[13]))
    return false;
  if (!CHECKRANDCHAR (s[14]))
    return false;
  if (!CHECKRANDCHAR (s[15]))
    return false;
  if (!CHECKRANDCHAR (s[16]))
    return false;
  if (!CHECKRANDCHAR (s[17]))
    return false;
  if (!CHECKRANDCHAR (s[18]))
    return false;
  if (!CHECKRANDCHAR (s[19]))
    return false;
  if (!CHECKRANDCHAR (s[20]))
    return false;
  if (!CHECKRANDCHAR (s[21]))
    return false;
  if (!CHECKRANDCHAR (s[22]))
    return false;
  if (!CHECKRANDCHAR (s[23]))
    return false;
  if (isalnum (s[24]))
    return false;
  if (pend)
    *pend = s + 24;
  return true;
#undef CHECKRANDCHAR
}
