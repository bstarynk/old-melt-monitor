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


//#if 0 && old
static void
create_stuff_mom (void)
{
  momitem_t *ajax_objects_item = mom_get_item_of_name ("ajax_objects");
  assert (ajax_objects_item != NULL);
  momitem_t *display_value_item = mom_make_item ();
  mom_item_set_space (display_value_item, momspa_root);
  mom_register_item_named_cstr (display_value_item, "display_value");
  mom_item_start_routine (display_value_item, "display_value");
  display_value_item->i_attrs = mom_put_attribute
    (display_value_item->i_attrs,
     mom_named__comment,
     (momval_t)
     mom_make_string
     ("routine to display a value during edition in ajax_objects"));
#if 0
  ajax_objects_item->i_attrs =	//
    mom_put_attribute (ajax_objects_item->i_attrs, mom_named__web_handler,	//
		       (momval_t) mom_make_node_til_nil	//
		       (ajax_objects_item, (momval_t) mom_make_node_til_nil	//
			(display_value_item,
			 (momval_t)
			 mom_make_string
			 ("{spare1-display_value}"),
			 NULL),
			(momval_t)
			mom_make_string
			("{spare2-ajax_objects}"), MOM_EMPTY, NULL));
#endif
  MOM_INFORMPRINTF ("created display_value");
}



void
add_editors_mom (void)
{
  momitem_t *ajax_objects_item = mom_get_item_of_name ("ajax_objects");
  assert (ajax_objects_item != NULL);
  momitem_t *edit_value_item = mom_get_item_of_name ("edit_value");
  assert (edit_value_item != NULL);
  momitem_t *display_value_item = mom_get_item_of_name ("display_value");
  assert (display_value_item != NULL);
  momitem_t *editors_item = mom_get_item_of_name ("editors");
  assert (editors_item != NULL);
  mom_item_start_routine (ajax_objects_item, "ajax_objects");
  MOM_DEBUG (run, MOMOUT_LITERAL ("add_editors ajax_objects="),
	     MOMOUT_ITEM ((const momitem_t *) ajax_objects_item),
	     MOMOUT_LITERAL ("; edit_value="),
	     MOMOUT_ITEM ((const momitem_t *) edit_value_item),
	     MOMOUT_LITERAL ("; editors="),
	     MOMOUT_ITEM ((const momitem_t *) editors_item), NULL);
  mom_item_put_attribute	//
    (ajax_objects_item,		//
     mom_named__web_handler,	//
     (momval_t) mom_make_node_til_nil	//
     (ajax_objects_item,	//
      (momval_t) editors_item,	//
      (momval_t) mom_make_node_til_nil	//
      (edit_value_item, (momval_t) editors_item, (momval_t) mom_make_string ("{spare1-edit_value}"), NULL), (momval_t) mom_make_node_til_nil	//
      (display_value_item,
       (momval_t) editors_item,
       (momval_t) mom_make_string ("{spare1-display_value}"),
       NULL),
      (momval_t) mom_make_string ("{spare4-ajax_objects}"), MOM_EMPTY, NULL));
  MOM_INFORMPRINTF
    ("updated to keep editors in closures for edit_value & ajax_objects");
}

void
momplugin_after_load (void)
{
  MOM_DEBUGPRINTF (run,
		   "after load in " __FILE__ " build " __DATE__ "@" __TIME__);
  //create_stuff_mom ();
  add_editors_mom ();
}
