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

pthread_mutex_t mom_run_mtx = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static int nicelevel = 0;
static const char *json_file;
static const char *json_string;
static const char *wanted_dir;
static const char *web_host;
static bool json_indented;
static bool daemonize_me;
static bool want_syslog;
static bool using_syslog;
static bool dont_want_event_loop;


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
  xtraopt_noeventloop,
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
  {"json-file", required_argument, NULL, xtraopt_jsonfile},
  {"json-string", required_argument, NULL, xtraopt_jsonstring},
  {"json-indent", no_argument, NULL, xtraopt_jsonindent},
  {"load-state", required_argument, NULL, xtraopt_loadstate},
  {"dump-state", required_argument, NULL, xtraopt_dumpstate},
  {"chdir", required_argument, NULL, xtraopt_chdir},
  {"no-event-loop", no_argument, NULL, xtraopt_noeventloop},
  /* Terminating NULL placeholder.  */
  {NULL, no_argument, NULL, 0},
};

const char *const mom_debug_names[momdbg__last] = {
#define MOM_DEFINE_DBG_NAME(Dbg) [momdbg_##Dbg] #Dbg,
  MOM_DEBUG_LIST_OPTIONS (MOM_DEFINE_DBG_NAME)
};

static GOptionContext *option_ctx;
static const char **module_names;
static const char **module_arguments;
static GModule **module_handles;
static unsigned module_count;
static unsigned module_size;
static const char *load_state_path = MOM_DEFAULT_STATE_FILE;
static const char *dump_state_path;
static void
usage (const char *argv0)
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
  printf ("\t -l | --syslog " " \t# Log to syslog.\n");
  printf ("\t -n | --nice <nice-level> " " \t# Set process nice level.\n");
  printf ("\t -J | --jobs <nb-work-threads> " " \t# Start work threads.\n");
  printf ("\t -M | --module <module-name> <module-arg> "
	  " \t# load a plugin.\n");
  printf ("\t -W | --web <webhost>\n");
  putchar ('\n');
  printf ("\t --json-file <file-name>" "\t #parse JSON file for testing\n");
  printf ("\t --json-string <string>" "\t #parse JSON string for testing\n");
  printf ("\t --json-indent" "\t #output parsed JSON with indentation\n");
  printf ("\t --no-event-loop" "\t #Dont start the event loop thread\n");
  printf ("\t --load-state <sqlite-state>"
	  "\t #load an initial state, or noting if - or . \n");
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
add_debugging (const char *dbgopt)
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
      MOM_INFORM ("set all debugging");
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
	  MOM_FATAL ("unrecognized debug flag %s", pc);
      }
}


static void
load_module (GOptionContext * optctx, unsigned ix, const char *modname,
	     const char *modarg)
{
  GModule *modhdl = g_module_open (modname, 0);
  char *modbasename = basename (modname);
  if (!modhdl)
    MOM_FATAL ("failed to load module #%d named %s: %s",
	       ix, modname, g_module_error ());
  if (module_count + 1 >= module_size)
    {
      unsigned newsiz = (((3 * module_count / 2) + 10) | 0xf) + 1;
      GModule **newarrmod =
	MOM_GC_ALLOC ("grown module array", newsiz * sizeof (GModule *));
      const char **newarrname =
	MOM_GC_ALLOC ("grown module name array", newsiz * sizeof (char *));
      const char **newarrarg =
	MOM_GC_ALLOC ("grown module arg array", newsiz * sizeof (char *));
      if (module_count > 0)
	{
	  memcpy (newarrmod, module_handles,
		  module_count * sizeof (GModule *));
	  memcpy (newarrname, module_names, module_count * sizeof (char *));
	  memcpy (newarrarg, module_arguments,
		  module_count * sizeof (char *));
	}
      MOM_GC_FREE (module_handles);
      module_handles = newarrmod;
      MOM_GC_FREE (module_arguments);
      module_arguments = newarrarg;
      MOM_GC_FREE (module_names);
      module_names = newarrname;
      module_size = newsiz;
    }
  const char *modlic = NULL;
  if (!g_module_symbol
      (modhdl, "mom_GPL_friendly_module", (gpointer *) & modlic) || !modlic)
    MOM_FATAL
      ("module named %s without 'mom_GPL_friendly_module' symbol: %s",
       modname, g_module_error ());
  typedef void modinit_sig_t (const char *);
  modinit_sig_t *modinit = NULL;
  if (!g_module_symbol
      (modhdl, "mom_module_init", (gpointer *) & modinit) || !modinit)
    MOM_FATAL
      ("module named %s without 'mom_module_init' function: %s", modname,
       g_module_error ());
  modinit (modarg);
  typedef GOptionGroup *modoptgroup_sig_t (const char *modname);
  modoptgroup_sig_t *modoptgroup = NULL;
  if (g_module_symbol
      (modhdl, "mom_module_option_group", (gpointer *) & modoptgroup)
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
  while ((opt = getopt_long (argc, argv, "hVdln:M:W:J:D:",
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
	case 'D':
	  if (optarg)
	    add_debugging (optarg);
	  break;
	case 'J':
	  if (optarg)
	    mom_nb_workers = atoi (optarg);
	  break;
	case 'W':
	  web_host = optarg;
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
	    load_module (option_ctx, module_count, modnam, modarg);
	    module_count++;
	  }
	  break;
	case xtraopt_jsonfile:
	  json_file = optarg;
	  break;
	case xtraopt_jsonstring:
	  json_string = optarg;
	  break;
	case xtraopt_jsonindent:
	  json_indented = true;
	  break;
	case xtraopt_noeventloop:
	  dont_want_event_loop = true;
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
  herror_t *herr = httpd_init (argc - optind, argv + optind);
  if (herr)
    MOM_FATAL ("failed to initialize nanohttp: %s in %s",
	       herror_message (herr), herror_func (herr));
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
  curl_global_init (CURL_GLOBAL_DEFAULT);
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


//// run the post load routines inside modules
static void
modules_post_load (void)
{
  assert (module_count <= module_size);
  assert (module_handles != NULL);
  unsigned nbpostload = 0;
  for (unsigned ix = 0; ix < module_count; ix++)
    {
      GModule *modhdl = module_handles[ix];
      if (!modhdl)
	continue;
      typedef void post_load_sig_t (void);
      post_load_sig_t *modpostload = NULL;
      if (g_module_symbol
	  (modhdl, "mom_module_post_load", (gpointer *) & modpostload)
	  && modpostload)
	{
	  MOM_DEBUG (run, "before post loading module #%d", (int) ix);
	  modpostload ();
	  MOM_DEBUG (run, "after post loading module #%d", (int) ix);
	  nbpostload++;
	}
    };
  MOM_INFORM ("Done %d post load routines for %d modules", nbpostload,
	      module_count);
}




static void
do_json_file_test (void)
{
  FILE *fj = fopen (json_file, "r");
  MOM_INFORM ("starting JSON file test on %s", json_file);
  if (!fj)
    MOM_FATAL ("failed to open json file %s", json_file);
  struct jsonparser_st jp = { 0 };
  mom_initialize_json_parser (&jp, fj, NULL);
  char *errmsg = NULL;
  momval_t vp = mom_parse_json (&jp, &errmsg);
  if (errmsg)
    MOM_FATAL ("failed to parse %s json file: %s", json_file, errmsg);
  mom_close_json_parser (&jp);
  struct jsonoutput_st jo = { 0 };
  mom_json_output_initialize
    (&jo, stdout, NULL,
     (json_indented ? jsof_indent : 0) | jsof_flush | jsof_cname);
  printf ("parsed JSON from file %s\n", json_file);
  mom_output_json (&jo, vp);
  putchar ('\n');
  mom_dbgout_value (vp);
  mom_json_output_end (&jo);
  MOM_INFORM ("ended JSON file test on %s", json_file);
}

static void
do_json_string_test (void)
{
  MOM_INFORM ("starting JSON string test on %s", json_string);
  FILE *fj = fmemopen ((void *) json_string, strlen (json_string), "r");
  if (!fj)
    MOM_FATAL ("failed to open json string %s", json_string);
  struct jsonparser_st jp = { 0 };
  mom_initialize_json_parser (&jp, fj, NULL);
  char *errmsg = NULL;
  momval_t vp = mom_parse_json (&jp, &errmsg);
  if (errmsg)
    MOM_FATAL ("failed to parse %s json string: %s", json_string, errmsg);
  mom_close_json_parser (&jp);
  struct jsonoutput_st jo = { 0 };
  mom_json_output_initialize
    (&jo, stdout, NULL,
     (json_indented ? jsof_indent : 0) | jsof_flush | jsof_cname);
  printf ("parsed JSON from string %s\n", json_string);
  mom_output_json (&jo, vp);
  mom_json_output_end (&jo);
  putchar ('\n');
  putchar ('\n');
  if (*vp.ptype == momty_jsonarray)
    MOM_INFORM ("parsed JSON array of %d components",
		mom_json_array_size (vp));
  else if (*vp.ptype == momty_jsonobject)
    MOM_INFORM ("parsed JSON object of %d entries", mom_jsonob_size (vp));
  else if (!vp.ptr)
    MOM_INFORM ("parsed nil");
  else
    MOM_INFORM ("parsed value of type %d", (int) mom_type (vp));
  mom_dbgout_value (vp);
  MOM_INFORM ("ended JSON string test on %s", json_string);
}



// call strftime but replace .__ with centiseconds for current time
static void
strftime_centi_now (char *buf, size_t len, char *fmt)
{
  struct tm tm = { };
  time_t now = 0;
  struct timespec ts = { };
  if (!buf || !fmt)
    return;
  memset (buf, 0, len);
  clock_gettime (CLOCK_REALTIME, &ts);
  now = ts.tv_sec;
  strftime (buf, len, fmt, localtime_r (&now, &tm));
  char *dotundund = strstr (buf, ".__");
  if (dotundund)
    {
      char minibuf[8];
      memset (minibuf, 0, sizeof (minibuf));
      snprintf (minibuf, sizeof (minibuf), "%02d",
		(int) (ts.tv_nsec / 10000000));
      dotundund[1] = minibuf[0];
      dotundund[2] = minibuf[1];
    }
}

void
mom_fatal_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  int err = errno;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  strftime_centi_now (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S.__ %Z");
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  va_list args;
  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  strftime_centi_now (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S.__ %Z");
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  va_list args;
  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
      fprintf (stderr, "\nMONIMELT INFO @%s:%d <%s> %s %s\n",
	       fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
      fflush (stderr);
    }
}


static pthread_mutex_t dbg_mtx = PTHREAD_MUTEX_INITIALIZER;

void
mom_debug_at (enum mom_debug_en dbg, const char *fil, int lin,
	      const char *fmt, ...)
{
  int len = 0;
  static long dbgcount;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  pthread_mutex_lock (&dbg_mtx);
  strftime_centi_now (timbuf, sizeof (timbuf),
		      (dbgcount % 16 ==
		       0) ? "%Y-%b-%d %H:%M:%S.__ %Z" : "%H:%M:%S.__");
  dbgcount++;
  va_list args;
  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
      syslog (LOG_DEBUG, "MONIMELT DEBUG %s <%s> @%s:%d %s %s",
	      mom_debug_names[dbg], thrname,
	      fil, lin, timbuf, bigbuf ? bigbuf : buf);
    }
  else
    {
      fprintf (stderr, "MONIMELT DEBUG %s <%s> @%s:%d %s %s\n",
	       mom_debug_names[dbg], thrname,
	       fil, lin, timbuf, bigbuf ? bigbuf : buf);
      fflush (stderr);
    }
  if (bigbuf)
    free (bigbuf);
  pthread_mutex_unlock (&dbg_mtx);
}

void
mom_dbg_item_at (enum mom_debug_en dbg, const char *file, int line,
		 const char *msg, const mom_anyitem_t * itm)
{
  char thrname[24];
  char timbuf[64];
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  strftime_centi_now (timbuf, sizeof (timbuf), "%H:%M:%S.__");
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  pthread_mutex_lock (&dbg_mtx);
  fprintf (stderr, "MONIMELT DBG_ITEM %s <%s> %s @%s:%d %s",
	   mom_debug_names[dbg], thrname, timbuf, file, line, msg);
  mom_debugprint_item (stderr, itm);
  if (mom_name_of_item (itm))
    {
      char uidstr[UUID_PARSED_LEN];
      memset (uidstr, 0, sizeof (uidstr));
      fprintf (stderr, " {%s}", mom_unparse_item_uuid (itm, uidstr));
    }
  if (itm && itm->typnum < momty__last
      && mom_typedescr_array[itm->typnum] != NULL)
    fprintf (stderr, " /.%s", mom_typedescr_array[itm->typnum]->ityp_name);
  putc ('\n', stderr);
  fflush (stderr);
  pthread_mutex_unlock (&dbg_mtx);
}


void
mom_dbg_value_at (enum mom_debug_en dbg, const char *fil, int lin,
		  const char *msg, const momval_t val)
{
  char thrname[24];
  char timbuf[64];
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  strftime_centi_now (timbuf, sizeof (timbuf), "%H:%M:%S.__");
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  pthread_mutex_lock (&dbg_mtx);
  fprintf (stderr, "MOM_DBG_VALUE %s <%s> %s %s:%d:%s",
	   mom_debug_names[dbg], thrname, timbuf, fil, lin, msg);
  mom_debugprint_value (stderr, val);
  unsigned tynum = 0;
  if (val.ptr && (tynum = *val.ptype) > momty__itemlowtype
      && tynum < momty__last && mom_typedescr_array[tynum])
    fprintf (stderr, " /.%s", mom_typedescr_array[tynum]->ityp_name);
  putc ('\n', stderr);
  fflush (stderr);
  pthread_mutex_unlock (&dbg_mtx);
}

void
mom_warning_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[128];
  char timbuf[64];
  char *bigbuf = NULL;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  strftime_centi_now (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S.__ %Z");
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  va_list args;
  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);
  if (MOM_UNLIKELY (len >= sizeof (buf) - 1))
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
      syslog (LOG_WARNING, "MONIMELT WARNING @%s:%d <%s> %s %s - %m",
	      fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
    }
  else
    {
      fprintf (stderr, "\nMONIMELT WARNING @%s:%d <%s> %s %s - %m\n",
	       fil, lin, thrname, timbuf, bigbuf ? bigbuf : buf);
      fflush (stderr);
    }
}


static void
logexit_cb (void)
{
  char timbuf[64];
  strftime_centi_now (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S.__ %Z");
  syslog (LOG_INFO, "monimelt exiting at %s", timbuf);
}

static gpointer
checked_gc_malloc (gsize sz)
{
  void *p = GC_MALLOC (sz);	// leave that GC_MALLOC
  if (MOM_UNLIKELY (!p))
    MOM_FATAL ("failed to GC malloc %ld bytes", (long) sz);
  memset (p, 0, sz);
  return p;
}

static gpointer
checked_gc_realloc (gpointer m, gsize sz)
{
  void *p = GC_REALLOC (m, sz);	// leave that GC_REALLOC
  if (MOM_UNLIKELY (!p))
    MOM_FATAL ("failed to GC realloc %ld bytes", (long) sz);
  return p;
}

static gpointer
checked_gc_calloc (gsize nblock, gsize bsize)
{
  void *p = GC_MALLOC (nblock * bsize);	// leave that GC_MALLOC
  if (MOM_UNLIKELY (!p && nblock > 0 && bsize > 0))
    MOM_FATAL ("failed to GC calloc %ld blocks of %ld bytes",
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

static double startime = 0.0;

double
mom_elapsed_real_time (void)
{
  return mom_clock_time (CLOCK_REALTIME) - startime;
}

int
main (int argc, char **argv)
{
  bool explicit_boehm_gc_thread = false;
  GC_INIT ();
#if MOM_EXPLICIT_GC_THREAD
  GC_allow_register_threads ();
  explicit_boehm_gc_thread = true;
#endif
  pthread_setname_np (pthread_self (), "mom-main");
  // we heavily depend on having the "C" locale, e.g. for strftime
  // calls in nanohttp/nanohttp-server.c
  setlocale (LC_ALL, "C");
  g_mem_gc_friendly = TRUE;
  g_mem_set_vtable (&gc_mem_vtable);
  mom_initialize ();
  startime = mom_clock_time (CLOCK_REALTIME);
  parse_program_arguments_and_load_modules (argc, argv);
  MOM_INFORM ("starting Monimelt built timestamp %s gitcommit %s",
	      monimelt_timestamp, monimelt_lastgitcommit);
  if (json_file)
    do_json_file_test ();
  if (json_string)
    do_json_string_test ();
  if (nicelevel != 0)
    {
      if (MOM_UNLIKELY (nice (nicelevel) == -1 && errno))
	MOM_FATAL ("failed to nice at level %d", nicelevel);
    }
  if (wanted_dir)
    {
      if (MOM_UNLIKELY (chdir (wanted_dir)))
	MOM_FATAL ("failed to chdir to %s", wanted_dir);
    }
  if (daemonize_me)
    {
      fprintf (stderr, "%s before daemonizing pid %d\n", argv[0],
	       (int) getpid ());
      fflush (NULL);
      if (MOM_UNLIKELY (daemon ( /*nochdir */ 1, /*noclose */ 0)))
	MOM_FATAL ("failed to daemonize from pid #%d", (int) getpid ());
      want_syslog = true;
    }
  if (want_syslog)
    {
      char timbuf[64];
      strftime_centi_now (timbuf, sizeof (timbuf), "%Y-%b-%d %H:%M:%S.__ %Z");
      openlog ("monimelt",
	       LOG_PID | LOG_CONS | (daemonize_me ? 0 : LOG_PERROR),
	       LOG_LOCAL2);
      syslog (LOG_INFO, "monimelt starting at %s in %s",
	      timbuf, get_current_dir_name ());
      using_syslog = true;
      atexit (logexit_cb);
    }
  MOM_INFORM ("explicit Boehm GC registration = %d",
	      (int) explicit_boehm_gc_thread);

  if (load_state_path && load_state_path[0] && strcmp (load_state_path, ".")
      && strcmp (load_state_path, "_") && strcmp (load_state_path, "-"))
    {
      mom_initial_load (load_state_path);
      if (module_count > 0)
	modules_post_load ();
    }
  else
    MOM_WARNING ("no load state path");
  if (!dont_want_event_loop)
    {
      extern void mom_initialize_signals (void);
      MOM_INFORM ("before initializing signals");
      mom_initialize_signals ();
    }
  if (web_host)
    {
      extern void mom_start_web (const char *);
      MOM_INFORM ("before  web %s", web_host);
      mom_start_web (web_host);
      MOM_INFORM ("started web %s", web_host);
    }
  if (!dont_want_event_loop)
    {
      extern void mom_start_event_loop (void);
      MOM_INFORM ("before starting event loop");
      mom_start_event_loop ();
    }
  else
    MOM_WARNING ("did not start event loop");
  if (mom_nb_workers > 0 && web_host && !dont_want_event_loop)
    {
      MOM_INFORM ("start %d workers", (int) mom_nb_workers);
      mom_run ("main working run");
    }
  else
    MOM_INFORM ("did not run workers");
  MOM_DEBUG (run, "before potential final dump");
  if (dump_state_path && !web_host)
    {
      mom_full_dump ("final dump", dump_state_path);
    }
  usleep (3000);

  double endtime = mom_clock_time (CLOCK_REALTIME);
  MOM_INFORM
    ("monimelt ending normally pid %d, process cpu %.4f, real %.4f seconds\n",
     (int) getpid (), mom_clock_time (CLOCK_PROCESS_CPUTIME_ID),
     endtime - startime);
  return 0;
}
