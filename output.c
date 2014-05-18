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
    fputs ("null", out);
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
	if (atof (numbuf) == x && strlen (numbuf) < 30
	    && strchr (numbuf, '.'))
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
	snprintf (numbuf, sizeof (numbuf), "%#.5g", x);
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
	if (pout->mout_flags & (outf_jsonhalfindent | outf_jsonindent))
	  MOM_OUT (pout, MOMOUT_INDENT_LESS ());
	putc ('}', out);
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

static void
output_value_mom (momout_t *pout, const momval_t v)
{
  assert (pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
  if (!out)
    return;
  if (!v.ptr)
    {
      fputs ("nil", out);
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
  assert (pout->mout_magic == MOM_MOUT_MAGIC);
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
		    fputs ("\\'", out);
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
	  //
	case MOMOUTDO_JSON_VALUE:
	  {
	    momval_t v = va_arg (alist, momval_t);
	    if (mom_is_jsonable (v))
	      output_json_mom (pout, v);
	  }
	  break;
	  //
	case MOMOUTDO_HEX_INT:
	  {
	    int n = va_arg (alist, int);
	    fprintf (out, "%x", n);
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
	    pout->mout_indent++;
	  }
	  break;
	  ///
	case MOMOUTDO_INDENT_LESS:
	  {
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
	}			/// end switch dir
    }				/// end while again
}
