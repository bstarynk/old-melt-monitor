// file momplug_cold.c

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

const char mom_plugin_GPL_compatible[] = "GPLv3+";

void
mom_plugin_init (const char *arg)
{
  MOM_DEBUGPRINTF (run, "start of " __FILE__ " arg=%s", arg);
}

static void
cleanup_dict (void)
{
  momval_t setnameditems = (momval_t) mom_set_of_named_items ();
  unsigned nbnamed = mom_set_cardinal (setnameditems);
  unsigned nbforgetmom = 0;
  unsigned nbforgetmelt = 0;
  for (unsigned ix = 0; ix < nbnamed; ix++)
    {
      momitem_t *curnameditm = mom_set_nth_item (setnameditems, ix);
      momval_t curnamev = (momval_t) mom_item_get_name (curnameditm);
      const char *curnamstr = mom_string_cstr (curnamev);
      if (!strncmp (curnamstr, "mom", 3))
	{
	  mom_forget_name (curnamstr);
	  nbforgetmom++;
	}
      if (!strncmp (curnamstr, "melt", 3))
	{
	  mom_forget_name (curnamstr);
	  nbforgetmelt++;
	}
    }
  MOM_INFORMPRINTF ("forgot %u names suffixed with mom & %u with melt",
		    nbforgetmom, nbforgetmelt);
}

void
momplugin_after_load (void)
{
  cleanup_dict ();
  MOM_DEBUGPRINTF (run,
		   "after load in " __FILE__ " build " __DATE__ "@" __TIME__);
}
