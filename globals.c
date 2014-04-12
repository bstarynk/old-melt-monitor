// file globals.c

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

static pthread_mutex_t glob_mtx = PTHREAD_MUTEX_INITIALIZER;

struct name_entry_st {
  momstring_t *nam_str;
  mom_anyitem_t*nam_item;
};
static struct glob_dict_st {
  unsigned name_count;		// number of named entries
  unsigned name_size;		// size of hash tables
  struct name_entry_st* name_hashitem; // hash table on items
  struct name_entry_st* name_hashstr; // hash table on strings
} glob_dict;


void mom_initialize_globals(void)
{
  const unsigned dictsiz = 1024;
  pthread_mutex_lock (&glob_mtx);
  glob_dict.name_hashitem = GC_MALLOC(sizeof(struct name_entry_st)*dictsiz);
  memset (glob_dict.name_hashitem, 0, sizeof(struct name_entry_st)*dictsiz);
  glob_dict.name_hashstr = GC_MALLOC(sizeof(struct name_entry_st)*dictsiz);
  memset (glob_dict.name_hashstr, 0, sizeof(struct name_entry_st)*dictsiz);
  glob_dict.name_size = dictsiz;
  pthread_mutex_unlock(&glob_mtx);
}

#warning definir routines d ajout 
