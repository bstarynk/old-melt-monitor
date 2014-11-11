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
