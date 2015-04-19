// file state.c

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

////////////////

#define LOADER_MAGIC_MOM 0x169128bb
struct momloader_st
{
  unsigned ldmagic;		/* always LOADER_MAGIC_MOM */
  const char *ldglobalpath;
  const char *lduserpath;
  FILE *ldglobalfile;
  FILE *lduserfile;
  struct momhashset_st *lditemset;
};


static void
first_pass_load_mom (struct momloader_st *ld, const char *path, FILE *fil)
{
#warning first_pass_load_mom should probably collect module paths
  assert (ld && ld->ldmagic == LOADER_MAGIC_MOM);
  char *linbuf = NULL;
  size_t linsiz = 0;
  ssize_t linlen = 0;
  unsigned lincnt = 0;
  rewind (fil);
  linsiz = 128;
  linbuf = malloc (linsiz);	// for getline
  if (!linbuf)
    MOM_FATAPRINTF ("failed to allocate line of %zd bytes", linsiz);
  memset (linbuf, 0, linsiz);
  while ((linlen = getline (&linbuf, &linsiz, fil)) >= 0)
    {
      lincnt++;
      if (linlen < 4 || linbuf[0] != '*' || linbuf[1] != '*')
	continue;
      char *pc = linbuf + 2;
      char *end = NULL;
      momitem_t *itm = NULL;
      while (isspace (*pc))
	pc++;
      if (isalpha (*pc) && mom_valid_item_name_str (pc, (const char **) &end))
	{
	  assert (end);
	  char endch = *end;
	  *end = 0;
	  itm = mom_make_named_item (pc);
	  *end = endch;
	  ld->lditemset = mom_hashset_put (ld->lditemset, itm);
	}
      else if (*pc == '_' && mom_valid_item_id_str (pc, (const char **) &end))
	{
	  assert (end);
	  char endch = *end;
	  *end = 0;
	  itm = mom_make_anonymous_item_by_id (pc);
	  *end = endch;
	  ld->lditemset = mom_hashset_put (ld->lditemset, itm);
	}
      else
	MOM_FATAPRINTF ("invalid line #%d in file %s:\t%s", lincnt, path,
			linbuf);
    }
  free (linbuf);
}

void
mom_load_state (const char *globaldata, const char *userdata)
{
  assert (globaldata);
  struct momloader_st ldr;
  memset (&ldr, 0, sizeof (ldr));
  ldr.ldmagic = LOADER_MAGIC_MOM;
  ldr.ldglobalpath = globaldata;
  ldr.ldglobalfile = fopen (globaldata, "r");
  if (!ldr.ldglobalfile)
    MOM_FATAPRINTF ("failed to open global data %s: %m", globaldata);
  if (userdata)
    {
      ldr.lduserpath = userdata;
      ldr.lduserfile = fopen (userdata, "r");
      if (!ldr.lduserfile)
	MOM_FATAPRINTF ("failed to open user data %s: %m", userdata);
    };
  first_pass_load_mom (&ldr, ldr.ldglobalpath, ldr.ldglobalfile);
  if (ldr.lduserpath)
    first_pass_load_mom (&ldr, ldr.lduserpath, ldr.lduserfile);
#warning mom_load_state should probably build the collected modules
}



////////////////////////////////////////////////////////////////
#define DUMPER_MAGIC_MOM 0x1e78645f	/* dumper magic 511206495 */
struct momdumper_st
{
  unsigned dumagic;		/* always DUMPER_MAGIC_MOM */
  const char *duprefix;		/* file prefix */
  const char *duglobalpath;
  const char *duuserpath;
  struct momhashset_st *duitemset;
  struct momqueueitems_st duitemque;
};

void
mom_scan_dumped_item (struct momdumper_st *du, const momitem_t *itm)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  if (!itm || itm == MOM_EMPTY)
    return;
  if (mom_hashset_contains (du->duitemset, itm))
    return;
  du->duitemset = mom_hashset_put (du->duitemset, itm);
  mom_queue_push_back (&du->duitemque, itm);
}


static void
scan_predefined_items_mom (struct momdumper_st *du)
{
#define MOM_HAS_PREDEFINED_NAMED(Nam,Hash) mom_scan_dumped_item(du,mompi_##Nam);
#define MOM_HAS_PREDEFINED_ANONYMOUS(Id,Hash) mom_scan_dumped_item(du,mompi_##Id);
#define MOM_HAS_PREDEFINED_DELIM(Nam,Str)
#include "predef-monimelt.h"
}				/* end scan_predefined_items_mom */

void
mom_dump_state (const char *prefix, const char *globaldata,
		const char *userdata)
{
  struct momdumper_st dmp;
  memset (&dmp, 0, sizeof (dmp));
  dmp.dumagic = DUMPER_MAGIC_MOM;
  assert (globaldata);
  assert (userdata);
  dmp.duprefix = prefix;
  if (prefix && prefix[0])
    {
      char buf[512];
      memset (buf, 0, sizeof (buf));
      snprintf (buf, sizeof (buf), "%s/%s", prefix, globaldata);
      dmp.duglobalpath = MOM_GC_STRDUP ("dumper global", buf);
      snprintf (buf, sizeof (buf), "%s/%s", prefix, userdata);
      dmp.duuserpath = MOM_GC_STRDUP ("dumper user", buf);
    }
  else
    {
      dmp.duglobalpath = globaldata;
      dmp.duuserpath = userdata;
    };
  scan_predefined_items_mom (&dmp);
}
