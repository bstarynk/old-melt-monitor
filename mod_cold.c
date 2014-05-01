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
  char uistr[UUID_PARSED_LEN];
  MONIMELT_INFORM ("cold module init marg=%s", marg);
  momit_routine_t *rout_web_form_exit =
    mom_make_item_routine ("web_form_exit", MONIMELT_SPACE_ROOT);
  MONIMELT_INFORM ("cold monimelt_inform web_form_exit ~%s",
		   mom_unparse_item_uuid ((mom_anyitem_t *)
					  rout_web_form_exit, uistr));
  mom_register_new_name_item ("web_form_exit",
			      (mom_anyitem_t *) rout_web_form_exit);
  MONIMELT_INFORM ("cold name of web_form_exit=%s",
		   mom_string_cstr ((momval_t)
				    mom_name_of_item ((const mom_anyitem_t *)
						      rout_web_form_exit)));
  const momclosure_t *clos_web_form_exit =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_web_form_exit, NULL);
  MONIMELT_INFORM ("cold clos_web_form_exit@%p", (void *) clos_web_form_exit);
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "form_exit", (momval_t) clos_web_form_exit);
  MONIMELT_INFORM ("cold module end marg=%s", marg);
}
