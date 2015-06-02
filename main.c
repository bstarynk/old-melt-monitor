// file main.c - main program and utilities

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
static bool daemonize_mom;
static bool noclose_daemonize_mom;
static const char *wanted_dir_mom;
static const char *dump_exit_dir_mom;
static const char *write_pid_file_mom;
static const char *dump_cold_dir_mom;
static const char *dump_exit_dir_mom;
static const char *generate_c_module_mom;


unsigned nbmorepredef_mom;
#define MAX_NEW_PREDEF_MOM 16
char *newpredefname_mom[MAX_NEW_PREDEF_MOM];
char *newpredefcomment_mom[MAX_NEW_PREDEF_MOM];


pthread_cond_t mom_agenda_changed_condvar = PTHREAD_COND_INITIALIZER;

/************************* inform *************************/

void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[160];
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


/**** debug */
static const char *dbg_level_mom (enum mom_debug_en dbg);

static pthread_mutex_t dbgmtx_mom = PTHREAD_MUTEX_INITIALIZER;
void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
		    const char *fmt, ...)
{
  static long countdbg;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  int len = 0;
  char *msg = NULL;
  char *bigbuf = NULL;
  memset (thrname, 0, sizeof (thrname));
  memset (buf, 0, sizeof (buf));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
  mom_now_strftime_bufcenti (timbuf, "%H:%M:%S.__ ");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
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
  {
    pthread_mutex_lock (&dbgmtx_mom);
    long nbdbg = countdbg++;
#define DEBUG_DATE_PERIOD_MOM 64
    char datebuf[48] = { 0 };
    if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
      {
	mom_now_strftime_bufcenti (datebuf, "%Y-%b-%d@%H:%M:%S.__ %Z");
      };
    if (syslogging_mom)
      {
	syslog (LOG_DEBUG, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s",
		dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
	if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
	  syslog (LOG_DEBUG, "MONIMELT DEBUG#%04ld ~ %s *^*^*", nbdbg,
		  datebuf);
      }
    else
      {
	fprintf (stderr, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s\n",
		 dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
	fflush (stderr);
	if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
	  fprintf (stderr, "MONIMELT DEBUG#%04ld ~ %s *^*^*\n", nbdbg,
		   datebuf);
	fflush (NULL);
      }
    pthread_mutex_unlock (&dbgmtx_mom);
  }
  if (bigbuf)
    free (bigbuf);
}


/************************* warning *************************/

void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[160];
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
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 2))
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


/************************* fatal *************************/
void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[256];
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
  struct tm tm;
  time_t tim = (time_t) ti;
  memset (&tm, 0, sizeof (tm));
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

const char *const mom_debug_names[momdbg__last] = {
#define DEFINE_DBG_NAME_MOM(Dbg) [momdbg_##Dbg]= #Dbg,
  MOM_DEBUG_LIST_OPTIONS (DEFINE_DBG_NAME_MOM)
};

#undef DEFINE_DBG_NAME_MOM


/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_chdir = 1024,
  xtraopt_addpredef,
  xtraopt_writepid,
  xtraopt_loadstate,
  xtraopt_dumpstate,
  xtraopt_noeventloop,
  xtraopt_randomidstr,
  xtraopt_hashstr,
  xtraopt_dumpcoldstate,
  xtraopt_daemon_noclose,
  xtraopt_generate_c_module,
  xtraopt_system,
};

static const struct option mom_long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {"nice", required_argument, NULL, 'n'},
  {"plugin", required_argument, NULL, 'P'},
  {"password-file", required_argument, NULL, 'p'},
  {"jobs", required_argument, NULL, 'J'},
  {"daemon", no_argument, NULL, 'd'},
  {"syslog", no_argument, NULL, 'l'},
  {"web", required_argument, NULL, 'W'},
  {"socket", required_argument, NULL, 'S'},
  {"user-data", required_argument, NULL, 'U'},
  {"doc-root", required_argument, NULL, 'r'},
  // long-only options
  {"daemon-noclose", no_argument, NULL, xtraopt_daemon_noclose},
  {"write-pid", required_argument, NULL, xtraopt_writepid},
  {"load-state", required_argument, NULL, xtraopt_loadstate},
  {"dump-state", required_argument, NULL, xtraopt_dumpstate},
  {"chdir", required_argument, NULL, xtraopt_chdir},
  {"no-event-loop", no_argument, NULL, xtraopt_noeventloop},
  {"random-idstr", no_argument, NULL, xtraopt_randomidstr},
  {"string-hash", required_argument, NULL, xtraopt_hashstr},
  {"dump-cold-state", required_argument, NULL, xtraopt_dumpcoldstate},
  {"add-predefined", required_argument, NULL, xtraopt_addpredef},
  {"generate-c-module", required_argument, NULL, xtraopt_generate_c_module},
  {"system", required_argument, NULL, xtraopt_system},
  /* Terminating NULL placeholder.  */
  {NULL, no_argument, NULL, 0},
};


/*************************** plugins **************************/
struct plugin_mom_st
{
  const char *plugin_name;
  void *plugin_dlh;
};
static struct
{
  pthread_mutex_t plugins_mtx;
  unsigned plugins_size;
  unsigned plugins_count;
  struct plugin_mom_st *plugins_arr;
} plugins_mom =
{
.plugins_mtx = PTHREAD_MUTEX_INITIALIZER,.plugins_size = 0,.plugins_count =
    0,.plugins_arr = NULL};


void
mom_load_plugin (const char *plugname, const char *plugarg, int *pargc,
		 char ***pargv)
{
  char plugpath[MOM_PATH_MAX];
  memset (plugpath, 0, sizeof (plugpath));
  if (!plugname || !(isalnum (plugname[0]) || plugname[0] == '_')
      || strchr (plugname, '/') || strlen (plugname) > MOM_PATH_MAX - 32)
    MOM_FATAPRINTF ("invalid plugin name %s", plugname);
  snprintf (plugpath, sizeof (plugpath), "./" MOM_PLUGIN_PREFIX "%s.so",
	    plugname);
  pthread_mutex_lock (&plugins_mom.plugins_mtx);
  if (MOM_UNLIKELY
      (plugins_mom.plugins_count + 2 >= plugins_mom.plugins_size))
    {
      unsigned oldcnt = plugins_mom.plugins_count;
      unsigned oldsiz = plugins_mom.plugins_size;
      struct plugin_mom_st *oldarr = plugins_mom.plugins_arr;
      unsigned newsiz = 1 + (((3 * oldcnt / 2) + 10) | 0xf);
      struct plugin_mom_st *newarr =
	MOM_GC_ALLOC ("plugins", newsiz * sizeof (struct plugin_mom_st));
      if (oldcnt > 0)
	memcpy (newarr, oldarr, oldcnt * sizeof (struct plugin_mom_st));
      plugins_mom.plugins_arr = newarr;
      plugins_mom.plugins_size = newsiz;
      if (oldarr)
	{
	  MOM_GC_FREE (oldarr, oldsiz * sizeof (struct plugin_mom_st));
	}
    }
  void *plugdlh = GC_dlopen (plugpath, RTLD_NOW | RTLD_GLOBAL);
  if (!plugdlh)
    MOM_FATAPRINTF ("failed to load plugin %s: %s", plugpath, dlerror ());
  const char *pluggplcompatible =
    dlsym (plugdlh, "mom_plugin_GPL_compatible");
  if (!pluggplcompatible)
    MOM_FATAPRINTF
      ("plugin %s without 'mom_plugin_GPL_compatible' string: %s", plugpath,
       dlerror ());
  mom_plugin_init_t *pluginit =
    (mom_plugin_init_t *) dlsym (plugdlh, "mom_plugin_init");
  if (!pluginit)
    MOM_FATAPRINTF ("plugin %s without 'mom_plugin_init' function: %s",
		    plugpath, dlerror ());
  /// we don't use the index 0 on purpose!
  unsigned plugix = ++plugins_mom.plugins_count;
  plugins_mom.plugins_arr[plugix].plugin_name =
    MOM_GC_STRDUP ("plugin name", plugname);
  plugins_mom.plugins_arr[plugix].plugin_dlh = plugdlh;
  MOM_DEBUGPRINTF (run,
		   "initializing plugin #%d %s from %s argument %s GPL compatible %s",
		   plugix, plugname, plugpath, plugarg, pluggplcompatible);
  pluginit (plugarg, pargc, pargv);
  MOM_INFORMPRINTF ("using plugin #%d %s from %s, GPL compatible: %s",
		    plugix, plugname, plugpath, pluggplcompatible);
  pthread_mutex_unlock (&plugins_mom.plugins_mtx);
}


/// after the initial load, try to run the momplugin_after_load of
/// each plugin defining it
static void
do_after_initial_load_with_plugins_mom (void)
{
  for (unsigned plugix = 1; plugix <= plugins_mom.plugins_count; plugix++)
    {
      void *plugdlh = plugins_mom.plugins_arr[plugix].plugin_dlh;
      const char *plugnam = plugins_mom.plugins_arr[plugix].plugin_name;
      assert (plugdlh != NULL && plugnam != NULL);
      mom_plugin_after_load_t *plugafterload =
	dlsym (plugdlh, "momplugin_after_load");
      if (plugafterload)
	{
	  MOM_DEBUGPRINTF (run, "before after load of plugin#%d %s",
			   plugix, plugnam);
	  plugafterload ();
	  MOM_INFORMPRINTF ("done after load of plugin#%d %s",
			    plugix, plugnam);
	}
    }
}


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
  printf ("\t -P | --plugin <plugin-name> <plugin-arg> "
	  " \t# load a plugin.\n");
  printf ("\t -p | --password-file <password-file> "
	  " \t# Use the given password, or .mompasswd, à la htpasswd(5), for web password\n");
  printf ("\t -W | --web <webhost> "
	  " \t# e.g. -W localhost:8088 for web server\n");
  printf ("\t -r | --doc-root <web-doc-root> "
	  " \t# e.g. -r /var/www/ for adding some web document root\n");
  printf ("\t -S | --socket <socket>"
	  " \t #host:port for TCP socket, /absolute/path for UNIX socket\n");
  putchar ('\n');
  printf ("\t --chdir <directory>" "\t #change directory\n");
  printf ("\t --write-pid <file>"
	  "\t #write the pid (e.g. --write-pid /var/run/monimelt.pid)\n");
  printf ("\t --random-idstr" "\t #output a random idstr then exit\n");
  printf ("\t --string-hash <string>" "\t #output the hash of the string\n");
  printf ("\t --dump-cold-state <dumpdir>" "\t #dump the cold state\n");
  printf ("\t --dump-state <dumpdir>" "\t #dump the final state\n");
  printf ("\t --daemon-noclose"
	  "\t daemonize with daemon(3) with nochdir=true noclose=true\n");
  printf ("\t --add-predefined <predefname> [<comment>]"
	  "\t #add a new predefined and dump\n");
  printf ("\t --generate-c-module <moduleitem>" "\t #generate a C module\n");
  printf ("\t --system <command>"
	  "\t #run an arbitrary (perhaps dangereous) command at argument parsing time\n");
}

static void
print_version_mom (const char *argv0)
{
  printf ("%s built on %s gitcommit %s\n", argv0,
	  monimelt_timestamp, monimelt_lastgitcommit);
}

void
mom_set_debugging (const char *dbgopt)
{
  char dbuf[256];
  if (!dbgopt)
    return;
  memset (dbuf, 0, sizeof (dbuf));
  if (strlen (dbgopt) >= sizeof (dbuf) - 1)
    MOM_FATAPRINTF ("too long debug option %s", dbgopt);
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
#define MOM_TEST_DEBUG_OPTION(Nam)			\
	if (!strcmp(pc,#Nam))		{		\
	  mom_debugflags |=  (1<<momdbg_##Nam); } else	\
	  if (!strcmp(pc,"!"#Nam))			\
	    mom_debugflags &=  ~(1<<momdbg_##Nam); else
	if (!pc)
	  break;
	MOM_DEBUG_LIST_OPTIONS (MOM_TEST_DEBUG_OPTION) if (pc && *pc)
	  MOM_WARNPRINTF ("unrecognized debug flag %s", pc);
      }
  char alldebugflags[2 * sizeof (dbuf) + 120];
  memset (alldebugflags, 0, sizeof (alldebugflags));
  int nbdbg = 0;
#define MOM_SHOW_DEBUG_OPTION(Nam) do {		\
    if (mom_debugflags & (1<<momdbg_##Nam)) {	\
     strcat(alldebugflags, " " #Nam);		\
     assert (strlen(alldebugflags)		\
	     <sizeof(alldebugflags)-3);		\
     nbdbg++;					\
    } } while(0);
  MOM_DEBUG_LIST_OPTIONS (MOM_SHOW_DEBUG_OPTION);
  if (nbdbg > 0)
    MOM_INFORMPRINTF ("%d debug flags active:%s.", nbdbg, alldebugflags);
  else
    MOM_INFORMPRINTF ("no debug flags active.");
}

static void
parse_program_arguments_and_load_plugins_mom (int *pargc, char ***pargv)
{
  int argc = *pargc;
  char **argv = *pargv;
  int opt = -1;
  while ((opt = getopt_long (argc, argv, "lhVdn:P:W:J:D:S:r:p:",
			     mom_long_options, NULL)) >= 0)
    {
      switch (opt)
	{
	case 'h':
	  usage_mom (argv[0]);
	  putchar ('\n');
	  fputs ("\nVersion info:::::\n", stdout);
	  print_version_mom (argv[0]);
	  exit (EXIT_FAILURE);
	  return;
	case 'V':
	  print_version_mom (argv[0]);
	  exit (EXIT_SUCCESS);
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
	    mom_set_debugging (optarg);
	  break;
	case 'J':
	  if (optarg)
	    mom_nb_workers = atoi (optarg);
	  break;
	case 'W':
	  mom_web_host = optarg;
	  break;
	case 'S':
	  mom_socket = optarg;
	  break;
	case 'U':
	  mom_user_data = optarg;
	  break;
	case 'r':		// --doc-root to add a web root directory
	  if (optarg)
	    {
	      struct stat rdstat;
	      memset (&rdstat, 0, sizeof (rdstat));
	      errno = 0;
	      if (stat (optarg, &rdstat)
		  || (rdstat.st_mode & S_IFMT) != S_IFDIR)
		{
		  if (!errno)
		    errno = ENOTDIR;
		  MOM_FATAPRINTF
		    ("bad specified doc-root (for web service) %s directory (%m)",
		     optarg);
		};
	      if (mom_webdocroot[MOM_MAX_WEBDOCROOT - 1])
		MOM_FATAPRINTF ("too many %d web doc-root for %s",
				MOM_MAX_WEBDOCROOT, optarg);
	      for (int ix = 0; ix < MOM_MAX_WEBDOCROOT; ix++)
		if (!mom_webdocroot[ix])
		  {
		    mom_webdocroot[ix] = optarg;
		    MOM_INFORMPRINTF ("adding web doc-root directory %s",
				      optarg);
		    break;
		  };
	    }
	  break;
	case 'p':
	  {
	    if (optarg)
	      mom_webpasswdfile = optarg;
	    MOM_INFORMPRINTF ("using %s as web password file",
			      mom_webpasswdfile);
	  };
	  break;
	case 'P':
	  {
	    char *plugnam = optarg;
	    char *plugarg = argv[optind++];
	    mom_load_plugin (plugnam, plugarg, pargc, pargv);
	  }
	  break;
	case xtraopt_chdir:
	  wanted_dir_mom = optarg;
	  break;
	case xtraopt_dumpstate:
	  if (optarg && !strcmp (optarg, "."))
	    dump_exit_dir_mom = "./";
	  else if (optarg)
	    dump_exit_dir_mom = optarg;
	  break;
	case xtraopt_randomidstr:
	  {
	    errno = 0;
	    const momstring_t *randidstr =
	      mom_make_random_idstr (getpid () % 256, NULL);
	    printf ("%s\n", randidstr->cstr);
	    MOM_WARNPRINTF ("exiting after random id string %s",
			    randidstr->cstr);
	    exit (EXIT_SUCCESS);
	  }
	  break;
	case xtraopt_hashstr:
	  {
	    if (optarg)
	      {
		momhash_t hs = mom_cstring_hash (optarg);
		printf ("hash of string %s is %u = %#x\n", optarg, hs, hs);
		MOM_WARNPRINTF ("exiting after hashing string '%s' as %u",
				optarg, hs);
		exit (EXIT_SUCCESS);
	      }
	  }
	  break;
	case xtraopt_writepid:
	  {
	    write_pid_file_mom = optarg;
	  }
	  break;
	case xtraopt_dumpcoldstate:
	  {
	    dump_cold_dir_mom = optarg;
	  }
	  break;
	case xtraopt_addpredef:
	  {
	    if (nbmorepredef_mom >= MAX_NEW_PREDEF_MOM)
	      MOM_FATAPRINTF ("too many new predefined %d", nbmorepredef_mom);
	    char *predname = optarg;
	    char *comment = argv[optind];
	    if (predname && comment && isalnum (comment[0]))
	      {
		newpredefname_mom[nbmorepredef_mom] = predname;
		newpredefcomment_mom[nbmorepredef_mom] = comment;
		nbmorepredef_mom++;
	      }
	  }
	  break;
	case xtraopt_daemon_noclose:
	  noclose_daemonize_mom = true;
	  break;
	case xtraopt_generate_c_module:
	  {
	    generate_c_module_mom = optarg;
	  }
	  break;
	case xtraopt_system:
	  {
	    if (optarg)
	      {
		fflush (NULL);
		MOM_INFORMPRINTF
		  ("before running command with system(3) : %s", optarg);
		fflush (NULL);
		sleep (1);
		int ok = system (optarg);
		if (ok)
		  MOM_FATAPRINTF
		    ("failed to run command : %s ; got %d status",
		     optarg, ok);
		else
		  MOM_INFORMPRINTF ("did run command : %s", optarg);
	      }
	    else
	      MOM_FATAPRINTF ("invalid --system %s", optarg);
	  }
	  break;
	default:
	  {
	    if (opt > 0 && opt < UCHAR_MAX && isalpha ((char) opt))
	      MOM_FATAPRINTF ("unknown option %c", opt);
	    else
	      MOM_FATAPRINTF ("unknown option #%d", opt);
	  }
	  break;
	}
    }
  argc -= optind;
  argv += optind;
  *pargc = argc;
  *pargv = argv;
}



static void
do_generate_c_module_mom (void)
{
  bool backedup = false;
  momitem_t *moditm = mom_find_item (generate_c_module_mom);
  if (!moditm)
    MOM_FATAPRINTF ("cannot find C module %s", generate_c_module_mom);
  char cpcmdbuf[256];
  memset (cpcmdbuf, 0, sizeof (cpcmdbuf));
  char pathbuf[128];
  memset (pathbuf, 0, sizeof (pathbuf));
  if (snprintf (pathbuf, sizeof (pathbuf),
		MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX "%s.c",
		mom_item_cstring (moditm)) >=
      (int) sizeof (pathbuf) - /*space for .bad or % suffix */ 6)
    MOM_FATAPRINTF ("too long pathbuf %s for C module %s", pathbuf,
		    mom_item_cstring (moditm));
  if (!access (pathbuf, R_OK))
    {
      snprintf (cpcmdbuf, sizeof (cpcmdbuf),
		"cp -vb %s %s%%", pathbuf, pathbuf);
      MOM_INFORMPRINTF ("before backup: %s", cpcmdbuf);
      fflush (NULL);
      int backfail = system (cpcmdbuf);
      if (backfail)
	MOM_WARNPRINTF ("backup with %s failed %d", cpcmdbuf, backfail);
      else
	backedup = true;
    };
  momvalue_t valgen = MOM_NONEV;
  MOM_INFORMPRINTF ("before generating C module %s",
		    mom_item_cstring (moditm));
  if (!momhook_generate_c_module (moditm, &valgen))
    MOM_WARNPRINTF ("failed to generate C module %s",
		    mom_item_cstring (moditm));
  else
    MOM_INFORMPRINTF ("after generating C module %s got %s",
		      mom_item_cstring (moditm),
		      mom_output_gcstring (valgen));
  // if generation succeeded, we need to compile and dump
  if (valgen.typnum == momty_item && valgen.vitem == moditm)
    {
      char makecmdbuf[256];
      memset (makecmdbuf, 0, sizeof (makecmdbuf));
      snprintf (makecmdbuf, sizeof (makecmdbuf), "make "
		MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX "%s.so",
		mom_item_cstring (moditm));
      MOM_INFORMPRINTF ("before building generated module with : %s",
			makecmdbuf);
      fflush (NULL);
      int buildfail = system (makecmdbuf);
      if (buildfail)
	{
	  if (backedup)
	    {
	      char badbuf[sizeof (pathbuf)];
	      char backupbuf[sizeof (pathbuf)];
	      memset (badbuf, 0, sizeof (badbuf));
	      memset (backupbuf, 0, sizeof (backupbuf));
	      snprintf (badbuf, sizeof (badbuf), "%s.bad", pathbuf);
	      snprintf (backupbuf, sizeof (backupbuf), "%s%%", pathbuf);
	      if (MOM_UNLIKELY (rename (pathbuf, badbuf)))
		MOM_FATAPRINTF ("failed to rename %s as %s - %m", pathbuf,
				badbuf);
	      if (MOM_UNLIKELY (rename (backupbuf, pathbuf)))
		MOM_FATAPRINTF ("failed to rename backup %s as %s - %m",
				backupbuf, pathbuf);
	      MOM_INFORMPRINTF
		("restored backed up %s as %s, and renamed generated file to bad file %s",
		 backupbuf, pathbuf, badbuf);
	    };
	  MOM_FATAPRINTF ("failed to build generated with : %s (got %d)",
			  makecmdbuf, buildfail);
	}
      else
	MOM_INFORMPRINTF ("successfully built generated module with : %s",
			  makecmdbuf);
      if (!dump_exit_dir_mom)
	{
	  MOM_INFORMPRINTF
	    ("after successful generation of C module %s will dump state",
	     mom_item_cstring (moditm));
	  dump_exit_dir_mom = "./";
	}
    }
}


static bool daemonize_mom = false;
static bool noclose_daemonize_mom = false;

static void *
jansson_malloc_mom (size_t sz)
{
  return MOM_GC_ALLOC ("jansson malloc", sz);
};

int
main (int argc_main, char **argv_main)
{
  GC_INIT ();
  char **argv = argv_main;
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
  json_set_alloc_funcs (jansson_malloc_mom, GC_free);
  mom_initialize_random ();
  json_object_seed (0);		/* use random device ie system entropy source */
  mom_prog_dlhandle = GC_dlopen (NULL, RTLD_NOW | RTLD_GLOBAL);
  if (MOM_UNLIKELY (!mom_prog_dlhandle))
    MOM_FATAPRINTF ("failed to dlopen the program: %s", dlerror ());
  {
    int arg0len = 0;
    if (argc > 1 && (arg0len = strlen (argv[0]) > 5)
	&& !strcmp (argv[0] + arg0len - 4, ".run"))
      {
	char **newargv = GC_MALLOC ((argc + 3) * sizeof (char *));
	if (!newargv)
	  MOM_FATAPRINTF ("failed to grow argv for .run suffix: %m");
	memset (newargv, 0, (argc + 3) * sizeof (char *));
	newargv[0] = argv[0];
	newargv[1] = "--run";
	for (int ix = 1; ix < argc; ix++)
	  newargv[ix + 1] = argv[ix];
	argv = newargv;
      }
  }
  parse_program_arguments_and_load_plugins_mom (&argc, &argv);
  mom_initialize_items ();
  if (mom_nb_workers < MOM_MIN_WORKERS)
    mom_nb_workers = MOM_MIN_WORKERS;
  else if (mom_nb_workers > MOM_MAX_WORKERS)
    mom_nb_workers = MOM_MAX_WORKERS;
  mom_load_state ();
  MOM_DEBUGPRINTF (load, "loaded state");
  MOM_INFORMPRINTF ("MONIMELT loaded state, process %d", (int) getpid ());
  if (nbmorepredef_mom > 0)
    {
      for (unsigned prix = 0; prix < nbmorepredef_mom; prix++)
	{
	  momitem_t *pritm =
	    mom_make_predefined_named_item (newpredefname_mom[prix]);
	  if (newpredefcomment_mom[prix]
	      && strlen (newpredefcomment_mom[prix]) > 0)
	    {
	      momvalue_t valstr =
		mom_stringv (mom_make_string_cstr
			     (newpredefcomment_mom[prix]));
	      pritm->itm_attrs	//
		= mom_attributes_put (pritm->itm_attrs,
				      MOM_PREDEFINED_NAMED (comment),
				      &valstr);
	    }
	  MOM_INFORMPRINTF ("made predefined %s", pritm->itm_name->cstr);
	  if (!dump_exit_dir_mom || !dump_exit_dir_mom[0]
	      || !strcmp (dump_exit_dir_mom, "."))
	    dump_exit_dir_mom = "./";
	}
    }
  do_after_initial_load_with_plugins_mom ();
  if (generate_c_module_mom)
    do_generate_c_module_mom ();
  if (mom_web_host || mom_socket)
    mom_run_workers ();
  MOM_INFORMPRINTF
    ("after run sizeof(momvalue_t)=%zd sizeof(momvaltype_t)=%zd sizeof(momitem_t)=%zd\n",
     sizeof (momvalue_t), sizeof (momvaltype_t), sizeof (momitem_t));
  if (dump_exit_dir_mom)
    mom_dump_state (dump_exit_dir_mom);
  if (nbmorepredef_mom > 0)
    {
      char makebuf[64];
      memset (makebuf, 0, sizeof (makebuf));
      snprintf (makebuf, sizeof (makebuf), "make -j %d", mom_nb_workers);
      if (system (makebuf))
	MOM_FATAPRINTF ("failed to make -j %d after predefined",
			mom_nb_workers);
    }
  return 0;
}
