// file main.c

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


/// declare the named items
#define MONIMELT_NAMED(Name,Type,Uid) \
  momit_##Type##_t* mom_item__##Name;
#include "monimelt-names.h"

void
mom_create_items (void)
{
  uuid_t uid;
  //
  // create the named items
#define MONIMELT_NAMED(Name,Type,Uids) do {		\
  memset(&uid, 0, sizeof(uid));				\
  uuid_parse(Uids, uid);				\
  mom_item__##Name = mom_create__##Type (Name,uid);	\
  } while(0);
#include "monimelt-names.h"
    //
}
