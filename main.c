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

pthread_mutex_t mom_run_mtx = PTHREAD_MUTEX_INITIALIZER;

static int nicelevel = 0;
static const char *json_file;
static const char *json_string;
static const char *wanted_dir;
static bool json_indented;
static bool daemonize_me;
static bool want_syslog;
static bool using_syslog;



/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_jsonfile = 1024,
  xtraopt_jsonstring,
  xtraopt_jsonindent,
  xtraopt_chdir,
  xtraopt_loadstate,
  xtraopt_dumpstate,
};

static const struct option mom_long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {"nice", required_argument, NULL, 'n'},
  {"module", required_argument, NULL, 'M'},
  {"jobs", required_argument, NULL, 'J'},
  {"daemon", no_argument, NULL, 'd'},
  {"syslog", no_argument, NULL, 'l'},
  // long-only options
  {"json-file", required_argument, NULL, xtraopt_jsonfile},
  {"json-string", required_argument, NULL, xtraopt_jsonstring},
  {"json-indent", no_argument, NULL, xtraopt_jsonindent},
  {"load-state", required_argument, NULL, xtraopt_loadstate},
  {"dump-state", required_argument, NULL, xtraopt_dumpstate},
  {"chdir", required_argument, NULL, xtraopt_chdir},
  /* Terminating NULL placeholder.  */
  {NULL, no_argument, NULL, 0},
};

static GOptionContext *option_ctx;
static const char **module_names;
static const char **module_arguments;
static GModule **module_handles;
static unsigned module_count;
static unsigned module_size;
static const char *load_state_path = MONIMELT_DEFAULT_STATE_FILE;
static const char *dump_state_path;
static void
usage (const char *argv0)
{
  printf ("Usage: %s\n", argv0);
  printf ("\t -h | --help " " \t# Give this help.\n");
  printf ("\t -V | --version " " \t# Give version information.\n");
  printf ("\t -d | --daemon " " \t# Daemonize.\n");
  printf ("\t -l | --syslog " " \t# Log to syslog.\n");
  printf ("\t -n | --nice <nice-level> " " \t# Set process nice level.\n");
  printf ("\t -J | --jobs <nb-work-threads> " " \t# Start work threads.\n");
  printf ("\t -M | --module <module-name> <module-arg> "
	  " \t# load a plugin.\n");
  putchar ('\n');
  printf ("\t --json-file <file-name>" "\t #parse JSON file for testing\n");
  printf ("\t --json-string <string>" "\t #parse JSON string for testing\n");
  printf ("\t --json-indent" "\t #output parsed JSON with indentation\n");
  printf ("\t --load-state <sqlite-state>" "\t #load an initial state\n");
  printf ("\t --dump-state <sqlite-state>" "\t #dump an initial state\n");
  printf ("\t --chdir <directory>" "\t #change directory\n");
  if (option_ctx)
    {
      printf ("## additional help\n%s\n",
	      g_option_context_get_help (option_ctx, FALSE, NULL));
    }
}

static void
print_version (const char *argv0)
{
  printf ("%s built on " __DATE__ "@" __TIME__ "\n", argv0);
}


static void
load_module (GOptionContext * optctx, unsigned ix, const char *modname,
	     const char *modarg)
{
  GModule *modhdl = g_module_open (modname, 0);
  char *modbasename = basename (modname);
  if (!modhdl)
    MONIMELT_FATAL ("failed to load module #%d named %s: %s",
		    ix, modname, g_module_error ());
  const char *modlic = NULL;
  if (!g_module_symbol
      (modhdl, "monimelt_GPL_friendly_module", (gpointer *) & modlic)
      || !modlic)
    MONIMELT_FATAL
      ("module named %s without 'monimelt_GPL_friendly_module' symbol: %s",
       modname, g_module_error ());
  typedef void modinit_sig_t (const char *);
  modinit_sig_t *modinit = NULL;
  if (!g_module_symbol
      (modhdl, "monimelt_module_init", (gpointer *) & modinit) || !modinit)
    MONIMELT_FATAL
      ("module named %s without 'monimelt_module_init' function: %s", modname,
       g_module_error ());
  modinit (modarg);
  typedef GOptionGroup *modoptgroup_sig_t (const char *modname);
  modoptgroup_sig_t *modoptgroup = NULL;
  if (g_module_symbol
      (modhdl, "monimelt_module_option_group", (gpointer *) & modoptgroup)
      && modoptgroup)
    {
      GOptionGroup *optgrp = modoptgroup (modbasename);
      if (optgrp)
	g_option_context_add_group (optctx, optgrp);
    }
  if (using_syslog)
    syslog (LOG_INFO, "loaded module #%d %s with argument %s",
	    ix, modname, modarg);
  module_handles[ix] = modhdl;
}

static void
parse_program_arguments_and_load_modules (int argc, char **argv)
{
  int opt = -1;
  option_ctx = g_option_context_new ("monimelt");
  while ((opt = getopt_long (argc, argv, "hVdln:M:J:",
			     mom_long_options, NULL)) >= 0)
    {
      switch (opt)
	{
	case 'h':
	  usage (argv[0]);
	  exit (EXIT_FAILURE);
	  return;
	case 'V':
	  print_version (argv[0]);
	  break;
	case 'l':
	  want_syslog = true;
	  break;
	case 'd':
	  daemonize_me = true;
	  break;
	case 'J':
	  if (optarg)
	    mom_nb_workers = atoi (optarg);
	  break;
	case 'M':
	  {
	    char *modnam = optarg;
	    char *modarg = argv[optind++];
	    if (!modnam || !modarg)
	      {
		fprintf (stderr, "monimelt bad module specification\n");
		usage (argv[0]);
		exit (EXIT_FAILURE);
	      };
	    if (MONIMELT_UNLIKELY (module_count + 1 >= module_size))
	      {
		unsigned newsize = ((5 * module_count / 4 + 10) | 0xf) + 1;
		const char **newnames = GC_MALLOC (newsize * sizeof (char *));
		const char **newargs = GC_MALLOC (newsize * sizeof (char *));
		if (!newnames || !newargs)
		  MONIMELT_FATAL ("failed to grow modules to %d",
				  (int) newsize);
		memset (newnames, 0, newsize * sizeof (char *));
		memset (newargs, 0, newsize * sizeof (char *));
		if (module_names)
		  memcpy (newnames, module_names,
			  sizeof (char *) * module_count);
		if (module_arguments)
		  memcpy (newargs, module_arguments,
			  sizeof (char *) * module_count);
		module_names = newnames;
		module_arguments = newargs;
		module_size = newsize;
	      }
	    load_module (option_ctx, module_count, modnam, modarg);
	    module_count++;
	  }
	case xtraopt_jsonfile:
	  json_file = optarg;
	  break;
	case xtraopt_jsonstring:
	  json_string = optarg;
	  break;
	case xtraopt_jsonindent:
	  json_indented = true;
	  break;
	case xtraopt_chdir:
	  wanted_dir = optarg;
	  break;
	case xtraopt_loadstate:
	  load_state_path = optarg;
	  break;
	case xtraopt_dumpstate:
	  dump_state_path = optarg;
	  break;
	default:
	  break;
	}
    }
}

void
mom_initialize (void)
{
  extern void mom_initialize_items (void);
  extern void mom_initialize_globals (void);
  extern void mom_initialize_spaces (void);
  extern void mom_create_items ();
  static int inited;
  if (inited)
    return;
  inited = 1;
  pthread_mutexattr_init (&mom_normal_mutex_attr);
  pthread_mutexattr_init (&mom_recursive_mutex_attr);
  mom_prog_module = g_module_open (NULL, 0);
  pthread_mutexattr_settype (&mom_normal_mutex_attr, PTHREAD_MUTEX_NORMAL);
  pthread_mutexattr_settype (&mom_recursive_mutex_attr,
			     PTHREAD_MUTEX_RECURSIVE);
  mom_initialize_items ();
  mom_initialize_spaces ();
  mom_initialize_globals ();
  mom_create_items ();
}

static void
do_json_file_test (void)
{
  FILE *fj = fopen (json_file, "r");
  if (!fj)
    MONIMELT_FATAL ("failed to open json file %s", json_file);
  struct jsonparser_st jp = { 0 };
  mom_initialize_json_parser (&jp, fj, NULL);
  char *errmsg = NULL;
  momval_t vp = mom_parse_json (&jp, &errmsg);
  if (errmsg)
    MONIMELT_FATAL ("failed to parse %s json file: %s", json_file, errmsg);
  mom_close_json_parser (&jp);
  struct jsonoutput_st jo = { 0 };
  mom_json_output_initialize
    (&jo, stdout, NULL,
     (json_indented ? jsof_indent : 0) | jsof_flush | jsof_cname);
  printf ("parsed JSON from file %s\n", json_file);
  mom_output_json (&jo, vp);
  putchar ('\n');
  mom_json_output_end (&jo);
}

static void
do_json_string_test (void)
{
  FILE *fj = fmemopen ((void *) json_string, strlen (json_string), "r");
  if (!fj)
    MONIMELT_FATAL ("failed to open json string %s", json_string);
  struct jsonparser_st jp = { 0 };
  mom_initialize_json_parser (&jp, fj, NULL);
  char *errmsg = NULL;
  momval_t vp = mom_parse_json (&jp, &errmsg);
  if (errmsg)
    MONIMELT_FATAL ("failed to parse %s json string: %s", json_string,
		    errmsg);
  mom_close_json_parser (&jp);
  struct jsonoutput_st jo = { 0 };
  mom_json_output_initialize
    (&jo, stdout, NULL,
     (json_indented ? jsof_indent : 0) | jsof_flush | jsof_cname);
  printf ("parsed JSON from string %s\n", json_string);
  mom_output_json (&jo, vp);
  putchar ('\n');
  mom_json_output_end (&jo);
}


void
mom_fatal_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  struct tm tm = { };
  int err = errno;
  time_t now = 0;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  time (&now);
  strftime (timbuf, sizeof (timbuf), "%Y-%b-%d %T %Z",
	    localtime_r (&now, &tm));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  va_list args;
  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MONIMELT_UNLIKELY (len >= sizeof (buf) - 1))
    {
      bigbuf = malloc (len + 10);
      if (bigbuf)
	{
	  memset (bigbuf, 0, len + 10);
	  va_start (args, fmt);
	  (void) vsnprintf (bigbuf, len + 1, fmt, args);
	  va_end (args);
	}
    }
  if (using_syslog)
    {
      if (err)
	syslog (LOG_ALERT, "MONIMELT FATAL @%s:%d <%s> %s %s (%s)",
		fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf,
		strerror (err));
      else
	syslog (LOG_ALERT, "MONIMELT FATAL @%s:%d <%s> %s %s",
		fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
    }
  else
    {
      fputc ('\n', stderr);
      if (err)
	fprintf (stderr, "MONIMELT FATAL @%s:%d <%s> %s %s (%s)\n",
		 fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf,
		 strerror (err));
      else
	fprintf (stderr, "MONIMELT FATAL @%s:%d <%s> %s %s\n",
		 fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
      fflush (stderr);
    }
  abort ();
}


void
mom_inform_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  struct tm tm = { };
  time_t now = 0;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  time (&now);
  strftime (timbuf, sizeof (timbuf), "%Y-%b-%d %T %Z",
	    localtime_r (&now, &tm));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  va_list args;
  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MONIMELT_UNLIKELY (len >= sizeof (buf) - 1))
    {
      bigbuf = malloc (len + 10);
      if (bigbuf)
	{
	  memset (bigbuf, 0, len + 10);
	  va_start (args, fmt);
	  (void) vsnprintf (bigbuf, len + 1, fmt, args);
	  va_end (args);
	}
    }
  if (using_syslog)
    {
      syslog (LOG_INFO, "MONIMELT INFO @%s:%d <%s> %s %s",
	      fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
    }
  else
    {
      fputc ('\n', stderr);
      fprintf (stderr, "MONIMELT INFO @%s:%d <%s> %s %s\n",
	       fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
      fflush (stderr);
    }
}


static void
logexit_cb (void)
{
  char timbuf[64];
  struct tm tm = { };
  time_t now = 0;
  memset (timbuf, 0, sizeof (timbuf));
  time (&now);
  strftime (timbuf, sizeof (timbuf), "%Y-%b-%d %T %Z",
	    localtime_r (&now, &tm));
  syslog (LOG_INFO, "monimelt exiting at %s", timbuf);
}

static gpointer
checked_gc_malloc (gsize sz)
{
  void *p = GC_MALLOC (sz);
  if (MONIMELT_UNLIKELY (!p))
    MONIMELT_FATAL ("failed to GC malloc %ld bytes", (long) sz);
  memset (p, 0, sz);
  return p;
}

static gpointer
checked_gc_realloc (gpointer m, gsize sz)
{
  void *p = GC_REALLOC (m, sz);
  if (MONIMELT_UNLIKELY (!p))
    MONIMELT_FATAL ("failed to GC realloc %ld bytes", (long) sz);
  return p;
}

static gpointer
checked_gc_calloc (gsize nblock, gsize bsize)
{
  void *p = GC_MALLOC (nblock * bsize);
  if (MONIMELT_UNLIKELY (!p))
    MONIMELT_FATAL ("failed to GC calloc %ld blocks of %ld bytes",
		    (long) nblock, (long) bsize);
  memset (p, 0, nblock * bsize);
  return p;
}

static GMemVTable gc_mem_vtable = {
  .malloc = checked_gc_malloc,
  .realloc = checked_gc_realloc,
  .free = GC_free,
  .calloc = checked_gc_calloc,
  .try_malloc = GC_malloc,
  .try_realloc = GC_realloc
};

int
main (int argc, char **argv)
{
  GC_INIT ();
  pthread_setname_np (pthread_self (), "monimelt-main");
  g_mem_gc_friendly = TRUE;
  g_mem_set_vtable (&gc_mem_vtable);
  mom_initialize ();
  parse_program_arguments_and_load_modules (argc, argv);
  if (json_file)
    do_json_file_test ();
  if (json_string)
    do_json_string_test ();
  if (nicelevel != 0)
    {
      if (MONIMELT_UNLIKELY (nice (nicelevel) == -1 && errno))
	MONIMELT_FATAL ("failed to nice at level %d", nicelevel);
    }
  if (wanted_dir)
    {
      if (MONIMELT_UNLIKELY (chdir (wanted_dir)))
	MONIMELT_FATAL ("failed to chdir to %s", wanted_dir);
    }
  if (daemonize_me)
    {
      fprintf (stderr, "%s before daemonizing pid %d\n", argv[0],
	       (int) getpid ());
      fflush (NULL);
      if (MONIMELT_UNLIKELY (daemon ( /*nochdir */ 1, /*noclose */ 0)))
	MONIMELT_FATAL ("failed to daemonize from pid #%d", (int) getpid ());
      want_syslog = true;
    }
  if (want_syslog)
    {
      char timbuf[64];
      struct tm tm = { };
      time_t now = 0;
      memset (timbuf, 0, sizeof (timbuf));
      time (&now);
      strftime (timbuf, sizeof (timbuf), "%Y-%b-%d %T %Z",
		localtime_r (&now, &tm));
      openlog ("monimelt",
	       LOG_PID | LOG_CONS | (daemonize_me ? 0 : LOG_PERROR),
	       LOG_LOCAL2);
      syslog (LOG_INFO, "monimelt starting at %s in %s",
	      timbuf, get_current_dir_name ());
      using_syslog = true;
      atexit (logexit_cb);
    }
  if (load_state_path && load_state_path[0])
    {
      mom_initial_load (load_state_path);
    }
  if (mom_nb_workers > 0)
    {
      MONIMELT_INFORM ("start %d workers", (int) mom_nb_workers);
      mom_run ();
      mom_wait_for_stop ();
      MONIMELT_INFORM ("done %d workers", (int) mom_nb_workers);
    }
  if (dump_state_path)
    {
      mom_full_dump (dump_state_path);
    }
  return 0;
}
