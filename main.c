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


/************************* debugging *************************/

static bool syslogging_mom;

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
#undef LEVDBG
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
mom_debug_at (enum mom_debug_en dbg, const char *sfil, int slin, ...)
{
  struct momout_st outd;
  char *membuf = NULL;
  size_t memsize = 0;
  memset (&outd, 0, sizeof (outd));
  outd.mout_magic = MOM_MOUT_MAGIC;
  outd.mout_file = open_memstream (&membuf, &memsize);
  va_list alist;
  va_start (alist, slin);
  mom_outva_at (sfil, slin, &outd, alist);
  va_end (alist);
  fclose (outd.mout_file);
  memset (&outd, 0, sizeof (outd));
  mom_debugprintf_at (dbg, sfil, slin, "%s", membuf);
  free (membuf), membuf = 0;
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
  fflush (NULL);
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
  if (syslogging_mom)
    syslog (LOG_DEBUG, "MONIMELT DEBUG %s <%s> @%s:%d %s %s",
	    dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
  else
    {
      fprintf (stderr, "MONIMELT DEBUG %s <%s> @%s:%d %s %s\n",
	       dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
      fflush (NULL);
    }
  if (bigbuf)
    free (bigbuf);
}



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
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
  if (syslogging_mom)
    {
      if (err)
	syslog (LOG_ALERT, "MONIMELT FATAL @%s:%d <%s:%d> %s %s (%s)",
		fil, lin, thrname, (int) mom_gettid (), timbuf,
		msg, strerror (err));
      else
	syslog (LOG_ALERT, "MONIMELT FATAL @%s:%d <%s:%d> %s %s",
		fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
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
      fflush (NULL);
    }
  if (bigbuf)
    free (bigbuf);
  abort ();
}


void
mom_fatal_at (const char *sfil, int slin, ...)
{
  struct momout_st outd;
  char *membuf = NULL;
  size_t memsize = 0;
  memset (&outd, 0, sizeof (outd));
  outd.mout_magic = MOM_MOUT_MAGIC;
  outd.mout_file = open_memstream (&membuf, &memsize);
  va_list alist;
  va_start (alist, slin);
  mom_outva_at (sfil, slin, &outd, alist);
  va_end (alist);
  fclose (outd.mout_file);
  memset (&outd, 0, sizeof (outd));
  mom_fataprintf_at (sfil, slin, "%s", membuf);
  free (membuf), membuf = 0;
}




/************************* warning *************************/

void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
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
  fflush (NULL);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
  if (syslogging_mom)
    {
      if (err)
	syslog (LOG_WARNING, "MONIMELT WARNING @%s:%d <%s:%d> %s %s (%s)",
		fil, lin, thrname, (int) mom_gettid (), timbuf,
		msg, strerror (err));
      else
	syslog (LOG_WARNING, "MONIMELT WARNING @%s:%d <%s:%d> %s %s",
		fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
    {
      if (err)
	fprintf (stderr, "MONIMELT WARNING @%s:%d <%s:%d> %s %s (%s)\n",
		 fil, lin, thrname, (int) mom_gettid (), timbuf,
		 msg, strerror (err));
      else
	fprintf (stderr, "MONIMELT WARNING @%s:%d <%s:%d> %s %s\n",
		 fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      fflush (NULL);
    }
  if (bigbuf)
    free (bigbuf);
}


void
mom_warning_at (const char *sfil, int slin, ...)
{
  struct momout_st outd;
  char *membuf = NULL;
  size_t memsize = 0;
  memset (&outd, 0, sizeof (outd));
  outd.mout_magic = MOM_MOUT_MAGIC;
  outd.mout_file = open_memstream (&membuf, &memsize);
  va_list alist;
  va_start (alist, slin);
  mom_outva_at (sfil, slin, &outd, alist);
  va_end (alist);
  fclose (outd.mout_file);
  memset (&outd, 0, sizeof (outd));
  mom_warnprintf_at (sfil, slin, "%s", membuf);
  free (membuf), membuf = 0;
}





/************************* inform *************************/

void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  char *msg = NULL;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
  if (syslogging_mom)
    {
      syslog (LOG_INFO, "MONIMELT INFORM @%s:%d <%s:%d> %s %s",
	      fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
    {
      fprintf (stderr, "MONIMELT INFORM @%s:%d <%s:%d> %s %s\n",
	       fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      fflush (NULL);
    }
  if (bigbuf)
    free (bigbuf);
}


void
mom_inform_at (const char *sfil, int slin, ...)
{
  struct momout_st outd;
  char *membuf = NULL;
  size_t memsize = 0;
  memset (&outd, 0, sizeof (outd));
  outd.mout_magic = MOM_MOUT_MAGIC;
  outd.mout_file = open_memstream (&membuf, &memsize);
  va_list alist;
  va_start (alist, slin);
  mom_outva_at (sfil, slin, &outd, alist);
  va_end (alist);
  fclose (outd.mout_file);
  memset (&outd, 0, sizeof (outd));
  mom_informprintf_at (sfil, slin, "%s", membuf);
  free (membuf), membuf = 0;
}


/*********************** misc **********************/

static const char *wanted_dir_mom;
static const char *write_pid_file_mom;

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


static void
initialize_mom (void)
{
  //// initialize items
  {
    extern void mom_initialize_items (void);
    mom_initialize_items ();
  }
  //// initialize some thread attributes
  pthread_mutexattr_init (&mom_normal_mutex_attr);
  pthread_mutexattr_init (&mom_recursive_mutex_attr);
  pthread_mutexattr_settype (&mom_normal_mutex_attr, PTHREAD_MUTEX_NORMAL);
  pthread_mutexattr_settype (&mom_recursive_mutex_attr,
			     PTHREAD_MUTEX_RECURSIVE);
  //// create the predefined items
  {
    extern void mom_create_predefined_items (void);
    mom_create_predefined_items ();
  }
}


static bool daemonize_mom = false;
/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_chdir = 1024,
  xtraopt_writepid,
  xtraopt_loadstate,
  xtraopt_dumpstate,
  xtraopt_noeventloop,
  xtraopt_randomidstr,
};

static const struct option mom_long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {"nice", required_argument, NULL, 'n'},
  {"module", required_argument, NULL, 'M'},
  {"jobs", required_argument, NULL, 'J'},
  {"daemon", no_argument, NULL, 'd'},
  {"syslog", no_argument, NULL, 'l'},
  {"web", required_argument, NULL, 'W'},
  // long-only options
  {"write-pid", required_argument, NULL, xtraopt_writepid},
  {"load-state", required_argument, NULL, xtraopt_loadstate},
  {"dump-state", required_argument, NULL, xtraopt_dumpstate},
  {"chdir", required_argument, NULL, xtraopt_chdir},
  {"no-event-loop", no_argument, NULL, xtraopt_noeventloop},
  {"random-idstr", no_argument, NULL, xtraopt_randomidstr},
  /* Terminating NULL placeholder.  */
  {NULL, no_argument, NULL, 0},
};

const char *const mom_debug_names[momdbg__last] = {
#define DEFINE_DBG_NAME_MOM(Dbg) [momdbg_##Dbg] #Dbg,
  MOM_DEBUG_LIST_OPTIONS (DEFINE_DBG_NAME_MOM)
};

#undef DEFINE_DBG_NAME_MOM

static void
usage_mom (const char *argv0)
{
  printf ("Usage: %s\n", argv0);
  printf ("\t -h | --help " " \t# Give this help.\n");
  printf ("\t -V | --version " " \t# Give version information.\n");
  printf ("\t -d | --daemon " " \t# Daemonize.\n");
  printf ("\t -D | --debug <debug-features>"
	  " \t# Debugging comma separated features\n\t\t##");
  for (unsigned ix = 1; ix < momdbg__last; ix++)
    printf (" %s", mom_debug_names[ix]);
  putchar ('\n');
  printf ("\t -n | --nice <nice-level> " " \t# Set process nice level.\n");
  printf ("\t -J | --jobs <nb-work-threads> " " \t# Start work threads.\n");
  printf ("\t -M | --module <module-name> <module-arg> "
	  " \t# load a plugin.\n");
  printf ("\t -W | --web <webhost>\n");
  putchar ('\n');
  printf ("\t --chdir <directory>" "\t #change directory\n");
  printf ("\t --write-pid <file>"
	  "\t #write the pid (e.g. --write-pid /var/run/monimelt.pid)\n");
  printf ("\t --random-idstr" "\t #output a random idstr then exit\n");
}

static void
print_version_mom (const char *argv0)
{
  printf ("%s built on %s gitcommit %s\n", argv0,
	  monimelt_timestamp, monimelt_lastgitcommit);
}

static void
add_debugging_mom (const char *dbgopt)
{
  char dbuf[256];
  memset (dbuf, 0, sizeof (dbuf));
  if (strlen (dbgopt) >= sizeof (dbuf) - 1)
    MOM_FATAL ("too long debug option %s", dbgopt);
  strcpy (dbuf, dbgopt);
  char *comma = NULL;
  if (!strcmp (dbuf, ".") || !strcmp (dbuf, "_"))
    {
      mom_debugflags = ~0;
      MOM_INFORMPRINTF ("set all debugging");
    }
  else
    for (char *pc = dbuf; pc != NULL; pc = comma ? comma + 1 : NULL)
      {
	comma = strchr (pc, ',');
	if (comma)
	  *comma = (char) 0;
#define MOM_TEST_DEBUG_OPTION(Nam)					\
	if (!strcmp(pc,#Nam)) mom_debugflags |=  (1<<momdbg_##Nam); else
	if (!pc)
	  break;
	MOM_DEBUG_LIST_OPTIONS (MOM_TEST_DEBUG_OPTION)
	  MOM_FATAPRINTF ("unrecognized debug flag %s", pc);
      }
}

static void
parse_program_arguments_and_load_modules_mom (int *pargc, char **argv)
{
  int argc = *pargc;
  int opt = -1;
  while ((opt = getopt_long (argc, argv, "lhVdn:M:W:J:D:",
			     mom_long_options, NULL)) >= 0)
    {
      switch (opt)
	{
	case 'h':
	  usage_mom (argv[0]);
	  exit (EXIT_FAILURE);
	  return;
	case 'V':
	  print_version_mom (argv[0]);
	  break;
	case 'd':
	  daemonize_mom = true;
	  syslogging_mom = true;
	  break;
	case 'l':
	  syslogging_mom = true;
	  break;
	case 'D':
	  if (optarg)
	    add_debugging_mom (optarg);
	  break;
	case 'J':
	  if (optarg)
	    mom_nb_workers = atoi (optarg);
	  break;
	case 'W':
	  mom_web_host = optarg;
	  break;
	case 'M':
	  {
	    char *modnam = optarg;
	    char *modarg = argv[optind++];
#warning load of module unimplemented
	    MOM_FATAPRINTF ("should load module %s arg %s", modnam, modarg);
	  }
	  break;
	case xtraopt_chdir:
	  wanted_dir_mom = optarg;
	  break;
	case xtraopt_randomidstr:
	  {
	    errno = 0;
	    const momstring_t *randidstr = mom_make_random_idstr ();
	    printf ("%s\n", randidstr->cstr);
	    MOM_WARNPRINTF ("exiting after random id string %s",
			    randidstr->cstr);
	    exit (EXIT_SUCCESS);
	  }
	  break;
	case xtraopt_writepid:
	  {
	    write_pid_file_mom = optarg;
	  }
	  break;
	default:
	  {
	    if (opt > 0 && opt < UCHAR_MAX && isalpha (opt))
	      MOM_FATAPRINTF ("unknown option %c", opt);
	    else
	      MOM_FATAPRINTF ("unknown option #%d", opt);
	  }
	  break;
	}
    }

}

int
main (int argc, char **argv)
{
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
  parse_program_arguments_and_load_modules_mom (&argc, argv);
  if (daemonize_mom)
    {
      if (daemon (true, false))
	MOM_FATAPRINTF ("failed to daemonize");
    }
  if (syslogging_mom)
    {
      //// initialize logging
      openlog ("monimelt",
	       LOG_PID | (daemonize_mom ? 0 : (LOG_CONS | LOG_PERROR)),
	       LOG_LOCAL2);
      {
	char hnam[64];
	char timbuf[64];
	char dirnam[MOM_PATH_MAX];
	memset (timbuf, 0, sizeof (timbuf));
	memset (hnam, 0, sizeof (hnam));
	memset (dirnam, 0, sizeof (dirnam));
	gethostname (hnam, sizeof (hnam) - 1);
	mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
	syslog (LOG_INFO,
		"MONIMELT starting on %s at %s in %s,\n.. built %s gitcommit %s",
		hnam, timbuf, getcwd (dirnam, sizeof (dirnam)),
		monimelt_timestamp, monimelt_lastgitcommit);
	atexit (logexit_cb_mom);
      }
    }
  /// change directory if asked
  if (wanted_dir_mom)
    {
      char dirpath[MOM_PATH_MAX];
      memset (dirpath, 0, sizeof (dirpath));
      if (chdir (wanted_dir_mom))
	MOM_FATAPRINTF ("failed to chdir %s", wanted_dir_mom);
      else
	MOM_INFORMPRINTF ("changed directory to %s, now in %s",
			  wanted_dir_mom, getcwd (dirpath, sizeof (dirpath)));
    };
  // write the pid to the given file if so asked
  if (write_pid_file_mom)
    {
      FILE *pidf = fopen (write_pid_file_mom, "w");
      if (!pidf)
	MOM_FATAPRINTF ("failed to open write pid file %s",
			write_pid_file_mom);
      fprintf (pidf, "%d\n", (int) getpid ());
      if (fclose (pidf))
	MOM_FATAPRINTF ("failed to close pid file %s", write_pid_file_mom);
      MOM_INFORMPRINTF ("wrote my pid into %s", write_pid_file_mom);
    }
  ///
  initialize_mom ();
  ///
  return 0;
}
