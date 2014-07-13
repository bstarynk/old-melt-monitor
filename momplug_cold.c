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


#if 0 && old
static void
create_stuff_mom (void)
{
  momitem_t *ajax_edit_item = mom_get_item_of_name ("ajax_edit");
  assert (ajax_edit_item != NULL);
  momitem_t *display_value_item = mom_make_item ();
  mom_item_set_space (display_value_item, momspa_root);
  mom_register_item_named_cstr (display_value_item, "display_value");
  mom_item_start_routine (display_value_item, "display_value");
  display_value_item->i_attrs = mom_put_attribute
    (display_value_item->i_attrs,
     mom_named__comment,
     (momval_t)
     mom_make_string
     ("routine to display a value during edition in ajax_edit"));
  ajax_edit_item->i_attrs =	//
    mom_put_attribute (ajax_edit_item->i_attrs, mom_named__web_handler,	//
		       (momval_t) mom_make_node_til_nil	//
		       (ajax_edit_item, (momval_t) mom_make_node_til_nil	//
			(display_value_item,
			 (momval_t)
			 mom_make_string
			 ("{spare1-display_value}"),
			 NULL),
			(momval_t)
			mom_make_string
			("{spare2-ajax_edit}"), MOM_EMPTY, NULL));
  MOM_INFORMPRINTF ("created display_value");
}



void
add_editors_mom (void)
{
  momitem_t *ajax_edit_item = mom_get_item_of_name ("ajax_edit");
  assert (ajax_edit_item != NULL);
  momitem_t *edit_value_item = mom_get_item_of_name ("edit_value");
  assert (edit_value_item != NULL);
  momitem_t *display_value_item = mom_get_item_of_name ("display_value");
  assert (display_value_item != NULL);
  momitem_t *editors_item = mom_get_item_of_name ("editors");
  assert (editors_item != NULL);
  momitem_t *update_display_value_item =
    mom_get_item_of_name ("update_display_value");
  assert (update_display_value_item != NULL);
  //mom_item_start_routine (ajax_edit_item, "ajax_edit");
  MOM_DEBUG (run, MOMOUT_LITERAL ("add_editors ajax_edit="),
	     MOMOUT_ITEM ((const momitem_t *) ajax_edit_item),
	     MOMOUT_LITERAL (" !: "),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) ajax_edit_item),
	     MOMOUT_LITERAL ("; edit_value="),
	     MOMOUT_ITEM ((const momitem_t *) edit_value_item),
	     MOMOUT_LITERAL (" !: "),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) edit_value_item),
	     MOMOUT_LITERAL ("; editors="),
	     MOMOUT_ITEM ((const momitem_t *) editors_item),
	     MOMOUT_LITERAL (" !: "),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) editors_item), NULL);
  momval_t nodev = (momval_t) mom_make_node_til_nil	//
    (ajax_edit_item,		//
     (momval_t) editors_item,	//
     (momval_t) mom_make_node_til_nil	//
     (edit_value_item, (momval_t) editors_item, (momval_t) mom_make_string ("{spare1-edit_value}"), NULL),	//
     (momval_t) mom_make_node_til_nil	//
     (display_value_item,
      (momval_t) editors_item,
      (momval_t) mom_make_string ("{spare1-display_value}"),
      NULL),
     (momval_t) mom_make_node_til_nil	//
     (update_display_value_item,
      (momval_t) editors_item,
      (momval_t) mom_make_string ("{spare1-update_display_value}"),
      NULL),
     (momval_t) mom_make_string ("{spare5-ajax_edit}"), MOM_EMPTY, NULL);
  mom_item_put_attribute	//
    (ajax_edit_item,		//
     mom_named__web_handler,	//
     nodev);
  MOM_DEBUG (run, MOMOUT_LITERAL ("ajax_edit="),
	     MOMOUT_ITEM ((const momitem_t *) ajax_edit_item),
	     MOMOUT_LITERAL (" !: "),
	     MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *) ajax_edit_item),
	     MOMOUT_LITERAL (" nodev="),
	     MOMOUT_VALUE ((const momval_t) nodev));
  MOM_INFORMPRINTF
    ("updated to keep editors in closures for edit_value & ajax_edit");
}
#endif

static void
make_closure_json_rpc_status_mom (void)
{
  momitem_t *json_rpc_status_item = mom_get_item_of_name ("json_rpc_status");
  assert (json_rpc_status_item != NULL);
  momitem_t *state_item = mom_get_item_of_name ("state");
  assert (state_item != NULL);

  mom_item_start_closure_named (json_rpc_status_item, "json_rpc_status", 2);
  mom_item_closure_set_nth (json_rpc_status_item, 0,
			    (momval_t)
			    mom_make_string
			    ("{spare closed-value json-rpc-status-0}"));
  mom_item_closure_set_nth (json_rpc_status_item, 1,
			    (momval_t)
			    mom_make_string
			    ("{spare closed-value json-rpc-status-1}"));
  mom_item_put_attribute (state_item, mom_named__jsonrpc,
			  (momval_t) json_rpc_status_item);
  MOM_DEBUG (run, MOMOUT_LITERAL ("closure json_rpc_status"),
	     MOMOUT_VALUE ((const momval_t) json_rpc_status_item), NULL);
}



void
momplugin_after_load (void)
{
  MOM_DEBUGPRINTF (run,
		   "after load in " __FILE__ " build " __DATE__ "@" __TIME__);
  make_closure_json_rpc_status_mom ();
}
