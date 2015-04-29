// file state.c - manage the persistent state load & dump

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

struct transformpair_mom_st
{
  const momitem_t *transp_itm;
  const momnode_t *transp_node;
};

struct transformvect_mom_st
{
  unsigned transf_size;		/* allocated size */
  unsigned transf_count;	/* used count */
  struct transformpair_mom_st transf_pairs[];	/* of length transf_size */
};

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
  struct transformvect_mom_st *ldtransfvect;
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
add_load_transformer_mom (const momitem_t *itm, const momvalue_t vtrans)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (itm && itm != MOM_EMPTY);
  assert (vtrans.typnum == momty_node);
  assert (vtrans.vnode != NULL);
  struct transformvect_mom_st *transvec = loader_mom->ldtransfvect;
  unsigned trcount = 0;
  if (MOM_UNLIKELY (!transvec))
    {
      unsigned newsiz = 32;
      struct transformvect_mom_st *newtrv =	//
	MOM_GC_ALLOC ("load transformer",
		      sizeof (struct transformvect_mom_st)
		      + newsiz * sizeof (struct transformpair_mom_st));
      newtrv->transf_size = newsiz;
      loader_mom->ldtransfvect = transvec = newtrv;
    }
  else if ((trcount = transvec->transf_count),
	   MOM_UNLIKELY (trcount + 1 >= transvec->transf_size))
    {
      unsigned newsiz = ((5 * trcount / 4 + 30) | 0x1f) + 1;
      assert (newsiz + 1 > trcount);
      struct transformvect_mom_st *newtrv =	//
	MOM_GC_ALLOC ("load transformer",
		      sizeof (struct transformvect_mom_st)
		      + newsiz * sizeof (struct transformpair_mom_st));
      newtrv->transf_size = newsiz;
      memcpy (newtrv->transf_pairs, transvec->transf_pairs,
	      trcount * sizeof (struct transformpair_mom_st));
      newtrv->transf_count = trcount;
      MOM_GC_FREE (transvec,
		   sizeof (struct transformvect_mom_st)
		   +
		   transvec->transf_size *
		   sizeof (struct transformpair_mom_st));
      loader_mom->ldtransfvect = transvec = newtrv;
    }
  assert (trcount < transvec->transf_size);
  transvec->transf_pairs[trcount].transp_itm = itm;
  transvec->transf_pairs[trcount].transp_node = vtrans.vnode;
  transvec->transf_count = trcount + 1;
}

const char *
load_position_mom (char *buf, size_t siz, int lineno)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (buf)
    {
      snprintf (buf, siz, "%s file %s line %d",	//
		loader_mom->ldforglobals ? "global" : "user", loader_mom->ldforglobals	//
		? loader_mom->ldglobalpath	//
		: loader_mom->lduserpath,
		(lineno > 0) ? lineno : (int) loader_mom->ldlinecount);
      return buf;
    }
  else
    {
      char lbuf[192];
      memset (lbuf, 0, sizeof (lbuf));
      snprintf (lbuf, sizeof (lbuf), "%s file %s line %d",	//
		loader_mom->ldforglobals ? "global" : "user", loader_mom->ldforglobals	//
		? loader_mom->ldglobalpath	//
		: loader_mom->lduserpath,
		(lineno > 0) ? lineno : (int) loader_mom->ldlinecount);
      return MOM_GC_STRDUP ("location message", lbuf);
    }
}

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



const momitem_t *
mom_load_itemref_at (const char *fil, int lin)
{
  momvalue_t valtok = MOM_NONEV;
  MOM_DEBUGPRINTF (load, "load_itemref@%s:%d start", fil, lin);
  valtok = mom_peek_token_load_at (fil, lin);
  if (valtok.typnum == momty_item)
    {
      const momitem_t *itm = valtok.vitem;
      assert (itm);
      (void) mom_token_load_at (&valtok, fil, lin);
      return itm;
    }
  else if (mom_value_is_delim (valtok, "_*"))
    {
      (void) mom_token_load_at (&valtok, fil, lin);
      return mom_load_new_anonymous_item (true);
    }
  else if (mom_value_is_delim (valtok, "_:"))
    {
      (void) mom_token_load_at (&valtok, fil, lin);
      return mom_load_new_anonymous_item (false);
    }
  return NULL;
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
      const momstring_t *mstr = mom_make_string_sprintf ("modules/momg_%s.so",
							 moditm->itm_str->
							 cstr);
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
  pval->vstr = mom_make_string_cstr (buf);
  return true;
}


const char *const delim_mom[] = {
  /// first the 2 bytes delimiters; notice that degree-sign °, section-sign §, are two UTF-8 bytes
  "==",
  "**", "++", "--", "[[", "]]", "..", "_*", "_:", "°", "§", "*", "(", ")",
  "[", "]",
  "{", "}", "<", ">", "^", "!", "%", "@", "|", "&",
  NULL
};

momvalue_t
mom_peek_token_load_at (const char *fil, int lin)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  MOM_DEBUGPRINTF (load, "peek_token_load@%s:%d: token-queue-size %lu",
		   fil, lin, mom_queuevalue_size (&loader_mom->ldquetokens));
  if (0 == mom_queuevalue_size (&loader_mom->ldquetokens))
    {
      momvalue_t valtoken = MOM_NONEV;
      if (mom_token_load_at (&valtoken, fil, lin))
	{
	  mom_load_push_front_token (valtoken);
	  MOM_DEBUGPRINTF (load,
			   "peek_token_load@%s:%d: pushed parsed token %s",
			   fil, lin, mom_output_gcstring (valtoken));
	  return valtoken;
	}
      return MOM_NONEV;
    }
  else
    {
      momvalue_t val = mom_queuevalue_peek_front (&loader_mom->ldquetokens);
      MOM_DEBUGPRINTF (load, "peek_token_load@%s:%d: queued token %s",
		       fil, lin, mom_output_gcstring (val));
      return val;
    }

}

bool
mom_token_load_at (momvalue_t *pval, const char *fil, int lin)
{
  char locbuf[128];
  memset (locbuf, 0, sizeof (locbuf));
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (pval != NULL);
  memset (pval, 0, sizeof (momvalue_t));
  if (mom_queuevalue_size (&loader_mom->ldquetokens))
    {
      MOM_DEBUGPRINTF (load, "token_load@%s:%d: popping queued token", fil,
		       lin);
      *pval = mom_queuevalue_pop_front (&loader_mom->ldquetokens);
      MOM_DEBUGPRINTF (load, "token_load@%s:%d: got popped token %s near %s",
		       fil, lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return true;
    }
readagain:
  memset (locbuf, 0, sizeof (locbuf));
  if (!loader_mom->ldlinebuf
      || loader_mom->ldlinecol >= loader_mom->ldlinelen)
    {
      if (loader_mom->ldlinebuf)
	memset (loader_mom->ldlinebuf, 0, loader_mom->ldlinesize);
      loader_mom->ldlinelen =
	getline (&loader_mom->ldlinebuf, &loader_mom->ldlinesize,
		 loader_mom->ldforglobals ? loader_mom->
		 ldglobalfile : loader_mom->lduserfile);
      if (loader_mom->ldlinelen <= 0)
	{
	  MOM_DEBUGPRINTF (load, "token_load@%s:%d: got EOF at %s", fil, lin,
			   load_position_mom (locbuf, sizeof (locbuf), 0));
	  return false;
	}
      loader_mom->ldlinecol = 0;
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
  if (pstart[0] == '/' && pstart[1] == '/')
    {
      loader_mom->ldlinecol = loader_mom->ldlinelen;
      goto readagain;
    }
  else if (pstart[0] == '/' && pstart[1] == '*')
    {
      char *endcomm = strstr (pstart + 2, "*/");
      if (!endcomm)
	MOM_FATAPRINTF
	  ("unterminated /* single-line comment in %s",
	   load_position_mom (locbuf, sizeof (locbuf), 0));
      loader_mom->ldlinecol += (endcomm + 2 - pstart);
      goto readagain;
    }
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
      MOM_DEBUGPRINTF (load, "token_load@%s:%d: got number token %s at %s",
		       fil, lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return true;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (loader_mom->ldlinebuf + loader_mom->ldlinecol + 1,
			    "NAN", 3))
    {
      pval->typnum = momty_double;
      pval->vdbl = NAN;
      loader_mom->ldlinecol += 4;
      MOM_DEBUGPRINTF (load,
		       "token_load@%s:%d: got number NAN token %s at %s", fil,
		       lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return true;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (loader_mom->ldlinebuf + loader_mom->ldlinecol + 1,
			    "INF", 3))
    {
      pval->typnum = momty_double;
      pval->vdbl = INFINITY;
      loader_mom->ldlinecol += 4;
      MOM_DEBUGPRINTF (load,
		       "token_load@%s:%d: got number INF token %s at %s", fil,
		       lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return true;
    }
  else if (c == '"')
    {
      bool res = token_string_load_mom (pval);
      MOM_DEBUGPRINTF (load, "token_load@%s:%d: got string token %s at %s",
		       fil, lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return res;
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
	      loader_mom->ldlinecol += strlen (delim_mom[ix]);
	      MOM_DEBUGPRINTF (load,
			       "token_load@%s:%d: got delim token %s at %s",
			       fil, lin, mom_output_gcstring (*pval),
			       load_position_mom (locbuf, sizeof (locbuf),
						  0));
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
	  assert (itm->itm_str);
	  MOM_DEBUGPRINTF (load, "token_load@%s:%d: found item %s",
			   fil, lin, mom_item_cstring (itm));
	  pval->vitem = (momitem_t *) itm;
	  pval->typnum = momty_item;
	  loader_mom->ldlinecol += end - pstart;
	  MOM_DEBUGPRINTF (load,
			   "token_load@%s:%d: got anon-item token %s at %s",
			   fil, lin, mom_output_gcstring (*pval),
			   load_position_mom (locbuf, sizeof (locbuf), 0));
	  *end = olde;
	  return true;
	}
      else
	MOM_DEBUGPRINTF (load,
			 "token_load@%s:%d: did not found anon %s at %s", fil,
			 lin, pstart, load_position_mom (locbuf,
							 sizeof (locbuf), 0));
    }
  else if (c == '_' && pstart[1] == '*')
    {
      // _* is a delimiter to make a global new anonymous item
      pval->typnum = momty_delim;
      strcpy (pval->vdelim.delim, "_*");
      loader_mom->ldlinecol += 2;
      MOM_DEBUGPRINTF (load,
		       "token_load@%s:%d: got quasidelim token %s at %s", fil,
		       lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return true;
    }
  else if (c == '_' && pstart[1] == ':')
    {
      // _: is a delimiter to make a user new anonymous item
      pval->typnum = momty_delim;
      strcpy (pval->vdelim.delim, "_:");
      loader_mom->ldlinecol += 2;
      MOM_DEBUGPRINTF (load,
		       "token_load@%s:%d: got quasidelim token %s at %s", fil,
		       lin, mom_output_gcstring (*pval),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return true;
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
	  MOM_DEBUGPRINTF (load,
			   "token_load@%s:%d: got named-item token %s at %s",
			   fil, lin, mom_output_gcstring (*pval),
			   load_position_mom (locbuf, sizeof (locbuf), 0));
	  *end = olde;
	  return true;
	}
      else
	MOM_DEBUGPRINTF (load,
			 "token_load@%s:%d: did not found named %s at %s",
			 fil, lin, pstart, load_position_mom (locbuf,
							      sizeof (locbuf),
							      0));
    }

  MOM_DEBUGPRINTF (load,
		   "token_load@%s:%d: failing  linecol %d linebuf %s at %s",
		   fil, lin,
		   (int) loader_mom->ldlinecol, loader_mom->ldlinebuf,
		   load_position_mom (locbuf, sizeof (locbuf), 0));
  return false;
}


////////////////
void
load_fill_item_mom (momitem_t *itm)
{				// keep in sync with emit_content_dumped_item_mom
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (itm && itm->itm_str);
  MOM_DEBUGPRINTF (load, "start load_fill_item %s", mom_item_cstring (itm));
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
	    {
	      MOM_DEBUGPRINTF (load, "load_fill_item %s itmat=%s vat=%s",
			       mom_item_cstring (itm), itmat->itm_str->cstr,
			       mom_output_gcstring (vat));
	      itm->itm_attrs =
		mom_attributes_put (itm->itm_attrs, itmat, &vat);
	    }
	}
      if (!mom_value_is_delim (vtokbis, "}"))
	MOM_FATAPRINTF ("expecting } but got %s to end attributes of item %s"
			" in %s file %s line %d",
			mom_output_gcstring (vtokbis),
			mom_item_cstring (itm),
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath,
			(int) loader_mom->ldlinecount);
    }
  // should load the components
  vtok = MOM_NONEV;
  if (mom_token_load (&vtok) && mom_value_is_delim (vtok, "[["))
    {
      momvalue_t valcomp = MOM_NONEV;
      while ((valcomp = MOM_NONEV), mom_load_value (&valcomp))
	{
	  MOM_DEBUGPRINTF (load, "load_fill_item %s valcomp %s",
			   mom_item_cstring (itm),
			   mom_output_gcstring (valcomp));
	  itm->itm_comps = mom_components_append1 (itm->itm_comps, valcomp);
	}
      int lineno = loader_mom->ldlinecount;
      if (!((vtokbis = MOM_NONEV), !mom_token_load (&vtokbis))
	  || !mom_value_is_delim (vtokbis, "]]"))
	MOM_FATAPRINTF ("expecting ]] but got %s to end attributes of item %s"
			" in %s",
			mom_output_gcstring (vtokbis),
			mom_item_cstring (itm), load_position_mom (NULL, 0,
								   lineno));
    }
  // should load the transformer closure, if given
  vtok = MOM_NONEV;
  if (mom_token_load (&vtok) && mom_value_is_delim (vtok, "%"))
    {
      momvalue_t valtransf = MOM_NONEV;
      int lineno = loader_mom->ldlinecount;
      if (!mom_load_value (&valtransf))
	MOM_FATAPRINTF
	  ("missing transformer for item %s in %s",
	   mom_item_cstring (itm), load_position_mom (NULL, 0, lineno));
      if (valtransf.typnum != momty_node)
	MOM_FATAPRINTF
	  ("bad transformer %s for item %s in %s",
	   mom_output_gcstring (valtransf), mom_item_cstring (itm),
	   load_position_mom (NULL, 0, lineno));
      add_load_transformer_mom (itm, valtransf);
    }
}				/* end load_fill_item_mom */

////////////////
const momitem_t *
mom_load_new_anonymous_item (bool global)
{
  momitem_t *newitm = mom_make_anonymous_item ();
  MOM_DEBUGPRINTF (load, "load_new_anonymous_item %s newitm=%s",
		   global ? "global" : "user", mom_item_cstring (newitm));
  if (global)
    newitm->itm_space = momspa_global;
  else
    newitm->itm_space = momspa_user;
  momvalue_t vtok = MOM_NONEV;
  if (!mom_token_load (&vtok))
    MOM_FATAPRINTF ("failed to load new anonymous item in %s",
		    load_position_mom (NULL, 0, 0));
  if (vtok.typnum == momty_string)
    {
      mom_item_unsync_put_attribute (newitm, MOM_PREDEFINED_NAMED (comment),
				     vtok);
    }
#warning mom_load_new_anonymous_item incomplete
  return newitm;
}

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
  rewind (loader_mom->ldforglobals ? loader_mom->ldglobalfile : loader_mom->
	  lduserfile);
  loader_mom->ldlinecol = loader_mom->ldlinelen = loader_mom->ldlinecount = 0;
  do
    {
      if (loader_mom->ldlinebuf)
	memset (loader_mom->ldlinebuf, 0, loader_mom->ldlinesize);
      loader_mom->ldlinelen =
	getline (&loader_mom->ldlinebuf, &loader_mom->ldlinesize,
		 loader_mom->ldforglobals ? loader_mom->
		 ldglobalfile : loader_mom->lduserfile);
      if (loader_mom->ldlinelen <= 0)
	return;
      loader_mom->ldlinecol = 0;
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
	    MOM_FATAPRINTF ("invalid line %d '%s' of %s",
			    (int) loader_mom->ldlinecount,
			    loader_mom->ldlinebuf, load_position_mom (NULL, 0,
								      0));
	  MOM_DEBUGPRINTF (load, "second %s pass filling item %s",
			   loader_mom->ldforglobals ? "global" : "user",
			   val.vitem->itm_str->cstr);
	  memset (&loader_mom->ldquetokens, 0,
		  sizeof (loader_mom->ldquetokens));
	  load_fill_item_mom (val.vitem);
	  MOM_DEBUGPRINTF (load, "second %s pass filled item %s\n",
			   loader_mom->ldforglobals ? "global" : "user",
			   val.vitem->itm_str->cstr);
	}
    }
  while (!feof
	 (loader_mom->ldforglobals ? loader_mom->ldglobalfile : loader_mom->
	  lduserfile));
}

void
mom_load_push_front_token (momvalue_t valtok)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  MOM_DEBUGPRINTF (load, "load_push_front_token %s, queued %d",
		   mom_output_gcstring (valtok),
		   mom_load_nb_queued_tokens ());
  if (valtok.typnum != momty_null)
    mom_queuevalue_push_front (&loader_mom->ldquetokens, valtok);
}


void
mom_load_push_back_token (momvalue_t valtok)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  MOM_DEBUGPRINTF (load, "load_push_back_token %s, queued %d",
		   mom_output_gcstring (valtok),
		   mom_load_nb_queued_tokens ());
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
load_metavalue_mom (momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (!pval)
    return false;
  momvalue_t vtok = MOM_NONEV;
  vtok = mom_peek_token_load ();
  if (mom_value_is_delim (vtok, "!"))
    {
      mom_token_load (&vtok);
      return mom_load_value (pval);
    }
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
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath, linecnt);
    }
  if (mom_value_is_delim (vtok, "["))
    {				////tuples
      int linecnt = loader_mom->ldlinecount;
      if (!mom_token_load (&vtokbis))
	MOM_FATAPRINTF ("missing tuple content after ["
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath, linecnt);
      momvalue_t metav = MOM_NONEV;
      const momitem_t *curitm = NULL;
      struct momqueueitems_st quitems;
      memset (&quitems, 0, sizeof (quitems));
      load_metavalue_mom (&metav);
      while ((curitm = mom_load_itemref ()) != NULL)
	{
	  mom_queueitem_push_back (&quitems, curitm);
	  linecnt = loader_mom->ldlinecount;
	}
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, "]"))
	MOM_FATAPRINTF ("missing ] after tuple content"
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath, linecnt);
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
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath, linecnt);
      momvalue_t metav = MOM_NONEV;
      const momitem_t *curitm = NULL;
      struct momqueueitems_st quitems;
      memset (&quitems, 0, sizeof (quitems));
      load_metavalue_mom (&metav);
      while ((curitm = mom_load_itemref ()) != NULL)
	{
	  mom_queueitem_push_back (&quitems, curitm);
	  linecnt = loader_mom->ldlinecount;
	}
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, "}"))
	MOM_FATAPRINTF ("missing } after set content"
			" in %s file %s line %d",
			loader_mom->ldforglobals ? "global" : "user",
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath, linecnt);
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
      if (!connitm)
	MOM_FATAPRINTF ("missing connective item after ^ "
			" in %s", load_position_mom (NULL, 0, linecnt));
      load_metavalue_mom (&metav);
      linecnt = loader_mom->ldlinecount;
      if (!mom_token_load (&vtokter) || !mom_value_is_delim (vtokter, "("))
	MOM_FATAPRINTF ("missing ( -got %s- in loaded node "
			" in %s",
			mom_output_gcstring (vtokter),
			load_position_mom (NULL, 0, linecnt));
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
			loader_mom->ldforglobals ? loader_mom->
			ldglobalpath : loader_mom->lduserpath, linecnt);
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
  /// execute the transformations by applying each transformer to the
  /// corresponding item value
  unsigned nbtransf = 0;
  if (ldr.ldtransfvect && (nbtransf = ldr.ldtransfvect->transf_count) > 0)
    {
      MOM_INFORMPRINTF
	("loaded %d items are needing %d transformations",
	 (int) mom_hashset_count (ldr.lditemset), (int) nbtransf);
      for (unsigned ix = 0; ix < nbtransf; ix++)
	{
	  momitem_t *itmtr =
	    (momitem_t *) ldr.ldtransfvect->transf_pairs[ix].transp_itm;
	  const momnode_t *trnode =
	    ldr.ldtransfvect->transf_pairs[ix].transp_node;
	  momvalue_t vnode = mom_nodev (trnode);
	  momvalue_t vitmtr = mom_itemv (itmtr);
	  MOM_DEBUGPRINTF (load, "transforming item#%d: %s with %s",
			   ix, itmtr->itm_str->cstr,
			   mom_output_gcstring (vnode));
	  if (!mom_applyval_1val_to_void (vnode, vitmtr))
	    MOM_FATAPRINTF ("failed to transform item#%d: %s with %s",
			    ix, itmtr->itm_str->cstr,
			    mom_output_gcstring (vnode));
	}
    }
  MOM_INFORMPRINTF
    ("loaded %d items and %d modules with %d transforms from global %s and user %s files",
     (int) mom_hashset_count (ldr.lditemset),
     (int) mom_hashset_count (ldr.ldmoduleset), nbtransf, ldr.ldglobalpath,
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
  struct momattributes_st *dukindscannermap;
  struct momattributes_st *dukindemittermap;
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
    MOM_FATAPRINTF ("scan dumped value outside of dumping");
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
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan predefined items outside of dumping");
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
  momitem_t *itmkd = (momitem_t *) itm->itm_kind;
  if (itmkd && itmkd != MOM_EMPTY)
    {
      momvalue_t valscanner = MOM_NONEV;
      mom_scan_dumped_item (itmkd);
      struct momentry_st *ent =	//
	mom_attributes_find_entry (dumper_mom->dukindscannermap, itmkd);
      if (ent)
	valscanner = ent->ent_val;
      if (valscanner.typnum == momty_null)
	{
	  valscanner =		//
	    mom_item_unsync_get_attribute (itmkd,
					   MOM_PREDEFINED_NAMED
					   (dumped_item_scanner));
	  if (valscanner.typnum == momty_null)
	    return;
	  if (valscanner.typnum != momty_node)
	    MOM_FATAPRINTF
	      ("the `dumped_item_scanner` attribute of kind %s is a non-node value %s",
	       itmkd->itm_str->cstr, mom_output_gcstring (valscanner));
	  dumper_mom->dukindscannermap =	//
	    mom_attributes_put (dumper_mom->dukindscannermap, itmkd,
				&valscanner);
	}
      if (!mom_applyval_1val_to_void (valscanner, mom_itemv (itm)))
	MOM_FATAPRINTF ("failed to apply scanner %s to item %s of kind %s",
			mom_output_gcstring (valscanner),
			mom_item_cstring (itm), itmkd->itm_str->cstr);
    }
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
  momitem_t *itmkd = (momitem_t *) itm->itm_kind;
  if (itmkd && mom_dumpable_item (itmkd))
    {
      momvalue_t valemitter = MOM_NONEV;
      struct momentry_st *ent =	//
	mom_attributes_find_entry (dumper_mom->dukindemittermap, itmkd);
      if (ent)
	valemitter = ent->ent_val;
      if (valemitter.typnum == momty_null)
	{
	  valemitter =		//
	    mom_item_unsync_get_attribute (itmkd,
					   MOM_PREDEFINED_NAMED
					   (dumped_item_emitter));
	  if (valemitter.typnum == momty_null)
	    return;
	  if (valemitter.typnum != momty_node)
	    MOM_FATAPRINTF
	      ("the `dumped_item_emitter` attribute of kind %s is a non-node value %s",
	       itmkd->itm_str->cstr, mom_output_gcstring (valemitter));
	  dumper_mom->dukindemittermap =	//
	    mom_attributes_put (dumper_mom->dukindemittermap, itmkd,
				&valemitter);
	}
      /// we should apply the emitter to get a transformer node value, which
      /// would be later applied at load time....
      momvalue_t valtransformer = MOM_NONEV;
      if (mom_applyval_1val_to_val
	  (valemitter, mom_itemv (itm), &valtransformer)
	  && valtransformer.typnum == momty_node)
	{
	  fputs ("% ", dumper_mom->dufile);
	  mom_emit_dumped_value (valtransformer);
	  mom_emit_dumped_newline ();
	}
    }
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
  fprintf (dumper_mom->dufile, "** %s\n", mom_item_cstring (itm));
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
  dumper_mom->dulastnloff = ftell (dumper_mom->dufile);
  for (int ix = dumper_mom->duindentation % DUMP_INDENT_MAX_MOM; ix >= 0;
       ix--)
    fputc (' ', dumper_mom->dufile);
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
      fprintf (dumper_mom->dufile, "%s", mom_item_cstring (itm));
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
  if (!itm || itm == MOM_EMPTY)
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


#define VALOUT_MAGIC_MOM 886023773	/* valout_magic_mom 0x34cfa65d */
struct momvaloutput_st
{
  unsigned vout_magic;		/* always VALOUT_MAGIC_MOM */
  char *vout_buffer;
  size_t vout_size;
  FILE *vout_file;
  long vout_lastnl;
  int vout_indentation;
};

static void output_val_mom (struct momvaloutput_st *ov, const momvalue_t val);

const char *
mom_output_gcstring (const momvalue_t val)
{
  struct momvaloutput_st vout;
  memset (&vout, 0, sizeof (vout));
  unsigned bufsiz = 256;
  vout.vout_buffer = malloc (bufsiz);
  if (MOM_UNLIKELY (vout.vout_buffer == NULL))
    MOM_FATAPRINTF
      ("failed to allocate internal string buffer of %u for output string",
       bufsiz);
  memset (vout.vout_buffer, 0, bufsiz);
  vout.vout_size = bufsiz;
  vout.vout_file = open_memstream (&vout.vout_buffer, &vout.vout_size);
  if (MOM_UNLIKELY (!vout.vout_file))
    MOM_FATAPRINTF ("failed to open_memstream a buffer @%p of %u bytes : %m",
		    vout.vout_buffer, bufsiz);
  vout.vout_magic = VALOUT_MAGIC_MOM;
  output_val_mom (&vout, val);
  if (ftell (vout.vout_file) - vout.vout_lastnl > DUMP_WIDTH_MAX_MOM / 2)
    fputc ('\n', vout.vout_file);
  fflush (vout.vout_file);
  const char *res = MOM_GC_STRDUP ("output gcstring", vout.vout_buffer);
  free (vout.vout_buffer), vout.vout_buffer = NULL;
  memset (&vout, 0, sizeof (vout));
  return res;
}

const momstring_t *
mout_output_string (const momvalue_t val)
{
  struct momvaloutput_st vout;
  memset (&vout, 0, sizeof (vout));
  unsigned bufsiz = 256;
  vout.vout_buffer = malloc (bufsiz);
  if (MOM_UNLIKELY (vout.vout_buffer != NULL))
    MOM_FATAPRINTF
      ("failed to allocate internal string buffer of %u for output string",
       bufsiz);
  memset (vout.vout_buffer, 0, bufsiz);
  vout.vout_size = bufsiz;
  vout.vout_file = open_memstream (&vout.vout_buffer, &vout.vout_size);
  if (MOM_UNLIKELY (!vout.vout_file))
    MOM_FATAPRINTF ("failed to open_memstream a buffer @%p of %u bytes : %m",
		    vout.vout_buffer, bufsiz);
  vout.vout_magic = VALOUT_MAGIC_MOM;
  output_val_mom (&vout, val);
  if (ftell (vout.vout_file) - vout.vout_lastnl > DUMP_WIDTH_MAX_MOM / 2)
    fputc ('\n', vout.vout_file);
  fflush (vout.vout_file);
  const momstring_t *res = mom_make_string_cstr (vout.vout_buffer);
  free (vout.vout_buffer), vout.vout_buffer = NULL;
  memset (&vout, 0, sizeof (vout));
  return res;
}


static void
output_indent_mom (struct momvaloutput_st *ov)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  ov->vout_indentation++;
}

static void
output_outdent_mom (struct momvaloutput_st *ov)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  ov->vout_indentation--;
  assert (ov->vout_indentation >= 0);
}

static void
output_newline_mom (struct momvaloutput_st *ov)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  fputc ('\n', ov->vout_file);
  ov->vout_lastnl = ftell (ov->vout_file);
  for (int ix = ov->vout_indentation % DUMP_INDENT_MAX_MOM; ix >= 0; ix--)
    fputc (' ', ov->vout_file);
}

static void
output_space_mom (struct momvaloutput_st *ov)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  if (ftell (ov->vout_file) - ov->vout_lastnl < DUMP_WIDTH_MAX_MOM)
    fputc (' ', ov->vout_file);
  else
    output_newline_mom (ov);
}

static void
output_item_mom (struct momvaloutput_st *ov, const momitem_t *itm)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  if (!itm || itm == MOM_EMPTY)
    return;
  assert (itm->itm_str);
  fputs (mom_item_cstring (itm), ov->vout_file);
  if (itm->itm_anonymous)
    {
      momvalue_t vcomm = MOM_NONEV;
      mom_item_lock ((momitem_t *) itm);
      vcomm =
	mom_item_unsync_get_attribute ((momitem_t *) itm,
				       MOM_PREDEFINED_NAMED (comment));
      mom_item_unlock ((momitem_t *) itm);
      const char *comstr = mom_value_cstr (vcomm);
      if (comstr && comstr[0] && comstr[1] && comstr[2] && comstr[3])
	{
	  char combuf[80];
	  memset (combuf, 0, sizeof (combuf));
	  const char *nl = strchr (comstr, '\n');
	  const char *eoc = strstr (comstr, "*/");
	  if (!nl && !eoc)
	    strncpy (combuf, comstr, sizeof (combuf) - 1);
	  else if (nl && eoc && nl >= comstr + 3 && eoc >= comstr + 3)
	    {
	      int nbc = (nl > eoc) ? (comstr - nl - 1) : (comstr - eoc - 1);
	      if (nbc >= (int) sizeof (combuf))
		nbc = sizeof (combuf) - 1;
	      strncpy (combuf, comstr, nbc);
	    }
	  else if (nl && nl >= comstr + 3)
	    {
	      int nbc = (comstr - nl - 1);
	      if (nbc >= (int) sizeof (combuf))
		nbc = sizeof (combuf) - 1;
	      strncpy (combuf, comstr, nbc);
	    }
	  else if (eoc && eoc >= comstr + 3)
	    {
	      int nbc = (comstr - eoc - 1);
	      if (nbc >= (int) sizeof (combuf))
		nbc = sizeof (combuf) - 1;
	      strncpy (combuf, comstr, nbc);
	    }
	  if (strlen (combuf) > 4)
	    {
	      assert (!strchr (combuf, '\n'));
	      assert (!strstr (combuf, "*/"));
	      fprintf (ov->vout_file, "/*%s*/", combuf);
	      output_space_mom (ov);
	    }
	}
    }
}


/// similar to mom_emit_dump_value, except that we don't care about
/// non-dumpable items, etc...
static void
output_val_mom (struct momvaloutput_st *ov, const momvalue_t val)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  switch ((enum momvaltype_en) val.typnum)
    {
    case momty_null:
      fputs ("~", ov->vout_file);
      return;
    case momty_double:
      {
	double x = val.vdbl;
	if (isnan (x))
	  fputs ("+NAN", ov->vout_file);
	else if (isinf (x) > 0)
	  fputs ("+INF", ov->vout_file);
	else if (isinf (x) < 0)
	  fputs ("-INF", ov->vout_file);
	char fbuf[48];
	snprintf (fbuf, sizeof (fbuf), "%.5f", x);
	if (atof (fbuf) == x)
	  {
	    fputs (fbuf, ov->vout_file);
	    return;
	  };
	snprintf (fbuf, sizeof (fbuf), "%.9f", x);
	if (atof (fbuf) == x)
	  {
	    fputs (fbuf, ov->vout_file);
	    return;
	  };
	snprintf (fbuf, sizeof (fbuf), "%.15g", x);
	if (atof (fbuf) == x)
	  {
	    fputs (fbuf, ov->vout_file);
	    return;
	  };
	fprintf (ov->vout_file, "%a", x);
	break;
      }
    case momty_int:
      fprintf (ov->vout_file, "%lld", (long long) val.vint);
      break;
    case momty_delim:
      {
	char dbuf[8 + sizeof (val.vdelim)];
	memset (dbuf, 0, sizeof (dbuf));
	strncpy (dbuf, val.vdelim.delim, sizeof (val.vdelim));
	fputs ("° \"", ov->vout_file);
	mom_output_utf8cstr_cencoded (ov->vout_file, dbuf, -1);
	fputs ("\"", ov->vout_file);
      }
      break;
    case momty_string:
      fputs ("\"", ov->vout_file);
      mom_output_utf8cstr_cencoded (ov->vout_file, val.vstr->cstr, -1);
      fputs ("\"", ov->vout_file);
      break;
    case momty_item:
      assert (val.vitem);
      output_item_mom (ov, val.vitem);
      break;
    case momty_tuple:
      {
	momseq_t *tup = val.vtuple;
	bool something = false;
	assert (tup);
	fputs ("[", ov->vout_file);
	output_indent_mom (ov);
	if (tup->meta.typnum != momty_null)
	  {
	    output_space_mom (ov);
	    fputs ("!", ov->vout_file);
	    output_val_mom (ov, tup->meta);
	    something = true;
	  }
	unsigned len = tup->slen;
	for (unsigned ix = 0; ix < len; ix++)
	  {
	    if (something)
	      output_space_mom (ov);
	    output_item_mom (ov, tup->arritm[ix]);
	    something = true;
	  }
	output_outdent_mom (ov);
	fputs ("]", ov->vout_file);
      }
      break;
    case momty_set:
      {
	momseq_t *tup = val.vtuple;
	bool something = false;
	assert (tup);
	fputs ("{", ov->vout_file);
	output_indent_mom (ov);
	if (tup->meta.typnum != momty_null)
	  {
	    output_space_mom (ov);
	    fputs ("!", ov->vout_file);
	    output_val_mom (ov, tup->meta);
	    something = true;
	  }
	unsigned len = tup->slen;
	for (unsigned ix = 0; ix < len; ix++)
	  {
	    if (something)
	      output_space_mom (ov);
	    output_item_mom (ov, tup->arritm[ix]);
	    something = true;
	  }
	output_outdent_mom (ov);
	fputs ("}", ov->vout_file);
      }
      break;
    case momty_node:
      {
	momnode_t *nod = val.vnode;
	assert (nod);
	unsigned ln = nod->slen;
	fputs ("^", ov->vout_file);
	output_item_mom (ov, nod->conn);
	if (nod->meta.typnum != momty_null)
	  {
	    output_space_mom (ov);
	    fputs ("!", ov->vout_file);
	    output_val_mom (ov, nod->meta);
	  }
	fputs ("(", ov->vout_file);
	output_indent_mom (ov);
	for (unsigned ix = 0; ix < ln; ix++)
	  {
	    if (ix > 0)
	      output_space_mom (ov);
	    output_val_mom (ov, nod->arrsons[ix]);
	  }
	output_outdent_mom (ov);
	fputs (")", ov->vout_file);
      }
      break;
    }
}				/* end output_val_mom */


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
	strncpy (dbuf, val.vdelim.delim, sizeof (val.vdelim));
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
	fprintf (dumper_mom->dufile, "!! %s\n", mom_item_cstring (moditm));
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
	fprintf (dumper_mom->dufile, "!! %s\n", mom_item_cstring (moditm));
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
