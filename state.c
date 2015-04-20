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
  struct momhashset_st *ldmoduleset;
  bool ldforglobals;
  char *ldlinebuf;
  size_t ldlinesize;
  size_t ldlinelen;
  size_t ldlinecol;
  size_t ldlinecount;
};


static void
first_pass_load_mom (struct momloader_st *ld, const char *path, FILE *fil)
{
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
      /// lines like: ** <item-name> are defining an item
      if (linlen >= 4 && linbuf[0] == '*' && linbuf[1] == '*')
	{
	  char *pc = linbuf + 2;
	  char *end = NULL;
	  momitem_t *itm = NULL;
	  while (isspace (*pc))
	    pc++;
	  if (isalpha (*pc)
	      && mom_valid_item_name_str (pc, (const char **) &end))
	    {
	      assert (end);
	      char endch = *end;
	      *end = 0;
	      itm = mom_make_named_item (pc);
	      *end = endch;
	      ld->lditemset = mom_hashset_put (ld->lditemset, itm);
	    }
	  else if (*pc == '_'
		   && mom_valid_item_id_str (pc, (const char **) &end))
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
      /// lines like: == <item-name> are requesting a module
      else if (linlen >= 4 && linbuf[0] == '=' && linbuf[1] == '=')
	{
	  char *pc = linbuf + 2;
	  char *end = NULL;
	  momitem_t *itm = NULL;
	  while (isspace (*pc))
	    pc++;
	  if (isalpha (*pc)
	      && mom_valid_item_name_str (pc, (const char **) &end))
	    {
	      assert (end);
	      char endch = *end;
	      *end = 0;
	      itm = mom_make_named_item (pc);
	      *end = endch;
	      ld->ldmoduleset = mom_hashset_put (ld->ldmoduleset, itm);
	    }
	  else if (*pc == '_'
		   && mom_valid_item_id_str (pc, (const char **) &end))
	    {
	      assert (end);
	      char endch = *end;
	      *end = 0;
	      itm = mom_make_anonymous_item_by_id (pc);
	      *end = endch;
	      ld->ldmoduleset = mom_hashset_put (ld->ldmoduleset, itm);
	    }
	  else
	    MOM_FATAPRINTF ("invalid line #%d in file %s:\t%s", lincnt, path,
			    linbuf);
	}
    }
  free (linbuf);
}


static void
make_modules_load_mom (struct momloader_st *ld)
{
  char makecmd[64];
  memset (makecmd, 0, sizeof (makecmd));
  assert (ld && ld->ldmagic == LOADER_MAGIC_MOM);
  snprintf (makecmd, sizeof (makecmd), "make -j %d modules", mom_nb_workers);
  const momseq_t *setmod = mom_hashset_elements_set (ld->ldmoduleset);
  assert (setmod != NULL);
  fflush (NULL);
  MOM_INFORMPRINTF ("running %s for %d modules",
		    makecmd, mom_hashset_count (ld->ldmoduleset));
  int ok = system (makecmd);
  if (!ok)
    MOM_FATAPRINTF ("failed to run %s", makecmd);
  int nbmod = 0;
  for (unsigned mix = 0; mix < setmod->slen; mix++)
    {
      const momitem_t *moditm = setmod->arritm[mix];
      assert (moditm && moditm != MOM_EMPTY);
      assert (moditm->itm_str);
      const momstring_t *mstr = mom_string_sprintf ("modules/momg_%s.so",
						    moditm->itm_str->cstr);
      void *dlh = dlopen (mstr->cstr, RTLD_NOW | RTLD_GLOBAL);
      if (!dlh)
	MOM_FATAPRINTF ("failed to dlopen %s : %s", mstr->cstr, dlerror ());
      nbmod++;
    }
  MOM_INFORMPRINTF ("loaded %d modules", nbmod);
}

bool
mom_token_load (struct momloader_st *ld, momvalue_t *pval)
{
  assert (ld && ld->ldmagic == LOADER_MAGIC_MOM);
  assert (pval != NULL);
  memset (pval, 0, sizeof (momvalue_t));
readagain:
  if (!ld->ldlinebuf || ld->ldlinecol >= ld->ldlinelen)
    {
      if (ld->ldlinebuf)
	memset (ld->ldlinebuf, 0, ld->ldlinesize);
      ld->ldlinelen =
	getline (&ld->ldlinebuf, &ld->ldlinesize,
		 ld->ldforglobals ? ld->ldglobalfile : ld->lduserfile);
      if (ld->ldlinelen <= 0)
	return false;
      ld->ldlinecount++;
      if (ld->ldlinebuf[0] == '/' && ld->ldlinebuf[1] == '/')
	{
	  ld->ldlinecol = ld->ldlinelen;
	  goto readagain;
	}
    };
  char c = ld->ldlinebuf[ld->ldlinecol];
  if (isspace (c))
    {
      ld->ldlinecol++;
      goto readagain;
    }
  else if (isdigit (c)
	   || ((c == '+' || c == '-')
	       && isdigit (ld->ldlinebuf[ld->ldlinecol + 1])))
    {
      char *endflo = NULL;
      char *endnum = NULL;
      const char *startc = ld->ldlinebuf + ld->ldlinecol;
      long long ll = strtol (startc, &endnum, 0);
      double x = strtod (startc, &endflo);
      if (endflo > endnum)
	{
	  pval->typnum = momty_double;
	  pval->vdbl = x;
	  ld->ldlinecol += endflo - startc;
	}
      else
	{
	  pval->typnum = momty_int;
	  pval->vint = (intptr_t) ll;
	  ld->ldlinecol += endnum - startc;
	}
      return true;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (ld->ldlinebuf + ld->ldlinecol + 1, "NAN", 3))
    {
      pval->typnum = momty_double;
      pval->vdbl = NAN;
      ld->ldlinecol += 4;
      return true;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (ld->ldlinebuf + ld->ldlinecol + 1, "INF", 3))
    {
      pval->typnum = momty_double;
      pval->vdbl = INFINITY;
      ld->ldlinecol += 4;
      return true;
    }
  else if (c == '"')
    {
      const char *startc = ld->ldlinebuf + ld->ldlinecol + 1;
      const char *eol = ld->ldlinebuf + ld->ldlinelen;
      char *buf = MOM_GC_SCALAR_ALLOC ("string buffer", eol - startc + 2);
      unsigned bufsiz = eol - startc + 1;
      int blen = 0;
      const char *pc = startc;
      for (pc = startc; pc < eol && *pc && *pc != '"'; pc++)
	{
	  if (*pc != '\\')
	    buf[blen++] = *pc;
	  else
	    {
	      pc++;
	      switch (*pc)
		{
		case '\"':
		  buf[blen++] = '\"';
		  pc++;
		  break;
		case '\'':
		  buf[blen++] = '\'';
		  pc++;
		  break;
		case '\\':
		  buf[blen++] = '\\';
		  pc++;
		  break;
		case 'a':
		  buf[blen++] = '\a';
		  pc++;
		  break;
		case 'b':
		  buf[blen++] = '\b';
		  pc++;
		  break;
		case 'f':
		  buf[blen++] = '\f';
		  pc++;
		  break;
		case 'n':
		  buf[blen++] = '\n';
		  pc++;
		  break;
		case 'r':
		  buf[blen++] = '\r';
		  pc++;
		  break;
		case 't':
		  buf[blen++] = '\t';
		  pc++;
		  break;
		case 'v':
		  buf[blen++] = '\v';
		  pc++;
		  break;
		case 'e':
		  buf[blen++] = 033 /*ESCAPE*/;
		  pc++;
		  break;
		case 'x':
		  {
		    unsigned hc = 0;
		    if (sscanf (pc + 1, "%02x", &hc) > 0)
		      {
			buf[blen++] = (char) hc;
			pc += 3;
		      }
		    else
		      {
			buf[blen++] = 'x';
			pc++;
		      }
		  }
		  break;
		case 'u':
		  {
		    int gap = 0;
		    unsigned hc = 0;
		    if (sscanf (pc, "u%04x%n", &hc, &gap) > 0)
		      {
			int nb =
			  u8_uctomb ((uint8_t *) buf + blen, (ucs4_t) hc,
				     bufsiz - blen);
			if (nb > 0)
			  blen += nb;
			pc += gap;
		      }
		    else
		      {
			buf[blen++] = 'u';
			pc++;
		      }
		    break;
		  }
		case 'U':
		  {
		    int gap = 0;
		    unsigned hc = 0;
		    if (sscanf (pc, "u%08x%n", &hc, &gap) > 0)
		      {
			int nb =
			  u8_uctomb ((uint8_t *) buf + blen, (ucs4_t) hc,
				     bufsiz - blen);
			if (nb > 0)
			  blen += nb;
			pc += gap;
		      }
		    else
		      {
			buf[blen++] = 'U';
			pc++;
		      }
		    break;
		  }
		}
	    }
	}
      if (*pc == '"')
	pc++;
      ld->ldlinecol += pc - startc + 1;
      pval->typnum = momty_string;
      pval->vstr = mom_make_string (buf);
      return true;
    }
}

void
second_pass_load_mom (struct momloader_st *ld, bool global)
{
  assert (ld && ld->ldmagic == LOADER_MAGIC_MOM);
  if (ld->ldlinebuf)
    {
      free (ld->ldlinebuf);
      ld->ldlinebuf = NULL;
      ld->ldlinesize = 0;
      ld->ldlinelen = 0;
      ld->ldlinecol = 0;
    }
}

void
mom_load_state ()
{
  struct momloader_st ldr;
  memset (&ldr, 0, sizeof (ldr));
  ldr.ldmagic = LOADER_MAGIC_MOM;
  ldr.ldglobalpath = MOM_GLOBAL_DATA_PATH;
  ldr.ldglobalfile = fopen (MOM_GLOBAL_DATA_PATH, "r");
  if (!ldr.ldglobalfile)
    MOM_FATAPRINTF ("failed to open global data %s: %m",
		    MOM_GLOBAL_DATA_PATH);
  ldr.lduserfile = fopen (MOM_USER_DATA_PATH, "r");
  if (!ldr.lduserfile)
    MOM_WARNPRINTF ("failed to open user data %s: %m", MOM_USER_DATA_PATH);
  else
    ldr.lduserpath = MOM_USER_DATA_PATH;
  first_pass_load_mom (&ldr, ldr.ldglobalpath, ldr.ldglobalfile);
  if (ldr.lduserpath)
    first_pass_load_mom (&ldr, ldr.lduserpath, ldr.lduserfile);
  if (ldr.ldmoduleset)
    {
      make_modules_load_mom (&ldr);
    }
  // second pass for global data
  ldr.ldforglobals = true;
  second_pass_load_mom (&ldr, true);
  // second pass for user data
  if (ldr.lduserfile)
    {
      ldr.ldforglobals = false;
      second_pass_load_mom (&ldr, false);
    }
}



////////////////////////////////////////////////////////////////
#define DUMPER_MAGIC_MOM 0x1e78645f	/* dumper magic 511206495 */
enum dumper_state_mom_en
{
  dump_none,
  dump_scan,
  dump_emit
};

struct momdumper_st
{
  unsigned dumagic;		/* always DUMPER_MAGIC_MOM */
  enum dumper_state_mom_en dustate;
  const char *duprefix;		/* file prefix */
  const char *durandsuffix;	/* random temporary suffix */
  const char *dupredefheaderpath;
  struct momhashset_st *duitemuserset;
  struct momhashset_st *duitemglobalset;
  struct momhashset_st *duitemmoduleset;
  struct momhashset_st *dupredefineditemset;
  struct momqueueitems_st duitemque;
  FILE *dufile;
};

bool
mom_scan_dumped_item (struct momdumper_st *du, const momitem_t *itm)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  if (!itm || itm == MOM_EMPTY)
    return false;
  if (du->dustate != dump_scan)
    return false;
  mom_item_lock ((momitem_t *) itm);
  if (itm->itm_space == momspa_none || itm->itm_space == momspa_transient)
    {
      mom_item_unlock ((momitem_t *) itm);
      return false;
    }
  if (mom_hashset_contains (du->duitemuserset, itm))
    return true;
  else if (mom_hashset_contains (du->duitemglobalset, itm))
    return true;
  if (itm->itm_space == momspa_user)
    du->duitemuserset = mom_hashset_put (du->duitemuserset, itm);
  else
    du->duitemglobalset = mom_hashset_put (du->duitemglobalset, itm);
  mom_queue_push_back (&du->duitemque, itm);
  return true;
}


void
mom_scan_dumped_module_item (struct momdumper_st *du, const momitem_t *moditm)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  if (!moditm || moditm == MOM_EMPTY)
    return;
  if (du->dustate != dump_scan)
    return;
  mom_scan_dumped_item (du, moditm);
  du->duitemmoduleset = mom_hashset_put (du->duitemmoduleset, moditm);
}

void
mom_scan_dumped_value (struct momdumper_st *du, const momvalue_t val)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  if (val.istransient)
    return;
  switch ((enum momvaltype_en) val.typnum)
    {
    case momty_double:
    case momty_int:
    case momty_null:
    case momty_string:
      return;
    case momty_item:
      mom_scan_dumped_item (du, val.vitem);
      return;
    case momty_set:
    case momty_tuple:
      {
	momseq_t *sq = val.vsequ;
	assert (sq);
	mom_scan_dumped_value (du, sq->meta);
	unsigned slen = sq->slen;
	for (unsigned ix = 0; ix < slen; ix++)
	  mom_scan_dumped_item (du, sq->arritm[ix]);
	return;
      }
    case momty_node:
      {
	momnode_t *nod = val.vnode;
	assert (nod);
	if (!mom_scan_dumped_item (du, nod->conn))
	  return;
	mom_scan_dumped_value (du, nod->meta);
	unsigned slen = nod->slen;
	for (unsigned ix = 0; ix < slen; ix++)
	  mom_scan_dumped_value (du, nod->arrsons[ix]);
	return;
      }
    }
}

static void
scan_predefined_items_mom (struct momdumper_st *du)
{
  const momseq_t *set = mom_predefined_items_set ();
  assert (set);
  unsigned slen = set->slen;
  for (unsigned ix = 0; ix < slen; ix++)
    mom_scan_dumped_item (du, set->arritm[ix]);
}				/* end scan_predefined_items_mom */

static void
scan_inside_dumped_item_mom (struct momdumper_st *du, momitem_t *itm)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (itm && itm != MOM_EMPTY);
  assert (mom_hashset_contains (du->duitemglobalset, itm)
	  || mom_hashset_contains (du->duitemuserset, itm));
  if (itm->itm_space == momspa_predefined)
    du->dupredefineditemset = mom_hashset_put (du->dupredefineditemset, itm);
  if (itm->itm_attrs)
    mom_attributes_scan_dump (itm->itm_attrs, du);
  if (itm->itm_comps)
    mom_components_scan_dump (itm->itm_comps, du);
  if (itm->itm_kind)
    {
      mom_scan_dumped_item (du, itm->itm_kind);
    }
#warning a completer scan_inside_dumped_item_mom
}

static FILE *
open_generated_file_dump_mom (struct momdumper_st *du, const char *path)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (strlen (path) < 128);
  assert (isalpha (path[0]));
  char pathbuf[256];
  memset (pathbuf, 0, sizeof (pathbuf));
  snprintf (pathbuf, sizeof (pathbuf), "%s%s%s",
	    du->duprefix, path, du->durandsuffix);
  FILE *out = fopen (pathbuf, "w");
  if (!out)
    MOM_FATAPRINTF ("failed to open generated file %s: %m", pathbuf);
  return out;
}

static void
close_generated_file_dump_mom (struct momdumper_st *du, FILE *fil,
			       const char *path)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (strlen (path) < 128);
  assert (fil);
  if (fclose (fil))
    MOM_FATAPRINTF ("failed to close generated file %s: %m", path);
  char newpathbuf[256];
  memset (newpathbuf, 0, sizeof (newpathbuf));
  snprintf (newpathbuf, sizeof (newpathbuf), "%s%s%s",
	    du->duprefix, path, du->durandsuffix);
  char oldpathbuf[256];
  memset (oldpathbuf, 0, sizeof (oldpathbuf));
  snprintf (oldpathbuf, sizeof (oldpathbuf), "%s%s", du->duprefix, path);
  char backpathbuf[256];
  memset (backpathbuf, 0, sizeof (backpathbuf));
  snprintf (backpathbuf, sizeof (backpathbuf), "%s%s~", du->duprefix, path);
  FILE *newout = fopen (newpathbuf, "r");
  if (!newout)
    MOM_FATAPRINTF ("failed to reopen generated file %s: %m", newpathbuf);
  FILE *oldout = fopen (oldpathbuf, "r");
  bool same = oldout != NULL;
  while (same)
    {
      int oldc = fgetc (oldout);
      int newc = fgetc (newout);
      same = (oldc == newc);
      if (oldc == EOF || newc == EOF)
	break;
    };
  if (oldout)
    fclose (oldout);
  if (newout)
    fclose (newout);
  if (same)
    {
      remove (newpathbuf);
      return;
    }
  else
    {
      rename (oldpathbuf, backpathbuf);
      if (rename (newpathbuf, oldpathbuf))
	MOM_FATAPRINTF ("failed to rename %s as %s : %m", newpathbuf,
			oldpathbuf);
      return;
    }
}

static void
emit_predefined_header_mom (struct momdumper_st *du)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  FILE *hdout = open_generated_file_dump_mom (du, MOM_PREDEFINED_PATH);
  mom_output_gplv3_notice (hdout, "///", "+++", MOM_PREDEFINED_PATH);
  fprintf (hdout, "#ifndef" " MOM_HAS_PREDEFINED_NAMED" "\n");
  fprintf (hdout, "#error missing " "MOM_HAS_PREDEFINED_NAMED" "\n");
  fprintf (hdout, "#endif" " /*MOM_HAS_PREDEFINED_NAMED*/" "\n");
  fprintf (hdout, "#ifndef" " MOM_HAS_PREDEFINED_ANONYMOUS" "\n");
  fprintf (hdout, "#error missing " "MOM_HAS_PREDEFINED_ANONYMOUS" "\n");
  fprintf (hdout, "#endif" " /*MOM_HAS_PREDEFINED_ANONYMOUS*/" "\n\n");
  const momseq_t *setpredef =
    mom_hashset_elements_set (du->dupredefineditemset);
  assert (setpredef);
  unsigned nbpredef = setpredef->slen;
  unsigned cntpredefanon = 0;
  unsigned cntpredefnamed = 0;
  for (unsigned ix = 0; ix < nbpredef; ix++)
    {
      const momitem_t *itmpredef = setpredef->arritm[ix];
      assert (itmpredef && itmpredef != MOM_EMPTY);
      if (itmpredef->itm_anonymous)
	{
	  cntpredefanon++;
	  fprintf (hdout, "MOM_HAS_PREDEFINED_ANONYMOUS(%s,%u)\n",
		   itmpredef->itm_id->cstr,
		   (unsigned) itmpredef->itm_id->shash);
	}
      else
	{
	  cntpredefnamed++;
	  fprintf (hdout, "MOM_HAS_PREDEFINED_NAMED(%s,%u)\n",
		   itmpredef->itm_name->cstr,
		   (unsigned) itmpredef->itm_name->shash);
	}
    }
  fprintf (hdout, "\n\n" "#ifndef" " MOM_NB_PREDEFINED_ANONYMOUS\n");
  fprintf (hdout, "#define" " MOM_NB_PREDEFINED_ANONYMOUS" " %u\n",
	   cntpredefanon);
  fprintf (hdout, "#endif /*MOM_NB_PREDEFINED_ANONYMOUS*/\n");
  fprintf (hdout, "#ifndef" " MOM_NB_PREDEFINED_NAMED\n");
  fprintf (hdout, "#define" " MOM_NB_PREDEFINED_NAMED" " %u\n",
	   cntpredefnamed);
  fprintf (hdout, "#endif /*MOM_NB_PREDEFINED_NAMED*/\n");
  fprintf (hdout, "#undef " "MOM_HAS_PREDEFINED_ANONYMOUS\n");
  fprintf (hdout, "#undef " "MOM_HAS_PREDEFINED_NAMED\n");

  fprintf (hdout, "\n // end of generated file %s\n", MOM_PREDEFINED_PATH);
  close_generated_file_dump_mom (du, hdout, MOM_PREDEFINED_PATH);
}

static void
emit_dumped_item_mom (struct momdumper_st *du, const momitem_t *itm)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (du->dustate == dump_emit);
  assert (du->dufile);
  if (!mom_hashset_contains (du->duitemuserset, itm)
      && !mom_hashset_contains (du->duitemglobalset, itm))
    return;
  putc ('\n', du->dufile);
  fprintf (du->dufile, "** %s\n", itm->itm_str->cstr);
}

bool
mom_emit_dumped_itemref (struct momdumper_st * du, const momitem_t *itm)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (du->dustate == dump_emit);
  assert (du->dufile);
  if (!itm || itm == MOM_EMPTY
      || (!mom_hashset_contains (du->duitemuserset, itm)
	  && !mom_hashset_contains (du->duitemglobalset, itm)))
    {
      fputs ("~", du->dufile);
      return false;
    }
  else
    {
      assert (itm->itm_str);
      fprintf (du->dufile, "%s", itm->itm_str->cstr);
      return true;
    }
}

void
mom_emit_dumped_value (struct momdumper_st *du, const momvalue_t val)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (du->dustate == dump_emit);
  assert (du->dufile);
}

static void
emit_global_items_mom (struct momdumper_st *du)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (du->dustate == dump_emit);
  const momseq_t *set = mom_hashset_elements_set (du->duitemglobalset);
  assert (set);
  unsigned nbel = set->slen;
  for (unsigned ix = 0; ix < nbel; ix++)
    emit_dumped_item_mom (du, set->arritm[ix]);
}

static void
emit_global_modules_mom (struct momdumper_st *du, const momseq_t *setmod)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  if (!setmod)
    return;
  unsigned nbmod = setmod->slen;
  for (unsigned ix = 0; ix < nbmod; ix++)
    {
      const momitem_t *moditm = setmod->arritm[ix];
      assert (moditm && moditm != MOM_EMPTY);
      if (moditm->itm_space == momspa_predefined
	  || moditm->itm_space == momspa_global)
	fprintf (du->dufile, "!! %s\n", moditm->itm_str->cstr);
    }
  if (nbmod > 0)
    fputc ('\n', du->dufile);
}

static void
emit_user_items_mom (struct momdumper_st *du)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  assert (du->dustate == dump_emit);
  const momseq_t *set = mom_hashset_elements_set (du->duitemuserset);
  if (!set)
    return;
  unsigned nbel = set->slen;
  for (unsigned ix = 0; ix < nbel; ix++)
    emit_dumped_item_mom (du, set->arritm[ix]);
}

static void
emit_user_modules_mom (struct momdumper_st *du, const momseq_t *setmod)
{
  assert (du && du->dumagic == DUMPER_MAGIC_MOM);
  if (!setmod)
    return;
  unsigned nbmod = setmod->slen;
  for (unsigned ix = 0; ix < nbmod; ix++)
    {
      const momitem_t *moditm = setmod->arritm[ix];
      assert (moditm && moditm != MOM_EMPTY);
      if (moditm->itm_space == momspa_user)
	fprintf (du->dufile, "!! %s\n", moditm->itm_str->cstr);
    }
  if (nbmod > 0)
    fputc ('\n', du->dufile);
}


void
mom_dump_state (const char *prefix)
{
  struct momdumper_st dmp;
  memset (&dmp, 0, sizeof (dmp));
  dmp.dumagic = DUMPER_MAGIC_MOM;
  dmp.duprefix = prefix;
  {
    char sufbuf[64];
    memset (sufbuf, 0, sizeof (sufbuf));
    snprintf (sufbuf, sizeof (sufbuf), "+p%d-r%u.tmp", (int) getpid (),
	      (unsigned) mom_random_nonzero_32_here ());
    dmp.durandsuffix = MOM_GC_STRDUP ("random suffix", sufbuf);
  }
  if (prefix && prefix[0])
    {
      char buf[512];
      memset (buf, 0, sizeof (buf));
      if (strlen (prefix) > 100)
	MOM_FATAPRINTF ("too long dump prefix %s", prefix);
    }
  dmp.dustate = dump_scan;
  scan_predefined_items_mom (&dmp);
  while (mom_queue_size (&dmp.duitemque) > 0)
    {
      const momitem_t *curitm = mom_queue_pop_front (&dmp.duitemque);
      scan_inside_dumped_item_mom (&dmp, (momitem_t *) curitm);
    }
  emit_predefined_header_mom (&dmp);
  MOM_INFORMPRINTF ("dumped state to prefix %s : %u global + %u user items",
		    prefix, mom_hashset_count (dmp.duitemglobalset),
		    mom_hashset_count (dmp.duitemuserset));
  ////
  dmp.dustate = dump_emit;
  {
    dmp.dufile = open_generated_file_dump_mom (&dmp, MOM_GLOBAL_DATA_PATH);
    mom_output_gplv3_notice (dmp.dufile, "///", "", MOM_GLOBAL_DATA_PATH);
    fputc ('\n', dmp.dufile);
    const momseq_t *setmod = mom_hashset_elements_set (dmp.duitemmoduleset);
    emit_global_modules_mom (&dmp, setmod);
    emit_global_items_mom (&dmp);
    fprintf (dmp.dufile, "//// end of global file %s\n",
	     MOM_GLOBAL_DATA_PATH);
    close_generated_file_dump_mom (&dmp, dmp.dufile, MOM_GLOBAL_DATA_PATH);
    dmp.dufile = NULL;
  }
  {
    dmp.dufile = open_generated_file_dump_mom (&dmp, MOM_USER_DATA_PATH);
    mom_output_gplv3_notice (dmp.dufile, "///", "", MOM_USER_DATA_PATH);
    fputc ('\n', dmp.dufile);
    const momseq_t *setmod = mom_hashset_elements_set (dmp.duitemmoduleset);
    emit_user_modules_mom (&dmp, setmod);
    emit_user_items_mom (&dmp);
    fprintf (dmp.dufile, "//// end of user file %s\n", MOM_USER_DATA_PATH);
    close_generated_file_dump_mom (&dmp, dmp.dufile, MOM_USER_DATA_PATH);
    dmp.dufile = NULL;
  }
}


#define BASE_YEAR_MOM 2015
void
mom_output_gplv3_notice (FILE *out, const char *prefix, const char *suffix,
			 const char *filename)
{
  time_t now = 0;
  time (&now);
  struct tm nowtm;
  memset (&nowtm, 0, sizeof (nowtm));
  localtime_r (&now, &nowtm);
  if (!prefix)
    prefix = "";
  if (!suffix)
    suffix = "";
  fprintf (out, "%s *** generated file %s - DO NOT EDIT %s\n", prefix,
	   filename, suffix);
  if (1900 + nowtm.tm_year != BASE_YEAR_MOM)
    fprintf (out,
	     "%s Copyright (C) %d - %d Free Software Foundation, Inc. %s\n",
	     prefix, BASE_YEAR_MOM, 1900 + nowtm.tm_year, suffix);
  else
    fprintf (out,
	     "%s Copyright (C) %d Free Software Foundation, Inc. %s\n",
	     prefix, BASE_YEAR_MOM, suffix);
  fprintf (out,
	   "%s MONIMELT is a monitor for MELT - see http://gcc-melt.org/ %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s This generated file %s is part of MONIMELT, part of GCC %s\n",
	   prefix, filename, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
  fprintf (out,
	   "%s GCC is free software; you can redistribute it and/or modify %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s it under the terms of the GNU General Public License as published by %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s the Free Software Foundation; either version 3, or (at your option) %s\n",
	   prefix, suffix);
  fprintf (out, "%s any later version. %s\n", prefix, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
  fprintf (out,
	   "%s  GCC is distributed in the hope that it will be useful, %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s  but WITHOUT ANY WARRANTY; without even the implied warranty of %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the %s\n",
	   prefix, suffix);
  fprintf (out, "%s  GNU General Public License for more details. %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s  You should have received a copy of the GNU General Public License %s\n",
	   prefix, suffix);
  fprintf (out,
	   "%s  along with GCC; see the file COPYING3.   If not see %s\n",
	   prefix, suffix);
  fprintf (out, "%s  <http://www.gnu.org/licenses/>. %s\n", prefix, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
}


void
mom_output_utf8cstr_cencoded (FILE *fil, const char *str, int len)
{
  if (!fil || !str)
    return;
  if (len < 0)
    len = strlen (str);
  const char *pc = str;
  const char *pend = str + len;
  while (pc < pend)
    {
      ucs4_t uc = 0;
      int lc = u8_strmbtouc (&uc, (const uint8_t *) pc);
      if (lc <= 0)
	break;
      switch (uc)
	{
	case (ucs4_t) '\"':
	  fputs ("\\\"", fil);
	  break;
	case (ucs4_t) '\'':
	  fputs ("\\\'", fil);
	  break;
	case (ucs4_t) '\\':
	  fputs ("\\\\", fil);
	  break;
	case (ucs4_t) '\a':
	  fputs ("\\a", fil);
	  break;
	case (ucs4_t) '\b':
	  fputs ("\\b", fil);
	  break;
	case (ucs4_t) '\f':
	  fputs ("\\f", fil);
	  break;
	case (ucs4_t) '\n':
	  fputs ("\\n", fil);
	  break;
	case (ucs4_t) '\r':
	  fputs ("\\r", fil);
	  break;
	case (ucs4_t) '\t':
	  fputs ("\\t", fil);
	  break;
	case (ucs4_t) '\v':
	  fputs ("\\v", fil);
	  break;
	default:
	  if (uc >= (ucs4_t) ' ' && uc <= 0x7f && isprint ((char) (uc)))
	    fputc (uc, fil);
	  else if (uc <= (ucs4_t) 0xffff)
	    fprintf (fil, "\\u%04x", uc & 0xffff);
	  else
	    fprintf (fil, "\\U%08x", (unsigned) uc);
	}
      pc += lc;
    }
}
