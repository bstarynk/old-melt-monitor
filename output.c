// file output.c

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

static pthread_mutex_t renamcontchg_mtx_mom = PTHREAD_MUTEX_INITIALIZER;
bool
mom_rename_if_content_changed (const char *origpath, const char *destpath)
{
  FILE *origfil = NULL;
  FILE *destfil = NULL;
  bool samecontents = false;
  assert (origpath && origpath[0]);
  assert (destpath && destpath[0]);
  assert (strlen (origpath) < MOM_PATH_MAX - 1);
  assert (strlen (destpath) < MOM_PATH_MAX - 1);
  assert (strcmp (origpath, destpath) != 0);
  pthread_mutex_lock (&renamcontchg_mtx_mom);
  destfil = fopen (destpath, "r");
  if (!destfil)
    {
      if (rename (origpath, destpath))
	MOM_FATAPRINTF ("failed to rename %s as new %s", origpath, destpath);
      goto end;
    };
  flock (fileno (destfil), LOCK_EX);
  origfil = fopen (origpath, "r");
  if (!origfil)
    MOM_FATAPRINTF ("failed to open original file %s to be renamed as %s",
		    origpath, destpath);
  flock (fileno (origfil), LOCK_EX);
  struct stat origstat, deststat;
  memset (&origstat, 0, sizeof (struct stat));
  memset (&deststat, 0, sizeof (struct stat));
  if (fstat (fileno (origfil), &origstat))
    MOM_FATAPRINTF ("failed to fstat fd#%d for original %s", fileno (origfil),
		    origpath);
  if (fstat (fileno (destfil), &deststat))
    MOM_FATAPRINTF ("failed to fstat fd#%d for destination %s",
		    fileno (destfil), destpath);
  if (origstat.st_size != deststat.st_size)
    samecontents = false;
  else
    {
      samecontents = true;
      do
	{
	  int oc = getc (origfil);
	  int dc = getc (destfil);
	  samecontents = (oc == dc);
	  if (oc == EOF || dc == EOF)
	    break;
	}
      while (samecontents);
    };
  if (samecontents)
    {
      if (remove (origpath))
	MOM_FATAPRINTF ("failed to remove original %s same as destination %s",
			origpath, destpath);
    }
  else
    {				// different content
      char backpath[MOM_PATH_MAX];
      memset (backpath, 0, sizeof (backpath));
      snprintf (backpath, sizeof (backpath), "%s~", destpath);
      if (rename (destpath, backpath))
	MOM_FATAPRINTF ("failed to backup %s as %s", destpath, backpath);
      if (rename (origpath, destpath))
	MOM_FATAPRINTF ("failed to rename %s different as destination %s",
			origpath, destpath);
    }
end:
  if (origfil)
    {
      flock (fileno (origfil), LOCK_UN);
      fclose (origfil);
    };
  if (destfil)
    {
      flock (fileno (origfil), LOCK_UN);
      fclose (destfil);
    };
  pthread_mutex_unlock (&renamcontchg_mtx_mom);
  return samecontents;
}


void
mom_copy_file (const char *origpath, const char *destpath)
{
  static int count;
  char tempath[MOM_PATH_MAX + 40];
  memset (tempath, 0, sizeof (tempath));
  if (!origpath || !destpath)
    return;
  if (!origpath[0] || !destpath[0] || strlen (destpath) >= MOM_PATH_MAX)
    MOM_FATAPRINTF
      ("bad arguments to mom_copy_file: origpath=%s, destpath=%s", origpath,
       destpath);
  {
    static char cpbuf[8192];
    pthread_mutex_lock (&renamcontchg_mtx_mom);
    count++;
    snprintf (tempath, sizeof (tempath), "%s-cp%d-p%d-r%x", destpath, count,
	      (int) getpid (),
	      (int) mom_random_nonzero_32 () & (INT_MAX / 2));
    FILE *ftmp = fopen (tempath, "w");
    if (!ftmp)
      MOM_FATAPRINTF
	("mom_copy_file failed to open temporary destination %s with orig %s",
	 tempath, origpath);
    FILE *forig = fopen (origpath, "r");
    size_t nr = 0;
    if (!forig)
      MOM_FATAPRINTF ("mom_copy_file failed to open original file %s",
		      origpath);
    do
      {
	nr = 0;
	memset (cpbuf, 0, sizeof (cpbuf));
	nr = fread (cpbuf, 1, sizeof (cpbuf), forig);
	if (nr < 0)
	  MOM_FATAPRINTF ("mom_copy_file fail to read %s (%m)", origpath);
	else if (nr > 0)
	  {
	    size_t nw = fwrite (cpbuf, 1, nr, ftmp);
	    if (nw < nr)
	      MOM_FATAPRINTF ("mom_copy_file fail to write %s (%m)", tempath);
	  }
      }
    while (!feof (forig) && nr > 0);
    fclose (forig);
    if (fclose (ftmp))
      MOM_FATAPRINTF ("mom_copy_file fail to close written %s (%m)",
		      tempath);;
    pthread_mutex_unlock (&renamcontchg_mtx_mom);
  }
  mom_rename_if_content_changed (tempath, destpath);
}

void
mom_initialize_buffer_output (struct momout_st *out, unsigned flags)
{
  if (!out)
    return;
  memset (out, 0, sizeof (struct momout_st));
  out->mout_magic = MOM_MOUT_MAGIC;
  out->mout_file =
    open_memstream ((char **) &out->mout_data, &out->mout_size);
  if (!out->mout_file)
    MOM_FATAPRINTF ("failed to initialize buffer output");
  out->mout_flags = flags | outf_isbuffer;
}

void
mom_finalize_buffer_output (struct momout_st *out)
{
  if (!out)
    return;
  assert (out->mout_magic == MOM_MOUT_MAGIC);
  assert (out->mout_flags & outf_isbuffer);
  if (out->mout_file)
    fclose (out->mout_file), out->mout_file = NULL;
  if (out->mout_data)
    free (out->mout_data), out->mout_data = NULL;
  memset (out, 0, sizeof (struct momout_st));
}

void
mom_out_at (const char *sfil, int lin, momout_t *pout, ...)
{
  va_list alist;
  if (!sfil || !pout)
    return;
  va_start (alist, pout);
  mom_outva_at (sfil, lin, pout, alist);
  va_end (alist);
}


static void
output_json_mom (momout_t *pout, momval_t v)
{
  assert (pout && pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  assert (mom_is_jsonable (v));
  if (!v.ptr)
    {
      fputs ("null", out);
      return;
    }
  momtynum_t typ = *v.ptype;
  switch ((enum momvaltype_en) typ)
    {
    case momty_item:
      if (v.pitem == mom_named__json_true)
	fputs ("true", out);
      else if (v.pitem == mom_named__json_false)
	fputs ("false", out);
      else
	{
	  const momstring_t *nams = mom_item_get_name_or_idstr (v.pitem);
	  assert (nams && nams->typnum == momty_string);
	  if ((pout->mout_flags & outf_cname) && isalpha (nams->cstr[0]))
	    fputs (nams->cstr, out);
	  else
	    fprintf (out, "\"%s\"", nams->cstr);
	}
      break;
    case momty_int:
      fprintf (out, "%lld", (long long) v.pint->intval);
      break;
    case momty_string:
      putc ('"', out);
      MOM_OUT (pout, MOMOUT_JS_STRING ((const char *) v.pstring->cstr));
      putc ('"', out);
      break;
    case momty_double:
      {
	double x = v.pdouble->dblval;
	char numbuf[48];
	memset (numbuf, 0, sizeof (numbuf));
	snprintf (numbuf, sizeof (numbuf), "%.3f", x);
	if (atof (numbuf) == x && strlen (numbuf) < 20
	    && strchr (numbuf, '.'))
	  {
	    fputs (numbuf, out);
	    return;
	  }
	snprintf (numbuf, sizeof (numbuf), "%.6f", x);
	if ((atof (numbuf) == x || (pout->mout_flags & outf_shortfloat))
	    && strlen (numbuf) < 30 && strchr (numbuf, '.'))
	  {
	    fputs (numbuf, out);
	    return;
	  }
	snprintf (numbuf, sizeof (numbuf), "%#.6g", x);
	if ((atof (numbuf) == x || (pout->mout_flags & outf_shortfloat))
	    && strlen (numbuf) < 30 && strchr (numbuf, '.'))
	  {
	    fputs (numbuf, out);
	    return;
	  }
	snprintf (numbuf, sizeof (numbuf), "%.9f", x);
	if (atof (numbuf) == x && strlen (numbuf) < 30
	    && strchr (numbuf, '.'))
	  {
	    fputs (numbuf, out);
	    return;
	  }
	snprintf (numbuf, sizeof (numbuf), "%#.10g", x);
	if (atof (numbuf) == x && strlen (numbuf) < 30
	    && strchr (numbuf, '.'))
	  {
	    fputs (numbuf, out);
	    return;
	  }
	if (isnan (x))
	  {
	    if (pout->mout_flags & outf_cname)
	      fputs ("nan", out);
	    else
	      fputs ("null", out);
	  }
	snprintf (numbuf, sizeof (numbuf), "%#.15g", x);
	fputs (numbuf, out);
	return;
      }
      break;
    case momty_jsonobject:
      {
	unsigned joblen = v.pjsonobj->slen;
	putc ('{', out);
	if (pout->mout_flags & (outf_jsonhalfindent | outf_jsonindent))
	  MOM_OUT (pout, MOMOUT_INDENT_MORE ());
	for (unsigned ix = 0; ix < joblen; ix++)
	  {
	    if (ix > 0)
	      {
		putc (',', out);
		if (pout->mout_flags & outf_jsonhalfindent)
		  MOM_OUT (pout, MOMOUT_SMALL_SPACE (72));
		else if (pout->mout_flags & outf_jsonindent)
		  MOM_OUT (pout, MOMOUT_SPACE (48));
	      }
	    output_json_mom (pout, v.pjsonobj->jobjtab[ix].je_name);
	    putc (':', out);
	    if (pout->mout_flags & outf_jsonhalfindent)
	      MOM_OUT (pout, MOMOUT_SMALL_SPACE (80));
	    else if (pout->mout_flags & outf_jsonindent)
	      MOM_OUT (pout, MOMOUT_SPACE (64));
	    output_json_mom (pout, v.pjsonobj->jobjtab[ix].je_attr);
	  }
	if (pout->mout_flags & outf_jsonindent)
	  MOM_OUT (pout, MOMOUT_SPACE (48));
	putc ('}', out);
	if (pout->mout_flags & (outf_jsonhalfindent | outf_jsonindent))
	  MOM_OUT (pout, MOMOUT_INDENT_LESS ());
      }
      break;
    case momty_jsonarray:
      {
	unsigned jarlen = v.pjsonarr->slen;
	putc ('[', out);
	if (pout->mout_flags & (outf_jsonhalfindent | outf_jsonindent))
	  MOM_OUT (pout, MOMOUT_INDENT_MORE ());
	for (unsigned ix = 0; ix < jarlen; ix++)
	  {
	    if (ix > 0)
	      {
		putc (',', out);
		if (pout->mout_flags & outf_jsonhalfindent)
		  MOM_OUT (pout, MOMOUT_SMALL_SPACE (72));
		else if (pout->mout_flags & outf_jsonindent)
		  MOM_OUT (pout, MOMOUT_SPACE (48));
	      }
	    output_json_mom (pout, v.pjsonarr->jarrtab[ix]);
	  }
	if (pout->mout_flags & (outf_jsonhalfindent | outf_jsonindent))
	  MOM_OUT (pout, MOMOUT_INDENT_LESS ());
	putc (']', out);
      }
      break;
    case momty_set:
    case momty_tuple:
    case momty_node:
    case momty_null:		// should never happen
      MOM_FATAPRINTF ("corrupted json value to output");
      break;
    }
}

static void output_item_mom (momout_t *pout, const momitem_t *itm);

static void output_item_attributes_mom (momout_t *pout, const momitem_t *itm);

static void output_item_payload_mom (momout_t *pout, const momitem_t *itm);

static void
output_value_mom (momout_t *pout, const momval_t v)
{
  assert (pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  if (!v.ptr)
    {
      fputs ("__ ", out);
      return;
    }
  unsigned tynum = *v.ptype;
  switch ((enum momvaltype_en) tynum)
    {
    case momty_null:
      MOM_FATAPRINTF ("invalid value at %p", v.ptr);
      break;
    case momty_int:
      fprintf (out, "%lld", (long long) v.pint->intval);
      break;
    case momty_double:
      fprintf (out, "%#.7g", v.pdouble->dblval);
      break;
    case momty_string:
      MOM_OUT (pout,
	       MOMOUT_SMALL_SPACE (64),
	       MOMOUT_LITERAL ("\""),
	       MOMOUT_JS_STRING ((const char *) v.pstring->cstr),
	       MOMOUT_LITERAL ("\""));
      break;
    case momty_item:
      output_item_mom (pout, v.pitem);
      break;
    case momty_tuple:
      {
	unsigned tuplen = v.ptuple->slen;
	MOM_OUT (pout, MOMOUT_SMALL_SPACE (48),
		 MOMOUT_LITERAL ("@tup["), MOMOUT_INDENT_MORE ());
	for (unsigned ix = 0; ix < tuplen; ix++)
	  {
	    if (ix > 0)
	      {
		MOM_OUT (pout, MOMOUT_LITERAL (","), MOMOUT_SMALL_SPACE (60));
		if (ix % 10 == 9)
		  MOM_OUT (pout, MOMOUT_SMALL_SPACE (60));
	      }
	    output_item_mom (pout, v.ptuple->itemseq[ix]);
	  }
	MOM_OUT (pout, MOMOUT_INDENT_LESS (), MOMOUT_LITERAL ("]"));
      }
      break;
    case momty_set:
      {
	unsigned setlen = v.pset->slen;
	MOM_OUT (pout, MOMOUT_SMALL_SPACE (48),
		 MOMOUT_LITERAL ("@set{"), MOMOUT_INDENT_MORE ());
	for (unsigned ix = 0; ix < setlen; ix++)
	  {
	    if (ix > 0)
	      {
		MOM_OUT (pout, MOMOUT_LITERAL (","), MOMOUT_SMALL_SPACE (60));
		if (ix % 10 == 9)
		  MOM_OUT (pout, MOMOUT_SMALL_SPACE (60));
	      }
	    output_item_mom (pout, v.pset->itemseq[ix]);
	  }
	MOM_OUT (pout, MOMOUT_INDENT_LESS (), MOMOUT_LITERAL ("}"));
      }
      break;
    case momty_jsonarray:
      MOM_OUT (pout, MOMOUT_SMALL_SPACE (48),
	       MOMOUT_LITERAL ("@jsar"), MOMOUT_JSON_VALUE (v));
      break;
    case momty_jsonobject:
      MOM_OUT (pout, MOMOUT_SMALL_SPACE (48),
	       MOMOUT_LITERAL ("@jsob"), MOMOUT_JSON_VALUE (v));
      break;
    case momty_node:
      {
	unsigned nbsons = v.pnode->slen;
	MOM_OUT (pout, MOMOUT_SMALL_SPACE (48),
		 MOMOUT_LITERAL ("*"), MOMOUT_ITEM (v.pnode->connitm),
		 MOMOUT_LITERAL ("("), MOMOUT_INDENT_MORE ());
	for (unsigned six = 0; six < nbsons; six++)
	  {
	    if (six > 0)
	      {
		MOM_OUT (pout, MOMOUT_LITERAL (","), MOMOUT_SMALL_SPACE (60));
		if (six % 10 == 9)
		  MOM_OUT (pout, MOMOUT_SMALL_SPACE (60));
	      }
	    output_value_mom (pout, v.pnode->sontab[six]);
	  }
	MOM_OUT (pout, MOMOUT_INDENT_LESS (), MOMOUT_LITERAL (")"));
      }
      break;
    }
}

static void
output_item_mom (momout_t *pout, const momitem_t *itm)
{
  assert (pout && pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  if (!itm)
    {
      fputs ("?nilitem?", out);
      return;
    }
  assert (itm->i_typnum == momty_item);
  fputs (mom_string_cstr
	 ((momval_t) mom_item_get_name_or_idstr ((momitem_t *) itm)), out);
}


void
mom_output_attributes (struct momout_st *pout,
		       struct mom_itemattributes_st *attrs)
{
  if (!pout || !attrs || !pout->mout_file)
    return;
  fprintf (pout->mout_file, "[%d attrs]", attrs->nbattr);
  for (unsigned ix = 0; ix < attrs->size; ix++)
    {
      momitem_t *curatitm = attrs->itattrtab[ix].aten_itm;
      if (!curatitm || curatitm == MOM_EMPTY)
	continue;
      momval_t curval = attrs->itattrtab[ix].aten_val;
      MOM_OUT (pout, MOMOUT_NEWLINE (),
	       MOMOUT_LITERAL ("*"), MOMOUT_DEC_INT ((int) ix),
	       MOMOUT_LITERAL (": "),
	       MOMOUT_ITEM ((const momitem_t *) curatitm),
	       MOMOUT_LITERAL (":: "),
	       MOMOUT_VALUE ((const momval_t) curval), NULL);
    }
}

static void
output_item_attributes_mom (momout_t *pout, const momitem_t *itm)
{
  assert (pout && pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  if (!itm)
    return;
  assert (itm->i_typnum == momty_item);
  struct mom_itemattributes_st *attrs = itm->i_attrs;
  if (!attrs)
    {
      fputs ("?noattrs?", out);
      return;
    }
  else
    mom_output_attributes (pout, attrs);
}

static void
output_item_payload_mom (momout_t *pout, const momitem_t *itm)
{
  assert (pout && pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  if (!itm)
    return;
  MOM_OUT (pout, MOMOUT_LITERALV (mom_item_payload_kindstr (itm)));
  unsigned k = itm->i_paylkind;
  if (k > 0 && k < mompayk__last)
    {
      struct mom_payload_descr_st *pd = mom_payloadescr[k];
      if (pd && pd->dpayl_outputfun)
	{
	  assert (pd->dpayl_magic == MOM_PAYLOAD_MAGIC);
	  void *payl = itm->i_payload;
	  pd->dpayl_outputfun (pout, (momitem_t *) itm, payl);
	}
    }
}

static void
output_backtrace_mom (momout_t *pout, void **bbuf, int depth, int lev)
{
  assert (pout && pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  char **backsyms = backtrace_symbols (bbuf, depth);
  if (MOM_UNLIKELY (!backsyms))
    MOM_FATAPRINTF ("backtrace failing for depth %d, level %d", depth, lev);
  for (unsigned ix = 0; ix < (unsigned) depth; ix++)
    {
      MOM_OUT (pout, MOMOUT_LITERAL ("#"), MOMOUT_DEC_INT ((int) ix),
	       MOMOUT_LITERAL (": "),
	       MOMOUT_LITERALV ((const char *) backsyms[ix]),
	       MOMOUT_NEWLINE ());
    }
  free (backsyms);
}


#define INITIAL_COPYRIGHT_YEAR_MOM 2014
#define MAX_INDENT_MOM 256

static void
output_gplv3p_notice_mom (momout_t *pout, const char *forfile)
{
  time_t now = 0;
  struct tm nowtm = { 0 };
  time (&now);
  localtime_r (&now, &nowtm);
  assert (pout != NULL);
  MOM_OUT (pout, MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL
	   ("////////////////////////////////////////////////////////////////++"),
	   MOMOUT_NEWLINE ());
  /// output the copyright line, little trick to avoid 2014-2014 in the generated notice
  MOM_OUT (pout, MOMOUT_NEWLINE (),
	   MOMOUT_LITERAL ("/// generated file "),
	   MOMOUT_LITERALV (forfile),
	   MOMOUT_LITERAL (" ** DO NOT EDIT"), MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE (), MOMOUT_LITERAL ("/// Copyright (C) "));
  int nowyear = nowtm.tm_year + 1900;
  /// In the year of the Lord 2222, some artificial intelligence
  /// should change the following assert :-)  :-)
  /// But I'll be dead long time ago, and probably also this very software...
  assert (nowyear > 2000 && nowyear < 2222);
  if (nowyear > INITIAL_COPYRIGHT_YEAR_MOM)
    MOM_OUT (pout, MOMOUT_DEC_INT (INITIAL_COPYRIGHT_YEAR_MOM),
	     MOMOUT_LITERAL ("-"), MOMOUT_DEC_INT (nowyear));
  else
    MOM_OUT (pout, MOMOUT_DEC_INT (nowyear));
  MOM_OUT (pout, MOMOUT_LITERAL (" Free Software Foundation, Inc."),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// MONIMELT is a monitor for MELT - see http://gcc-melt.org/"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout, MOMOUT_LITERAL ("// This file is part of GCC."),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// GCC is free software; you can redistribute it and/or modify"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// it under the terms of the GNU General Public License as published by"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// the Free Software Foundation; either version 3, or (at your option)"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout, MOMOUT_LITERAL ("// any later version."), MOMOUT_NEWLINE (),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// GCC is distributed in the hope that it will be useful,"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// but WITHOUT ANY WARRANTY; without even the implied warranty of"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL ("// GNU General Public License for more details."),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// You should have received a copy of the GNU General Public License"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("// along with GCC; see the file COPYING3.   If not see"),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout, MOMOUT_LITERAL ("//  <http://www.gnu.org/licenses/>."),
	   MOMOUT_NEWLINE ());
  MOM_OUT (pout,
	   MOMOUT_LITERAL
	   ("////////////////////////////////////////////////////////////////--"),
	   MOMOUT_NEWLINE (), MOMOUT_NEWLINE ());

}



#define SMALL_INDENT_MOM 8
#define INDENT_MOM 16
void
mom_outva_at (const char *sfil, int lin, momout_t *pout, va_list alist)
{
  bool again = true;
  assert (pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  while (again)
    {
      bool jsctrl = false;
      enum momoutdir_en dir = va_arg (alist, enum momoutdir_en);
      switch (dir)
	{
	case MOMOUTDO__END:
	  again = false;
	  break;
	case MOMOUTDO_LITERAL:
	  {
	    const char *s = va_arg (alist, const char *);
	    if (s)
	      fputs (s, out);
	  }
	  break;
	case MOMOUTDO_JS_HTML:
	  jsctrl = true;
	  // failthru!
	case MOMOUTDO_HTML:
	  {
	    const char *s = va_arg (alist, const char *);
	    int len = s ? strlen (s) : 0;
	    const gchar *end = NULL;
	    if (MOM_UNLIKELY
		(s && !g_utf8_validate ((const gchar *) s, len, &end)))
	      MOM_FATAPRINTF ("from %s:%d invalid string to htmlize: %s",
			      sfil, lin, s);
	    for (const gchar * pc = s; pc && *pc && pc < s + len;
		 pc = g_utf8_next_char (pc))
	      {
		gunichar c = g_utf8_get_char (pc);
		switch (c)
		  {
		  case '\n':
		    if (jsctrl)
		      fputs ("\\n", out);
		    else
		      putc ('\n', out);
		    break;
		  case '\r':
		    if (jsctrl)
		      fputs ("\\r", out);
		    else
		      putc ('\r', out);
		    break;
		  case '\t':
		    if (jsctrl)
		      fputs ("\\t", out);
		    else
		      putc ('\t', out);
		    break;
		  case '\v':
		    if (jsctrl)
		      fputs ("\\v", out);
		    else
		      putc ('\v', out);
		    break;
		  case '\f':
		    if (jsctrl)
		      fputs ("\\f", out);
		    else
		      putc ('\f', out);
		    break;
		  case '\\':
		    if (jsctrl)
		      fputs ("\\\\", out);
		    else
		      putc ('\\', out);
		    break;
		  case '&':
		    fputs ("&amp;", out);
		    break;
		  case '<':
		    fputs ("&lt;", out);
		    break;
		  case '>':
		    fputs ("&gt;", out);
		    break;
		  case '\'':
		    fputs ("&apos;", out);
		    break;
		  case '\"':
		    fputs ("&quot;", out);
		    break;
		  case 160:
		    fputs ("&nbsp;", out);
		    break;
		  default:
		    if (c >= 127)
		      fprintf (out, "&#%d;", (int) c);
		    else
		      putc (c, out);
		    break;
		  }
	      }
	  }
	  break;
	  //
	case MOMOUTDO_SLASHCOMMENT_STRING:
	  {
	    const char *s = va_arg (alist, const char *);
	    int len = s ? strlen (s) : 0;
	    if (len > 0)
	      {
		int nbl = 0;
		fputs ("////!", out);
		for (const char *pc = s; *pc; pc++)
		  if (*pc == '\n' || *pc == '\r' || *pc == '\f'
		      || *pc == '\v')
		    {
		      nbl++;
		      MOM_OUT (pout, MOMOUT_SMALL_NEWLINE (),
			       MOMOUT_LITERAL ("////+"));
		    }
		  else		// we don't care about UTF8 here
		    putc (*pc, out);
		MOM_OUT (pout, MOMOUT_SMALL_NEWLINE ());
	      }
	  }
	  break;
	  //
	case MOMOUTDO_C_STRING:
	  {
	    const char *s = va_arg (alist, const char *);
	    int len = s ? strlen (s) : 0;
	    for (const char *pc = s; pc && *pc && pc < s + len; pc++)
	      {
		char c = *pc;
		switch (c)
		  {
		  case '\\':
		    fputs ("\\\\", out);
		    break;
		  case '\'':
		    fputs ("\'", out);
		    break;
		  case '\"':
		    fputs ("\\\"", out);
		    break;
		  case '\n':
		    fputs ("\\n", out);
		    break;
		  case '\r':
		    fputs ("\\r", out);
		    break;
		  case '\t':
		    fputs ("\\t", out);
		    break;
		  case '\v':
		    fputs ("\\v", out);
		    break;
		  case '\f':
		    fputs ("\\f", out);
		    break;
		  default:
		    if (c >= 127 || c < ' ')
		      fprintf (out, "\\x%02x", ((int) c) & 0xff);
		    else
		      fputc (c, out);
		    break;
		  }
	      }
	  }
	  break;
	  //
	case MOMOUTDO_JS_STRING:
	  {
	    const char *s = va_arg (alist, const char *);
	    int len = s ? strlen (s) : 0;
	    const gchar *end = NULL;
	    if (MOM_UNLIKELY (s && !g_utf8_validate (s, len, &end)))
	      MOM_FATAPRINTF ("from %s:%d invalid string to jsonize: %s",
			      sfil, lin, s);
	    for (const gchar * pc = s; pc && *pc && pc < s + len;
		 pc = g_utf8_next_char (pc))
	      {
		gunichar c = g_utf8_get_char (pc);
		switch (c)
		  {
		  case '\\':
		    fputs ("\\\\", out);
		    break;
		  case '\'':
		    fputs ("\'", out);
		    break;
		  case '\"':
		    fputs ("\\\"", out);
		    break;
		  case '\n':
		    fputs ("\\n", out);
		    break;
		  case '\r':
		    fputs ("\\r", out);
		    break;
		  case '\t':
		    fputs ("\\t", out);
		    break;
		  case '\v':
		    fputs ("\\v", out);
		    break;
		  case '\f':
		    fputs ("\\f", out);
		    break;
		  default:
		    if (c >= 127 || c < ' ')
		      fprintf (out, "\\u%04x", (unsigned) c);
		    else
		      fputc (c, out);
		    break;
		  }
	      }
	  }
	  break;
	  //
	case MOMOUTDO_DEC_INT:
	  {
	    int n = va_arg (alist, int);
	    fprintf (out, "%d", n);
	  }
	  break;
	case MOMOUTDO_HEX_INT:
	  {
	    int n = va_arg (alist, int);
	    fprintf (out, "%x", n);
	  }
	  break;
	  //
	case MOMOUTDO_JSON_VALUE:
	  {
	    momval_t v = va_arg (alist, momval_t);
	    if (mom_is_jsonable (v))
	      output_json_mom (pout, v);
	  }
	  break;
	  //
	case MOMOUTDO_STRING_VALUE:
	  {
	    momval_t v = va_arg (alist, momval_t);
	    if (mom_is_string (v))
	      fputs (v.pstring->cstr, out);
	  }
	  break;
	  //
	case MOMOUTDO_DEC_LONG:
	  {
	    long n = va_arg (alist, long);
	    fprintf (out, "%ld", n);
	  }
	  break;
	  //
	case MOMOUTDO_HEX_LONG:
	  {
	    long n = va_arg (alist, long);
	    fprintf (out, "%ld", n);
	  }
	  break;
	  //
	case MOMOUTDO_DEC_INTPTR_T:
	  {
	    intptr_t n = va_arg (alist, intptr_t);
	    fprintf (out, "%" PRIdPTR, n);
	  }
	  break;
	  //
	case MOMOUTDO_HEX_INTPTR_T:
	  {
	    intptr_t n = va_arg (alist, intptr_t);
	    fprintf (out, "%" PRIxPTR, n);
	  }
	  break;
	  //
	case MOMOUTDO_DOUBLE_TIME:
	  {
	    char timbuf[64];
	    memset (timbuf, 0, sizeof (timbuf));
	    const char *fmt = va_arg (alist, const char *);
	    double ti = va_arg (alist, double);
	    fputs (mom_strftime_centi (timbuf, sizeof (timbuf), fmt, ti),
		   out);
	  }
	  break;
	  //
	case MOMOUTDO_DOUBLE_G:
	  {
	    double x = va_arg (alist, double);
	    fprintf (out, "%g", x);
	  }
	  break;
	  ///
	case MOMOUTDO_DOUBLE_F:
	  {
	    double x = va_arg (alist, double);
	    fprintf (out, "%f", x);
	  }
	  break;
	  ///
	case MOMOUTDO_FMT_DOUBLE:
	  {
	    const char *fmt = va_arg (alist, const char *);
	    double x = va_arg (alist, double);
	    assert (strchr (fmt, '%') == strrchr (fmt, '%'));
	    fprintf (out, fmt, x);
	  }
	  break;
	  ///
	case MOMOUTDO_FMT_LONG:
	  {
	    const char *fmt = va_arg (alist, const char *);
	    long l = va_arg (alist, long);
	    assert (strchr (fmt, '%') == strrchr (fmt, '%'));
	    fprintf (out, fmt, l);
	  }
	  break;
	  ///
	case MOMOUTDO_FMT_LONG_LONG:
	  {
	    const char *fmt = va_arg (alist, const char *);
	    long ll = va_arg (alist, long long);
	    assert (strchr (fmt, '%') == strrchr (fmt, '%'));
	    fprintf (out, fmt, ll);
	  }
	  break;
	  ///
	case MOMOUTDO_FMT_INT:
	  {
	    const char *fmt = va_arg (alist, const char *);
	    int i = va_arg (alist, int);
	    assert (strchr (fmt, '%') == strrchr (fmt, '%'));
	    fprintf (out, fmt, i);
	  }
	  break;
	  ///
	case MOMOUTDO_FMT_UNSIGNED:
	  {
	    const char *fmt = va_arg (alist, const char *);
	    unsigned u = va_arg (alist, unsigned);
	    assert (strchr (fmt, '%') == strrchr (fmt, '%'));
	    fprintf (out, fmt, u);
	  }
	  break;
	  ///
	case MOMOUTDO_VERBATIM_FILE:
	  {
	    FILE *cfil = va_arg (alist, FILE *);
	    if (cfil)
	      {
		int c = EOF;
		do
		  {
		    c = getc (cfil);
		    putc (c, out);
		  }
		while (c != EOF);
	      }
	  }
	  break;
	  ///
	case MOMOUTDO_HTML_FILE:
	  {
	    char *lin = NULL;
	    size_t sz = 0;
	    FILE *cfil = va_arg (alist, FILE *);
	    if (cfil)
	      {
		do
		  {
		    ssize_t linsiz = getline (&lin, &sz, cfil);
		    if (linsiz < 0)
		      break;
		    MOM_OUT (pout, MOMOUT_HTML ((const char *) lin));
		  }
		while (!feof (cfil));
		free (lin);
	      }
	  }
	  break;
	  ///
	  ///
	case MOMOUTDO_JS_FILE:
	  {
	    char *lin = NULL;
	    size_t sz = 0;
	    FILE *cfil = va_arg (alist, FILE *);
	    if (cfil)
	      {
		do
		  {
		    ssize_t linsiz = getline (&lin, &sz, cfil);
		    if (linsiz < 0)
		      break;
		    MOM_OUT (pout, MOMOUT_JS_STRING ((const char *) lin));
		  }
		while (!feof (cfil));
		free (lin);
	      }
	  }
	  break;
	  ///
	case MOMOUTDO_INDENT_MORE:
	  {
	    if (pout->mout_indent < MAX_INDENT_MOM)
	      pout->mout_indent++;
	  }
	  break;
	  ///
	case MOMOUTDO_INDENT_LESS:
	  {
	    if (pout->mout_indent > 0)
	      pout->mout_indent--;
	  }
	  break;
	  ///
	case MOMOUTDO_NEWLINE:
	  {
	    putc ('\n', out);
	    pout->mout_lastnl = ftell (out);
	    for (unsigned nsp = pout->mout_indent % INDENT_MOM; nsp > 0;
		 nsp--)
	      putc (' ', out);
	  }
	  break;
	  ///
	case MOMOUTDO_SMALL_NEWLINE:
	  {
	    putc ('\n', out);
	    pout->mout_lastnl = ftell (out);
	    for (unsigned nsp = pout->mout_indent % SMALL_INDENT_MOM; nsp > 0;
		 nsp--)
	      putc (' ', out);
	  }
	  break;
	  ///
	case MOMOUTDO_SPACE:
	  {
	    unsigned maxwl = va_arg (alist, unsigned);
	    if (ftell (out) > pout->mout_lastnl + maxwl)
	      {
		putc ('\n', out);
		pout->mout_lastnl = ftell (out);
		for (unsigned nsp = pout->mout_indent % INDENT_MOM; nsp > 0;
		     nsp--)
		  putc (' ', out);
	      }
	    else
	      putc (' ', out);
	  }
	  break;
	  ///
	case MOMOUTDO_SMALL_SPACE:
	  {
	    unsigned maxwl = va_arg (alist, unsigned);
	    if (ftell (out) > pout->mout_lastnl + maxwl)
	      {
		putc ('\n', out);
		pout->mout_lastnl = ftell (out);
		for (unsigned nsp = pout->mout_indent % SMALL_INDENT_MOM;
		     nsp > 0; nsp--)
		  putc (' ', out);
	      }
	    else
	      putc (' ', out);
	  }
	  break;
	  ///
	case MOMOUTDO_VALUE:
	  {
	    const momval_t v = va_arg (alist, const momval_t);
	    output_value_mom (pout, v);
	  }
	  break;
	  ///
	case MOMOUTDO_ITEM:
	  {
	    const momitem_t *itm = va_arg (alist, momitem_t *);
	    output_item_mom (pout, itm);
	  }
	  break;
	  ///
	case MOMOUTDO_ITEM_ATTRIBUTES:
	  {
	    const momitem_t *itm = va_arg (alist, momitem_t *);
	    if (itm)
	      output_item_attributes_mom (pout, itm);
	  }
	  break;
	  ///
	case MOMOUTDO_ITEM_PAYLOAD:
	  {
	    const momitem_t *itm = va_arg (alist, momitem_t *);
	    if (itm)
	      output_item_payload_mom (pout, itm);
	  }
	  break;
	  ///
	case MOMOUTDO_FLUSH:
	  {
	    if (fflush (out))
	      MOM_FATAPRINTF ("failed to flush out #%d", fileno (out));
	  }
	  break;
	  ///
	case MOMOUTDO_BACKTRACE:
	  {
	    int lev = va_arg (alist, int);
	    if (lev < 3)
	      lev = 3;
	    else if (lev > 256)
	      lev = 256;
	    void **bbuf = MOM_GC_ALLOC ("backtrace", lev * sizeof (void *));
	    int depth = backtrace (bbuf, lev);
	    output_backtrace_mom (pout, bbuf, depth, lev);
	    MOM_GC_FREE (bbuf);
	  }
	  break;
	  ///
	case MOMOUTDO_GPLV3P_NOTICE:
	  {
	    const char *forfile = va_arg (alist, const char *);
	    output_gplv3p_notice_mom (pout, forfile);
	  }
	  break;
	}			/// end switch dir
    }				/// end while again
}

momval_t
mom_outstring_at (const char *sfil, int lin, unsigned flags, ...)
{
  va_list alist;
  momval_t res = MOM_NULLV;
  struct momout_st mout;
  memset (&mout, 0, sizeof (mout));
  mom_initialize_buffer_output (&mout, flags);
  va_start (alist, flags);
  mom_outva_at (sfil, lin, &mout, alist);
  va_end (alist);
  fflush (mout.mout_file);
  res = (momval_t) mom_make_string (mout.mout_data);
  mom_finalize_buffer_output (&mout);
  return res;
}
