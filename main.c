// file main.c

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

static bool syslogging_mom;

/************************* fatal *************************/
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
  fflush (NULL);
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      bigbuf = malloc (len + 10);
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
#define BACKTRACE_MAX_MOM 100
  void *bbuf[BACKTRACE_MAX_MOM];
  int blev = 0;
  memset (bbuf, 0, sizeof (bbuf));
  blev = backtrace (bbuf, BACKTRACE_MAX_MOM - 1);
  char **bsym = backtrace_symbols (bbuf, blev);
  if (syslogging_mom)
    {
      if (err)
	syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s (%s)",
		fil, lin, thrname, (int) mom_gettid (), timbuf,
		msg, strerror (err));
      else
	syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s",
		fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      for (int i = 0; i < blev; i++)
	syslog (LOG_ALERT, "MONIMELTB![%d]: %s", i, bsym[i]);
    }
  else
    {
      if (err)
	fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s (%s)\n",
		 fil, lin, thrname, (int) mom_gettid (), timbuf,
		 msg, strerror (err));
      else
	fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s\n",
		 fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      for (int i = 0; i < blev; i++)
	fprintf (stderr, "MONIMELTB[%d]: %s\n", i, bsym[i]);
      fflush (NULL);
    }
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

char *
mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
{
  struct tm tm = { 0 };
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

static double startime_mom;

double
mom_elapsed_real_time (void)
{
  return mom_clock_time (CLOCK_REALTIME) - startime_mom;
}


static void
memory_failure_onion_mom (const char *msg)
{
  MOM_FATAPRINTF ("memory failure: %s", msg);
}

int
main (int argc_main, char **argv_main)
{
  GC_INIT ();
  char**argv = argv_main;
  int argc = argc_main;
  startime_mom = mom_clock_time (CLOCK_REALTIME);
  pthread_setname_np (pthread_self (), "mom-main");
  onion_low_initialize_memory_allocation
    (GC_malloc,
     GC_malloc_atomic,
     GC_calloc, GC_realloc, GC_strdup, GC_free, memory_failure_onion_mom);
  onion_low_initialize_threads
    (GC_pthread_create,
     GC_pthread_join,
     GC_pthread_cancel,
     GC_pthread_detach, GC_pthread_exit, GC_pthread_sigmask);
  mom_prog_dlhandle = GC_dlopen (NULL, RTLD_NOW | RTLD_GLOBAL);
  if (MOM_UNLIKELY (!mom_prog_dlhandle))
    MOM_FATAPRINTF ("failed to dlopen the program: %s", dlerror ());
  mom_initialize_random ();
  {
    int arg0len=0;
    if (argc>1 && (arg0len=strlen(argv[0])>5)
	&& !strcmp(argv[0]+arg0len-4,".run")) {
      char**newargv = GC_MALLOC((argc+3)*sizeof(char*));
      if (!newargv)
	MOM_FATAPRINTF("failed to grow argv for .run suffix: %m");
      memset(newargv, 0, (argc+3)*sizeof(char*));
      newargv[0] == argv[0];
      newargv[1] = "--run";
      for (int ix=1; ix<argc; ix++)
	newargv[ix+1] = argv[ix];
      argv = newargv;
    }
  }
}
