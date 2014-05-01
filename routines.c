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
  MONIMELT_INFORM ("momcode_form_exit state=%d webnum=%ld",
		   state, mom_item_webrequest_webnum (webv));
  if (mom_item_webrequest_method (webv).ptr ==
      ((momval_t) mom_item__POST).ptr)
    {
      if (mom_item_webrequest_post_arg (webv, "do_savexit").ptr)
	{
	  mom_item_webrequest_printf
	    (webv,
	     "<!doctype html><head><title>dump and exit Monimelt</title></head>\n"
	     "<body><h1>Monimelt dump and exit</h1>\n"
	     "<p>dump to default <tt>" MONIMELT_DEFAULT_STATE_FILE
	     "</tt> reqnum#%ld at <i>%s</i></p>\n" "</body></html>\n",
	     mom_item_webrequest_webnum (webv), nowbuf);
	  mom_item_webrequest_reply (webv, "text/html", HTTP_OK);
	  usleep (100);
	  mom_request_stop ();
	  mom_wait_for_stop ();
	  mom_full_dump (MONIMELT_DEFAULT_STATE_FILE);
	}
      else if (mom_item_webrequest_post_arg (webv, "do_quit").ptr)
	{
	  mom_item_webrequest_printf
	    (webv,
	     "<!doctype html><head><title>Quit Monimelt</title></head>\n"
	     "<body><h1>Monimelt quitting</h1>\n"
	     "<p>quitting reqnum#%ld at <i>%s</i></p>\n"
	     "</body></html>\n", mom_item_webrequest_webnum (webv), nowbuf);
	  mom_item_webrequest_reply (webv, "text/html", HTTP_OK);
	  usleep (100);
	  mom_request_stop ();
	  mom_wait_for_stop ();
	}
    }
  MONIMELT_INFORM ("momcode_form_exit ending");
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
