// file web-onion.c

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


onion *mom_onion = NULL;
onion_url *mom_onion_root = NULL;
void
mom_start_web (const char *webhost)
{
  char webuf[128];
  memset (webuf, 0, sizeof (webuf));
  if (strlen (webhost) + 2 >= sizeof (webuf))
    MONIMELT_FATAL ("too long webhost %s", webhost);
  strncpy (webuf, webhost, sizeof (webuf) - 1);
  mom_onion = onion_new (O_ONE_LOOP);
  char *lastcolon = strchr (webuf, ':');
  if (lastcolon && isdigit (lastcolon[1]))
    {
      *lastcolon = (char) 0;
      onion_set_port (mom_onion, lastcolon + 1);
    }
  if (webuf[0])
    onion_set_hostname (mom_onion, webuf);
  mom_onion_root = onion_root_url (mom_onion);
#warning very incomplete
  //bof: onion_handler_add(mom_onion_root, onion_handler_export_local_new(MONIMELT_WEB_DIRECTORY));
}
