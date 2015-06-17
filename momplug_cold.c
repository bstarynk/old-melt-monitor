// file momplug_cold.c

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
const char mom_plugin_GPL_compatible[] = "GPLv3+";
void
mom_plugin_init (const char *pluginarg, int *pargc, char ***pargv)
{
  MOM_INFORMPRINTF ("%s plugin init pluginarg=%s\n", __FILE__, pluginarg);
  MOM_DEBUGPRINTF (load, "cold pluginarg=%s", pluginarg);
}

void
mom_plugin_after_load (void)
{
  MOM_INFORMPRINTF ("cold plugin after load");
  momitem_t *taskletitm = mom_make_anonymous_item ();
  MOM_DEBUGPRINTF (run, "coldplugin taskletitm %s",
		   mom_item_cstring (taskletitm));
  taskletitm->itm_kind = MOM_PREDEFINED_NAMED (tasklet);
  momhook_agenda_push_front (mom_itemv (taskletitm));
  MOM_DEBUGPRINTF (run, "coldplugin pushed agenda front taskletitm %s",
		   mom_item_cstring (taskletitm));
}
