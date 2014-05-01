// file mod_test1.c

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
const char monimelt_GPL_friendly_module[] = "mod_test1 is GPLv3";


void
monimelt_module_init (const char *marg)
{
  MONIMELT_INFORM ("test1 module init marg=%s", marg);
  MONIMELT_INFORM ("json_cmp agenda attr=%d",
		   mom_json_cmp ((momval_t) mom_item__agenda,
				 (momval_t) mom_item__attr));
  MONIMELT_INFORM ("json_cmp attr conn=%d",
		   mom_json_cmp ((momval_t) mom_item__attr,
				 (momval_t) mom_item__conn));
  MONIMELT_INFORM ("json_cmp frames conn=%d",
		   mom_json_cmp ((momval_t) mom_item__frames,
				 (momval_t) mom_item__conn));
  MONIMELT_INFORM ("json_cmp attr 'attr'=%d",
		   mom_json_cmp ((momval_t) mom_item__attr,
				 (momval_t) mom_make_string ("attr")));
}
