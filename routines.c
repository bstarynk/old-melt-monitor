// file routines.c

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

int
momcode_web_form_exit (int state, momit_tasklet_t * tasklet,
		       momclosure_t * closure, momval_t * locvals,
		       intptr_t * locnums, double *locdbls)
{
  momval_t webv = locvals[0];
  time_t now = 0;
  struct tm nowtm = { };
  char nowbuf[64] = "";
  time (&now);
  strftime (nowbuf, sizeof (nowbuf), "%c", localtime_r (&now, &nowtm));
  MONIMELT_DEBUG (web, "momcode_form_exit state=%d webnum=%ld nowbuf=%s",
		  state, mom_item_webrequest_webnum (webv), nowbuf);
  mom_dbg_item (web, "web_form_exit tasklet=",
		(const mom_anyitem_t *) tasklet);
  mom_dbg_value (web, "web_form_exit webv=", webv);
  mom_dbg_value (web, "web_form_exit closure=",
		 (momval_t) (const momclosure_t *) closure);
  mom_dbg_value (web, "web_form_exit method=",
		 (momval_t) mom_item_webrequest_method (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      MONIMELT_DEBUG (web, "momcode_form_exit POST");
      mom_dbg_value (web, "web_form_exit jsobpost=",
		     mom_item_webrequest_jsob_post (webv));
      if (mom_item_webrequest_post_arg (webv, "do_savexit").ptr)
	{
	  MONIMELT_DEBUG (web, "momcode_form_exit do_savexit");
	  mom_item_webrequest_printf
	    (webv,
	     "<!doctype html><head><title>dump and exit Monimelt</title></head>\n"
	     "<body><h1>Monimelt dump and exit</h1>\n"
	     "<p>dump to default <tt>" MONIMELT_DEFAULT_STATE_FILE
	     "</tt> reqnum#%ld at <i>%s</i></p>\n" "</body></html>\n",
	     mom_item_webrequest_webnum (webv), nowbuf);
	  mom_item_webrequest_reply (webv, "text/html", HTTP_OK);
	  usleep (1000);
	  MONIMELT_DEBUG (web,
			  "momcode_form_exit do_savexit before request stop");
	  mom_request_stop ();
	  usleep (2000);
	  MONIMELT_DEBUG (web,
			  "momcode_form_exit do_savexit before fulldump");
	  mom_full_dump ("web save&exit dump", MONIMELT_DEFAULT_STATE_FILE);;
	  MONIMELT_DEBUG (web, "momcode_form_exit do_savexit after fulldump");
	}
      else if (mom_item_webrequest_post_arg (webv, "do_quit").ptr)
	{
	  MONIMELT_DEBUG (web, "momcode_form_exit do_quit");
	  mom_item_webrequest_printf
	    (webv,
	     "<!doctype html><head><title>Quit Monimelt</title></head>\n"
	     "<body><h1>Monimelt quitting</h1>\n"
	     "<p>quitting reqnum#%ld at <i>%s</i></p>\n"
	     "</body></html>\n", mom_item_webrequest_webnum (webv), nowbuf);
	  mom_item_webrequest_reply (webv, "text/html", HTTP_OK);
	  usleep (100);
	  MONIMELT_DEBUG (web,
			  "momcode_form_exit do_quit before request stop");
	  mom_request_stop ();
	}
      else
	MONIMELT_WARNING ("unexpected post query for webnum#%ld",
			  mom_item_webrequest_webnum (webv));
    }
  usleep (5000);
  return -1;
}

const struct momroutinedescr_st momrout_web_form_exit =
  {.rout_magic = ROUTINE_MAGIC,
  .rout_minclosize = 0,
  .rout_frame_nbval = 1,
  .rout_frame_nbnum = 0,
  .rout_frame_nbdbl = 0,
  .rout_name = "web_form_exit",
  .rout_code = (const momrout_sig_t *) momcode_web_form_exit
};
