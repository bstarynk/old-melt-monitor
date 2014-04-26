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
static pthread_mutex_t mtx_onion = PTHREAD_MUTEX_INITIALIZER;
static long nb_weq_requests;

extern momit_dictionnary_t *mom_item__web_dictionnary;

static
  onion_connection_status mom_really_process_request (long webnum,
						      onion_request * req,
						      onion_response * res)
  __attribute__ ((noinline));

static onion_connection_status
process_request (void *ignore, onion_request * req, onion_response * res)
{
  struct GC_stack_base sb;
  memset (&sb, 0, sizeof (sb));
  GC_register_my_thread (&sb);
  long webnum = 0;
  {
    char thnambuf[32];
    memset (thnambuf, 0, sizeof (thnambuf));
    pthread_mutex_lock (&mtx_onion);
    nb_weq_requests++;
    webnum = nb_weq_requests;
    snprintf (thnambuf, sizeof (thnambuf), "monimelt-w%04ld", webnum);
    pthread_setname_np (pthread_self (), thnambuf);
    pthread_mutex_unlock (&mtx_onion);
  }
  onion_connection_status cs = mom_really_process_request (webnum, req, res);
  GC_unregister_my_thread ();
  return cs;
}

void
mom_start_web (const char *webhost)
{
  char webuf[128];
  memset (webuf, 0, sizeof (webuf));
  if (strlen (webhost) + 2 >= sizeof (webuf))
    MONIMELT_FATAL ("too long webhost %s", webhost);
  strncpy (webuf, webhost, sizeof (webuf) - 1);
  mom_onion = onion_new (O_THREADED);
  char *lastcolon = strchr (webuf, ':');
  if (lastcolon && isdigit (lastcolon[1]))
    {
      *lastcolon = (char) 0;
      onion_set_port (mom_onion, lastcolon + 1);
    }
  if (webuf[0])
    onion_set_hostname (mom_onion, webuf);
  mom_onion_root = onion_root_url (mom_onion);
  onion_handler *loch =
    onion_handler_export_local_new (MONIMELT_WEB_DIRECTORY);
  onion_url_add_handler (mom_onion_root, "status", onion_internal_status ());
  onion_url_add_handler (mom_onion_root, "^.*",
			 onion_handler_new ((onion_handler_handler)
					    process_request, NULL, NULL));
  onion_url_add_handler (mom_onion_root, "[A-Za-z0-9+-.]+", loch);
  MONIMELT_INFORM ("before listening web host %s", webhost);
  onion_listen (mom_onion);
  MONIMELT_INFORM ("after listening web host %s", webhost);
}

static onion_connection_status
mom_really_process_request (long webnum, onion_request * req,
			    onion_response * res)
{
  assert (mom_item__web_dictionnary != NULL);
  const char *fullpath = onion_request_get_fullpath (req);
  const char *path = onion_request_get_path (req);
  unsigned flags = onion_request_get_flags (req);
  const char *method = NULL;
  switch (flags & OR_METHODS)
    {
    case OR_GET:
      method = "GET";
      break;
    case OR_POST:
      method = "POST";
      break;
    case OR_HEAD:
      method = "HEAD";
      break;
    case OR_OPTIONS:
      method = "OPTIONS";
      break;
    default:
      MONIMELT_FATAL ("unexpected req#%ld fullpath=%s method %d flags %#x",
		      webnum, fullpath, flags & OR_METHODS, flags);
    }
  MONIMELT_INFORM ("request #%ld fullpath=%s path=%s method=%s",
		   webnum, fullpath, path, method);
  return OCS_NOT_PROCESSED;
#warning incomplete mom_really_process_request
}
