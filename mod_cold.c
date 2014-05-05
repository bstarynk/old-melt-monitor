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
const char monimelt_GPL_friendly_module[] = "mod_cold is GPLv3";


void
monimelt_module_init (const char *marg)
{
  MONIMELT_WARNING ("cold module init marg=%s do nothing", marg);

  MONIMELT_INFORM ("cold module end marg=%s", marg);
}

void
monimelt_module_post_load (void)
{
  char uistr[UUID_PARSED_LEN];
  memset (uistr, 0, sizeof (uistr));
  MONIMELT_INFORM ("cold post load " __DATE__ "@" __TIME__);
  mom_anyitem_t *rout_web_form_compile = mom_item_named ("web_form_compile");
  assert (rout_web_form_compile != NULL);

  momit_routine_t *rout_proc_compilation =
    mom_make_item_routine ("proc_compilation", MONIMELT_SPACE_ROOT);
  MONIMELT_INFORM ("cold rout_proc_compilation ~%s",
		   mom_unparse_item_uuid ((mom_anyitem_t *)
					  rout_proc_compilation, uistr));
  mom_register_new_name_item ("proc_compilation",
			      (mom_anyitem_t *) rout_proc_compilation);
  const momclosure_t *clos_proc_compilation =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_proc_compilation,
			      mom_make_string ("Gap*Proc_Compilation"), NULL);
  const momclosure_t *clos_web_form_compile =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_web_form_compile,
			      clos_proc_compilation, NULL);
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "form_compile",
				 (momval_t) clos_web_form_compile);
  mom_dbg_value (run, "clos_web_form_compile=",
		 (momval_t) clos_web_form_compile);
  MONIMELT_INFORM ("cold post load done");
}
