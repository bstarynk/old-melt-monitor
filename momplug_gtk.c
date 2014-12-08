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
   MONIMELTLIBS: `pkg-config --cflags --libs gtk+-3.0`
**/

#include "monimelt.h"

#include <gtk/gtk.h>
#include <regex.h>

const char mom_plugin_GPL_compatible[] = "GPLv3+";
void
mom_plugin_init (const char *arg, int *pargc, char ***pargv)
{
  MOM_INFORMPRINTF ("start of " __FILE__ " build %s arg=%s (argc=%d)",
		    __DATE__ "@" __TIME__, arg, *pargc);
  gtk_init (pargc, pargv);
}

void
momplugin_after_load (void)
{
  MOM_INFORMPRINTF ("after load " __FILE__);
}
