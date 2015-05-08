// file dynload.c - manage dynamic loading of modules

/**   Copyright (C)  2015 Free Software Foundation, Inc.
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

/***
We should dlopen every module with RTLD_GLOBAL | RTLD_NOW |
RTLD_DEEPBIND so that the symbols it is defining are visible from the
main program and from other modules. Then, we can always use dlopn on
the main program handle.
 ***/

static pthread_mutex_t dynload_mtx_mom = PTHREAD_MUTEX_INITIALIZER;


void *
mom_dynload_symbol (const char *name)
{
  void *ptr = NULL;
  assert (name && isalpha (name[0]));
  pthread_mutex_lock (&dynload_mtx_mom);
  // since plugins are dlopen-ed with RTLD_DEEPBIND we can just gind
  // symbols using the program handle.
  ptr = dlsym (mom_prog_dlhandle, name);
  if (!ptr)
    {
      MOM_WARNPRINTF ("dlsym %s failed : %s", name, dlerror ());
#warning temporary hack in mom_dynload_symbol for momfun_
      if (!strncmp (name, "momfunc_", strlen ("momfunc_")))
	{
	  char buf[128];
	  memset (buf, 0, sizeof (buf));
	  snprintf (buf, sizeof (buf), "momfun_%s",
		    name + strlen ("momfunc_"));
	  ptr = dlsym (mom_prog_dlhandle, buf);
	  if (!ptr)
	    MOM_WARNPRINTF ("dlsym %s hack-failed : %s", buf, dlerror ());
	  else
	    MOM_INFORMPRINTF ("dlsym hack %s @%p", buf, ptr);
	}
    }
  pthread_mutex_unlock (&dynload_mtx_mom);
  return ptr;
}

////////////////
