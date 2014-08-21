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

static void
make_closure_json_rpc_dump_exit_mom (void)
{
  momitem_t *json_rpc_dump_exit_item =
    mom_get_item_of_name ("json_rpc_dump_exit");
  assert (json_rpc_dump_exit_item != NULL);
  momitem_t *dump_item = mom_get_item_of_name ("dump");
  assert (dump_item != NULL);

  mom_item_start_closure_named (json_rpc_dump_exit_item, "json_rpc_dump_exit",
				2);
  mom_item_closure_set_nth (json_rpc_dump_exit_item, 0,
			    (momval_t)
			    mom_make_string
			    ("{spare closed-value json-rpc-dump-exit-0}"));
  mom_item_closure_set_nth (json_rpc_dump_exit_item, 1,
			    (momval_t)
			    mom_make_string
			    ("{spare closed-value json-rpc-dump-exit-1}"));
  mom_item_put_attribute (dump_item, mom_named__jsonrpc_handler,
			  (momval_t) json_rpc_dump_exit_item);
  MOM_DEBUG (run, MOMOUT_LITERAL ("closure json_rpc_dump_exit"),
	     MOMOUT_VALUE ((const momval_t) json_rpc_dump_exit_item), NULL);
}
#endif

static void
declare_jsonrpc_mom (const char *jsonrpcname,
		     const char *jsonrpccomment,
		     const char *methodname, const char *methodcomment)
{
  char buf[64];
  memset (buf, 0, sizeof (buf));
  assert (jsonrpcname && isalpha (jsonrpcname[0]));
  momitem_t *jsonrpcitem = mom_get_item_of_name (jsonrpcname);
  if (!jsonrpcitem)
    {
      jsonrpcitem = mom_make_item ();
      mom_item_set_space (jsonrpcitem, momspa_root);
      mom_register_item_named_cstr (jsonrpcitem, jsonrpcname);
    }
  assert (methodname && isalpha (methodname[0]));
  momitem_t *methoditem = mom_get_item_of_name (methodname);
  if (!methoditem)
    {
      methoditem = mom_make_item ();
      mom_item_set_space (methoditem, momspa_root);
      mom_register_item_named_cstr (methoditem, methodname);
    }
  const int nbspare = 3;
  mom_item_start_closure_named (jsonrpcitem, jsonrpcname, nbspare);
  for (int i = 0; i < nbspare; i++)
    {
      snprintf (buf, sizeof (buf), "%s spare %d", jsonrpcname, i);
      mom_item_closure_set_nth (jsonrpcitem, i,
				(momval_t) mom_make_string (buf));
      memset (buf, 0, sizeof (buf));
    }
  if (methodcomment && methodcomment[0])
    mom_item_put_attribute (methoditem, mom_named__comment,
			    (momval_t) mom_make_string (methodcomment));
  if (jsonrpccomment && jsonrpccomment[0])
    mom_item_put_attribute (jsonrpcitem, mom_named__comment,
			    (momval_t) mom_make_string (jsonrpccomment));
  mom_item_put_attribute (methoditem, mom_named__jsonrpc_handler,
			  (momval_t) jsonrpcitem);
}

void
momplugin_after_load (void)
{
  MOM_DEBUGPRINTF (run,
		   "after load in " __FILE__ " build " __DATE__ "@" __TIME__);
  declare_jsonrpc_mom ("json_rpc_meltmom_define_field",
		       "JSONRPC routine to define some field in union or record",
		       "meltmom_define_field",
		       "JSONRPC method to define some field");
}
