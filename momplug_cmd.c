// file momplug_cmd.c

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

/** following is for our Makefile, see rule for momplug_%.so there
   MONIMELTLIBS: -lreadline
**/

#include "monimelt.h"

#include <readline/readline.h>
#include <readline/history.h>

const char mom_plugin_GPL_compatible[] = "GPLv3+";

// the command stack has values and marks
static momval_t *vst_valarr_mom;
static char *vst_markarr_mom;
static unsigned vst_size;
static unsigned vst_top;

static void
cmd_stack_push_mom (momval_t val, char mark)
{
  if (vst_top >= vst_size)
    {
      unsigned newsiz = (4 * vst_size / 3 + 21) | 0x1f;
      momval_t *newvalarr =
	MOM_GC_ALLOC ("valarr", newsiz * sizeof (momval_t));
      char *newmarkarr = MOM_GC_SCALAR_ALLOC ("markarr", newsiz + 1);
      if (vst_top > 0)
	{
	  memcpy (newvalarr, vst_valarr_mom, vst_top * sizeof (momval_t));
	  memcpy (newmarkarr, vst_markarr_mom, vst_top);
	  MOM_GC_FREE (vst_valarr_mom);
	  MOM_GC_FREE (vst_markarr_mom);
	}
      vst_valarr_mom = newvalarr;
      vst_markarr_mom = newmarkarr;
    }
  vst_valarr_mom[vst_top] = val;
  vst_markarr_mom[vst_top] = mark;
  vst_top++;
}

static inline momval_t
cmd_stack_nth_value_mom (int rk)
{
  if (rk < 0)
    rk += vst_top;
  if (rk >= 0 && rk < (int) vst_top)
    return vst_valarr_mom[vst_top - rk];
  return MOM_NULLV;
}

static inline char
cmd_stack_nth_mark_mom (int rk)
{
  if (rk < 0)
    rk += vst_top;
  if (rk >= 0 && rk < (int) vst_top)
    return vst_markarr_mom[vst_top - rk];
  return 0;
}

static void
cmd_stack_pop_mom (unsigned nb)
{
  if (nb < vst_top && nb > 0)
    {
      memset (vst_valarr_mom + vst_top - nb, 0, sizeof (momval_t) * nb);
      memset (vst_markarr_mom + vst_top - nb, 0, sizeof (char) * nb);
      vst_top -= nb;
    }
}

#define COMMANDS				\
  CMD(quit,"quitting")				\
  CMD(exit,"dump & exit")			\
  CMD(stack,"print the stack")			\
  CMD(dup,"duplicate top")

static bool running_cmd_mom;
static char *prompt_cmd_mom = "monimelt: ";
static void
linehandler_cmd_mom (char *line)
{
  if (line == NULL || !strcmp (line, "quit"))
    {
      if (!line)
	printf ("\n");
      printf ("MOMCMD exiting\n");
      rl_callback_handler_remove ();
      running_cmd_mom = false;
    }
  else if (line && *line)
    {
      add_history (line);
      printf ("MOMCMD ignoring line %s\n", line);
      free (line);
    }
}



void
mom_plugin_init (const char *arg)
{
  MOM_DEBUGPRINTF (run, "start of " __FILE__ " arg=%s", arg);
}

#define POLL_TIMEOUT 500
void
momplugin_after_load (void)
{
  MOM_INFORMPRINTF ("momplug_cmd starting after load\n");
  /* Install the line handler. */
  rl_callback_handler_install (prompt_cmd_mom, linehandler_cmd_mom);
  /* Enter a simple event loop */
  running_cmd_mom = true;
  while (running_cmd_mom)
    {
      int rfd = fileno (rl_instream);
      struct pollfd polltab[2];
      memset (polltab, 0, sizeof (polltab));
      polltab[0].fd = rfd;
      polltab[0].events = POLLIN;
      if (poll (polltab, 1, POLL_TIMEOUT) > 0)
	{
	  rl_callback_read_char ();
	}
    }

#warning we should have a readline command line interpreter here.
}
