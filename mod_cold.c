// file mod_cold.c

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
const char mom_GPL_friendly_module[] = "mod_cold is GPLv3";


void
mom_module_init (const char *marg)
{
  MOM_WARNING ("cold module init marg=%s do nothing", marg);

  MOM_INFORM ("cold module end marg=%s", marg);
}

void
mom_module_post_load (void)
{
  char uistr[UUID_PARSED_LEN];
  memset (uistr, 0, sizeof (uistr));
  MOM_INFORM ("cold post load " __DATE__ "@" __TIME__);
  mom_anyitem_t *rout_web_form_handle_routine =
    mom_item_named ("web_form_handle_routine");
  MOM_DBG_VALUE (run, "rout_web_form_handle_routine=",
		 (momval_t) rout_web_form_handle_routine);
  MOM_INFORM ("cold rout_web_form_handle_routine ~%s",
	      mom_unparse_item_uuid ((mom_anyitem_t *)
				     rout_web_form_handle_routine, uistr));
  const momclosure_t *clos_web_form_handle_routine =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_web_form_handle_routine,
			      mom_make_string ("Gap*Web_Form_Handle_Routine"),
			      NULL);
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "web_form_handle_routine",
				 (momval_t) clos_web_form_handle_routine);
  MOM_DBG_VALUE (run, "clos_web_form_handle_routine=",
		 (momval_t) clos_web_form_handle_routine);
  MOM_INFORM ("cold post load done");
}
