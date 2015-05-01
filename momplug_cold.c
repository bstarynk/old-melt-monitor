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
momplugin_after_load (void)
{
  MOM_INFORMPRINTF ("cold plugin after load");
  MOM_PREDEFINED_NAMED (emitter_of_magic_attribute)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_val);
  MOM_PREDEFINED_NAMED (emitter_of_magic_attribute)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_val_emitter_of_magic_attribute");

  MOM_PREDEFINED_NAMED (scanner_of_magic_attribute)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
  MOM_PREDEFINED_NAMED (scanner_of_magic_attribute)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_scanner_of_magic_attribute");

  MOM_PREDEFINED_NAMED (filler_of_magic_attribute)->itm_kind
    = MOM_PREDEFINED_NAMED (signature_1itm_to_void);
  MOM_PREDEFINED_NAMED (filler_of_magic_attribute)->itm_data1 =
    mom_dynload_symbol ("momfun_1itm_to_void_filler_of_magic_attribute");
  MOM_DEBUGPRINTF (load, "set functions related to magic_attribute");
}
