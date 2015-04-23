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
  struct momqueuevalues_st ldquetokens;
  bool ldforglobals;
  char *ldlinebuf;
  size_t ldlinesize;
  size_t ldlinelen;
  size_t ldlinecol;
  size_t ldlinecount;
};

static struct momloader_st *loader_mom;

static void
first_pass_load_mom (const char *path, FILE *fil)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
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
	  MOM_DEBUGPRINTF (load, "first %s pass line#%d: %s",
			   loader_mom->ldforglobals ? "global" : "user",
			   lincnt, linbuf);
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
	      MOM_DEBUGPRINTF (load, "first %s pass named item @%p %s",
			       loader_mom->ldforglobals ? "global" : "user",
			       itm, pc);
	      *end = endch;
	      loader_mom->lditemset =
		mom_hashset_put (loader_mom->lditemset, itm);
	    }
	  else if (*pc == '_'
		   && mom_valid_item_id_str (pc, (const char **) &end))
	    {
	      assert (end);
	      char endch = *end;
	      *end = 0;
	      itm = mom_make_anonymous_item_by_id (pc);
	      MOM_DEBUGPRINTF (load, "first %s pass anonymous item @%p %s",
			       loader_mom->ldforglobals ? "global" : "user",
			       itm, pc);
	      *end = endch;
	      loader_mom->lditemset =
		mom_hashset_put (loader_mom->lditemset, itm);
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
	      loader_mom->ldmoduleset =
		mom_hashset_put (loader_mom->ldmoduleset, itm);
	    }
	  else if (*pc == '_'
		   && mom_valid_item_id_str (pc, (const char **) &end))
	    {
	      assert (end);
	      char endch = *end;
	      *end = 0;
	      itm = mom_make_anonymous_item_by_id (pc);
	      *end = endch;
	      loader_mom->ldmoduleset =
		mom_hashset_put (loader_mom->ldmoduleset, itm);
	    }
	  else
	    MOM_FATAPRINTF ("invalid line #%d in file %s:\t%s", lincnt, path,
			    linbuf);
	}
    }
  free (linbuf);
}


static void
make_modules_load_mom (void)
{
  char makecmd[64];
  memset (makecmd, 0, sizeof (makecmd));
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  snprintf (makecmd, sizeof (makecmd), "make -j %d modules", mom_nb_workers);
  const momseq_t *setmod = mom_hashset_elements_set (loader_mom->ldmoduleset);
  assert (setmod != NULL);
  fflush (NULL);
  MOM_INFORMPRINTF ("running %s for %d modules",
		    makecmd, mom_hashset_count (loader_mom->ldmoduleset));
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

static bool
token_string_load_mom (momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  const char *startc = loader_mom->ldlinebuf + loader_mom->ldlinecol + 1;
  const char *eol = loader_mom->ldlinebuf + loader_mom->ldlinelen;
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
		    int nb = u8_uctomb ((uint8_t *) buf + blen, (ucs4_t) hc,
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
		    int nb = u8_uctomb ((uint8_t *) buf + blen, (ucs4_t) hc,
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
  loader_mom->ldlinecol += pc - startc + 1;
  pval->typnum = momty_string;
  pval->vstr = mom_make_string (buf);
  return true;
}


const char *const delim_mom[] = {
  /// first the 2 char delimiters
  "==",
  "**", "++", "--", "[[", "]]", "..", "°", "*", "(", ")", "[", "]",
  "{", "}", "<", ">", "^", "!",
  NULL
};


bool
mom_token_load (momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (pval != NULL);
  memset (pval, 0, sizeof (momvalue_t));
  if (mom_queuevalue_size (&loader_mom->ldquetokens))
    {
      mom_queuevalue_pop_front (&loader_mom->ldquetokens);
      return true;
    }
readagain:
  if (!loader_mom->ldlinebuf
      || loader_mom->ldlinecol >= loader_mom->ldlinelen)
    {
      if (loader_mom->ldlinebuf)
	memset (loader_mom->ldlinebuf, 0, loader_mom->ldlinesize);
      loader_mom->ldlinelen =
	getline (&loader_mom->ldlinebuf, &loader_mom->ldlinesize,
		 loader_mom->
		 ldforglobals ? loader_mom->ldglobalfile : loader_mom->
		 lduserfile);
      if (loader_mom->ldlinelen <= 0)
	return false;
      loader_mom->ldlinecount++;
      if (loader_mom->ldlinebuf[0] == '/' && loader_mom->ldlinebuf[1] == '/')
	{
	  loader_mom->ldlinecol = loader_mom->ldlinelen;
	  goto readagain;
	}
    };
  char c = loader_mom->ldlinebuf[loader_mom->ldlinecol];
  char *pstart = loader_mom->ldlinebuf + loader_mom->ldlinecol;
  char *end = NULL;
  if (isspace (c))
    {
      loader_mom->ldlinecol++;
      goto readagain;
    }
  else if (isdigit (c)
	   || ((c == '+' || c == '-')
	       && isdigit (loader_mom->ldlinebuf[loader_mom->ldlinecol + 1])))
    {
      char *endflo = NULL;
      char *endnum = NULL;
      const char *startc = loader_mom->ldlinebuf + loader_mom->ldlinecol;
      long long ll = strtol (startc, &endnum, 0);
      double x = strtod (startc, &endflo);
      if (endflo > endnum)
	{
	  pval->typnum = momty_double;
	  pval->vdbl = x;
	  loader_mom->ldlinecol += endflo - startc;
	}
      else
	{
	  pval->typnum = momty_int;
	  pval->vint = (intptr_t) ll;
	  loader_mom->ldlinecol += endnum - startc;
	}
      return true;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (loader_mom->ldlinebuf + loader_mom->ldlinecol + 1,
			    "NAN", 3))
    {
      pval->typnum = momty_double;
      pval->vdbl = NAN;
      loader_mom->ldlinecol += 4;
      return true;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (loader_mom->ldlinebuf + loader_mom->ldlinecol + 1,
			    "INF", 3))
    {
      pval->typnum = momty_double;
      pval->vdbl = INFINITY;
      loader_mom->ldlinecol += 4;
      return true;
    }
  else if (c == '"')
    {
      return token_string_load_mom (pval);
    }
  else if (ispunct (c) || (unsigned char) c >= 0x7f)
    {
      for (int ix = 0; delim_mom[ix]; ix++)
	{
	  if (!strncmp
	      (loader_mom->ldlinebuf + loader_mom->ldlinecol, delim_mom[ix],
	       strlen (delim_mom[ix])))
	    {
	      pval->typnum = momty_delim;
	      strcpy (pval->vdelim.delim, delim_mom[ix]);
	      return true;
	    }
	}
    }
  else if (c == '_' && mom_valid_item_id_str (pstart, (const char **) &end)
	   && end && (!isalnum (*end) && *end != '_'))
    {
      char olde = *end;
      *end = '\0';
      const momitem_t *itm = mom_find_item (pstart);
      if (itm)
	{
	  pval->vitem = (momitem_t *) itm;
	  pval->typnum = momty_item;
	  loader_mom->ldlinecol += end - pstart;
	  *end = olde;
	  return true;
	}
    }
  else if (isalpha (c)
	   && mom_valid_item_name_str (pstart, (const char **) &end) && end
	   && (!isalnum (*end) && *end != '_'))
    {
      char olde = *end;
      *end = '\0';
      const momitem_t *itm = mom_find_item (pstart);
      if (itm)
	{
	  pval->typnum = momty_item;
	  pval->vitem = (momitem_t *) itm;
	  loader_mom->ldlinecol += end - pstart;
	  *end = olde;
	  return true;
	}
    }
  return false;
}


////////////////
void
load_fill_item_mom (momitem_t *itm)
{				// keep in sync with emit_content_dumped_item_mom
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (itm && itm->itm_str);
  MOM_DEBUGPRINTF (load, "load fill item %s", itm->itm_str->cstr);
  momvalue_t vtok = MOM_NONEV;
  momvalue_t vtokbis = MOM_NONEV;
  if (mom_token_load (&vtok) && mom_value_is_delim (vtok, "{"))
    {
      while ((vtokbis = MOM_NONEV, mom_token_load (&vtokbis))
	     && mom_value_is_delim (vtokbis, "*"))
	{
	  momvalue_t vat = MOM_NONEV;
	  const momitem_t *itmat = mom_load_itemref ();
	  if (!itmat)
	    break;
	  if (mom_load_value (&vat) && vat.typnum != momty_null)
	    itm->itm_attrs = mom_attributes_put (itm->itm_attrs, itmat, &vat);
	}
      if (!((vtokbis = MOM_NONEV), !mom_token_load (&vtokbis))
	  || !mom_value_is_delim (vtokbis, "}"))
	MOM_FATAPRINTF ("expecting } to end attributes of item %s"
			" in %s file %s line %d", itm->itm_str->cstr,
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, (int) loader_mom->ldlinecount);
    }
  // should load the components
  if (mom_token_load (&vtok) && mom_value_is_delim (vtok, "[["))
    {
      momvalue_t valcomp = MOM_NONEV;
      while ((valcomp = MOM_NONEV), mom_load_value (&valcomp))
	itm->itm_comps = mom_components_append1 (itm->itm_comps, valcomp);
      if (!((vtokbis = MOM_NONEV), !mom_token_load (&vtokbis))
	  || !mom_value_is_delim (vtokbis, "]]"))
	MOM_FATAPRINTF ("expecting ]] to end attributes of item %s"
			" in %s file %s line %d", itm->itm_str->cstr,
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, (int) loader_mom->ldlinecount);
    }
#warning load_fill_item_mom unimplemented, see emit_content_dumped_item_mom
  MOM_WARNPRINTF ("load_fill_item_mom %s unimplemented", itm->itm_str->cstr);
}				/* end load_fill_item_mom */

////////////////

void
second_pass_load_mom (bool global)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (loader_mom->ldlinebuf)
    free (loader_mom->ldlinebuf);
  {
    unsigned siz = 128;
    char *buf = malloc (siz);
    if (!buf)
      MOM_FATAPRINTF ("failed to malloc line buffer of %d", siz);
    memset (buf, 0, siz);
    loader_mom->ldlinebuf = buf;
    loader_mom->ldlinesize = siz;
  }
  loader_mom->ldlinelen = 0;
  loader_mom->ldforglobals = global;
  rewind (loader_mom->ldforglobals ? loader_mom->
	  ldglobalfile : loader_mom->lduserfile);
  loader_mom->ldlinecol = loader_mom->ldlinelen = loader_mom->ldlinecount = 0;
  do
    {
      if (loader_mom->ldlinebuf)
	memset (loader_mom->ldlinebuf, 0, loader_mom->ldlinesize);
      loader_mom->ldlinelen =
	getline (&loader_mom->ldlinebuf, &loader_mom->ldlinesize,
		 loader_mom->
		 ldforglobals ? loader_mom->ldglobalfile : loader_mom->
		 lduserfile);
      if (loader_mom->ldlinelen <= 0)
	return;
      loader_mom->ldlinecount++;
      MOM_DEBUGPRINTF (load, "second %s pass line#%d: %s",
		       loader_mom->ldforglobals ? "global" : "user",
		       (int) loader_mom->ldlinecount, loader_mom->ldlinebuf);
      if (loader_mom->ldlinelen > 4
	  && loader_mom->ldlinebuf[0] == '*'
	  && loader_mom->ldlinebuf[1] == '*')
	{
	  loader_mom->ldlinecol = 2;
	  momvalue_t val = MOM_NONEV;
	  if (!mom_token_load (&val) || val.typnum != momty_item)
	    MOM_FATAPRINTF ("invalid line %d '%s' of %s file %s",
			    (int) loader_mom->ldlinecount,
			    loader_mom->ldlinebuf,
			    loader_mom->ldforglobals ? "global" : "user",
			    loader_mom->ldforglobals ? loader_mom->
			    ldglobalpath : loader_mom->lduserpath);
	  MOM_DEBUGPRINTF (load, "second %s pass item %s",
			   loader_mom->ldforglobals ? "global" : "user",
			   val.vitem->itm_str->cstr);
	  memset (&loader_mom->ldquetokens, 0,
		  sizeof (loader_mom->ldquetokens));
	  load_fill_item_mom (val.vitem);
	}
    }
  while (!feof
	 (loader_mom->ldforglobals ? loader_mom->
	  ldglobalfile : loader_mom->lduserfile));
}

void
mom_load_push_front_token (momvalue_t valtok)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (valtok.typnum != momty_null)
    mom_queuevalue_push_front (&loader_mom->ldquetokens, valtok);
}


void
mom_load_push_back_token (momvalue_t valtok)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (valtok.typnum != momty_null)
    mom_queuevalue_push_back (&loader_mom->ldquetokens, valtok);
}

unsigned
mom_load_nb_queued_tokens (void)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  return mom_queuevalue_size (&loader_mom->ldquetokens);
}

const momnode_t *
mom_load_queued_tokens_mode (const momitem_t *connitm, momvalue_t meta)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (!connitm || mom_queuevalue_size (&loader_mom->ldquetokens) == 0)
    return NULL;
  return mom_queuevalue_node (&loader_mom->ldquetokens, connitm, meta);
}



static bool
load_metavalue_mom (momvalue_t vtok, momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (!pval)
    return false;
  if (mom_value_is_delim (vtok, "!"))
    return mom_load_value (pval);
  return false;
}

bool				// should be in sync with mom_emit_dumped_value
mom_load_value (momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (!pval)
    return false;
  memset (pval, 0, sizeof (momvalue_t));
  momvalue_t vtok = MOM_NONEV;
  momvalue_t vtokbis = MOM_NONEV;
  momvalue_t vtokter = MOM_NONEV;
  if (!mom_token_load (&vtok))
    return false;
  if (vtok.typnum == momty_item)
    {				// items
      *pval = vtok;
      return true;
    }
  if (mom_value_is_delim (vtok, "~"))
    {				// null value
      *pval = MOM_NONEV;
      return true;
    }
  if (vtok.typnum == momty_int || vtok.typnum == momty_string
      || vtok.typnum == momty_double)
    {				/// scalars
      *pval = vtok;
      return true;
    }
  if (mom_value_is_delim (vtok, "°"))
    {				/// delimiters
      int linecnt = loader_mom->ldlinecount;
      if (mom_token_load (&vtokbis) && vtokbis.typnum == momty_string)
	{
	  pval->typnum = momty_delim;
	  strncpy (pval->vdelim.delim, vtokbis.vstr->cstr,
		   sizeof (pval->vdelim.delim));
	  return true;
	}
      else
	MOM_FATAPRINTF ("expecting string for delimiter after °"
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
    }
  if (mom_value_is_delim (vtok, "["))
    {				////tuples
      int linecnt = loader_mom->ldlinecount;
      if (!mom_token_load (&vtokbis))
	MOM_FATAPRINTF ("missing tuple content after ["
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      momvalue_t metav = MOM_NONEV;
      const momitem_t *curitm = NULL;
      struct momqueueitems_st quitems;
      memset (&quitems, 0, sizeof (quitems));
      load_metavalue_mom (vtokbis, &metav);
      while ((curitm = mom_load_itemref ()) != NULL)
	{
	  mom_queueitem_push_back (&quitems, curitm);
	  linecnt = loader_mom->ldlinecount;
	}
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, "]"))
	MOM_FATAPRINTF ("missing ] after tuple content"
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      pval->typnum = momty_tuple;
      pval->vtuple = (momseq_t *) mom_queueitem_tuple (&quitems, metav);
      return true;
    }				/* done tuples */
  if (mom_value_is_delim (vtok, "{"))
    {				//// sets
      int linecnt = loader_mom->ldlinecount;
      if (!mom_token_load (&vtokbis))
	MOM_FATAPRINTF ("missing set content after {"
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      momvalue_t metav = MOM_NONEV;
      const momitem_t *curitm = NULL;
      struct momqueueitems_st quitems;
      memset (&quitems, 0, sizeof (quitems));
      load_metavalue_mom (vtokbis, &metav);
      while ((curitm = mom_load_itemref ()) != NULL)
	{
	  mom_queueitem_push_back (&quitems, curitm);
	  linecnt = loader_mom->ldlinecount;
	}
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, "}"))
	MOM_FATAPRINTF ("missing } after set content"
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      {
	pval->typnum = momty_set;
	const momseq_t *tup = mom_queueitem_tuple (&quitems, MOM_NONEV);
	assert (tup);
	pval->vset =
	  (momseq_t *) mom_make_sized_meta_set (metav, tup->slen,
						(const momitem_t **)
						tup->arritm);
	return true;
      }
    }				/* done sets */
  if (mom_value_is_delim (vtok, "^"))
    {				//// nodes
      int linecnt = loader_mom->ldlinecount;
      const momitem_t *connitm = mom_load_itemref ();
      momvalue_t metav = MOM_NONEV;
      if (!connitm || !mom_token_load (&vtokbis))
	MOM_FATAPRINTF ("missing connective item after ^ "
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      load_metavalue_mom (vtokbis, &metav);
      linecnt = loader_mom->ldlinecount;
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, "("))
	MOM_FATAPRINTF ("missing ( in node " " in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      struct momqueuevalues_st quvals;
      memset (&quvals, 0, sizeof (quvals));
      momvalue_t vson = MOM_NONEV;
      while ((vson = MOM_NONEV), mom_load_value (&vson))
	{
	  mom_queuevalue_push_back (&quvals, vson);
	  linecnt = loader_mom->ldlinecount;
	}
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, ")"))
	MOM_FATAPRINTF ("missing ) to end node " " in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->
			ldforglobals ? loader_mom->ldglobalpath : loader_mom->
			lduserpath, linecnt);
      pval->typnum = momty_node;
      pval->vnode =
	(momnode_t *) mom_queuevalue_node (&quvals, connitm, metav);
      return true;
    }				/* done nodes */
  if (vtok.typnum != momty_null)
    mom_load_push_front_token (vtok);
  return false;
}				/* end mom_load_value */



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
  ldr.ldforglobals = true;
  loader_mom = &ldr;
  first_pass_load_mom (ldr.ldglobalpath, ldr.ldglobalfile);
  if (ldr.lduserpath)
    {
      ldr.ldforglobals = false;
      first_pass_load_mom (ldr.lduserpath, ldr.lduserfile);
    }
  if (ldr.ldmoduleset)
    {
      make_modules_load_mom ();
    }
  // second pass for global data
  ldr.ldforglobals = true;
  second_pass_load_mom (true);
  // second pass for user data
  if (ldr.lduserfile)
    {
      ldr.ldforglobals = false;
      second_pass_load_mom (false);
    }
  MOM_INFORMPRINTF
    ("loaded %d items and %d modules from global %s and user %s files",
     (int) mom_hashset_count (ldr.lditemset),
     (int) mom_hashset_count (ldr.ldmoduleset), ldr.ldglobalpath,
     ldr.lduserpath);
  loader_mom = NULL;
  memset (&ldr, 0, sizeof (ldr));
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
  int duindentation;
  long dulastnloff;
  struct momhashset_st *duitemuserset;
  struct momhashset_st *duitemglobalset;
  struct momhashset_st *duitemmoduleset;
  struct momhashset_st *dupredefineditemset;
  struct momqueueitems_st duitemque;
  FILE *dufile;
};

static _Thread_local struct momdumper_st *dumper_mom;

bool
mom_scan_dumped_item (const momitem_t *itm)
{
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan item outside of dumping");
  if (!itm || itm == MOM_EMPTY)
    return false;
  if (dumper_mom->dustate != dump_scan)
    return false;
  mom_item_lock ((momitem_t *) itm);
  if (itm->itm_space == momspa_none || itm->itm_space == momspa_transient)
    {
      mom_item_unlock ((momitem_t *) itm);
      return false;
    }
  if (mom_hashset_contains (dumper_mom->duitemuserset, itm))
    return true;
  else if (mom_hashset_contains (dumper_mom->duitemglobalset, itm))
    return true;
  if (itm->itm_space == momspa_user)
    dumper_mom->duitemuserset =
      mom_hashset_put (dumper_mom->duitemuserset, itm);
  else
    dumper_mom->duitemglobalset =
      mom_hashset_put (dumper_mom->duitemglobalset, itm);
  mom_queueitem_push_back (&dumper_mom->duitemque, itm);
  return true;
}


void
mom_scan_dumped_module_item (const momitem_t *moditm)
{
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan module outside of dumping");
  if (!moditm || moditm == MOM_EMPTY)
    return;
  if (dumper_mom->dustate != dump_scan)
    return;
  mom_scan_dumped_item (moditm);
  dumper_mom->duitemmoduleset =
    mom_hashset_put (dumper_mom->duitemmoduleset, moditm);
}

void
mom_scan_dumped_value (const momvalue_t val)
{
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan value outside of dumping");
  if (val.istransient)
    return;
  switch ((enum momvaltype_en) val.typnum)
    {
    case momty_double:
    case momty_int:
    case momty_null:
    case momty_string:
    case momty_delim:
      return;
    case momty_item:
      mom_scan_dumped_item (val.vitem);
      return;
    case momty_set:
    case momty_tuple:
      {
	momseq_t *sq = val.vsequ;
	assert (sq);
	mom_scan_dumped_value (sq->meta);
	unsigned slen = sq->slen;
	for (unsigned ix = 0; ix < slen; ix++)
	  mom_scan_dumped_item (sq->arritm[ix]);
	return;
      }
    case momty_node:
      {
	momnode_t *nod = val.vnode;
	assert (nod);
	if (!mom_scan_dumped_item (nod->conn))
	  return;
	mom_scan_dumped_value (nod->meta);
	unsigned slen = nod->slen;
	for (unsigned ix = 0; ix < slen; ix++)
	  mom_scan_dumped_value (nod->arrsons[ix]);
	return;
      }
    }
}

static void
scan_predefined_items_mom (void)
{
  const momseq_t *set = mom_predefined_items_set ();
  assert (set);
  unsigned slen = set->slen;
  for (unsigned ix = 0; ix < slen; ix++)
    mom_scan_dumped_item (set->arritm[ix]);
}				/* end scan_predefined_items_mom */

static void
scan_inside_dumped_item_mom (momitem_t *itm)
{
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan inside item outside of dumping");
  assert (itm && itm != MOM_EMPTY);
  assert (mom_hashset_contains (dumper_mom->duitemglobalset, itm)
	  || mom_hashset_contains (dumper_mom->duitemuserset, itm));
  if (itm->itm_space == momspa_predefined)
    dumper_mom->dupredefineditemset =
      mom_hashset_put (dumper_mom->dupredefineditemset, itm);
  if (itm->itm_attrs)
    mom_attributes_scan_dump (itm->itm_attrs);
  if (itm->itm_comps)
    mom_components_scan_dump (itm->itm_comps);
  if (itm->itm_kind)
    {
      mom_scan_dumped_item (itm->itm_kind);
    }
#warning a completer scan_inside_dumped_item_mom
}

static FILE *
open_generated_file_dump_mom (const char *path)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (strlen (path) < 128);
  assert (isalpha (path[0]));
  char pathbuf[256];
  memset (pathbuf, 0, sizeof (pathbuf));
  snprintf (pathbuf, sizeof (pathbuf), "%s%s%s",
	    dumper_mom->duprefix, path, dumper_mom->durandsuffix);
  FILE *out = fopen (pathbuf, "w");
  if (!out)
    MOM_FATAPRINTF ("failed to open generated file %s: %m", pathbuf);
  return out;
}

static void
close_generated_file_dump_mom (FILE *fil, const char *path)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (strlen (path) < 128);
  assert (fil);
  if (fclose (fil))
    MOM_FATAPRINTF ("failed to close generated file %s: %m", path);
  char newpathbuf[256];
  memset (newpathbuf, 0, sizeof (newpathbuf));
  snprintf (newpathbuf, sizeof (newpathbuf), "%s%s%s",
	    dumper_mom->duprefix, path, dumper_mom->durandsuffix);
  char oldpathbuf[256];
  memset (oldpathbuf, 0, sizeof (oldpathbuf));
  snprintf (oldpathbuf, sizeof (oldpathbuf), "%s%s", dumper_mom->duprefix,
	    path);
  char backpathbuf[256];
  memset (backpathbuf, 0, sizeof (backpathbuf));
  snprintf (backpathbuf, sizeof (backpathbuf), "%s%s~", dumper_mom->duprefix,
	    path);
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
emit_predefined_header_mom (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  FILE *hdout = open_generated_file_dump_mom (MOM_PREDEFINED_PATH);
  mom_output_gplv3_notice (hdout, "///", "+++", MOM_PREDEFINED_PATH);
  fprintf (hdout, "#ifndef" " MOM_HAS_PREDEFINED_NAMED" "\n");
  fprintf (hdout, "#error missing " "MOM_HAS_PREDEFINED_NAMED" "\n");
  fprintf (hdout, "#endif" " /*MOM_HAS_PREDEFINED_NAMED*/" "\n");
  fprintf (hdout, "#ifndef" " MOM_HAS_PREDEFINED_ANONYMOUS" "\n");
  fprintf (hdout, "#error missing " "MOM_HAS_PREDEFINED_ANONYMOUS" "\n");
  fprintf (hdout, "#endif" " /*MOM_HAS_PREDEFINED_ANONYMOUS*/" "\n\n");
  const momseq_t *setpredef =
    mom_hashset_elements_set (dumper_mom->dupredefineditemset);
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
  close_generated_file_dump_mom (hdout, MOM_PREDEFINED_PATH);
}

////////////////
static void
emit_content_dumped_item_mom (const momitem_t *itm)
{				//// keep in sync with load_fill_item_mom
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (itm && itm != MOM_EMPTY);
  if (mom_attributes_count (itm->itm_attrs) > 0)
    {
      fputs ("{", dumper_mom->dufile);
      mom_emit_dump_indent ();
      const momseq_t *setat = mom_attributes_set (itm->itm_attrs, MOM_NONEV);
      if (setat)
	for (unsigned ix = 0; ix < setat->slen; ix++)
	  {
	    const momitem_t *itmat = setat->arritm[ix];
	    struct momentry_st *ent =
	      mom_attributes_find_entry (itm->itm_attrs, itmat);
	    if (!ent)
	      continue;
	    mom_emit_dumped_newline ();
	    fputs ("* ", dumper_mom->dufile);
	    mom_emit_dumped_itemref (itmat);
	    mom_emit_dumped_space ();
	    mom_emit_dumped_value (ent->ent_val);
	  };
      mom_emit_dumped_space ();
      mom_emit_dump_outdent ();
      fputs ("}", dumper_mom->dufile);
      mom_emit_dumped_newline ();
    }
  unsigned cnt = mom_components_count (itm->itm_comps);
  if (cnt > 0)
    {
      fputs ("[[", dumper_mom->dufile);
      mom_emit_dump_indent ();
      for (unsigned ix = 0; ix < cnt; ix++)
	{
	  mom_emit_dumped_space ();
	  mom_emit_dumped_value (mom_components_nth (itm->itm_comps,
						     (int) ix));
	}
      mom_emit_dumped_space ();
      mom_emit_dump_outdent ();
      fputs ("]]", dumper_mom->dufile);
      mom_emit_dumped_newline ();
    }
#warning emit_content_dumped_item_mom unimplemented, see load_fill_item_mom
}				/* end emit_content_dumped_item_mom */



////////////////
static void
emit_dumped_item_mom (const momitem_t *itm)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  if (!mom_hashset_contains (dumper_mom->duitemuserset, itm)
      && !mom_hashset_contains (dumper_mom->duitemglobalset, itm))
    return;
  putc ('\n', dumper_mom->dufile);
  fprintf (dumper_mom->dufile, "** %s\n", itm->itm_str->cstr);
  dumper_mom->duindentation = 0;
  dumper_mom->dulastnloff = ftell (dumper_mom->dufile);
  emit_content_dumped_item_mom (itm);
  fputs ("\n..\n\n", dumper_mom->dufile);
  dumper_mom->duindentation = 0;
  dumper_mom->dulastnloff = ftell (dumper_mom->dufile);
}

#define DUMP_INDENT_MAX_MOM 16
#define DUMP_WIDTH_MAX_MOM 72
void
mom_emit_dumped_newline (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  fputc ('\n', dumper_mom->dufile);
  for (int ix = dumper_mom->duindentation % DUMP_INDENT_MAX_MOM; ix >= 0;
       ix--)
    fputc (' ', dumper_mom->dufile);
  dumper_mom->dulastnloff = ftell (dumper_mom->dufile);
}

void
mom_emit_dumped_space (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  if (ftell (dumper_mom->dufile) - dumper_mom->dulastnloff <
      DUMP_WIDTH_MAX_MOM)
    fputc (' ', dumper_mom->dufile);
  else
    mom_emit_dumped_newline ();
}

bool
mom_emit_dumped_itemref (const momitem_t *itm)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  if (!itm || itm == MOM_EMPTY
      || (!mom_hashset_contains (dumper_mom->duitemuserset, itm)
	  && !mom_hashset_contains (dumper_mom->duitemglobalset, itm)))
    {
      fputs ("~", dumper_mom->dufile);
      return false;
    }
  else
    {
      assert (itm->itm_str);
      fprintf (dumper_mom->dufile, "%s", itm->itm_str->cstr);
      return true;
    }
}

void
mom_emit_dump_indent (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  dumper_mom->duindentation++;
}

void
mom_emit_dump_outdent (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  dumper_mom->duindentation++;
}

bool
mom_dumpable_item (const momitem_t *itm)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  if (!itm)
    return false;
  return (mom_hashset_contains (dumper_mom->duitemuserset, itm)
	  || mom_hashset_contains (dumper_mom->duitemglobalset, itm));
}

bool
mom_dumpable_value (const momvalue_t val)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  if (val.istransient)
    return false;
  switch ((enum momvaltype_en) val.typnum)
    {
    case momty_null:
      return false;
    case momty_double:
    case momty_int:
    case momty_delim:
    case momty_string:
    case momty_tuple:
    case momty_set:
      return true;
    case momty_item:
      return (mom_dumpable_item (val.vitem));
    case momty_node:
      {
	momnode_t *nod = val.vnode;
	assert (nod);
	return (mom_dumpable_item (nod->conn));
      }
    }
  return false;
}

void				// see also mom_load_value
mom_emit_dumped_value (const momvalue_t val)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  mom_emit_dumped_space ();
  if (val.istransient || !mom_dumpable_value (val))
    goto emit_null;
  switch ((enum momvaltype_en) val.typnum)
    {
    case momty_null:
    emit_null:
      fputs ("~", dumper_mom->dufile);
      return;
    case momty_double:
      {
	double x = val.vdbl;
	if (isnan (x))
	  fputs ("+NAN", dumper_mom->dufile);
	else if (isinf (x) > 0)
	  fputs ("+INF", dumper_mom->dufile);
	else if (isinf (x) < 0)
	  fputs ("-INF", dumper_mom->dufile);
	char fbuf[48];
	snprintf (fbuf, sizeof (fbuf), "%.5f", x);
	if (atof (fbuf) == x)
	  {
	    fputs (fbuf, dumper_mom->dufile);
	    return;
	  };
	snprintf (fbuf, sizeof (fbuf), "%.9f", x);
	if (atof (fbuf) == x)
	  {
	    fputs (fbuf, dumper_mom->dufile);
	    return;
	  };
	snprintf (fbuf, sizeof (fbuf), "%.15g", x);
	if (atof (fbuf) == x)
	  {
	    fputs (fbuf, dumper_mom->dufile);
	    return;
	  };
	fprintf (dumper_mom->dufile, "%a", x);
	break;
      }
    case momty_int:
      fprintf (dumper_mom->dufile, "%lld", (long long) val.vint);
      break;
    case momty_delim:
      {
	char dbuf[8 + sizeof (val.vdelim)];
	memset (dbuf, 0, sizeof (dbuf));
	memcpy (dbuf, &val.vdelim, sizeof (val.vdelim));
	fputs ("° \"", dumper_mom->dufile);
	mom_output_utf8cstr_cencoded (dumper_mom->dufile, dbuf, -1);
	fputs ("\"", dumper_mom->dufile);
      }
      break;
    case momty_string:
      fputs ("\"", dumper_mom->dufile);
      mom_output_utf8cstr_cencoded (dumper_mom->dufile, val.vstr->cstr, -1);
      fputs ("\"", dumper_mom->dufile);
      break;
    case momty_item:
      assert (val.vitem && val.vitem->itm_str);
      if (mom_hashset_contains (dumper_mom->duitemuserset, val.vitem)
	  || mom_hashset_contains (dumper_mom->duitemglobalset, val.vitem))
	fputs (val.vitem->itm_str->cstr, dumper_mom->dufile);
      else
	fputs ("~", dumper_mom->dufile);
      break;
    case momty_tuple:
      {
	momseq_t *tup = val.vtuple;
	bool something = false;
	assert (tup);
	fputs ("[", dumper_mom->dufile);
	mom_emit_dump_indent ();
	if (mom_dumpable_value (tup->meta))
	  {
	    mom_emit_dumped_space ();
	    fputs ("!", dumper_mom->dufile);
	    mom_emit_dumped_value (tup->meta);
	    something = true;
	  }
	unsigned len = tup->slen;
	for (unsigned ix = 0; ix < len; ix++)
	  {
	    if (mom_dumpable_item (tup->arritm[ix]))
	      {
		if (something)
		  mom_emit_dumped_space ();
		mom_emit_dumped_itemref (tup->arritm[ix]);
		something = true;
	      }
	  }
	mom_emit_dump_outdent ();
	fputs ("]", dumper_mom->dufile);
      }
      break;
    case momty_set:
      {
	momseq_t *tup = val.vtuple;
	bool something = false;
	assert (tup);
	fputs ("{", dumper_mom->dufile);
	mom_emit_dump_indent ();
	if (mom_dumpable_value (tup->meta))
	  {
	    mom_emit_dumped_space ();
	    fputs ("!", dumper_mom->dufile);
	    mom_emit_dumped_value (tup->meta);
	    something = true;
	  }
	unsigned len = tup->slen;
	for (unsigned ix = 0; ix < len; ix++)
	  {
	    if (mom_dumpable_item (tup->arritm[ix]))
	      {
		if (something)
		  mom_emit_dumped_space ();
		mom_emit_dumped_itemref (tup->arritm[ix]);
		something = true;
	      }
	  }
	mom_emit_dump_outdent ();
	fputs ("}", dumper_mom->dufile);
      }
      break;
    case momty_node:
      {
	momnode_t *nod = val.vnode;
	assert (nod);
	unsigned ln = nod->slen;
	if (!mom_dumpable_item (nod->conn))
	  goto emit_null;
	fputs ("^", dumper_mom->dufile);
	mom_emit_dumped_itemref (nod->conn);
	if (mom_dumpable_value (nod->meta))
	  {
	    mom_emit_dumped_space ();
	    fputs ("!", dumper_mom->dufile);
	    mom_emit_dumped_value (nod->meta);
	  }
	fputs ("(", dumper_mom->dufile);
	mom_emit_dump_indent ();
	for (unsigned ix = 0; ix < ln; ix++)
	  {
	    if (ix > 0)
	      mom_emit_dumped_space ();
	    mom_emit_dumped_value (nod->arrsons[ix]);
	  }
	mom_emit_dump_outdent ();
	fputs (")", dumper_mom->dufile);
      }
      break;
    }
}				/* end mom_emit_dumped_value */

static void
emit_global_items_mom (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  const momseq_t *set =
    mom_hashset_elements_set (dumper_mom->duitemglobalset);
  assert (set);
  unsigned nbel = set->slen;
  for (unsigned ix = 0; ix < nbel; ix++)
    emit_dumped_item_mom (set->arritm[ix]);
}

static void
emit_global_modules_mom (const momseq_t *setmod)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  if (!setmod)
    return;
  unsigned nbmod = setmod->slen;
  for (unsigned ix = 0; ix < nbmod; ix++)
    {
      const momitem_t *moditm = setmod->arritm[ix];
      assert (moditm && moditm != MOM_EMPTY);
      if (moditm->itm_space == momspa_predefined
	  || moditm->itm_space == momspa_global)
	fprintf (dumper_mom->dufile, "!! %s\n", moditm->itm_str->cstr);
    }
  if (nbmod > 0)
    fputc ('\n', dumper_mom->dufile);
}

static void
emit_user_items_mom (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  const momseq_t *set = mom_hashset_elements_set (dumper_mom->duitemuserset);
  if (!set)
    return;
  unsigned nbel = set->slen;
  for (unsigned ix = 0; ix < nbel; ix++)
    emit_dumped_item_mom (set->arritm[ix]);
}

static void
emit_user_modules_mom (const momseq_t *setmod)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  if (!setmod)
    return;
  unsigned nbmod = setmod->slen;
  for (unsigned ix = 0; ix < nbmod; ix++)
    {
      const momitem_t *moditm = setmod->arritm[ix];
      assert (moditm && moditm != MOM_EMPTY);
      if (moditm->itm_space == momspa_user)
	fprintf (dumper_mom->dufile, "!! %s\n", moditm->itm_str->cstr);
    }
  if (nbmod > 0)
    fputc ('\n', dumper_mom->dufile);
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
  dumper_mom = &dmp;
  scan_predefined_items_mom ();
  while (mom_queueitem_size (&dmp.duitemque) > 0)
    {
      const momitem_t *curitm = mom_queueitem_pop_front (&dmp.duitemque);
      scan_inside_dumped_item_mom ((momitem_t *) curitm);
    }
  emit_predefined_header_mom ();
  MOM_INFORMPRINTF ("dumped state to prefix %s : %u global + %u user items",
		    prefix, mom_hashset_count (dmp.duitemglobalset),
		    mom_hashset_count (dmp.duitemuserset));
  ////
  dmp.dustate = dump_emit;
  {
    dmp.dufile = open_generated_file_dump_mom (MOM_GLOBAL_DATA_PATH);
    mom_output_gplv3_notice (dmp.dufile, "///", "", MOM_GLOBAL_DATA_PATH);
    fputc ('\n', dmp.dufile);
    const momseq_t *setmod = mom_hashset_elements_set (dmp.duitemmoduleset);
    emit_global_modules_mom (setmod);
    emit_global_items_mom ();
    fprintf (dmp.dufile, "//// end of global file %s\n",
	     MOM_GLOBAL_DATA_PATH);
    close_generated_file_dump_mom (dmp.dufile, MOM_GLOBAL_DATA_PATH);
    dmp.dufile = NULL;
  }
  {
    dmp.dufile = open_generated_file_dump_mom (MOM_USER_DATA_PATH);
    mom_output_gplv3_notice (dmp.dufile, "///", "", MOM_USER_DATA_PATH);
    fputc ('\n', dmp.dufile);
    const momseq_t *setmod = mom_hashset_elements_set (dmp.duitemmoduleset);
    emit_user_modules_mom (setmod);
    emit_user_items_mom ();
    fprintf (dmp.dufile, "//// end of user file %s\n", MOM_USER_DATA_PATH);
    close_generated_file_dump_mom (dmp.dufile, MOM_USER_DATA_PATH);
    dmp.dufile = NULL;
  }
  memset (&dmp, 0, sizeof (dmp));
  dumper_mom = NULL;
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
