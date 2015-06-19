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

struct loaderaliasent_st
{
  const momstring_t *al_name;
  momitem_t *al_itm;
};

#define LOADER_MAGIC_MOM 0x169128bb
struct momloader_st
{
  unsigned ldmagic;		/* always LOADER_MAGIC_MOM */
  double ldstartelapsedtime;
  double ldstartcputime;
  const char *ldcurpath;
  FILE *ldcurfile;
  struct loaderaliasent_st *ldaliasentarr;	// of ldaliaslen size
  unsigned ldaliaslen;
  unsigned ldaliascount;
  struct momhashset_st *lditemset;
  struct momhashset_st *ldmoduleset;
  struct transformvect_mom_st *ldtransfvect;
  struct momqueuevalues_st ldquetokens;
  char *ldlinebuf;
  size_t ldlinesize;
  size_t ldlinelen;
  size_t ldlinecol;
  size_t ldlinecount;
  long ldlineoff;
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
      snprintf (buf, siz, "file %s line %d",	//
		loader_mom->ldcurpath,	//
		(lineno > 0) ? lineno : (int) loader_mom->ldlinecount);
      return buf;
    }
  else
    {
      char lbuf[192];
      memset (lbuf, 0, sizeof (lbuf));
      snprintf (lbuf, sizeof (lbuf), "file %s line %d",	//
		loader_mom->ldcurpath,
		(lineno > 0) ? lineno : (int) loader_mom->ldlinecount);
      return MOM_GC_STRDUP ("location message", lbuf);
    }
}

static void
raw_add_alias_mom (const momstring_t *nam, const momitem_t *itm)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (nam != NULL && nam != MOM_EMPTY && nam->slen > 0);
  assert (itm != NULL && itm != MOM_EMPTY);
  assert (6 * loader_mom->ldaliascount < 5 * loader_mom->ldaliaslen);
  unsigned h = nam->shash;
  assert (h != 0);
  unsigned asiz = loader_mom->ldaliaslen;
  struct loaderaliasent_st *alarr = loader_mom->ldaliasentarr;
  assert (alarr != NULL);
  assert (asiz > 10 && asiz % 2 != 0 && asiz % 3 != 0);
  unsigned startix = h % asiz;
  int pos = -2;
  for (unsigned ix = startix; ix < asiz; ix++)
    {
      const momstring_t *curname = alarr[ix].al_name;
      if (!curname)
	{
	  if (pos < 0)
	    pos = ix;
	  break;
	}
      else if (curname == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = ix;
	  continue;
	}
      else if (curname->shash == h && !strcmp (curname->cstr, nam->cstr))
	{
	  alarr[ix].al_itm = (momitem_t *) itm;
	  return;
	}
    };
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const momstring_t *curname = alarr[ix].al_name;
      if (!curname)
	{
	  if (pos < 0)
	    pos = ix;
	  break;
	}
      else if (curname == MOM_EMPTY)
	{
	  if (pos < 0)
	    pos = ix;
	  continue;
	}
      else if (curname->shash == h && !strcmp (curname->cstr, nam->cstr))
	{
	  alarr[ix].al_itm = (momitem_t *) itm;
	  return;
	}
    };
  assert (pos >= 0);
  alarr[pos].al_itm = (momitem_t *) itm;
  alarr[pos].al_name = nam;
  loader_mom->ldaliascount++;
}				/* end of raw_add_alias_mom */


static momitem_t *
find_alias_item_mom (const momstring_t *nam)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (nam != NULL && nam != MOM_EMPTY && nam->slen > 0);
  assert (6 * loader_mom->ldaliascount < 5 * loader_mom->ldaliaslen);
  unsigned h = nam->shash;
  assert (h != 0);
  unsigned asiz = loader_mom->ldaliaslen;
  struct loaderaliasent_st *alarr = loader_mom->ldaliasentarr;
  assert (alarr != NULL);
  assert (asiz > 10 && asiz % 2 != 0 && asiz % 3 != 0);
  unsigned startix = h % asiz;
  for (unsigned ix = startix; ix < asiz; ix++)
    {
      const momstring_t *curname = alarr[ix].al_name;
      if (!curname)
	return NULL;
      if (curname == MOM_EMPTY)
	continue;
      if (curname->shash && !strcmp (curname->cstr, nam->cstr))
	return alarr[ix].al_itm;
    };
  for (unsigned ix = 0; ix < startix; ix++)
    {
      const momstring_t *curname = alarr[ix].al_name;
      if (!curname)
	return NULL;
      if (curname == MOM_EMPTY)
	continue;
      if (curname->shash && !strcmp (curname->cstr, nam->cstr))
	return alarr[ix].al_itm;
    };
  return NULL;
}				/* end of find_alias_item_mom */


static void
alias_reorganize_mom (void)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  unsigned oldsiz = loader_mom->ldaliaslen;
  struct loaderaliasent_st *oldarr = loader_mom->ldaliasentarr;
  unsigned oldcnt = loader_mom->ldaliascount;
  unsigned newsiz = mom_prime_above (3 * oldcnt / 2 + 30);
  if (newsiz == oldsiz)
    return;
  loader_mom->ldaliasentarr =	//
    MOM_GC_ALLOC ("newalias", newsiz * sizeof (struct loaderaliasent_st));
  loader_mom->ldaliascount = 0;
  loader_mom->ldaliaslen = newsiz;
  for (unsigned ix = 0; ix < oldsiz; ix++)
    {
      const momstring_t *curnam = oldarr[ix].al_name;
      if (!curnam || curnam == MOM_EMPTY)
	continue;
      const momitem_t *curitm = oldarr[ix].al_itm;
      if (!curitm || curitm == MOM_EMPTY)
	continue;
      raw_add_alias_mom (curnam, curitm);
    }
  assert (loader_mom->ldaliascount == oldcnt);
}				/* end of alias_reorganize_mom */


static void
add_alias_mom (const momstring_t *nam, momitem_t *itm)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (nam != NULL && nam != MOM_EMPTY && nam->slen > 0);
  assert (itm != NULL && itm != MOM_EMPTY);
  if (MOM_UNLIKELY
      (5 * loader_mom->ldaliascount + 2 >= 4 * loader_mom->ldaliaslen))
    alias_reorganize_mom ();
  raw_add_alias_mom (nam, itm);
}				/* end of add_alias_mom */


static void
first_pass_load_mom (const char *path, FILE *fil)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  char *linbuf = NULL;
  size_t linsiz = 0;
  ssize_t linlen = 0;
  unsigned lincnt = 0;
  loader_mom->ldcurpath = path;
  loader_mom->ldcurfile = fil;
  rewind (fil);
  linsiz = 128;
  linbuf = malloc (linsiz);	// for getline
  if (!linbuf)
    MOM_FATAPRINTF ("failed to allocate line of %zd bytes", linsiz);
  memset (linbuf, 0, linsiz);
  while ((linlen = getline (&linbuf, &linsiz, fil)) >= 0)
    {
      lincnt++;
      /// lines like: ** <item-name> or *: <item-name> are defining an item
      if (linlen >= 4 && linbuf[0] == '*'
	  && (linbuf[1] == '*' || linbuf[1] == ':'))
	{
	  bool isuser = linbuf[1] == ':';
	  char locbuf[64];
	  memset (locbuf, 0, sizeof (locbuf));
	  MOM_DEBUGPRINTF (load, "first pass file %s line#%d: %s",
			   loader_mom->ldcurpath, lincnt, linbuf);
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
	      MOM_DEBUGPRINTF (load, "first pass file %s named item @%p %s",
			       loader_mom->ldcurpath, itm, pc);
	      if (itm->itm_space == momspa_transient)
		{
		  if (!isuser)
		    itm->itm_space = momspa_global;
		  else
		    itm->itm_space = momspa_user;
		}
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
	      MOM_DEBUGPRINTF (load, "first pass anonitem <%s> near %s",
			       pc, load_position_mom (locbuf, sizeof (locbuf),
						      0));
	      itm = mom_make_anonymous_item_by_id (pc);
	      if (itm->itm_space == momspa_transient)
		{
		  if (!isuser)
		    itm->itm_space = momspa_global;
		  else
		    itm->itm_space = momspa_user;
		};
	      MOM_DEBUGPRINTF (load,
			       "first pass file %s anonymous item %s (%s) @%p %s",
			       loader_mom->ldcurpath,
			       mom_item_cstring (itm),
			       mom_item_space_string (itm), itm, pc);
	      *end = endch;
	      loader_mom->lditemset =
		mom_hashset_put (loader_mom->lditemset, itm);
	    }
	  // lines like ** __<aliasname> are defining an alias
	  else if (*pc == '_' && pc[1] == '_' && isalpha (pc[2]))
	    {
	      char aliasname[80];
	      memset (aliasname, 0, sizeof (aliasname));
	      if (sscanf (pc, "%78[A-Za-z0-9_]", aliasname) <= 0
		  || aliasname[0] != '_' || aliasname[1] != '_'
		  || !isalpha (aliasname[2]))
		MOM_FATAPRINTF ("invalid alias name '%s' near %s",
				aliasname, load_position_mom (locbuf,
							      sizeof (locbuf),
							      0));
	      aliasname[sizeof (aliasname) - 1] = '\0';
	      MOM_DEBUGPRINTF (load,
			       "first pass file %s alias %s line#%d: %s",
			       loader_mom->ldcurpath, aliasname, lincnt,
			       linbuf);
	      const momstring_t *aliastr = mom_make_string_cstr (aliasname);
	      itm = mom_make_anonymous_item ();
	      if (!isuser)
		itm->itm_space = momspa_global;
	      else
		itm->itm_space = momspa_user;
	      MOM_DEBUGPRINTF (load,
			       "first pass new %s alias %s for %s at %s",
			       isuser ? "user" : "global", aliasname,
			       mom_item_cstring (itm),
			       load_position_mom (locbuf, sizeof (locbuf),
						  0));
	      add_alias_mom (aliastr, itm);
	    }
	  else
	    MOM_FATAPRINTF ("invalid line #%d in file %s:\t%s", lincnt, path,
			    linbuf);
	}
      /// lines like: !! <item-name> are requesting a module
      else if (linlen >= 4 && linbuf[0] == '!' && linbuf[1] == '!')
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
  loader_mom->ldcurpath = NULL;
  loader_mom->ldcurfile = NULL;
}				/* end first_pass_load_mom */



static void load_fill_item_mom (momitem_t *itm, bool internal);

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
      mom_eat_token_load_at (fil, lin);
      valtok = mom_peek_token_load_at (fil, lin);
      if (!mom_value_is_delim (valtok, "(!"))
	return itm;
      mom_eat_token_load ();
      load_fill_item_mom ((momitem_t *) itm, true);
      valtok = mom_peek_token_load ();
      if (!mom_value_is_delim (valtok, "!)"))
	MOM_FATAPRINTF ("unexpected token %s in %s for item %s, expected !)",
			mom_output_gcstring (valtok),
			load_position_mom (NULL, 0, 0),
			mom_item_cstring (itm));
      mom_eat_token_load ();
      return itm;
    }
  else if (mom_value_is_delim (valtok, "_*"))
    {
      mom_eat_token_load_at (fil, lin);
      MOM_DEBUGPRINTF (load, "load_itemref@%s:%d start global anonymous", fil,
		       lin);
      return mom_load_new_anonymous_item (true);
    }
  else if (mom_value_is_delim (valtok, "_:"))
    {
      mom_eat_token_load_at (fil, lin);
      MOM_DEBUGPRINTF (load, "load_itemref@%s:%d start user anonymous", fil,
		       lin);
      return mom_load_new_anonymous_item (false);
    }
  else if (mom_value_is_delim (valtok, "_!"))
    {
      momvalue_t valitm = MOM_NONEV;
      int linum = (int) loader_mom->ldlinecount;
      mom_eat_token_load_at (fil, lin);
      MOM_DEBUGPRINTF (load, "load_itemref@%s:%d start value2item near %s",
		       fil, lin, load_position_mom (NULL, 0, 0));
      if (!mom_load_value (&valitm))
	MOM_FATAPRINTF
	  ("load_itemref _! failed to load value to convert near %s",
	   load_position_mom (NULL, 0, 0));
      MOM_DEBUGPRINTF (load, "load_itemref@%s:%d valitm=%s near %s", fil, lin,
		       mom_output_gcstring (valitm),
		       load_position_mom (NULL, 0, 0));
      if (valitm.typnum != momty_item)
	MOM_FATAPRINTF ("loading _! near %s does not get an item, but %s",
			load_position_mom (NULL, 0, linum),
			mom_output_gcstring (valitm));
      valtok = mom_peek_token_load_at (fil, lin);
      if (!mom_value_is_delim (valtok, "(!"))
	return valitm.vitem;
      load_fill_item_mom (valitm.vitem, true);
      valtok = mom_peek_token_load ();
      if (!mom_value_is_delim (valtok, "!)"))
	MOM_FATAPRINTF ("unexpected token %s in %s, expected !)",
			mom_output_gcstring (valtok),
			load_position_mom (NULL, 0, 0));
      mom_eat_token_load ();
      return valitm.vitem;
    }
  else if (valtok.typnum == momty_node
	   && mom_node_conn (valtok.vnode) == MOM_PREDEFINED_NAMED (item))
    {
      // handle aliases
      const momstring_t *namstr =
	mom_value_to_string (mom_node_nth (valtok.vnode, 0));
      assert (namstr != NULL);
      mom_eat_token_load_at (fil, lin);
      momitem_t *alitm = find_alias_item_mom (namstr);
      if (alitm)
	{			/* existing alias */
	  MOM_DEBUGPRINTF (load,
			   "load_itemref@%s:%d namstr=%s found alitm=%s near %s",
			   fil, lin, namstr->cstr, mom_item_cstring (alitm),
			   load_position_mom (NULL, 0, 0));
	  return alitm;
	}
      else
	{
	  MOM_FATAPRINTF ("unknown alias %s near %s",
			  namstr->cstr, load_position_mom (NULL, 0, 0));
	};
    }
  return NULL;
}				/* end mom_load_itemref_at */


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
  fflush (NULL);
  int ok = system (makecmd);
  MOM_INFORMPRINTF ("after running %s got %d", makecmd, ok);
  if (!ok > 1)
    MOM_FATAPRINTF ("failed to run %s : got exit code %d", makecmd, ok);
  int nbmod = 0;
  for (unsigned mix = 0; mix < setmod->slen; mix++)
    {
      const momitem_t *moditm = setmod->arritm[mix];
      assert (moditm && moditm != MOM_EMPTY);
      MOM_DEBUGPRINTF (load, "loading module mix#%d moditm %s",
		       mix, mom_item_cstring (moditm));
      const momstring_t *mstr =	//
	mom_make_string_sprintf (MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX
				 "%s.so",
				 mom_item_cstring (moditm));
      void *dlh = GC_dlopen (mstr->cstr, RTLD_NOW | RTLD_GLOBAL);
      if (!dlh)
	MOM_FATAPRINTF ("failed to dlopen %s : %s", mstr->cstr, dlerror ());
      nbmod++;
    }
  if (nbmod > 0)
    MOM_INFORMPRINTF ("loaded %d modules : %s", nbmod,
		      mom_output_gcstring (mom_unsafe_setv (setmod)));
  else
    MOM_INFORMPRINTF ("loaded no modules");
}				/* end make_modules_load_mom */



static bool
token_string_load_mom (momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  const char *startc = loader_mom->ldlinebuf + loader_mom->ldlinecol + 1;
  const char *eol = loader_mom->ldlinebuf + loader_mom->ldlinelen;
  char *buf = MOM_GC_SCALAR_ALLOC ("string buffer", eol - startc + 3);
  unsigned bufsiz = eol - startc + 1;
  int blen = 0;
  int nbescapes = 0;
  const char *pc = startc;
  MOM_DEBUGPRINTF (load, "token_string_load startc %s", startc);
  for (pc = startc; pc < eol && *pc && *pc != '"'; pc++)
    {
      if (*pc != '\\')
	buf[blen++] = *pc;
      else
	{
	  nbescapes++;
	  pc++;
	  MOM_DEBUGPRINTF (load,
			   "token_string_load escape#%d buf=<%s> blen#%d pc=<%s>",
			   nbescapes, buf, blen, pc);
	  switch (*pc)
	    {
	    case '\"':
	      buf[blen++] = '\"';
	      break;
	    case '\'':
	      buf[blen++] = '\'';
	      break;
	    case '\\':
	      buf[blen++] = '\\';
	      break;
	    case 'a':
	      buf[blen++] = '\a';
	      break;
	    case 'b':
	      buf[blen++] = '\b';
	      break;
	    case 'f':
	      buf[blen++] = '\f';
	      break;
	    case 'n':
	      buf[blen++] = '\n';
	      break;
	    case 'r':
	      buf[blen++] = '\r';
	      break;
	    case 't':
	      buf[blen++] = '\t';
	      break;
	    case 'v':
	      buf[blen++] = '\v';
	      break;
	    case 'e':
	      buf[blen++] = 033 /*ESCAPE*/;
	      break;
	    case 'x':
	      {
		unsigned hc = 0;
		if (sscanf (pc + 1, "%02x", &hc) > 0)
		  {
		    buf[blen++] = (char) hc;
		    pc += 2;
		  }
		else
		  {
		    buf[blen++] = 'x';
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
		    pc += gap - 1;
		  }
		else
		  {
		    buf[blen++] = 'u';
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
		    pc += gap - 1;
		  }
		else
		  {
		    buf[blen++] = 'U';
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
  if (nbescapes > 0)
    MOM_DEBUGPRINTF (load,
		     "token_string_load nbescapes=%d escapedbuf <%s> startc: %s",
		     nbescapes, buf, startc);
  else
    MOM_DEBUGPRINTF (load, "token_string_load buf %s", buf);
  return true;
}


const char *const delim_mom[] = {
  /// first the 2 bytes delimiters; notice that degree-sign °, section-sign §, are two UTF-8 bytes
  "==",
  "**", "*:", "++", "--", "[[", "]]", "..", "_*", "_:", "_!",
  "(!", "!)",
  "°", "§", "*", "(", ")",
  "[", "]",
  "~", "=",
  "{", "}", "<", ">", "^", "!", "%", "@", "|", "&",
  NULL
};


#define token_parse_load_mom() token_parse_load_mom_at(__FILE__,__LINE__)

static momvalue_t
token_parse_load_mom_at (const char *fil, int lin)
{
  momvalue_t valtok = MOM_NONEV;
  char locbuf[128];
  //memset (locbuf, 0, sizeof (locbuf));
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
readagain:
  memset (locbuf, 0, sizeof (locbuf));
  valtok = MOM_NONEV;
  if (!loader_mom->ldlinebuf
      || loader_mom->ldlinecol >= loader_mom->ldlinelen)
    {
      if (loader_mom->ldlinebuf)
	memset (loader_mom->ldlinebuf, 0, loader_mom->ldlinesize);
      FILE *f = loader_mom->ldcurfile;
      loader_mom->ldlineoff = ftell (f);
      loader_mom->ldlinelen =
	getline (&loader_mom->ldlinebuf, &loader_mom->ldlinesize, f);
      if (loader_mom->ldlinelen <= 0)
	{
	  MOM_DEBUGPRINTF (load, "token_parse_load@%s:%d: got EOF at %s", fil,
			   lin, load_position_mom (locbuf, sizeof (locbuf),
						   0));
	  return MOM_NONEV;
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
  if (isspace (c))
    {
      loader_mom->ldlinecol++;
      goto readagain;
    }
  if (c == '/' && pstart[1] == '/')
    {
      loader_mom->ldlinecol = loader_mom->ldlinelen;
      goto readagain;
    }
  else if (c == '/' && pstart[1] == '*')
    {
      char *endcomm = strstr (pstart + 2, "*/");
      if (!endcomm)
	MOM_FATAPRINTF
	  ("unterminated /* single-line comment in %s",
	   load_position_mom (locbuf, sizeof (locbuf), 0));
      loader_mom->ldlinecol += (endcomm + 2 - pstart);
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
	  valtok.typnum = momty_double;
	  valtok.vdbl = x;
	  loader_mom->ldlinecol += endflo - startc;
	}
      else
	{
	  valtok.typnum = momty_int;
	  valtok.vint = (intptr_t) ll;
	  loader_mom->ldlinecol += endnum - startc;
	}
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got number token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return valtok;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (loader_mom->ldlinebuf + loader_mom->ldlinecol + 1,
			    "NAN", 3))
    {
      valtok.typnum = momty_double;
      valtok.vdbl = NAN;
      loader_mom->ldlinecol += 4;
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got number NAN token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return valtok;
    }
  else if ((c == '+' || c == '-')
	   && !strncasecmp (loader_mom->ldlinebuf + loader_mom->ldlinecol + 1,
			    "INF", 3))
    {
      valtok.typnum = momty_double;
      valtok.vdbl = INFINITY;
      loader_mom->ldlinecol += 4;
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got number INF token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return valtok;
    }
  else if (c == '"')
    {
      bool res = token_string_load_mom (&valtok);
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got string token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      if (res)
	return valtok;
      else
	MOM_FATAPRINTF
	  ("failed to parse string (token_parse_load@%s:%d) in %s", fil, lin,
	   load_position_mom (locbuf, sizeof (locbuf), 0));
    }
  else if (c == '_' && mom_valid_item_id_str (pstart, (const char **) &end)
	   && end && !isalnum (*end) && *end != '_')
    {
      char olde = *end;
      *end = '\0';
      MOM_DEBUGPRINTF (load, "token_parse_load@%s:%d: anonitem <%s>  at %s",
		       fil, lin, pstart, load_position_mom (locbuf,
							    sizeof (locbuf),
							    0));
      const momitem_t *itm = mom_find_item (pstart);
      if (itm)
	{
	  assert (itm->itm_str);
	  MOM_DEBUGPRINTF (load,
			   "token_parse_load@%s:%d: found anonitem %s (%s)",
			   fil, lin, mom_item_cstring (itm),
			   mom_item_space_string (itm));
	  valtok.vitem = (momitem_t *) itm;
	  valtok.typnum = momty_item;
	  loader_mom->ldlinecol += end - pstart;
	  MOM_DEBUGPRINTF (load,
			   "token_parse_load@%s:%d: got anon-item token %s at %s",
			   fil, lin, mom_output_gcstring (valtok),
			   load_position_mom (locbuf, sizeof (locbuf), 0));
	  *end = olde;
	  return valtok;
	}
      else
	MOM_DEBUGPRINTF (load,
			 "token_parse_load@%s:%d: did not found anon %s at %s",
			 fil, lin, pstart, load_position_mom (locbuf,
							      sizeof (locbuf),
							      0));
    }
  else if (c == '_' && pstart[1] == '*')
    {
      // _* is a delimiter to make a global new anonymous item
      valtok.typnum = momty_delim;
      strcpy (valtok.vdelim.delim, "_*");
      loader_mom->ldlinecol += 2;
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got quasidelim token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return valtok;
    }
  else if (c == '_' && pstart[1] == ':')
    {
      // _: is a delimiter to make a user new anonymous item
      valtok.typnum = momty_delim;
      strcpy (valtok.vdelim.delim, "_:");
      loader_mom->ldlinecol += 2;
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got quasidelim token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return valtok;
    }
  else if (c == '_' && pstart[1] == '_' && isalpha (pstart[2]))
    {
      // item alias __<aliasname>, tokenized as ^item(<fullaliasname>)
      // where <fullaliasname> is a string starting with __
      char *p = pstart;
      while (*p && (isalnum (*p) || *p == '_'))
	p++;
      char oldc = *p;
      *p = 0;
      const momstring_t *namstr = mom_make_string_cstr (pstart);
      *p = oldc;
      loader_mom->ldlinecol += (p - pstart);
      valtok =
	mom_nodev_new (MOM_PREDEFINED_NAMED (item), 1, mom_stringv (namstr));
      MOM_DEBUGPRINTF (load,
		       "token_parse_load@%s:%d: got alias token %s at %s",
		       fil, lin, mom_output_gcstring (valtok),
		       load_position_mom (locbuf, sizeof (locbuf), 0));
      return valtok;
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
	  valtok.typnum = momty_item;
	  valtok.vitem = (momitem_t *) itm;
	  loader_mom->ldlinecol += end - pstart;
	  MOM_DEBUGPRINTF (load,
			   "token_parse_load@%s:%d: got token %s, named-item %s (%s) at %s",
			   fil, lin, mom_output_gcstring (valtok),
			   mom_item_cstring (itm),
			   mom_item_space_string (itm),
			   load_position_mom (locbuf, sizeof (locbuf), 0));
	  *end = olde;
	  return valtok;
	}
      else
	MOM_WARNPRINTF
	  ("token_parse_load@%s:%d: did not found named %s at %s", fil, lin,
	   pstart, load_position_mom (locbuf, sizeof (locbuf), 0));
    }
  else if (ispunct (c) || (unsigned char) c >= 0x7f)
    {
      for (int ix = 0; delim_mom[ix]; ix++)
	{
	  if (!strncmp
	      (loader_mom->ldlinebuf + loader_mom->ldlinecol, delim_mom[ix],
	       strlen (delim_mom[ix])))
	    {
	      valtok.typnum = momty_delim;
	      strcpy (valtok.vdelim.delim, delim_mom[ix]);
	      loader_mom->ldlinecol += strlen (delim_mom[ix]);
	      MOM_DEBUGPRINTF (load,
			       "token_parse_load@%s:%d: got delim token %s at %s",
			       fil, lin, mom_output_gcstring (valtok),
			       load_position_mom (locbuf, sizeof (locbuf),
						  0));
	      return valtok;
	    }
	}
    }

  MOM_FATAPRINTF
    ("token_parse_load@%s:%d: failing linecol %d linebuf %s at %s", fil, lin,
     (int) loader_mom->ldlinecol, loader_mom->ldlinebuf,
     load_position_mom (locbuf, sizeof (locbuf), 0));
}				/* end token_parse_load_mom_at */




momvalue_t
mom_peek_token_load_at (const char *fil, int lin)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  MOM_DEBUGPRINTF (load, "peek_token_load@%s:%d: token-queue-size %lu",
		   fil, lin, mom_queuevalue_size (&loader_mom->ldquetokens));
  if (0 == mom_queuevalue_size (&loader_mom->ldquetokens))
    {
      momvalue_t valtoken = token_parse_load_mom_at (fil, lin);
      if (valtoken.typnum != momty_null)
	{
	  mom_queuevalue_push_back (&loader_mom->ldquetokens, valtoken);
	  MOM_DEBUGPRINTF (load,
			   "peek_token_load@%s:%d: pushed parsed token %s",
			   fil, lin, mom_output_gcstring (valtoken));
	  return valtoken;
	}
      else
	{
	  MOM_DEBUGPRINTF (load, "peek_token_load@%s:%d: EOF", fil, lin);
	  return MOM_NONEV;
	}
    }
  else
    {
      momvalue_t val = mom_queuevalue_peek_front (&loader_mom->ldquetokens);
      MOM_DEBUGPRINTF (load, "peek_token_load@%s:%d: queued-front token %s",
		       fil, lin, mom_output_gcstring (val));
      return val;
    }
}				/* end mom_peek_token_load_at */


momvalue_t
mom_peek_next_token_load_at (const char *fil, int lin)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  int qsiz = mom_queuevalue_size (&loader_mom->ldquetokens);
  if (qsiz == 0)
    {
      momvalue_t valfirsttoken = token_parse_load_mom_at (fil, lin);
      if (valfirsttoken.typnum != momty_null)
	{
	  mom_queuevalue_push_back (&loader_mom->ldquetokens, valfirsttoken);
	  MOM_DEBUGPRINTF (load,
			   "peek_next_token_load@%s:%d: pushed parsed first token %s",
			   fil, lin, mom_output_gcstring (valfirsttoken));
	}
      momvalue_t valnexttoken = token_parse_load_mom_at (fil, lin);
      if (valnexttoken.typnum != momty_null)
	{
	  mom_queuevalue_push_back (&loader_mom->ldquetokens, valnexttoken);
	  MOM_DEBUGPRINTF (load,
			   "peek_next_token_load@%s:%d: pushed and returning next token %s",
			   fil, lin, mom_output_gcstring (valnexttoken));
	  return valnexttoken;
	}
    }
  else if (qsiz == 1)
    {
      momvalue_t valnexttoken = token_parse_load_mom_at (fil, lin);
      if (valnexttoken.typnum != momty_null)
	{
	  mom_queuevalue_push_back (&loader_mom->ldquetokens, valnexttoken);
	  MOM_DEBUGPRINTF (load,
			   "peek_next_token_load@%s:%d: pushed and returning next token %s",
			   fil, lin, mom_output_gcstring (valnexttoken));
	  return valnexttoken;
	}
    }
  else
    {
      momvalue_t valsecondtoken =
	mom_queuevalue_peek_nth (&loader_mom->ldquetokens, 1);
      MOM_DEBUGPRINTF (load, "peek_next_token_load@%s:%d: retrieved %s", fil, lin,	//
		       mom_output_gcstring (valsecondtoken));
      return valsecondtoken;
    }
  MOM_DEBUGPRINTF (load, "peek_next_token_load@%s:%d: nothing", fil, lin);
  return MOM_NONEV;
}


void
mom_eat_token_load_at (const char *fil, int lin)
{
  char posbuf[96];
  memset (posbuf, 0, sizeof (posbuf));
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  MOM_DEBUGPRINTF (load, "eat_token_load@%s:%d qusiz=%d near %s", fil, lin,
		   (int) mom_queuevalue_size (&loader_mom->ldquetokens),
		   load_position_mom (posbuf, sizeof (posbuf), 0));
  if (mom_queuevalue_size (&loader_mom->ldquetokens) > 0)
    {
      MOM_DEBUGPRINTF (load, "eat_token_load@%s:%d popping token", fil, lin);
      (void) mom_queuevalue_pop_front (&loader_mom->ldquetokens);
    };
  if (mom_queuevalue_size (&loader_mom->ldquetokens) == 0
      && !feof (loader_mom->ldcurfile))
    {
      MOM_DEBUGPRINTF (load, "eat_token_load@%s:%d parsing near %s",
		       fil, lin,
		       load_position_mom (posbuf, sizeof (posbuf), 0));
      momvalue_t valtoken = token_parse_load_mom_at (fil, lin);
      if (valtoken.typnum != momty_null)
	{
	  MOM_DEBUGPRINTF (load,
			   "eat_token_load@%s:%d: pushing parsed token %s near %s",
			   fil, lin, mom_output_gcstring (valtoken),
			   load_position_mom (posbuf, sizeof (posbuf), 0));
	  mom_queuevalue_push_back (&loader_mom->ldquetokens, valtoken);
	  return;
	}
      else
	MOM_DEBUGPRINTF (load,
			 "eat_token_load@%s:%d: null probably EOF near %s",
			 fil, lin, load_position_mom (posbuf, sizeof (posbuf),
						      0));
    }
  else
    MOM_DEBUGPRINTF (load, "eat_token_load@%s:%d: front token %s near %s", fil, lin,	//
		     mom_output_gcstring (mom_queuevalue_peek_front (&loader_mom->ldquetokens)),	//
		     load_position_mom (posbuf, sizeof (posbuf), 0));
}				/* end mom_eat_token_load_at */

////////////////
static void
load_fill_item_mom (momitem_t *itm, bool internal)
{				// keep in sync with emit_content_dumped_item_mom
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (itm && itm->itm_str);
  MOM_DEBUGPRINTF (load, "load_fill_item start %s (%s), at %s",
		   mom_item_cstring (itm), mom_item_space_string (itm),
		   load_position_mom (NULL, 0, 0));
  momvalue_t vtok = MOM_NONEV;
  /// load the attributes
  vtok = mom_peek_token_load ();
  MOM_DEBUGPRINTF (load, "load_fill_item %s vtok=%s for attributes at %s",	//
		   mom_item_cstring (itm), mom_output_gcstring (vtok),	//
		   load_position_mom (NULL, 0, 0));
  if (mom_value_is_delim (vtok, "{"))
    {
      MOM_DEBUGPRINTF (load, "load_fill_item attributes of %s at %s", mom_item_cstring (itm),	//
		       load_position_mom (NULL, 0, 0));
      mom_eat_token_load ();
      while ((
	       {
	       vtok = mom_peek_token_load ();
	       MOM_DEBUGPRINTF (load,
				"load_fill_item for %s attr.vtok %s near %s",
				mom_item_cstring (itm),
				mom_output_gcstring (vtok),
				load_position_mom (NULL, 0, 0)); vtok;}
	     ).typnum != momty_null && mom_value_is_delim (vtok, "*"))
	{
	  momvalue_t vat = MOM_NONEV;
	  mom_eat_token_load ();
	  MOM_DEBUGPRINTF (load, "load_fill_item insideattrs of %s near %s",
			   mom_item_cstring (itm), load_position_mom (NULL, 0,
								      0));
	  const momitem_t *itmat = mom_load_itemref ();
	  MOM_DEBUGPRINTF (load, "load_fill_item %s itmat=%s",
			   mom_item_cstring (itm), mom_item_cstring (itmat));
	  if (!itmat)
	    break;
	  if (mom_load_value (&vat) && vat.typnum != momty_null)
	    {
	      MOM_DEBUGPRINTF (load,
			       "load_fill_item %s itmat=%s vat=%s (atcnt %d atlen %d)",
			       mom_item_cstring (itm),
			       mom_item_cstring (itmat),
			       mom_output_gcstring (vat),
			       itm->itm_attrs ? itm->itm_attrs->at_cnt : 0,
			       itm->itm_attrs ? itm->itm_attrs->at_len : 0);
	      itm->itm_attrs =
		mom_attributes_put (itm->itm_attrs, itmat, &vat);
	      MOM_DEBUGPRINTF (load, "load_fill_item %s done itmat=%s",
			       mom_item_cstring (itm),
			       mom_item_cstring (itmat));
	    }
	}
      if (!mom_value_is_delim (vtok, "}"))
	MOM_FATAPRINTF ("expecting } but got %s to end attributes of item %s"
			" in %s",
			mom_output_gcstring (vtok),
			mom_item_cstring (itm),
			load_position_mom (NULL, 0, 0));
      mom_eat_token_load ();
    }
  else
    MOM_DEBUGPRINTF (load, "load_fill_item no attributes for %s",
		     mom_item_cstring (itm));
  // should load the components
  vtok = mom_peek_token_load ();
  MOM_DEBUGPRINTF (load, "load_fill_item %s vtok=%s for components at %s",	//
		   mom_item_cstring (itm), mom_output_gcstring (vtok),	//
		   load_position_mom (NULL, 0, 0));
  if (mom_value_is_delim (vtok, "[["))
    {
      MOM_DEBUGPRINTF (load, "load_fill_item components of %s at %s", mom_item_cstring (itm),	//
		       load_position_mom (NULL, 0, 0));
      mom_eat_token_load ();
      momvalue_t valcomp = MOM_NONEV;
      while ((valcomp = MOM_NONEV), mom_load_value (&valcomp))
	{
	  MOM_DEBUGPRINTF (load, "load_fill_item %s valcomp %s",
			   mom_item_cstring (itm),
			   mom_output_gcstring (valcomp));
	  itm->itm_comps = mom_components_append1 (itm->itm_comps, valcomp);
	}
      int lineno = loader_mom->ldlinecount;
      vtok = mom_peek_token_load ();
      if (!mom_value_is_delim (vtok, "]]"))
	MOM_FATAPRINTF ("expecting ]] but got %s to end components of item %s"
			" in %s",
			mom_output_gcstring (vtok),
			mom_item_cstring (itm), load_position_mom (NULL, 0,
								   lineno));
      mom_eat_token_load ();
    }
  else
    MOM_DEBUGPRINTF (load, "load_fill_item no components for %s",
		     mom_item_cstring (itm));

  // should load the transformer closure, if given
  vtok = mom_peek_token_load ();
  MOM_DEBUGPRINTF (load, "load_fill_item %s vtok=%s for transformers at %s",	//
		   mom_item_cstring (itm), mom_output_gcstring (vtok),	//
		   load_position_mom (NULL, 0, 0));
  while (mom_value_is_delim (vtok, "%"))
    {
      MOM_DEBUGPRINTF (load, "load_fill_item transformer of %s at %s", mom_item_cstring (itm),	//
		       load_position_mom (NULL, 0, 0));
      mom_eat_token_load ();
      momvalue_t valtransf = MOM_NONEV;
      int lineno = loader_mom->ldlinecount;
      if (!mom_load_value (&valtransf))
	MOM_FATAPRINTF
	  ("missing transformer for item %s in %s",
	   mom_item_cstring (itm), load_position_mom (NULL, 0, lineno));
      if (valtransf.typnum != momty_node)
	MOM_FATAPRINTF
	  ("bad non-node transformer %s for item %s in %s",
	   mom_output_gcstring (valtransf), mom_item_cstring (itm),
	   load_position_mom (NULL, 0, lineno));
      add_load_transformer_mom (itm, valtransf);
      vtok = mom_peek_token_load ();
    }
  MOM_DEBUGPRINTF (load, "load_fill_item final vtok %s near %s",
		   mom_output_gcstring (vtok),
		   load_position_mom (NULL, 0, 0));
  if (internal)
    {
      if (!mom_value_is_delim (vtok, "!)"))
	MOM_FATAPRINTF ("unexpected token %s in %s, expected !)",
			mom_output_gcstring (vtok),
			load_position_mom (NULL, 0, 0));
    }
  else
    {
      if (!mom_value_is_delim (vtok, ".."))
	MOM_FATAPRINTF ("unexpected token %s in %s, expected ..",
			mom_output_gcstring (vtok),
			load_position_mom (NULL, 0, 0));
    }
  // we don't eat the .. token; the caller would skip that line
  MOM_DEBUGPRINTF (load, "load_fill_item done %s at %s\n",
		   mom_item_cstring (itm), load_position_mom (NULL, 0, 0));
}				/* end load_fill_item_mom */

////////////////
const momitem_t *
mom_load_new_anonymous_item (bool global)
{
  momitem_t *newitm = mom_make_anonymous_item ();
  MOM_DEBUGPRINTF (load, "load_new_anonymous_item %s newitm=%s at %s",
		   global ? "global" : "user", mom_item_cstring (newitm),
		   load_position_mom (NULL, 0, 0));
  if (global)
    newitm->itm_space = momspa_global;
  else
    newitm->itm_space = momspa_user;
  momvalue_t vtok = MOM_NONEV;
  vtok = mom_peek_token_load ();
  if (vtok.typnum == momty_null)
    MOM_FATAPRINTF ("failed to load new anonymous item in %s",
		    load_position_mom (NULL, 0, 0));
  if (mom_value_is_delim (vtok, "="))
    {
      mom_eat_token_load ();
      vtok = mom_peek_token_load ();
      if (vtok.typnum != momty_node
	  || mom_node_conn (vtok.vnode) != MOM_PREDEFINED_NAMED (item))
	MOM_FATAPRINTF ("expecting alias after = in anonymous item near %s",
			load_position_mom (NULL, 0, 0));
      const momstring_t *namstr =
	mom_value_to_string (mom_node_nth (vtok.vnode, 0));
      assert (namstr != NULL);
      MOM_DEBUGPRINTF (load, "load_new_anonymous_item newitm %s namstr %s",
		       mom_item_cstring (newitm), namstr->cstr);
      if (find_alias_item_mom (namstr))
	MOM_FATAPRINTF ("redefining alias %s in anonymous item %s near %s",
			namstr->cstr, mom_item_cstring (newitm),
			load_position_mom (NULL, 0, 0));
      add_alias_mom (namstr, newitm);
      mom_eat_token_load ();
      vtok = mom_peek_token_load ();
    }
  if (vtok.typnum == momty_string)
    {
      mom_item_unsync_put_attribute (newitm, MOM_PREDEFINED_NAMED (comment),
				     vtok);
      mom_eat_token_load ();
    }
  vtok = mom_peek_token_load ();
  if (!mom_value_is_delim (vtok, "(!"))
    return newitm;
  mom_eat_token_load ();
  load_fill_item_mom (newitm, true);
  vtok = mom_peek_token_load ();
  if (!mom_value_is_delim (vtok, "!)"))
    MOM_FATAPRINTF ("unexpected token %s in %s, expected !)",
		    mom_output_gcstring (vtok),
		    load_position_mom (NULL, 0, 0));
  mom_eat_token_load ();
  return newitm;
}

////////////////

void
second_pass_load_mom (const char *curpath, FILE *curfil)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  assert (curpath != NULL);
  assert (curfil != NULL);
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
  loader_mom->ldcurpath = curpath;
  loader_mom->ldcurfile = curfil;
  rewind (curfil);
  loader_mom->ldlinecol = loader_mom->ldlinelen = loader_mom->ldlinecount = 0;
  do
    {
      if (loader_mom->ldlinebuf)
	memset (loader_mom->ldlinebuf, 0, loader_mom->ldlinesize);
      loader_mom->ldlineoff = ftell (curfil);
      loader_mom->ldlinelen =
	getline (&loader_mom->ldlinebuf, &loader_mom->ldlinesize, curfil);
      if (loader_mom->ldlinelen <= 0)
	{
	  MOM_DEBUGPRINTF (load, "second pass end-of-file %s at %s",
			   loader_mom->ldcurpath,
			   load_position_mom (NULL, 0, 0));
	  return;
	}
      loader_mom->ldlinecol = 0;
      loader_mom->ldlinecount++;
      if (loader_mom->ldlinelen > 4
	  && loader_mom->ldlinebuf[0] == '/'
	  && loader_mom->ldlinebuf[1] == '/')
	continue;
      MOM_DEBUGPRINTF (load, "second pass file %s line#%d: %s",
		       loader_mom->ldcurpath,
		       (int) loader_mom->ldlinecount, loader_mom->ldlinebuf);
      if (loader_mom->ldlinelen > 4
	  && loader_mom->ldlinebuf[0] == '*'
	  && (loader_mom->ldlinebuf[1] == '*'
	      || loader_mom->ldlinebuf[1] == ':'))
	{
	  char locbuf[72];
	  memset (locbuf, 0, sizeof (locbuf));
	  loader_mom->ldlinecol = 2;
	  MOM_DEBUGPRINTF (load, "second pass file %s defining line#%d: %s",
			   loader_mom->ldcurpath,
			   (int) loader_mom->ldlinecount,
			   loader_mom->ldlinebuf);
	  memset (&loader_mom->ldquetokens, 0,
		  sizeof (loader_mom->ldquetokens));
	  momvalue_t val = mom_peek_token_load ();
	  momitem_t *itm = NULL;
	  const momstring_t *namstr = NULL;
	  if (val.typnum == momty_item)
	    itm = val.vitem;
	  else if (val.typnum == momty_node
		   && mom_node_conn (val.vnode) == MOM_PREDEFINED_NAMED (item)
		   && (namstr =
		       mom_value_to_string (mom_node_nth (val.vnode, 0))) !=
		   NULL)
	    {
	      itm = find_alias_item_mom (namstr);
	      assert (itm != NULL && itm->itm_anonymous);
	    }
	  else
	    MOM_FATAPRINTF ("invalid line %d '%s' of %s (got %s, expecting some item) ",	//
			    (int) loader_mom->ldlinecount, loader_mom->ldlinebuf,	//
			    load_position_mom (locbuf, sizeof (locbuf), 0),
			    mom_output_gcstring (val));
	  MOM_DEBUGPRINTF (load, "second pass filling item %s near %s",
			   mom_item_cstring (itm),
			   load_position_mom (locbuf, sizeof (locbuf), 0));
	  mom_eat_token_load ();
	  load_fill_item_mom (itm, false);
	  MOM_DEBUGPRINTF (load, "second pass near %s filled item %s\n",
			   load_position_mom (locbuf, sizeof (locbuf), 0),
			   mom_item_cstring (itm));
	}
    }
  while (!feof (curfil));
  loader_mom->ldcurpath = NULL;
  loader_mom->ldcurfile = NULL;
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
  memset (pval, 0, sizeof (momvalue_t));
  momvalue_t vtok = MOM_NONEV;
  vtok = mom_peek_token_load ();
  if (mom_value_is_delim (vtok, "!"))
    {
      mom_eat_token_load ();
      return mom_load_value (pval);
    }
  return false;
}


////////////////
bool				// should be in sync with mom_emit_dumped_valueptr
mom_load_value (momvalue_t *pval)
{
  assert (loader_mom && loader_mom->ldmagic == LOADER_MAGIC_MOM);
  if (!pval)
    return false;
  memset (pval, 0, sizeof (momvalue_t));
  momvalue_t vtok = mom_peek_token_load ();
  const momstring_t *namstr = NULL;
  MOM_DEBUGPRINTF (load, "load_value vtok %s near %s",
		   mom_output_gcstring (vtok), load_position_mom (NULL, 0,
								  0));
  if (vtok.typnum == momty_null)
    return false;
  if (vtok.typnum == momty_item)
    {				// items
      *pval = vtok;
      mom_eat_token_load ();
      return true;
    }
  if (vtok.typnum == momty_node
      && mom_node_conn (vtok.vnode) == MOM_PREDEFINED_NAMED (item)
      && (namstr =
	  mom_value_to_string (mom_node_nth (vtok.vnode, 0))) != NULL)
    {				/* alias */
      momitem_t *alitm = find_alias_item_mom (namstr);
      if (!alitm)
	MOM_FATAPRINTF ("invalid alias %s near %s",
			mom_string_cstr (namstr), load_position_mom (NULL, 0,
								     0));
      vtok.typnum = momty_item;
      vtok.vitem = alitm;
      *pval = vtok;
      mom_eat_token_load ();
      return true;
    }
  if (mom_value_is_delim (vtok, "_*") || mom_value_is_delim (vtok, "_:")
      || mom_value_is_delim (vtok, "_!"))
    {
      momvalue_t vitem = MOM_NONEV;
      const momitem_t *anitm = mom_load_itemref ();
      MOM_DEBUGPRINTF (load, "value anonymous item %s near %s",
		       mom_item_cstring (anitm), load_position_mom (NULL, 0,
								    0));
      vitem = mom_itemv (anitm);
      *pval = vitem;
      return true;
    }
  if (mom_value_is_delim (vtok, "~"))
    {				// null value
      *pval = MOM_NONEV;
      mom_eat_token_load ();
      return true;
    }
  if (vtok.typnum == momty_int || vtok.typnum == momty_string
      || vtok.typnum == momty_double)
    {				/// scalars
      *pval = vtok;
      mom_eat_token_load ();
      return true;
    }
  if (mom_value_is_delim (vtok, "°"))
    {				/// delimiters
      int linecnt = loader_mom->ldlinecount;
      momvalue_t vtokbis = mom_peek_token_load ();
      if (vtokbis.typnum == momty_string)
	{
	  mom_eat_token_load ();
	  strncpy (vtok.vdelim.delim, vtokbis.vstr->cstr,
		   sizeof (vtok.vdelim.delim));
	  vtok.typnum = momty_delim;
	  *pval = vtok;
	  return true;
	}
      else
	MOM_FATAPRINTF ("expecting string for delimiter after °"
			" in %s", load_position_mom (NULL, 0, linecnt));
    }
  if (mom_value_is_delim (vtok, "["))
    {				////tuples
      int linecnt = loader_mom->ldlinecount;
      MOM_DEBUGPRINTF (load, "start of tuple at position %s",
		       load_position_mom (NULL, 0, linecnt));
      mom_eat_token_load ();
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
      momvalue_t vtokend = mom_peek_token_load ();
      if (!mom_value_is_delim (vtokend, "]"))
	MOM_FATAPRINTF ("missing ] after tuple content"
			" in %s", load_position_mom (NULL, 0, linecnt));
      mom_eat_token_load ();
      pval->typnum = momty_tuple;
      pval->vtuple = (momseq_t *) mom_queueitem_tuple (&quitems, metav);
      MOM_DEBUGPRINTF (load, "end of tuple %s at position %s",
		       mom_output_gcstring (*pval),
		       load_position_mom (NULL, 0, linecnt));
      return true;
    }				/* done tuples */
  if (mom_value_is_delim (vtok, "{"))
    {				//// sets
      int linecnt = loader_mom->ldlinecount;
      MOM_DEBUGPRINTF (load, "start of set at position %s",
		       load_position_mom (NULL, 0, linecnt));
      mom_eat_token_load ();
      momvalue_t metav = MOM_NONEV;
      const momitem_t *curitm = NULL;
      struct momqueueitems_st quitems;
      memset (&quitems, 0, sizeof (quitems));
      load_metavalue_mom (&metav);
      while ((curitm = mom_load_itemref ()) != NULL)
	{
	  mom_queueitem_push_back (&quitems, curitm);
	  linecnt = loader_mom->ldlinecount;
	  MOM_DEBUGPRINTF (load, "set element item %s at position %s",
			   mom_item_cstring (curitm), load_position_mom (NULL,
									 0,
									 linecnt));
	}
      momvalue_t vtokend = mom_peek_token_load ();
      MOM_DEBUGPRINTF (load, "end set vtokend=%s position %s",
		       mom_output_gcstring (vtokend), load_position_mom (NULL,
									 0,
									 0));
      if (!mom_value_is_delim (vtokend, "}"))
	MOM_FATAPRINTF ("missing } after set content"
			" in %s", load_position_mom (NULL, 0, linecnt));
      mom_eat_token_load ();
      {
	pval->typnum = momty_set;
	const momseq_t *tup = mom_queueitem_tuple (&quitems, MOM_NONEV);
	assert (tup);
	MOM_DEBUGPRINTF (load, "should make set from metav=%s tuple %s",
			 mom_output_gcstring (metav),
			 mom_output_gcstring (mom_tuplev (tup)));
	pval->vset =
	  (momseq_t *) mom_make_sized_meta_set (metav, tup->slen,
						(const momitem_t **)
						tup->arritm);
	MOM_DEBUGPRINTF (load, "end of set %s at position %s",
			 mom_output_gcstring (*pval), load_position_mom (NULL,
									 0,
									 linecnt));
	return true;
      }
    }				/* done sets */
  if (mom_value_is_delim (vtok, "^"))
    {				//// nodes
      int linecnt = loader_mom->ldlinecount;
      mom_eat_token_load ();
      const momitem_t *connitm = mom_load_itemref ();
      MOM_DEBUGPRINTF (load, "begin node connitm %s at position %s",
		       mom_item_cstring (connitm),
		       load_position_mom (NULL, 0, linecnt));
      momvalue_t metav = MOM_NONEV;
      if (!connitm)
	MOM_FATAPRINTF ("missing connective item after ^ "
			" in %s", load_position_mom (NULL, 0, linecnt));
      load_metavalue_mom (&metav);
      linecnt = loader_mom->ldlinecount;
      momvalue_t vtokpar = mom_peek_token_load ();
      if (!mom_value_is_delim (vtokpar, "("))
	MOM_FATAPRINTF ("missing ( -got %s- in loaded node "
			" in %s",
			mom_output_gcstring (vtokpar),
			load_position_mom (NULL, 0, linecnt));
      mom_eat_token_load ();
      struct momqueuevalues_st quvals;
      memset (&quvals, 0, sizeof (quvals));
      momvalue_t vson = MOM_NONEV;
      while ((vson = MOM_NONEV), mom_load_value (&vson))
	{
	  MOM_DEBUGPRINTF (load, "for node connitm %s vson %s at position %s",
			   mom_item_cstring (connitm),
			   mom_output_gcstring (vson),
			   load_position_mom (NULL, 0, linecnt));
	  mom_queuevalue_push_back (&quvals, vson);
	  linecnt = loader_mom->ldlinecount;
	}
      momvalue_t vtokend = mom_peek_token_load ();
      if (!mom_value_is_delim (vtokend, ")"))
	MOM_FATAPRINTF ("missing ) -got %s- to end node " " in %s",
			mom_output_gcstring (vtokend),
			load_position_mom (NULL, 0, linecnt));
      mom_eat_token_load ();
      pval->typnum = momty_node;
      pval->vnode =
	(momnode_t *) mom_queuevalue_node (&quvals, connitm, metav);
      MOM_DEBUGPRINTF (load, "done node %s at position %s",
		       mom_output_gcstring (*pval), load_position_mom (NULL,
								       0, 0));
      return true;
    }				/* done nodes */
  else if (mom_value_is_delim (vtok, "@"))
    {				//// read-time application: @ <value-clo> <value-arg1> [ & <value-arg2> ]
      int linecnt = loader_mom->ldlinecount;
      mom_eat_token_load ();
      momvalue_t vclonod = MOM_NONEV;
      momvalue_t vres = MOM_NONEV;
      if (!mom_load_value (&vclonod))
	MOM_FATAPRINTF
	  ("missing value for read-time application after @ near %s",
	   load_position_mom (NULL, 0, linecnt));
      linecnt = loader_mom->ldlinecount;
      momvalue_t varg1 = MOM_NONEV;
      if (!mom_load_value (&varg1))
	MOM_FATAPRINTF
	  ("missing first argument for read-time application after @ near %s",
	   load_position_mom (NULL, 0, linecnt));
      momvalue_t vtokamp = mom_peek_token_load ();
      if (mom_value_is_delim (vtokamp, "&"))
	{
	  mom_eat_token_load ();
	  linecnt = loader_mom->ldlinecount;
	  momvalue_t varg2 = MOM_NONEV;
	  if (!mom_load_value (&varg2))
	    MOM_FATAPRINTF
	      ("missing second argument for read-time application after & near %s",
	       load_position_mom (NULL, 0, linecnt));
	  MOM_DEBUGPRINTF (load,
			   "before two-arg read-time application vclonod=%s varg1=%s varg2=%s near %s",
			   mom_output_gcstring (vclonod),
			   mom_output_gcstring (varg1),
			   mom_output_gcstring (varg2),
			   load_position_mom (NULL, 0, linecnt));
	  if (!mom_applval_2val_to_val (vclonod, varg1, varg2, &vres))
	    MOM_FATAPRINTF
	      ("failed two-argument read-time application of %s to %s and %s after & near %s",
	       mom_output_gcstring (vclonod),
	       mom_output_gcstring (varg1),
	       mom_output_gcstring (varg2),
	       load_position_mom (NULL, 0, linecnt));
	  MOM_DEBUGPRINTF (load, "after two-arg read-time application"
			   " vclonod=%s varg1=%s varg2=%s vres=%s near %s",
			   mom_output_gcstring (vclonod),
			   mom_output_gcstring (varg1),
			   mom_output_gcstring (varg2),
			   mom_output_gcstring (vres),
			   load_position_mom (NULL, 0, linecnt));
	  *pval = vres;
	  return true;
	}
      else
	{
	  MOM_DEBUGPRINTF (load,
			   "before one-arg read-time application vclonod=%s varg1=%s near %s",
			   mom_output_gcstring (vclonod),
			   mom_output_gcstring (varg1),
			   load_position_mom (NULL, 0, linecnt));
	  if (!mom_applval_1val_to_val (vclonod, varg1, &vres))
	    MOM_FATAPRINTF
	      ("failed one-argument read-time application of %s to %s near %s",
	       mom_output_gcstring (vclonod),
	       mom_output_gcstring (varg1), load_position_mom (NULL, 0,
							       linecnt));
	  MOM_DEBUGPRINTF (load,
			   "after one-arg read-time application"
			   " vclonod=%s varg1=%s vres=%s near %s",
			   mom_output_gcstring (vclonod),
			   mom_output_gcstring (varg1),
			   mom_output_gcstring (vres),
			   load_position_mom (NULL, 0, linecnt));
	  *pval = vres;
	  return true;
	}
    }
  return false;
}				/* end mom_load_value */



void
mom_load_state (const char *xtrapath)
{
  const char *globalpath = MOM_GLOBAL_DATA_PATH;
  const char *userpath = MOM_USER_DATA_PATH;
  FILE *globalfile = NULL;
  FILE *userfile = NULL;
  FILE *xtrafile = NULL;
  struct momloader_st ldr;
  memset (&ldr, 0, sizeof (ldr));
  ldr.ldmagic = LOADER_MAGIC_MOM;
  ldr.ldstartelapsedtime = mom_elapsed_real_time ();
  ldr.ldstartcputime = mom_clock_time (CLOCK_PROCESS_CPUTIME_ID);
  globalfile = fopen (globalpath, "rm");
  if (!globalfile)
    MOM_FATAPRINTF ("failed to open global state file %s : %m", globalpath);
  userfile = fopen (userpath, "rm");
  if (!userfile)
    MOM_WARNPRINTF ("failed to open user state file %s : %m", userpath);
  if (xtrapath && xtrapath[0])
    {
      xtrafile = fopen (xtrapath, "rm");
      if (!xtrafile)
	MOM_WARNPRINTF ("failed to open xtra state file %s : %m", xtrapath);
    };
  {
    unsigned alisiz = 41;	/* a prime */
    ldr.ldaliasentarr =
      MOM_GC_ALLOC ("aliasentarr",
		    alisiz * sizeof (struct loaderaliasent_st));
    ldr.ldaliaslen = alisiz;
    ldr.ldaliascount = 0;
  }
  loader_mom = &ldr;
  MOM_DEBUGPRINTF (load, "first pass");
  first_pass_load_mom (globalpath, globalfile);
  if (userfile)
    first_pass_load_mom (userpath, userfile);
  if (xtrafile)
    first_pass_load_mom (xtrapath, xtrafile);
  if (ldr.ldmoduleset)
    {
      make_modules_load_mom ();
    }
  MOM_DEBUGPRINTF (load, "second pass");
  second_pass_load_mom (globalpath, globalfile);
  if (userfile)
    second_pass_load_mom (userpath, userfile);
  if (xtrafile)
    second_pass_load_mom (xtrapath, xtrafile);
  /// fill the predefined
  {
    // this function is in the generated file fill-monimelt.c, see MOM_FILL_PREDEFINED_PATH
    extern void mom_predefined_items_fill (void);
    MOM_DEBUGPRINTF (load, "before predefined items fill");
    mom_predefined_items_fill ();
  }
  MOM_DEBUGPRINTF (load, "before transformations");
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
	  MOM_DEBUGPRINTF (load, "transforming item#%d: %s with %s",
			   ix, itmtr->itm_str->cstr,
			   mom_output_gcstring (vnode));
	  if (!mom_applval_1itm_to_void (vnode, itmtr))
	    MOM_FATAPRINTF ("failed to transform item#%d: %s with %s",
			    ix, itmtr->itm_str->cstr,
			    mom_output_gcstring (vnode));
	}
    }
  MOM_INFORMPRINTF
    ("loaded %d items and %d modules with %d transforms in %.3f elapsed & %.3f cpu seconds",
     (int) mom_hashset_count (ldr.lditemset),
     (int) mom_hashset_count (ldr.ldmoduleset), nbtransf,
     mom_elapsed_real_time () - ldr.ldstartelapsedtime,
     mom_clock_time (CLOCK_PROCESS_CPUTIME_ID) - ldr.ldstartcputime);
  if (userfile != NULL)
    {
      if (xtrafile != NULL)
	MOM_INFORMPRINTF
	  ("loaded global %s, user %s, xtra %s files", globalpath, userpath,
	   xtrapath);
      else
	MOM_INFORMPRINTF ("loaded global %s, user %s files", globalpath,
			  userpath);
    }
  else
    MOM_INFORMPRINTF ("loaded global %s file", globalpath);
  if (xtrafile)
    fclose (xtrafile);
  if (userfile)
    fclose (userfile);
  fclose (globalfile);
  loader_mom = NULL;
  MOM_DEBUGPRINTF (load, "end loading");
  memset (&ldr, 0, sizeof (ldr));
}				// end function mom_load_state



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
  MOM_DEBUGPRINTF (dump, "dump-scanning item %s", mom_item_cstring (itm));
  if (itm->itm_space == momspa_none || itm->itm_space == momspa_transient)
    {
      return false;
    }
  if (mom_hashset_contains (dumper_mom->duitemuserset, itm))
    return true;
  else if (mom_hashset_contains (dumper_mom->duitemglobalset, itm))
    return true;
  mom_item_lock ((momitem_t *) itm);
  if (itm->itm_space == momspa_user)
    dumper_mom->duitemuserset =
      mom_hashset_put (dumper_mom->duitemuserset, itm);
  else
    dumper_mom->duitemglobalset =
      mom_hashset_put (dumper_mom->duitemglobalset, itm);
  mom_queueitem_push_back (&dumper_mom->duitemque, itm);
  MOM_DEBUGPRINTF (dump, "dump-scanned item %s", mom_item_cstring (itm));
  return true;
}


void
mom_scan_dumped_module_item (const momitem_t *moditm)
{
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan module outside of dumping");
  if (!moditm || moditm == MOM_EMPTY)
    return;
  MOM_DEBUGPRINTF (dump, "scan_dumped_module_item moditm %s",
		   mom_item_cstring (moditm));
  if (dumper_mom->dustate != dump_scan)
    return;
  mom_scan_dumped_item (moditm);
  dumper_mom->duitemmoduleset =
    mom_hashset_put (dumper_mom->duitemmoduleset, moditm);
}

void
mom_scan_dumped_valueptr (const momvalue_t *pval)
{
  if (!dumper_mom || dumper_mom->dumagic != DUMPER_MAGIC_MOM)
    MOM_FATAPRINTF ("scan dumped value outside of dumping");
  if (!pval || pval == MOM_EMPTY)
    return;
  if (mom_valueptr_is_transient (pval))
    {
      MOM_DEBUGPRINTF (dump, "ignoring transient dumped value %s",
		       mom_output_gcstring (*pval));
      return;
    }
  MOM_DEBUGPRINTF (dump, "start scanning dumped value %s",
		   mom_output_gcstring (*pval));
  switch ((enum momvaltype_en) pval->typnum)
    {
    case momty_double:
    case momty_int:
    case momty_null:
    case momty_string:
    case momty_delim:
      goto end;
    case momty_item:
      mom_scan_dumped_item (pval->vitem);
      goto end;
    case momty_set:
    case momty_tuple:
      {
	const momseq_t *sq = pval->vsequ;
	assert (sq);
	mom_scan_dumped_valueptr (&sq->meta);
	unsigned slen = sq->slen;
	for (unsigned ix = 0; ix < slen; ix++)
	  mom_scan_dumped_item (sq->arritm[ix]);
	goto end;
      }
    case momty_node:
      {
	const momnode_t *nod = pval->vnode;
	assert (nod);
	if (!mom_scan_dumped_item (nod->conn))
	  {
	    MOM_DEBUGPRINTF (dump, "non dumpable node connective %s",
			     mom_item_cstring (nod->conn));
	    goto end;
	  }
	mom_scan_dumped_valueptr (&nod->meta);
	unsigned slen = nod->slen;
	for (unsigned ix = 0; ix < slen; ix++)
	  mom_scan_dumped_valueptr (&nod->arrsons[ix]);
	goto end;
      }
    }
end:
  MOM_DEBUGPRINTF (dump, "done scanning dumped value %s",
		   mom_output_gcstring (*pval));
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
  MOM_DEBUGPRINTF (dump, "scanning inside dumped item %s",
		   mom_item_cstring (itm));
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
	    mom_item_unsync_get_attribute	//
	    (itmkd,		//
	     MOM_PREDEFINED_NAMED (dumped_item_scanner));
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
      if (!mom_applval_1itm_to_void (valscanner, itm))
	MOM_FATAPRINTF ("failed to apply scanner %s to item %s of kind %s",
			mom_output_gcstring (valscanner),
			mom_item_cstring (itm), itmkd->itm_str->cstr);
    }
  MOM_DEBUGPRINTF (dump, "done scanning inside dumped item %s\n",
		   mom_item_cstring (itm));
}

static FILE *
open_generated_file_dump_mom (const char *path)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (strlen (path) < 128);
  assert (isalpha (path[0]));
  char pathbuf[256];
  memset (pathbuf, 0, sizeof (pathbuf));
  if (snprintf (pathbuf, sizeof (pathbuf), "%s%s%s",
		dumper_mom->duprefix, path,
		dumper_mom->durandsuffix) >= (int) sizeof (pathbuf))
    MOM_FATAPRINTF
      ("too long path %s duprefix=%s durandsuffix=%s for open_generated_file_dump",
       path, dumper_mom->duprefix, dumper_mom->durandsuffix);
  FILE *out = fopen (pathbuf, "w");
  if (!out)
    MOM_FATAPRINTF ("failed to open generated file %s: %m", pathbuf);
  MOM_DEBUGPRINTF (dump, "open_generated_file_dump path=%s pathbuf=%s fd#%d",
		   path, pathbuf, fileno (out));
  return out;
}

static void
close_generated_file_dump_mom (FILE *fil, const char *path)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (fil);
  MOM_DEBUGPRINTF (dump, "close_generated_file_dump %s fd#%d", path,
		   fileno (fil));
  assert (strlen (path) < 128);
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

static void
emit_predefined_itemref_mom (FILE *out, const momitem_t *itm)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (itm && itm->itm_space == momspa_predefined);
  if (itm->itm_anonymous)
    fprintf (out, "MOM_PREDEFINED_ANONYMOUS(%s)", mom_item_cstring (itm));
  else
    fprintf (out, "MOM_PREDEFINED_NAMED(%s)", mom_item_cstring (itm));
}




static void
emit_signature_application_code_mom (FILE *foutaphd,
				     momitem_t *itmpredef,
				     momvalue_t vradix,
				     momvalue_t vinputy, momvalue_t voutputy)
{
  char radixbuf[256];
  char applyvalbuf[256];
  char applyclosbuf[256];
  memset (radixbuf, 0, sizeof (radixbuf));
  memset (applyvalbuf, 0, sizeof (applyvalbuf));
  memset (applyclosbuf, 0, sizeof (applyclosbuf));
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  MOM_DEBUGPRINTF (dump,
		   "emit_signature_application_code start itmpredef=%s vradix=%s vinputy=%s outputy=%s",
		   mom_item_cstring (itmpredef),
		   mom_output_gcstring (vradix),
		   mom_output_gcstring (vinputy),
		   mom_output_gcstring (voutputy));
  assert (foutaphd);
  assert (itmpredef);
  assert (vradix.typnum == momty_string);
  assert (vinputy.typnum == momty_tuple);
  assert (voutputy.typnum == momty_tuple);
  strncpy (radixbuf, mom_value_cstr (vradix), sizeof (radixbuf) - 1);
  if (strlen (radixbuf) >= sizeof (radixbuf) - 16)
    MOM_FATAPRINTF ("too long vradix %s for itmpredef %s",
		    mom_value_cstr (vradix), mom_item_cstring (itmpredef));
  char *suffix = radixbuf;
  unsigned nbinputy = vinputy.vtuple->slen;
  unsigned nboutputy = voutputy.vtuple->slen;
  fprintf (foutaphd, "\n\n"
	   "// signature application support for %s\n",
	   mom_item_cstring (itmpredef));
  fprintf (foutaphd,
	   "typedef bool mom_%s_sig_t (const momnode_t*nod_mom /* %u inputs, %u outputs: */",
	   suffix, nbinputy, nboutputy);
  for (unsigned ix = 0; ix < nbinputy; ix++)
    {
      momitem_t *itypitm = (momitem_t *) vinputy.vtuple->arritm[ix];
      MOM_DEBUGPRINTF (dump,
		       "emit_signature_application_code ix=%d itypitm=%s", ix,
		       mom_item_cstring (itypitm));
      assert (itypitm != NULL);
      assert (mom_hashset_contains (dumper_mom->duitemglobalset, itypitm));
      assert (itypitm->itm_kind == MOM_PREDEFINED_NAMED (type));
      momvalue_t vccode =	//
	mom_item_unsync_get_attribute (itypitm,
				       MOM_PREDEFINED_NAMED (code));
      assert (vccode.typnum == momty_string);
      fprintf (foutaphd, ",\n\t\t %s arg%d_mom", mom_value_cstr (vccode), ix);
    };
  for (unsigned ix = 0; ix < nboutputy; ix++)
    {
      momitem_t *otypitm = (momitem_t *) voutputy.vtuple->arritm[ix];
      MOM_DEBUGPRINTF (dump,
		       "emit_signature_application_code ix=%d otypitm=%s", ix,
		       mom_item_cstring (otypitm));
      assert (otypitm != NULL);
      assert (mom_hashset_contains (dumper_mom->duitemglobalset, otypitm));
      assert (otypitm->itm_kind == MOM_PREDEFINED_NAMED (type));
      momvalue_t vccode =	//
	mom_item_unsync_get_attribute (otypitm,
				       MOM_PREDEFINED_NAMED (code));
      assert (vccode.typnum == momty_string);
      fprintf (foutaphd, ",\n\t\t  %s* res%d_mom", mom_value_cstr (vccode),
	       ix);
    };
  fputs (");\n\n", foutaphd);
  fprintf (foutaphd,
	   "\n" "#define MOM_PREFIXFUN_%s \"" MOM_FUNCTION_PREFIX "%s\"\n",
	   suffix, mom_value_cstr (vradix));
  fprintf (foutaphd, "static inline mom_%s_sig_t mom_applclos_%s;\n", suffix,
	   suffix);
  fprintf (foutaphd,
	   "static inline bool mom_applval_%s(const momvalue_t clo_mom",
	   suffix);
  for (unsigned ix = 0; ix < nbinputy; ix++)
    {
      momitem_t *itypitm = (momitem_t *) vinputy.vtuple->arritm[ix];
      MOM_DEBUGPRINTF (dump,
		       "emit_signature_application_code ix=%d itypitm=%s", ix,
		       mom_item_cstring (itypitm));
      momvalue_t vccode =	//
	mom_item_unsync_get_attribute (itypitm,
				       MOM_PREDEFINED_NAMED (code));
      fprintf (foutaphd, ",\n\t\t %s arg%d_mom", mom_value_cstr (vccode), ix);
    };
  for (unsigned ix = 0; ix < nboutputy; ix++)
    {
      momitem_t *otypitm = (momitem_t *) voutputy.vtuple->arritm[ix];
      MOM_DEBUGPRINTF (dump,
		       "emit_signature_application_code ix=%d otypitm=%s", ix,
		       mom_item_cstring (otypitm));
      momvalue_t vccode =	//
	mom_item_unsync_get_attribute (otypitm,
				       MOM_PREDEFINED_NAMED (code));
      fprintf (foutaphd, ",\n\t\t  %s* res%d_mom", mom_value_cstr (vccode),
	       ix);
    };
  fputs (")\n{\n", foutaphd);
  fprintf (foutaphd, " if (clo_mom.typnum != momty_node) return false;\n");
  fprintf (foutaphd, " return mom_applclos_%s (clo_mom.vnode", suffix);
  for (unsigned ix = 0; ix < nbinputy; ix++)
    fprintf (foutaphd, ", arg%d_mom", ix);
  for (unsigned ix = 0; ix < nboutputy; ix++)
    fprintf (foutaphd, ", res%d_mom", ix);
  fprintf (foutaphd, ");\n} // end of mom_applval_%s \n", suffix);
  fprintf (foutaphd,
	   "\n" "static inline bool\n"
	   "mom_applclos_%s(const momnode_t* nod_mom", suffix);
  for (unsigned ix = 0; ix < nbinputy; ix++)
    {
      momitem_t *itypitm = (momitem_t *) vinputy.vtuple->arritm[ix];
      MOM_DEBUGPRINTF (dump,
		       "emit_signature_application_code ix=%d itypitm=%s", ix,
		       mom_item_cstring (itypitm));
      momvalue_t vccode =	//
	mom_item_unsync_get_attribute (itypitm,
				       MOM_PREDEFINED_NAMED (code));
      fprintf (foutaphd, ", %s arg%d_mom", mom_value_cstr (vccode), ix);
    };
  for (unsigned ix = 0; ix < nboutputy; ix++)
    {
      momitem_t *otypitm = (momitem_t *) voutputy.vtuple->arritm[ix];
      MOM_DEBUGPRINTF (dump,
		       "emit_signature_application_code ix=%d otypitm=%s", ix,
		       mom_item_cstring (otypitm));
      momvalue_t vccode =	//
	mom_item_unsync_get_attribute (otypitm,
				       MOM_PREDEFINED_NAMED (code));
      fprintf (foutaphd, ",  %s* res%d_mom", mom_value_cstr (vccode), ix);
    };
  fprintf (foutaphd, ")\n{\n bool ok_mom= false; //// generated in %s\n",
	   __FILE__);
  fprintf (foutaphd, "  if (!nod_mom) return false;\n");
  fprintf (foutaphd,
	   "  momitem_t* connitm_mom= (momitem_t*) nod_mom->conn;\n");
  fprintf (foutaphd, "  assert (connitm_mom != NULL);\n");
  fprintf (foutaphd,
	   "  if (MOM_UNLIKELY((const momitem_t*) connitm_mom->itm_kind\n");
  fprintf (foutaphd, "      != MOM_PREDEFINED_NAMED(%s))) goto end_mom;\n",
	   mom_item_cstring (itmpredef));
  fprintf (foutaphd, "  void* data1_mom = connitm_mom->itm_data1;\n");
  fprintf (foutaphd, "  if (MOM_UNLIKELY(data1_mom==NULL)) {");
  fprintf (foutaphd, "    char nambuf_mom[%d];\n",
	   (int) ((2 * strlen (suffix) + 192) | 0x3f) + 1);
  fprintf (foutaphd, "    memset (nambuf_mom, 0, sizeof(nambuf_mom));\n");
  fprintf (foutaphd,
	   "    if (snprintf(nambuf_mom, sizeof(nambuf_mom), \""
	   MOM_FUNCTION_PREFIX "%s__%%s\",\n", mom_value_cstr (vradix));
  fprintf (foutaphd,
	   "                 mom_item_cstring(connitm_mom)) < (int)sizeof(nambuf_mom))\n");
  fprintf (foutaphd,
	   "       ((momitem_t*)connitm_mom)->itm_data1 = data1_mom = mom_dynload_symbol(nambuf_mom);\n");
  fprintf (foutaphd,
	   "    else MOM_FATAPRINTF(\"too long function name %%s for %s\","
	   "  mom_item_cstring(connitm_mom));\n",
	   mom_item_cstring (itmpredef));
  fprintf (foutaphd, "  };\n");
  fprintf (foutaphd,
	   "  if (MOM_LIKELY(data1_mom != NULL && data1_mom != MOM_EMPTY)) {\n");
  fprintf (foutaphd,
	   "     mom_%s_sig_t* fun_mom = (mom_%s_sig_t*) data1_mom;\n",
	   suffix, suffix);
  fprintf (foutaphd, "     ok_mom = (*fun_mom) (nod_mom");
  for (unsigned ix = 0; ix < nbinputy; ix++)
    fprintf (foutaphd, ", arg%d_mom", ix);
  for (unsigned ix = 0; ix < nboutputy; ix++)
    fprintf (foutaphd, ", res%d_mom", ix);
  fprintf (foutaphd, ");\n");
  fprintf (foutaphd, "  };\n");
  fprintf (foutaphd, " end_mom:\n");
  fprintf (foutaphd, "  mom_item_unlock(connitm_mom);\n");
  fprintf (foutaphd, "  return ok_mom;\n");
  fprintf (foutaphd, "} // end of mom_applclos_%s\n\n\n", suffix);
  fflush (foutaphd);
}				/* end of emit_signature_application_code_mom */


static void
emit_signature_application_hook_mom (FILE *foutaphd,
				     momitem_t *itmpredef, momvalue_t vhook)
{
  char prefixbuf[256];
  char applyvalbuf[256];
  char applyclosbuf[256];
  memset (prefixbuf, 0, sizeof (prefixbuf));
  memset (applyvalbuf, 0, sizeof (applyvalbuf));
  memset (applyclosbuf, 0, sizeof (applyclosbuf));
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  momitem_t *itmkindsig = itmpredef->itm_kind;
  MOM_DEBUGPRINTF (dump,
		   "emit_signature_application_hook start itmpredef=%s itmkindsig=%s vhook=%s",
		   mom_item_cstring (itmpredef),
		   mom_item_cstring (itmkindsig),
		   mom_output_gcstring (vhook));
  assert (foutaphd);
  assert (itmpredef && itmpredef->itm_space == momspa_predefined);
  if (!itmkindsig
      || itmkindsig->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    MOM_FATAPRINTF
      ("emit_signature_application_hook itmpredef %s with invalid kind %s",
       mom_item_cstring (itmpredef), mom_item_cstring (itmkindsig));
  const momseq_t *seqins =	//
    mom_value_to_tuple (mom_item_unsync_get_attribute
			((momitem_t *) itmkindsig,
			 MOM_PREDEFINED_NAMED (input_types)));
  const momseq_t *seqouts =	//
    mom_value_to_tuple (mom_item_unsync_get_attribute
			((momitem_t *) itmkindsig,
			 MOM_PREDEFINED_NAMED (output_types)));
  if (!seqins || !seqouts)
    MOM_FATAPRINTF
      ("emit_signature_application_hook itmpredef %s with bad kind %s",
       mom_item_cstring (itmpredef), mom_item_cstring (itmkindsig));
  unsigned nbins = mom_seq_length (seqins);
  unsigned nbouts = mom_seq_length (seqouts);
  fprintf (foutaphd, "static inline bool\n"
	   "momhook_%s(", mom_item_cstring (itmpredef));
  int argcnt = 0;
  for (unsigned ixin = 0; ixin < nbins; ixin++)
    {
      momitem_t *intypitm = (momitem_t *) mom_seq_nth (seqins, ixin);
      if (argcnt++ > 0)
	fputs (", ", foutaphd);
      fprintf (foutaphd, "%s mom_arg%d",
	       mom_string_cstr (mom_value_to_string
				(mom_item_unsync_get_attribute
				 (intypitm, MOM_PREDEFINED_NAMED (code)))),
	       ixin);
    };
  for (unsigned ixout = 0; ixout < nbouts; ixout++)
    {
      momitem_t *outypitm = (momitem_t *) mom_seq_nth (seqouts, ixout);
      if (argcnt++ > 0)
	fputs (", ", foutaphd);
      fprintf (foutaphd, "%s* mom_res%d",
	       mom_string_cstr (mom_value_to_string
				(mom_item_unsync_get_attribute
				 (outypitm, MOM_PREDEFINED_NAMED (code)))),
	       ixout);
    }
  if (nbins == 0 && nbouts == 0)
    fprintf (foutaphd, "void");
  fprintf (foutaphd, ")\n{\n");
  fprintf (foutaphd, "  momvalue_t mom_clos = MOM_NONEV;\n");
  fprintf (foutaphd, "  momitem_t* mom_itm = ");
  if (itmpredef->itm_anonymous)
    fprintf (foutaphd, "  MOM_PREDEFINED_ANONYMOUS(%s);\n",
	     mom_item_cstring (itmpredef));
  else
    fprintf (foutaphd, " MOM_PREDEFINED_NAMED(%s);\n",
	     mom_item_cstring (itmpredef));
  fprintf (foutaphd, "  mom_item_lock(mom_itm);\n");
  if (vhook.typnum == momty_int)
    {
      fprintf (foutaphd,
	       "  mom_clos = mom_components_nth(mom_itm->itm_comps, %d);\n",
	       (int) vhook.vint);
    }
  else if (vhook.typnum == momty_item)
    {
      momitem_t *itmhook = vhook.vitem;
      if (itmhook->itm_space != momspa_predefined)
	MOM_FATAPRINTF ("bad hook item %s for %s", mom_item_cstring (itmhook),
			mom_item_cstring (itmpredef));
      fprintf (foutaphd,
	       "  mom_clos = mom_item_unsync_get_attribute(mom_item, ");
      if (itmhook->itm_anonymous)
	fprintf (foutaphd, "MOM_PREDEFINED_ANONYMOUS(%s)",
		 mom_item_cstring (itmhook));
      else
	fprintf (foutaphd, "MOM_PREDEFINED_NAMED(%s)",
		 mom_item_cstring (itmhook));
      fputs (");\n", foutaphd);
    }
  else
    MOM_FATAPRINTF ("bad hook %s for %s", mom_output_gcstring (vhook),
		    mom_item_cstring (itmpredef));
  fprintf (foutaphd, "  mom_item_unlock(mom_itm);\n");
  const momstring_t *strradix	//
    = mom_value_to_string (mom_item_unsync_get_attribute (itmkindsig,
							  MOM_PREDEFINED_NAMED
							  (function_radix)));
  if (!strradix)
    MOM_FATAPRINTF ("missing radix for hooked signature %s of %s",
		    mom_item_cstring (itmkindsig),
		    mom_item_cstring (itmpredef));
  fprintf (foutaphd, "  return mom_applval_%s(mom_clos", strradix->cstr);
  for (unsigned ixin = 0; ixin < nbins; ixin++)
    fprintf (foutaphd, ", mom_arg%d", (int) ixin);
  for (unsigned ixout = 0; ixout < nbouts; ixout++)
    fprintf (foutaphd, ", mom_res%d", (int) ixout);
  fputs (");", foutaphd);
  fprintf (foutaphd, "\n} // end momhook_%s of %s\n\n",
	   mom_item_cstring (itmpredef), mom_item_cstring (itmkindsig));
}				/* end of emit_signature_application_hook_mom   */



static void
emit_predefined_fill_mom (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  FILE *foutfp = open_generated_file_dump_mom (MOM_FILL_PREDEFINED_PATH);
  mom_output_gplv3_notice (foutfp, "///", "***", MOM_FILL_PREDEFINED_PATH);
  FILE *foutaphd = open_generated_file_dump_mom (MOM_APPLY_HEADER_PATH);
  mom_output_gplv3_notice (foutaphd, "///", "***", MOM_APPLY_HEADER_PATH);
  fprintf (foutfp, "\n" "#include" " " "\"monimelt.h\"\n\n");
  fprintf (foutfp, "void mom_predefined_items_fill (void) {\n");
  const momseq_t *setpredef =
    mom_hashset_elements_set (dumper_mom->dupredefineditemset);
  assert (setpredef);
  unsigned nbpredef = setpredef->slen;
  // first assign the kind
  fputs (" //// assign predefined kinds\n", foutfp);
  for (unsigned ix = 0; ix < nbpredef; ix++)
    {
      const momitem_t *itmpredef = setpredef->arritm[ix];
      assert (itmpredef && itmpredef->itm_space == momspa_predefined);
      MOM_DEBUGPRINTF (dump, "emit_predefined_fill ix#%d itmpredef %s", ix,
		       mom_item_cstring (itmpredef));
      const momitem_t *itmkind = itmpredef->itm_kind;
      if (itmkind && itmkind->itm_space == momspa_predefined)
	{
	  fprintf (foutfp, "// item %s of kind %s\n" " ",
		   mom_item_cstring (itmpredef), mom_item_cstring (itmkind));
	  emit_predefined_itemref_mom (foutfp, itmpredef);
	  fputs ("->itm_kind\n   = ", foutfp);
	  emit_predefined_itemref_mom (foutfp, itmkind);
	  fputs (";\n", foutfp);
	}
    };
  /// then, load into itm_data1 the symbol of functions
  fflush (foutfp);
  for (unsigned ix = 0; ix < nbpredef; ix++)
    {
      const momitem_t *itmpredef = setpredef->arritm[ix];
      assert (itmpredef && itmpredef->itm_space == momspa_predefined);
      const momitem_t *itmkind = itmpredef->itm_kind;
      if (!itmkind)
	continue;
      MOM_DEBUGPRINTF (dump,
		       "emit_predefined_fill ix#%d itmpredef %s kind %s (of kind %s)",
		       ix, mom_item_cstring (itmpredef),
		       mom_item_cstring (itmkind),
		       mom_item_cstring (itmkind->itm_kind) ? : "~");
      if (itmkind->itm_kind == MOM_PREDEFINED_NAMED (function_signature))
	{
	  momvalue_t vradix =
	    mom_item_unsync_get_attribute ((momitem_t *) itmkind,
					   MOM_PREDEFINED_NAMED
					   (function_radix));
	  MOM_DEBUGPRINTF (dump,
			   "emit_predefined_fill ix#%d itmpredef %s vradix %s",
			   ix, mom_item_cstring (itmpredef),
			   mom_output_gcstring (vradix));
	  MOM_DEBUGPRINTF (dump,
			   "emit_predefined_fill ix#%d itmpredef %s",
			   ix, mom_item_cstring (itmpredef));
	  if (vradix.typnum == momty_string)
	    {
	      MOM_DEBUGPRINTF (dump,
			       "emit_predefined_fill ix#%d itmpredef=%s itmkind=%s",
			       ix,
			       mom_item_cstring (itmpredef),
			       mom_item_cstring (itmkind));
	      fprintf (foutfp, "// function item %s of %s:\n",
		       mom_item_cstring (itmpredef),
		       mom_item_cstring (itmkind));
	      emit_predefined_itemref_mom (foutfp, itmpredef);
	      fprintf (foutfp,
		       "->itm_data1 =\n     mom_dynload_symbol(\"momfunc_%s__%s\");\n",
		       mom_value_cstr (vradix), mom_item_cstring (itmpredef));
	    }
	}

      if (itmpredef->itm_kind == MOM_PREDEFINED_NAMED (function_signature))
	{
	  momvalue_t vradix =
	    mom_item_unsync_get_attribute ((momitem_t *) itmpredef,
					   MOM_PREDEFINED_NAMED
					   (function_radix));
	  momvalue_t vinputy =
	    mom_item_unsync_get_attribute ((momitem_t *) itmpredef,
					   MOM_PREDEFINED_NAMED
					   (input_types));
	  momvalue_t voutputy =
	    mom_item_unsync_get_attribute ((momitem_t *) itmpredef,
					   MOM_PREDEFINED_NAMED
					   (output_types));
	  MOM_DEBUGPRINTF (dump,
			   "emit_predefined_fill ix#%d itmpredef %s vradix %s vinputy %s voutputy %s",
			   ix, mom_item_cstring (itmpredef),
			   mom_output_gcstring (vradix),
			   mom_output_gcstring (vinputy),
			   mom_output_gcstring (voutputy));
	  if (vinputy.typnum == momty_tuple && voutputy.typnum == momty_tuple
	      && vradix.typnum == momty_string)
	    {
	      MOM_DEBUGPRINTF (dump,
			       "emit_predefined_fill ix#%d itmpredef %s emitting signature app.code",
			       ix, mom_item_cstring (itmpredef));
	      emit_signature_application_code_mom (foutaphd,
						   (momitem_t *) itmpredef,
						   vradix, vinputy, voutputy);

	    }
	}
    }
  fprintf (foutfp, "\n} /* end mom_predefined_items_fill */\n");
  fflush (foutfp);
  for (unsigned ix = 0; ix < nbpredef; ix++)
    {
      const momitem_t *itmpredef = setpredef->arritm[ix];
      assert (itmpredef && itmpredef->itm_space == momspa_predefined);
      const momitem_t *itmkind = itmpredef->itm_kind;
      MOM_DEBUGPRINTF (dump,
		       "emit_predefined_fill ix#%d itmpredef %s itmkind %s for hook",
		       ix, mom_item_cstring (itmpredef),
		       mom_item_cstring (itmkind));
      if (!itmkind)
	continue;
      if (itmkind->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
	continue;
      MOM_DEBUGPRINTF (dump,
		       "emit_predefined_fill ix#%d itmpredef %s for hook",
		       ix, mom_item_cstring (itmpredef));
      momvalue_t vhook =	//
	mom_item_unsync_get_attribute ((momitem_t *) itmpredef,
				       MOM_PREDEFINED_NAMED (hook_closure));
      MOM_DEBUGPRINTF (dump,
		       "emit_predefined_fill ix#%d itmpredef %s vhook %s",
		       ix, mom_item_cstring (itmpredef),
		       mom_output_gcstring (vhook));
      if (vhook.typnum == momty_int || vhook.typnum == momty_node)
	emit_signature_application_hook_mom (foutaphd,
					     (momitem_t *) itmpredef, vhook);
    }
  fflush (foutfp);
  fprintf (foutfp, "\n // end of generated file %s\n",
	   MOM_FILL_PREDEFINED_PATH);
  close_generated_file_dump_mom (foutfp, MOM_FILL_PREDEFINED_PATH);
  foutfp = NULL;
  //
  fprintf (foutaphd, "\n // end of generated apply-header file %s\n",
	   MOM_APPLY_HEADER_PATH);
  close_generated_file_dump_mom (foutaphd, MOM_APPLY_HEADER_PATH);
  foutaphd = NULL;
}				/* end of emit_predefined_fill_mom */

////////////////
static void
emit_content_dumped_item_mom (const momitem_t *itm)
{				//// keep in sync with load_fill_item_mom
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (itm && itm != MOM_EMPTY);
  unsigned nbat = mom_attributes_count (itm->itm_attrs);
  unsigned nbcomp = mom_components_count (itm->itm_comps);
  MOM_DEBUGPRINTF (dump,
		   "emit_content_dumped_item start %s nbat %u nbcomp %u kind %s",
		   mom_item_cstring (itm), nbat, nbcomp,
		   mom_item_cstring (itm->itm_kind));
  if (nbat > 0)
    {
      fputs ("{", dumper_mom->dufile);
      const momseq_t *setat = mom_attributes_set (itm->itm_attrs, MOM_NONEV);
      MOM_DEBUGPRINTF (dump,
		       "emit_content_dumped_item item %s attributes set %s",
		       mom_item_cstring (itm),
		       mom_output_gcstring (mom_unsafe_setv (setat)));
      if (setat)
	for (unsigned ix = 0; ix < setat->slen; ix++)
	  {
	    const momitem_t *itmat = setat->arritm[ix];
	    MOM_DEBUGPRINTF (dump,
			     "emit_content_dumped_item item %s attribute %s ix#%u",
			     mom_item_cstring (itm), mom_item_cstring (itmat),
			     ix);
	    struct momentry_st *ent =
	      mom_attributes_find_entry (itm->itm_attrs, itmat);
	    if (!ent)
	      {
		MOM_WARNPRINTF
		  ("in content of dumped item %s missing attribute %s",
		   mom_item_cstring (itm), mom_item_cstring (itmat));
		continue;
	      }
	    momvalue_t *paval = &ent->ent_val;
	    MOM_DEBUGPRINTF (dump,
			     "emit_content_dumped_item entry itmat=%s val=%s",
			     mom_item_cstring (itmat),
			     mom_output_gcstring (*paval));
	    if (!mom_dumpable_item (itmat))
	      {
		MOM_DEBUGPRINTF (dump,
				 "emit_content_dumped_item non dumpable attribute %s",
				 mom_item_cstring (itmat));
		continue;
	      }
	    if (!mom_dumpable_valueptr (paval))
	      {
		MOM_DEBUGPRINTF (dump,
				 "emit_content_dumped_item non dumpable value %s of attribute %s",
				 mom_output_gcstring (*paval),
				 mom_item_cstring (itmat));
		continue;
	      }
	    dumper_mom->duindentation = 1;
	    mom_emit_dumped_newline ();
	    fputs ("* ", dumper_mom->dufile);
	    MOM_DEBUGPRINTF (dump,
			     "emit_content_dumped_item doing entry itmat=%s aval=%s",
			     mom_item_cstring (itmat),
			     mom_output_gcstring (*paval));
	    mom_emit_dumped_itemref (itmat);
	    mom_emit_dumped_space ();
	    mom_emit_dumped_valueptr (paval);
	  };
      dumper_mom->duindentation = 0;
      mom_emit_dumped_space ();
      fputs ("}", dumper_mom->dufile);
      mom_emit_dumped_newline ();
    }
  else
    MOM_DEBUGPRINTF (dump,
		     "emit_content_dumped_item no attributes for item %s",
		     mom_item_cstring (itm));
  MOM_DEBUGPRINTF (dump, "emit_content_dumped_item item %s nbcomp %u",
		   mom_item_cstring (itm), nbcomp);
  if (nbcomp > 0)
    {
      fputs ("[[", dumper_mom->dufile);
      mom_emit_dump_indent ();
      for (unsigned ix = 0; ix < nbcomp; ix++)
	{
	  dumper_mom->duindentation = 1;
	  mom_emit_dumped_space ();
	  const momvalue_t *pvalcomp =
	    mom_raw_item_get_indexed_component_ptr ((momitem_t *) itm, ix);
	  MOM_DEBUGPRINTF (dump, "emit_content_dumped_item %s comp#%d %s",
			   mom_item_cstring (itm), ix,
			   mom_output_gcstring (*pvalcomp));
	  mom_emit_dumped_valueptr (pvalcomp);
	}
      dumper_mom->duindentation = 0;
      mom_emit_dumped_space ();
      fputs ("]]", dumper_mom->dufile);
      mom_emit_dumped_newline ();
    }
  else
    MOM_DEBUGPRINTF (dump,
		     "emit_content_dumped_item no components for item %s",
		     mom_item_cstring (itm));
  momitem_t *itmkd = (momitem_t *) itm->itm_kind;
  MOM_DEBUGPRINTF (dump, "emit_content_dumped_item item %s of kind %s",
		   mom_item_cstring (itm), mom_item_cstring (itmkd));
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
	    mom_item_unsync_get_attribute	//
	    (itmkd,		//
	     MOM_PREDEFINED_NAMED (dumped_item_emitter));
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
      if (mom_applval_1itm_to_val
	  (valemitter, (momitem_t *) itm, &valtransformer)
	  && valtransformer.typnum == momty_node)
	{
	  dumper_mom->duindentation = 1;
	  fputs ("% ", dumper_mom->dufile);
	  mom_emit_dumped_valueptr (&valtransformer);
	}
      dumper_mom->duindentation = 0;
      mom_emit_dumped_newline ();
    }
  else
    MOM_DEBUGPRINTF (dump, "emit_content_dumped_item no kind for item %s",
		     mom_item_cstring (itm));
  dumper_mom->duindentation = 0;
  MOM_DEBUGPRINTF (dump, "emit_content_dumped_item done item %s\n",
		   mom_item_cstring (itm));
}				/* end emit_content_dumped_item_mom */



////////////////
static void
emit_dumped_item_mom (const momitem_t *itm)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  assert (itm != NULL);
  assert (itm->itm_space == momspa_predefined
	  || itm->itm_space == momspa_global
	  || itm->itm_space == momspa_user);
  bool isuser = itm->itm_space == momspa_user;
  MOM_DEBUGPRINTF (dump, "emit_dumped_item start %s", mom_item_cstring (itm));
  if (!mom_hashset_contains (dumper_mom->duitemuserset, itm)
      && !mom_hashset_contains (dumper_mom->duitemglobalset, itm))
    return;
  putc ('\n', dumper_mom->dufile);
  if (mom_hashset_contains (dumper_mom->dupredefineditemset, itm))
    fprintf (dumper_mom->dufile, "** %s   ////// PREDEFINED\n",
	     mom_item_cstring (itm));
  else if (isuser)
    fprintf (dumper_mom->dufile, "*: %s\n", mom_item_cstring (itm));
  else
    fprintf (dumper_mom->dufile, "** %s\n", mom_item_cstring (itm));
  dumper_mom->duindentation = 0;
  dumper_mom->dulastnloff = ftell (dumper_mom->dufile);
  emit_content_dumped_item_mom (itm);
  fputs ("\n..\n\n", dumper_mom->dufile);
  dumper_mom->duindentation = 0;
  dumper_mom->dulastnloff = ftell (dumper_mom->dufile);
  MOM_DEBUGPRINTF (dump, "emit_dumped_item done %s\n",
		   mom_item_cstring (itm));
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
  if (mom_hashset_contains (dumper_mom->duitemuserset, itm)
      || mom_hashset_contains (dumper_mom->duitemglobalset, itm))
    return true;
  MOM_DEBUGPRINTF (dump, "non-dumpable item %s", mom_item_cstring (itm));
  return false;
}				/* end of mom_dumpable_item */

bool
mom_dumpable_valueptr (const momvalue_t *pval)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  if (!pval || pval == MOM_EMPTY)
    return false;
  assert (dumper_mom->dustate == dump_emit);
  if (mom_valueptr_is_transient (pval))
    return false;
  switch ((enum momvaltype_en) pval->typnum)
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
      return (mom_dumpable_item (pval->vitem));
    case momty_node:
      {
	const momnode_t *nod = pval->vnode;
	assert (nod);
	return (mom_dumpable_item (nod->conn));
      }
    }
  return false;
}				/* end of mom_dumpable_valueptr */


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
	      int nbc = (nl > eoc) ? (nl - comstr - 1) : (eoc - comstr - 1);
	      if (nbc >= (int) sizeof (combuf))
		nbc = sizeof (combuf) - 1;
	      strncpy (combuf, comstr, nbc);
	    }
	  else if (nl && nl >= comstr + 3)
	    {
	      int nbc = (nl - comstr - 1);
	      if (nbc >= (int) sizeof (combuf))
		nbc = sizeof (combuf) - 1;
	      strncpy (combuf, comstr, nbc);
	    }
	  else if (eoc && eoc >= comstr + 3)
	    {
	      int nbc = (eoc - comstr - 1);
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


/// similar to mom_emit_dumped_valueptr, except that we don't care about
/// non-dumpable items, etc...
static void
output_val_mom (struct momvaloutput_st *ov, const momvalue_t val)
{
  assert (ov && ov->vout_magic == VALOUT_MAGIC_MOM);
  switch ((enum momvaltype_en) (val.typnum))
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
	return;
      }
    case momty_int:
      fprintf (ov->vout_file, "%lld", (long long) val.vint);
      return;
    case momty_delim:
      {
	char dbuf[8 + sizeof (val.vdelim)];
	memset (dbuf, 0, sizeof (dbuf));
	strncpy (dbuf, val.vdelim.delim, sizeof (val.vdelim));
	fputs ("° \"", ov->vout_file);
	mom_output_utf8cstr_cencoded (ov->vout_file, dbuf, -1);
	fputs ("\"", ov->vout_file);
      }
      return;
    case momty_string:
      fputs ("\"", ov->vout_file);
      mom_output_utf8cstr_cencoded (ov->vout_file, val.vstr->cstr, -1);
      fputs ("\"", ov->vout_file);
      return;
    case momty_item:
      assert (val.vitem);
      output_item_mom (ov, val.vitem);
      return;
    case momty_tuple:
      {
	const momseq_t *tup = val.vtuple;
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
	    if (ix % 5 == 0 && ix > 0)
	      output_space_mom (ov);
	    output_item_mom (ov, tup->arritm[ix]);
	    something = true;
	  }
	output_outdent_mom (ov);
	fputs ("]", ov->vout_file);
      }
      return;
    case momty_set:
      {
	const momseq_t *tup = val.vtuple;
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
	    if (ix % 5 == 0 && ix > 0)
	      output_space_mom (ov);
	    output_item_mom (ov, tup->arritm[ix]);
	    something = true;
	  }
	output_outdent_mom (ov);
	fputs ("}", ov->vout_file);
      }
      return;
    case momty_node:
      {
	const momnode_t *nod = val.vnode;
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
      return;
    default:
      fprintf (ov->vout_file, "/*strange type#%d*/ ", (int) (val.typnum));
    }
}				/* end output_val_mom */


void				// see also mom_load_value
mom_emit_dumped_valueptr (const momvalue_t *pval)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  assert (dumper_mom->dufile);
  if (!pval || pval == MOM_EMPTY)
    return;
  mom_emit_dumped_space ();
  if (mom_valueptr_is_transient (pval) || !mom_dumpable_valueptr (pval))
    goto emit_null;
  switch ((enum momvaltype_en) pval->typnum)
    {
    case momty_null:
    emit_null:
      fputs ("~", dumper_mom->dufile);
      return;
    case momty_double:
      {
	double x = pval->vdbl;
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
      fprintf (dumper_mom->dufile, "%lld", (long long) pval->vint);
      break;
    case momty_delim:
      {
	char dbuf[8 + sizeof (pval->vdelim)];
	memset (dbuf, 0, sizeof (dbuf));
	strncpy (dbuf, pval->vdelim.delim, sizeof (pval->vdelim));
	fputs ("° \"", dumper_mom->dufile);
	mom_output_utf8cstr_cencoded (dumper_mom->dufile, dbuf, -1);
	fputs ("\"", dumper_mom->dufile);
      }
      break;
    case momty_string:
      fputs ("\"", dumper_mom->dufile);
      mom_output_utf8cstr_cencoded (dumper_mom->dufile, pval->vstr->cstr, -1);
      fputs ("\"", dumper_mom->dufile);
      break;
    case momty_item:
      {
	momitem_t *itm = pval->vitem;
	assert (itm && itm->itm_str);
	if (mom_hashset_contains (dumper_mom->duitemuserset, itm)
	    || mom_hashset_contains (dumper_mom->duitemglobalset, itm))
	  fputs (itm->itm_str->cstr, dumper_mom->dufile);
	else
	  fputs ("~", dumper_mom->dufile);
      }
      break;
    case momty_tuple:
      {
	const momseq_t *tup = pval->vtuple;
	bool something = false;
	assert (tup);
	fputs ("[", dumper_mom->dufile);
	mom_emit_dump_indent ();
	if (mom_dumpable_valueptr (&tup->meta))
	  {
	    mom_emit_dumped_space ();
	    fputs ("!", dumper_mom->dufile);
	    mom_emit_dumped_valueptr (&tup->meta);
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
	const momseq_t *tup = pval->vtuple;
	bool something = false;
	assert (tup);
	fputs ("{", dumper_mom->dufile);
	mom_emit_dump_indent ();
	if (mom_dumpable_valueptr (&tup->meta))
	  {
	    mom_emit_dumped_space ();
	    fputs ("!", dumper_mom->dufile);
	    mom_emit_dumped_valueptr (&tup->meta);
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
	const momnode_t *nod = pval->vnode;
	assert (nod);
	unsigned ln = nod->slen;
	if (!mom_dumpable_item (nod->conn))
	  goto emit_null;
	fputs ("^", dumper_mom->dufile);
	mom_emit_dumped_itemref (nod->conn);
	if (mom_dumpable_valueptr (&nod->meta))
	  {
	    mom_emit_dumped_space ();
	    fputs ("!", dumper_mom->dufile);
	    mom_emit_dumped_valueptr (&nod->meta);
	  }
	fputs ("(", dumper_mom->dufile);
	mom_emit_dump_indent ();
	for (unsigned ix = 0; ix < ln; ix++)
	  {
	    if (ix > 0)
	      mom_emit_dumped_space ();
	    mom_emit_dumped_valueptr (&nod->arrsons[ix]);
	  }
	mom_emit_dump_outdent ();
	fputs (")", dumper_mom->dufile);
      }
      break;
    }
}				/* end mom_emit_dumped_valueptr */


static int
emit_global_items_mom (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  const momseq_t *set =
    mom_hashset_elements_set (dumper_mom->duitemglobalset);
  assert (set);
  MOM_DEBUGPRINTF (dump, "global items set %s",
		   mom_output_gcstring (mom_unsafe_setv (set)));
  unsigned nbel = set->slen;
  for (unsigned ix = 0; ix < nbel; ix++)
    emit_dumped_item_mom (set->arritm[ix]);
  return (int) nbel;
}

static int
emit_global_modules_mom (const momseq_t *setmod)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  if (!setmod)
    return 0;
  unsigned nbmod = setmod->slen;
  int modcnt = 0;
  for (unsigned ix = 0; ix < nbmod; ix++)
    {
      const momitem_t *moditm = setmod->arritm[ix];
      assert (moditm && moditm != MOM_EMPTY);
      if (moditm->itm_space == momspa_predefined
	  || moditm->itm_space == momspa_global)
	{
	  fprintf (dumper_mom->dufile, "!! %s\n", mom_item_cstring (moditm));
	  modcnt++;
	}
    }
  if (modcnt > 0)
    fputc ('\n', dumper_mom->dufile);
  return modcnt;
}

static int
emit_user_items_mom (void)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  assert (dumper_mom->dustate == dump_emit);
  const momseq_t *set = mom_hashset_elements_set (dumper_mom->duitemuserset);
  MOM_DEBUGPRINTF (dump, "user items set %s",
		   mom_output_gcstring (mom_unsafe_setv (set)));
  if (!set)
    return 0;
  unsigned nbel = set->slen;
  for (unsigned ix = 0; ix < nbel; ix++)
    emit_dumped_item_mom (set->arritm[ix]);
  return (int) nbel;
}

static int
emit_user_modules_mom (const momseq_t *setmod)
{
  assert (dumper_mom && dumper_mom->dumagic == DUMPER_MAGIC_MOM);
  if (!setmod)
    return 0;
  int modcnt = 0;
  unsigned nbmod = setmod->slen;
  for (unsigned ix = 0; ix < nbmod; ix++)
    {
      const momitem_t *moditm = setmod->arritm[ix];
      assert (moditm && moditm != MOM_EMPTY);
      if (moditm->itm_space == momspa_user)
	{
	  fprintf (dumper_mom->dufile, "!! %s\n", mom_item_cstring (moditm));
	  modcnt++;
	}
    }
  if (modcnt > 0)
    fputc ('\n', dumper_mom->dufile);
  return modcnt;
}


void
mom_dump_state (const char *prefix)
{
  double startelapsedtime = mom_elapsed_real_time ();
  double startcputime = mom_clock_time (CLOCK_THREAD_CPUTIME_ID);
  struct momdumper_st dmp;
  memset (&dmp, 0, sizeof (dmp));
  dmp.dumagic = DUMPER_MAGIC_MOM;
  dmp.duprefix = prefix;
  MOM_DEBUGPRINTF (dump, "start dumping prefix %s", prefix);
  {
    char sufbuf[64];
    memset (sufbuf, 0, sizeof (sufbuf));
    snprintf (sufbuf, sizeof (sufbuf), "+p%d-r%u.tmp", (int) getpid (),
	      (unsigned) mom_random_nonzero_32_here ());
    dmp.durandsuffix = MOM_GC_STRDUP ("random suffix", sufbuf);
    MOM_DEBUGPRINTF (dump, "dumping random suffix %s", dmp.durandsuffix);
  }
  if (prefix && prefix[0])
    {
      int preflen = strlen (prefix);
      if (preflen > 100)
	MOM_FATAPRINTF ("too long dump prefix %s", prefix);
      if (preflen > 1 && prefix[preflen - 1] == '/')
	{
	  MOM_DEBUGPRINTF (dump, "dumping into directory prefix %s", prefix);
	  if (access (prefix, F_OK) && errno == ENOENT)
	    {
	      if (!mkdir (prefix, 0750 /* drwxr-x--- */ ))
		MOM_INFORMPRINTF ("made dump directory %s", prefix);
	      else
		MOM_FATAPRINTF ("failed to make dump directory %s - %m",
				prefix);
	    }
	  else
	    MOM_INFORMPRINTF ("will dump into existing directory %s", prefix);
	}
    }
  dmp.dustate = dump_scan;
  dumper_mom = &dmp;
  MOM_DEBUGPRINTF (dump, "before scanning predefined items");
  scan_predefined_items_mom ();
  int nbscan = 0;
  MOM_DEBUGPRINTF (dump, "before dumpscan loop");
  while (mom_queueitem_size (&dmp.duitemque) > 0)
    {
      const momitem_t *curitm = mom_queueitem_pop_front (&dmp.duitemque);
      scan_inside_dumped_item_mom ((momitem_t *) curitm);
      nbscan++;
    }
  MOM_DEBUGPRINTF (dump, "scanned %d items", nbscan);
  emit_predefined_header_mom ();
  emit_predefined_fill_mom ();
  MOM_INFORMPRINTF ("dumped state to prefix %s : %u global + %u user items",
		    prefix, mom_hashset_count (dmp.duitemglobalset),
		    mom_hashset_count (dmp.duitemuserset));
  ////
  dmp.dustate = dump_emit;
  int nbglomod = 0, nbgloitm = 0;
  {
    dmp.dufile = open_generated_file_dump_mom (MOM_GLOBAL_DATA_PATH);
    mom_output_gplv3_notice (dmp.dufile, "///", "", MOM_GLOBAL_DATA_PATH);
    fputc ('\n', dmp.dufile);
    const momseq_t *setmod = mom_hashset_elements_set (dmp.duitemmoduleset);
    nbglomod = emit_global_modules_mom (setmod);
    nbgloitm = emit_global_items_mom ();
    fprintf (dmp.dufile, "//// end of global file %s\n",
	     MOM_GLOBAL_DATA_PATH);
    close_generated_file_dump_mom (dmp.dufile, MOM_GLOBAL_DATA_PATH);
    MOM_DEBUGPRINTF (dump, "emitted %d global modules and %d global items",
		     nbglomod, nbgloitm);
    dmp.dufile = NULL;
  }
  int nbusrmod = 0, nbusritm = 0;
  {
    dmp.dufile = open_generated_file_dump_mom (MOM_USER_DATA_PATH);
    mom_output_gplv3_notice (dmp.dufile, "///", "", MOM_USER_DATA_PATH);
    fputc ('\n', dmp.dufile);
    const momseq_t *setmod = mom_hashset_elements_set (dmp.duitemmoduleset);
    nbusrmod = emit_user_modules_mom (setmod);
    nbusritm = emit_user_items_mom ();
    fprintf (dmp.dufile, "//// end of user file %s\n", MOM_USER_DATA_PATH);
    close_generated_file_dump_mom (dmp.dufile, MOM_USER_DATA_PATH);
    MOM_DEBUGPRINTF (dump, "emitted %d user modules and %d user items",
		     nbusrmod, nbusritm);
    dmp.dufile = NULL;
  }
  // unlock all the dumped items
  {
    const momseq_t *userset =
      mom_hashset_elements_set (dumper_mom->duitemuserset);
    const momseq_t *globset =
      mom_hashset_elements_set (dumper_mom->duitemglobalset);
    unsigned nbuseritems = userset ? userset->slen : 0;
    for (unsigned ix = 0; ix < nbuseritems; ix++)
      mom_item_unlock ((momitem_t *) userset->arritm[ix]);
    unsigned nbglobitems = globset ? globset->slen : 0;
    for (unsigned ix = 0; ix < nbglobitems; ix++)
      mom_item_unlock ((momitem_t *) globset->arritm[ix]);
  }
  double endelapsedtime = mom_elapsed_real_time ();
  double endcputime = mom_clock_time (CLOCK_THREAD_CPUTIME_ID);
  MOM_INFORMPRINTF
    ("done dumping in %s, %.3f cpu, %.3f elapsed time in seconds", prefix,
     endcputime - startcputime, endelapsedtime - startelapsedtime);
  MOM_INFORMPRINTF
    ("dumped global %d modules & %d items, user %d modules & %d items",
     nbglomod, nbgloitm, nbusrmod, nbusritm);
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
}				/* end mom_output_utf8cstr_cencoded */



void
mom_output_utf8html_cencoded (FILE *fil, const char *str, int len, bool usebr)
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
	  fputs ("&quot;", fil);
	  break;
	case (ucs4_t) '\'':
	  fputs ("&apos;", fil);
	  break;
	case (ucs4_t) '<':
	  fputs ("&lt;", fil);
	  break;
	case (ucs4_t) '>':
	  fputs ("&gt;", fil);
	  break;
	case (ucs4_t) '&':
	  fputs ("&amp;", fil);
	  break;
	case (ucs4_t) '\n':
	  if (usebr)
	    fputs ("<br/>", fil);
	  else
	    fputc ('\n', fil);
	  break;
	default:
	  if (uc >= (ucs4_t) ' ' && uc <= 0x7f && isprint ((char) (uc)))
	    fputc (uc, fil);
	  else
	    fprintf (fil, "&#%d;", (unsigned) uc);
	}
      pc += lc;
    }
}				/* end mom_output_utf8html_cencoded */
