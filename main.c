// file main.c

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

struct momout_st mom_stdout_data = {
  .mout_magic = MOM_MOUT_MAGIC,
  .mout_indent = 0,
  .mout_file = NULL,
  .mout_lastnl = 0
};

struct momout_st mom_stderr_data = {
  .mout_magic = MOM_MOUT_MAGIC,
  .mout_indent = 0,
  .mout_file = NULL,
  .mout_lastnl = 0
};

static const char *
dbg_level_mom (enum mom_debug_en dbg)
{
#define LEVDBG(Dbg) case momdbg_##Dbg: return #Dbg;
  switch (dbg)
    {
      MOM_DEBUG_LIST_OPTIONS (LEVDBG);
    default:
      {
	static char dbglev[16];
	snprintf (dbglev, sizeof (dbglev), "?DBG?%d", (int) dbg);
	return dbglev;
      }
    }
}


char *
mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
{
  struct tm tm = { };
  time_t tim = (time_t) ti;
  if (!buf || !fmt || !len)
    return NULL;
  strftime (buf, len, fmt, localtime_r (&tim, &tm));
  char *dotundund = strstr (buf, ".__");
  if (dotundund)
    {
      double ind = 0.0;
      double fra = modf (ti, &ind);
      char minibuf[16];
      memset (minibuf, 0, sizeof (minibuf));
      snprintf (minibuf, sizeof (minibuf), "%.02f", fra);
      strncpy (dotundund, strchr (minibuf, '.'), 3);
    }
  return buf;
}


void
mom_debugprintf_at (enum mom_debug_en dbg, const char *fil, int lin,
		    const char *fmt, ...)
{
  char thrname[24];
  char buf[128];
  char timbuf[64];
  int len = 0;
  char *msg = NULL;
  char *bigbuf = NULL;
  memset (thrname, 0, sizeof (thrname));
  memset (buf, 0, sizeof (buf));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
    {
      char *bigbuf = malloc (len + 10);
      if (bigbuf)
	{
	  memset (bigbuf, 0, len + 10);
	  va_start (alist, fmt);
	  (void) vsnprintf (bigbuf, len + 1, fmt, alist);
	  va_end (alist);
	  msg = bigbuf;
	}
    }
  else
    msg = buf;
  syslog (LOG_DEBUG, "MONIMELT DEBUG %s <%s> @%s:%d %s %s",
	  dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
  if (bigbuf)
    free (bigbuf);
}

void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  char *msg = NULL;
  int err = errno;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
    {
      char *bigbuf = malloc (len + 10);
      if (bigbuf)
	{
	  memset (bigbuf, 0, len + 10);
	  va_start (alist, fmt);
	  (void) vsnprintf (bigbuf, len + 1, fmt, alist);
	  va_end (alist);
	  msg = bigbuf;
	}
    }
  else
    msg = buf;
  if (err)
    syslog (LOG_ALERT, "MONIMELT FATAL @%s:%d <%s:%d> %s %s (%s)",
	    fil, lin, thrname, (int) mom_gettid (), timbuf,
	    msg, strerror (err));
  else
    syslog (LOG_ALERT, "MONIMELT FATAL @%s:%d <%s:%d> %s %s",
	    fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
  if (bigbuf)
    free (bigbuf);
  abort ();
}


#if MOM_NEED_GC_CALLOC
void *
GC_calloc (size_t nbelem, size_t elsiz)
{
  if (nbelem == 0 || elsiz == 0)
    return NULL;
  size_t sz = nbelem * elsiz;
  void *p = GC_MALLOC (sz);
  if (p)
    memset (p, 0, sz);
  return p;
}
#endif /* MOM_NEED_GC_CALLOC */

static void
logexit_cb_mom (void)
{
  char timbuf[64];
  memset (timbuf, 0, sizeof (timbuf));
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  syslog (LOG_INFO, "MONIMELT exiting at %s", timbuf);
}



static gpointer
checked_gc_malloc (gsize sz)
{
  void *p = GC_MALLOC (sz);	// leave that GC_MALLOC
  if (MOM_UNLIKELY (!p))
    MOM_FATAPRINTF ("failed to GC malloc %ld bytes", (long) sz);
  memset (p, 0, sz);
  return p;
}

static gpointer
checked_gc_realloc (gpointer m, gsize sz)
{
  void *p = GC_REALLOC (m, sz);	// leave that GC_REALLOC
  if (MOM_UNLIKELY (!p))
    MOM_FATAPRINTF ("failed to GC realloc %ld bytes", (long) sz);
  return p;
}

static gpointer
checked_gc_calloc (gsize nblock, gsize bsize)
{
  void *p = GC_MALLOC (nblock * bsize);	// leave that GC_MALLOC
  if (MOM_UNLIKELY (!p && nblock > 0 && bsize > 0))
    MOM_FATAPRINTF ("failed to GC calloc %ld blocks of %ld bytes",
		    (long) nblock, (long) bsize);
  memset (p, 0, nblock * bsize);
  return p;
}

static GMemVTable gc_mem_vtable_mom = {
  .malloc = checked_gc_malloc,
  .realloc = checked_gc_realloc,
  .free = GC_free,
  .calloc = checked_gc_calloc,
  .try_malloc = GC_malloc,
  .try_realloc = GC_realloc
};

static void
memory_failure_onion_mom (const char *msg)
{
  MOM_FATAPRINTF ("memory failure: %s", msg);
}

int
main (int argc, char **argv)
{
  bool daemonize_me = false;
  GC_INIT ();
  pthread_setname_np (pthread_self (), "mom-main");
  g_mem_gc_friendly = TRUE;
  g_mem_set_vtable (&gc_mem_vtable_mom);
  mom_stdout_data.mout_file = stdout;
  mom_stderr_data.mout_file = stderr;
  onion_low_initialize_memory_allocation
    (GC_malloc,
     GC_malloc_atomic,
     GC_calloc, GC_realloc, GC_strdup, GC_free, memory_failure_onion_mom);
  onion_low_initialize_threads
    (GC_pthread_create,
     GC_pthread_join,
     GC_pthread_cancel,
     GC_pthread_detach, GC_pthread_exit, GC_pthread_sigmask);
  openlog ("monimelt",
	   LOG_PID | LOG_CONS | (daemonize_me ? 0 : LOG_PERROR), LOG_LOCAL2);
  {
    char hnam[64];
    char timbuf[64];
    memset (timbuf, 0, sizeof (timbuf));
    memset (hnam, 0, sizeof (hnam));
    gethostname (hnam, sizeof (hnam) - 1);
    mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
    syslog (LOG_INFO,
	    "MONIMELT starting on %s at %s in %s,\n.. built %s gitcommit %s",
	    hnam, timbuf, get_current_dir_name (), monimelt_timestamp,
	    monimelt_lastgitcommit);
    atexit (logexit_cb_mom);
  }
  ///

  return 0;
}
