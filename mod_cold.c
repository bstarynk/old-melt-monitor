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

static void
test_put_several_attrs (void)
{
  MOM_DBG_ITEM (run, "test_put_several_attrs mom_item__comment=",
		(mom_anyitem_t *) mom_item__comment);
  mom_item_put_several_attrs ((mom_anyitem_t *) mom_item__comment,
			      mom_item__comment,
			      mom_make_string
			      ("gives a human-readable comment string in items"),
			      mom_item__OPTIONS,
			      mom_make_string ("silly OPTIONS"),
			      mom_item__POST, mom_make_string ("silly POST"),
			      NULL);
  MOM_DEBUG (run, "end test_put_several_attrs");
}

void
mom_module_post_load (void)
{
  char uistr[UUID_PARSED_LEN];
  memset (uistr, 0, sizeof (uistr));
  MOM_INFORM ("cold post load " __DATE__ "@" __TIME__);

#if 0 && remove
  //
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "form_new_named", MOM_NULLV);
  mom_forget_name ("web_form_new_named");
  MOM_DEBUG (run, "forgot web_form_new_named");
  //
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "form_handle_routine", MOM_NULLV);
  mom_forget_name ("web_form_handle_routine");
  MOM_DEBUG (run, "forgot web_form_handle_routine");
  //
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "form_compile", MOM_NULLV);
  mom_forget_name ("web_form_compile");
  MOM_DEBUG (run, "forgot web_form_compile");
#endif
  //

#if 0
  mom_anyitem_t *rout_ajax_routine =
    (mom_anyitem_t *) mom_make_item_routine ("ajax_routine",
					     MOM_SPACE_ROOT);
  mom_anyitem_t *rout_ajax_routine = mom_item_named ("ajax_routine");
  MOM_DBG_ITEM (run, "rout_ajax_routine=", rout_ajax_routine);
  MOM_INFORM ("cold rout_ajax_routine ~%s",
	      mom_unparse_item_uuid ((mom_anyitem_t *)
				     rout_ajax_routine, uistr));
  mom_anyitem_t *rout_proc_compilation = mom_item_named ("proc_compilation");
  MOM_DBG_ITEM (run, "rout_proc_compilation=", rout_proc_compilation);
  const momclosure_t *clos_proc_compilation =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_proc_compilation,
			      mom_make_string ("Gap*Proc_Compilation"),
			      NULL);
  const momclosure_t *clos_ajax_routine =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_ajax_routine,
			      clos_proc_compilation,
			      mom_make_string ("Gap*Ajax_Routine"),
			      NULL);
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "ajax_routine",
				 (momval_t) clos_ajax_routine);
  MOM_DBG_VALUE (run, "clos_ajax_routine", (momval_t) clos_ajax_routine);
#endif

  mom_anyitem_t *rout_ajax_periodic =
    (mom_anyitem_t *) mom_make_item_routine ("ajax_periodic",
					     MOM_SPACE_ROOT);
  mom_register_new_name_item ("ajax_periodic", rout_ajax_periodic);
  const momclosure_t *clos_ajax_periodic =
    mom_make_closure_til_nil ((mom_anyitem_t *) rout_ajax_periodic,
			      mom_make_string ("Gap*Ajax_Periodic"),
			      NULL);
  mom_item_dictionnary_put_cstr ((momval_t) mom_item__web_dictionnary,
				 "ajax_periodic",
				 (momval_t) clos_ajax_periodic);
  MOM_DBG_VALUE (run, "clos_ajax_periodic", (momval_t) clos_ajax_periodic);

  MOM_INFORM ("cold post load done");
}
