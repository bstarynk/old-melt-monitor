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

static int nicelevel = 0;
static const char *json_file;
static const char *json_string;
bool json_indented;

/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_jsonfile = 1024,
  xtraopt_jsonstring,
  xtraopt_jsonindent,
};

static const struct option mom_long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {"nice", required_argument, NULL, 'n'},
  {"daemon", no_argument, NULL, 'd'},
  // long-only options
  {"json-file", required_argument, NULL, xtraopt_jsonfile},
  {"json-string", required_argument, NULL, xtraopt_jsonstring},
  {"json-indent", no_argument, NULL, xtraopt_jsonindent},
  /* Terminating NULL placeholder.  */
  {NULL, no_argument, NULL, 0},
};

static void
usage (const char *argv0)
{
  printf ("Usage: %s\n", argv0);
  printf ("\t -h | --help " " \t# Give this help.\n");
  printf ("\t -V | --version " " \t# Give version information.\n");
  printf ("\t -d | --daemon " " \t# Daemonize.\n");
  printf ("\t -n | --nice <nice-level> " " \t# Set process nice level.\n");
  putchar ('\n');
  printf ("\t --json-file <file-name>" "\t #parse JSON file for testing\n");
  printf ("\t --json-string <string>" "\t #parse JSON string for testing\n");
  printf ("\t --json-indent" "\t #output parsed JSON with indentation\n");
}

static void
print_version (const char *argv0)
{
  printf ("%s built on " __DATE__ "@" __TIME__, argv0);
}

static void
parse_program_arguments (int argc, char **argv)
{
  int opt = -1;
  while ((opt = getopt_long (argc, argv, "hVdn:",
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
	case xtraopt_jsonfile:
	  json_file = optarg;
	  break;
	case xtraopt_jsonstring:
	  json_string = optarg;
	  break;
	case xtraopt_jsonindent:
	  json_indented = true;
	  break;
	}
    }
}

void
mom_initialize (void)
{
  extern void mom_initialize_items (void);
  extern void mom_initialize_globals (void);
  extern void mom_create_items ();
  static int inited;
  if (inited)
    return;
  inited = 1;
  mom_initialize_items ();
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
  mom_json_output_end (&jo);
}

int
main (int argc, char **argv)
{
  GC_INIT ();
  pthread_setname_np (pthread_self (), "monimelt-main");
  mom_initialize ();
  parse_program_arguments (argc, argv);
  if (nicelevel != 0)
    {
      if (MONIMELT_UNLIKELY (nice (nicelevel) == -1 && errno))
	MONIMELT_FATAL ("failed to nice at level %d", nicelevel);
    }
  if (json_file)
    do_json_file_test ();
  if (json_string)
    do_json_string_test ();
  return 0;
}
