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



#define SMALL_INDENT_MOM 8
#define INDENT_MOM 16
void
mom_outva_at (const char *sfil, int lin, momout_t *pout, va_list alist)
{
  bool again = true;
  assert (pout->mout_magic == MOM_MOUT_MAGIC);
  FILE *out = pout->mout_file;
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
	case MOMOUTDO_JS:
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
		    MOM_OUT (pout, MOMOUT_JS ((const char *) lin));
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
	}			/// end switch dir
    }				/// end while again
}
