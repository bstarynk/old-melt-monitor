// file json.c

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

#define MOMJSONP_MAGIC 0x48b97a1d	/*jsonp magic 1220114973 */

void
mom_initialize_json_parser (struct jsonparser_st *jp, FILE * file, void *data)
{
  if (!file || !jp)
    return;
  memset (jp, 0, sizeof (struct jsonparser_st));
  pthread_mutex_init (&jp->jsonp_mtx, NULL);
  jp->jsonp_c = EOF;
  jp->jsonp_file = file;
  jp->jsonp_data = data;
  jp->jsonp_magic = MOMJSONP_MAGIC;
  jp->jsonp_error = NULL;
}

void *
mom_json_parser_data (const struct jsonparser_st *jp)
{
  if (!jp)
    return NULL;
  if (jp->jsonp_magic != MOMJSONP_MAGIC)
    MOM_FATAPRINTF ("invalid json parser (magic %x, expecting %x)",
		    jp->jsonp_magic, MOMJSONP_MAGIC);
  return jp->jsonp_data;
}

void
mom_end_json_parser (struct jsonparser_st *jp)
{
  if (!jp)
    return;
  if (jp->jsonp_magic != MOMJSONP_MAGIC)
    MOM_FATAPRINTF ("invalid json parser (magic %x, expecting %x)",
		    jp->jsonp_magic, MOMJSONP_MAGIC);
  pthread_mutex_destroy (&jp->jsonp_mtx);
  memset (jp, 0, sizeof (struct jsonparser_st));
}

void
mom_close_json_parser (struct jsonparser_st *jp)
{
  if (!jp)
    return;
  if (jp->jsonp_magic != MOMJSONP_MAGIC)
    MOM_FATAPRINTF ("invalid json parser (magic %x, expecting %x)",
		    jp->jsonp_magic, MOMJSONP_MAGIC);
  pthread_mutex_destroy (&jp->jsonp_mtx);
  fclose (jp->jsonp_file);
  memset (jp, 0, sizeof (struct jsonparser_st));
}

static momval_t parse_json_internal_mom (struct jsonparser_st *jp);

#define JSONPARSE_ERROR_AT(Fil,Lin,Jp,Fmt,...) do {	\
    static char buf##Lin[128];			\
    snprintf (buf##Lin, sizeof(buf##Lin),	\
	      "%s:%d: " Fmt,			\
	      Fil, Lin, ##__VA_ARGS__);		\
    jp->jsonp_error = GC_STRDUP(buf##Lin);	\
    longjmp (jp->jsonp_jmpbuf,Lin);		\
  } while(0)

#define JSONPARSE_ERROR_REALLY_AT(Fil,Lin,Jp,Fmt,...) \
  JSONPARSE_ERROR_AT(Fil,Lin,Jp,Fmt,##__VA_ARGS__)
#define JSONPARSE_ERROR(Jp,Fmt,...) \
    JSONPARSE_ERROR_REALLY_AT(__FILE__,__LINE__,(Jp),Fmt,##__VA_ARGS__)

static void compute_json_array_hash (momjsonarray_t *jarr);

momval_t
mom_parse_json (struct jsonparser_st *jp, char **perrmsg)
{
  momval_t res = MOM_NULLV;
  if (perrmsg)
    *perrmsg = NULL;
  if (!jp || jp->jsonp_magic != MOMJSONP_MAGIC)
    {
      if (perrmsg)
	{
	  *perrmsg = "invalid JSON parser";
	  return MOM_NULLV;
	}
    }
  pthread_mutex_lock (&jp->jsonp_mtx);
  if (setjmp (jp->jsonp_jmpbuf))
    {
      if (perrmsg)
	{
	  if (jp->jsonp_error)
	    *perrmsg = jp->jsonp_error;
	  else
	    *perrmsg = "JSON parsing failed";
	}
      pthread_mutex_unlock (&jp->jsonp_mtx);
      return MOM_NULLV;
    }
  res = parse_json_internal_mom (jp);
  if (res.ptr == MOM_EMPTY)
    JSONPARSE_ERROR (jp, "unexpected terminator %c", jp->jsonp_c);
  pthread_mutex_unlock (&jp->jsonp_mtx);
  return res;
}


static momval_t
json_item_or_string_mom (const char *buf)
{
  if (!buf || !buf[0])
    return MOM_NULLV;
  momitem_t *itm = NULL;
  if (isalpha (buf[0]) || buf[0] == '_')
    itm = mom_get_item_of_name_or_ident_cstr (buf);
  if (itm)
    return (momval_t) itm;
  else
    return (momval_t) mom_make_string (buf);
}

// may return MOM_EMPTY if found a terminator like comma,
// right-brace, right-bracket, colon
static momval_t
parse_json_internal_mom (struct jsonparser_st *jp)
{
again:
  if (jp->jsonp_c < 0)
    jp->jsonp_c = getc (jp->jsonp_file);
  if (jp->jsonp_c < 0)
    JSONPARSE_ERROR (jp, "end of file at offset %ld", ftell (jp->jsonp_file));
  if (isspace (jp->jsonp_c))
    {
      jp->jsonp_c = getc (jp->jsonp_file);
      goto again;
    }
  // terminating character, to be consumed by caller
  else if (jp->jsonp_c == ',' || jp->jsonp_c == ':' || jp->jsonp_c == '}'
	   || jp->jsonp_c == ']')
    // don't consume the terminator! Leave it available to the caller.
    return (momval_t) MOM_EMPTY;
  // extension, admit comments à la C slash slash or à la C++ slash star
  else if (jp->jsonp_c == '/')
    {
      long off = ftell (jp->jsonp_file);
      jp->jsonp_c = getc (jp->jsonp_file);
      if (jp->jsonp_c == '/')
	{
	  do
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  while (jp->jsonp_c >= 0 && jp->jsonp_c != '\n'
		 && jp->jsonp_c != '\r');
	  goto again;
	}
      else if (jp->jsonp_c == '*')
	{
	  jp->jsonp_c = EOF;
	  int pc = EOF;
	  do
	    {
	      pc = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  while (jp->jsonp_c >= 0 && jp->jsonp_c != '/' && pc != '*');
	  goto again;
	}
      else
	JSONPARSE_ERROR (jp, "bad slash at offset %ld", off);
    }
  // strings
  else if (jp->jsonp_c == '"')
    {
      jp->jsonp_c = getc (jp->jsonp_file);
      char tinyarr[2 * MOM_TINY_MAX];
      unsigned siz = 2 * MOM_TINY_MAX, cnt = 0;
      char *str = tinyarr;
      memset (str, 0, siz);
      do
	{
	  if (jp->jsonp_c == '"')
	    break;
	  // we need extraspace for \u-encoded unicode characters
	  if (MOM_UNLIKELY (cnt + 8 >= siz))
	    {
	      unsigned newsiz = (((5 * cnt / 4) + 12) | 0xf) + 1;
	      char *newstr =
		MOM_GC_SCALAR_ALLOC ("json grown string buffer", newsiz);
	      memcpy (newstr, str, cnt);
	      if (str != tinyarr)
		MOM_GC_FREE (str);
	      str = newstr;
	      siz = newsiz;
	    }
	  if (MOM_UNLIKELY (jp->jsonp_c < 0))
	    JSONPARSE_ERROR (jp, "unterminated string at offset %ld",
			     ftell (jp->jsonp_file));
	  if (jp->jsonp_c == '\\')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
#define ADD1CHAR(Ch) do { str[cnt++] = Ch;			\
		jp->jsonp_c = getc(jp->jsonp_file); } while(0)
	      switch (jp->jsonp_c)
		{
		case 'b':
		  ADD1CHAR ('\b');
		  break;
		case 'f':
		  ADD1CHAR ('\f');
		  break;
		case 'n':
		  ADD1CHAR ('\n');
		  break;
		case 'r':
		  ADD1CHAR ('\r');
		  break;
		case 't':
		  ADD1CHAR ('\t');
		  break;
		case '/':
		  ADD1CHAR ('/');
		  break;
		case '"':
		  ADD1CHAR ('"');
		  break;
		case '\\':
		  ADD1CHAR ('\\');
		  break;
		case 'u':
		  {
		    int h = 0;
		    int cc = -2;
		    char cbuf[8];
		    memset (cbuf, 0, sizeof (cbuf));
		    cc = jp->jsonp_c = getc (jp->jsonp_file);
		    if (isxdigit (cc))
		      cbuf[0] = cc;
		    else
		      continue;
		    cc = jp->jsonp_c = getc (jp->jsonp_file);
		    if (isxdigit (cc))
		      cbuf[1] = cc;
		    else
		      continue;
		    cc = jp->jsonp_c = getc (jp->jsonp_file);
		    if (isxdigit (cc))
		      cbuf[2] = cc;
		    else
		      continue;
		    cc = jp->jsonp_c = getc (jp->jsonp_file);
		    if (isxdigit (cc))
		      cbuf[3] = cc;
		    else
		      continue;
		    jp->jsonp_c = getc (jp->jsonp_file);
		    h = (int) strtol (cbuf, NULL, 16);
		    char hexd[8];
		    memset (hexd, 0, sizeof (hexd));
		    uint32_t c = (uint32_t) h;
		    if (!c)
		      MOM_WARNPRINTF
			("null character inside JSON string %s", str);
		    g_unichar_to_utf8 ((gunichar) c, hexd);
		    str[cnt++] = (hexd[0]);
		    if (hexd[1])
		      {
			str[cnt++] = (hexd[1]);
			if (hexd[2])
			  {
			    str[cnt++] = (hexd[2]);
			    if (hexd[3])
			      {
				str[cnt++] = (hexd[3]);
				if (hexd[4])
				  {
				    str[cnt++] = (hexd[4]);
				    if (hexd[5])
				      str[cnt++] = (hexd[5]);
				  }
			      }
			  }
		      }
		  }
		  break;
		}
	    }
	  else
	    ADD1CHAR (jp->jsonp_c);
#undef ADD1CHAR
	  if (jp->jsonp_c < 0)
	    break;
	}
      while (jp->jsonp_c != '"');
      jp->jsonp_c = getc (jp->jsonp_file);
      if ((isalpha (str[0]) || str[0] == '_') && strlen (str) == cnt)
	{
	  // for strings which happen to be names or identifiers, if
	  // the item is predefined, return that predefined item
	  // otherwise return the shared identifier or name string.
	  momitem_t *namitm = mom_get_item_of_name_or_ident_cstr (str);
	  if (namitm)
	    {
	      if (namitm->i_space == momspa_predefined)
		return (momval_t) namitm;
	      else if (str[0] == '_' && isdigit (str[1]))
		return (momval_t) namitm->i_idstr;
	      else if (isalpha (str[0]) && namitm->i_name)
		return (momval_t) namitm->i_name;
	    }
	}
      return (momval_t) mom_make_string (str);
    }

  else if (isdigit (jp->jsonp_c) || jp->jsonp_c == '+' || jp->jsonp_c == '-')
    {
      char numbuf[64];
      long off = ftell (jp->jsonp_file);
      memset (numbuf, 0, sizeof (numbuf));
      int ix = 0;
      bool isfloat = false;
      do
	{
	  if (ix < (int) sizeof (numbuf) - 1)
	    numbuf[ix++] = jp->jsonp_c;
	  jp->jsonp_c = getc (jp->jsonp_file);
	}
      while (isdigit (jp->jsonp_c));
      if (jp->jsonp_c == '.')
	{
	  isfloat = true;
	  do
	    {
	      if (ix < (int) sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  while (isdigit (jp->jsonp_c));
	  if (jp->jsonp_c == 'E' || jp->jsonp_c == 'e')
	    {
	      if (ix < (int) sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    };
	  if (jp->jsonp_c == '+' || jp->jsonp_c == '-')
	    {
	      if (ix < (int) sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    };
	  while (isdigit (jp->jsonp_c))
	    {
	      if (ix < (int) sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    };
	}
      numbuf[sizeof (numbuf) - 1] = (char) 0;
      char *end = NULL;
      if (isfloat)
	{
	  double x = strtod (numbuf, &end);
	  if (end && *end)
	    JSONPARSE_ERROR (jp,
			     "bad float number '%s' at offset %ld, followed by %s",
			     numbuf, off, end);
	  return (momval_t) mom_make_double (x);
	}
      else
	{
	  long l = strtol (numbuf, &end, 10);
	  if (end && *end)
	    JSONPARSE_ERROR (jp,
			     "bad number '%s' at offset %ld, followed by %s",
			     numbuf, off, end);
	  return (momval_t) mom_make_integer (l);
	}
    }
  // as an extension, C-identifier names can be read as strings or
  // JSON names
  else if (isalpha (jp->jsonp_c) || jp->jsonp_c == '_')
    {
      char namebuf[64];		// optimize for small stack-allocated names
      memset (namebuf, 0, sizeof (namebuf));
      char *namestr = namebuf;
      unsigned namesize = sizeof (namebuf) - 1;
      unsigned ix = 0;
      do
	{
	  if (ix < namesize)
	    namestr[ix++] = jp->jsonp_c;
	  else
	    {
	      unsigned newsize = ((5 * namesize / 4 + 10) | 0xf) + 1;
	      char *newname = MOM_GC_SCALAR_ALLOC ("JSON new name", newsize);
	      if (!newname)
		MOM_FATAPRINTF ("no space for JSON name of size %u", newsize);
	      memset (newname, 0, newsize);
	      strcpy (newname, namestr);
	      if (namestr != namebuf)
		MOM_GC_FREE (namestr);
	      namestr = newname;
	      namesize = newsize - 1;
	    }
	  jp->jsonp_c = getc (jp->jsonp_file);
	}
      while (isalnum (jp->jsonp_c) || jp->jsonp_c == '_');
      if (!strcmp (namestr, "null"))
	return MOM_NULLV;
      else if (!strcmp (namestr, "true"))
	return (momval_t) mom_get_item_bool (true);
      else if (!strcmp (namestr, "false"))
	return (momval_t) mom_get_item_bool (false);
      else
	return json_item_or_string_mom (namestr);
    }
  else if (jp->jsonp_c == '[')
    {
      unsigned arrsize = 8;
      unsigned arrlen = 0;
      momval_t *arrptr =
	MOM_GC_ALLOC ("json array elements", arrsize * sizeof (momval_t));
      jp->jsonp_c = getc (jp->jsonp_file);
      momval_t comp = MOM_NULLV;
      bool gotcomma = false;
      do
	{
	  comp = parse_json_internal_mom (jp);
	  if (comp.ptr == MOM_EMPTY)
	    {
	      if (jp->jsonp_c == ']')
		{
		  jp->jsonp_c = getc (jp->jsonp_file);
		  break;
		}
	      else if (jp->jsonp_c == ',')
		{
		  jp->jsonp_c = getc (jp->jsonp_file);
		  if (gotcomma)
		    JSONPARSE_ERROR (jp,
				     "consecutive commas in JSON array at offset %ld",
				     ftell (jp->jsonp_file));
		  gotcomma = true;
		  continue;
		}
	      else
		JSONPARSE_ERROR (jp,
				 "invalid char %c in JSON array at offset %ld",
				 jp->jsonp_c, ftell (jp->jsonp_file));
	    };
	  gotcomma = false;
	  if (MOM_UNLIKELY (arrlen + 1 >= arrsize))
	    {
	      unsigned newsize = ((5 * arrsize / 4 + 5) | 0xf) + 1;
	      momval_t *newarr = MOM_GC_ALLOC ("grown json array elements",
					       arrsize * sizeof (momval_t));
	      MOM_GC_FREE (arrptr);
	      arrptr = newarr;
	      arrsize = newsize;
	    }
	  arrptr[arrlen++] = comp;
	  comp = MOM_NULLV;
	}
      while (jp->jsonp_c >= 0);
      struct momjsonarray_st *jarr =
	MOM_GC_ALLOC ("parsed json array", sizeof (struct momjsonarray_st) +
		      arrlen * sizeof (momval_t));
      for (unsigned ix = 0; ix < arrlen; ix++)
	{
	  jarr->jarrtab[ix] = arrptr[ix];
	}
      jarr->slen = arrlen;
      compute_json_array_hash (jarr);
      jarr->typnum = momty_jsonarray;
      GC_FREE (arrptr);
      return (momval_t) (const struct momjsonarray_st *) jarr;
    }
  else if (jp->jsonp_c == '{')
    {
      unsigned jsize = MOM_TINY_MAX, jcount = 0;
      struct mom_jsonentry_st tinyent[MOM_TINY_MAX] = { };
      long off = ftell (jp->jsonp_file);
      jp->jsonp_c = getc (jp->jsonp_file);
      struct mom_jsonentry_st *jent = tinyent;
      memset (jent, 0, sizeof (struct mom_jsonentry_st) * jsize);
      do
	{
	  while (jp->jsonp_c > 0 && isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  if (jp->jsonp_c == '}')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
	      break;
	    }
	  momval_t namv = parse_json_internal_mom (jp);
	  if (namv.ptr == MOM_EMPTY)
	    {
	      namv = MOM_NULLV;
	      if (jp->jsonp_c == '}')
		{
		  jp->jsonp_c = getc (jp->jsonp_file);
		  break;
		}
	      else
		JSONPARSE_ERROR (jp,
				 "failed to parse attribute in JSON object at offset %ld",
				 off);
	    };
	  while (jp->jsonp_c > 0 && isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  if (jp->jsonp_c == ':')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  else
	    JSONPARSE_ERROR (jp,
			     "missing colon in JSON object after name at offset %ld",
			     ftell (jp->jsonp_file));
	  while (jp->jsonp_c > 0 && isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  momval_t valv = parse_json_internal_mom (jp);
	  if (valv.ptr == MOM_EMPTY)
	    JSONPARSE_ERROR (jp,
			     "failed to parse value in JSON object at offset %ld",
			     ftell (jp->jsonp_file));
	  while (isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  if (MOM_UNLIKELY (jcount >= jsize))
	    {
	      unsigned newjsize = ((5 * jcount / 4 + 5) | 0x7) + 1;
	      struct mom_jsonentry_st *newjent =
		MOM_GC_ALLOC ("grown json entry table",
			      sizeof (struct mom_jsonentry_st) * newjsize);
	      memcpy (newjent, jent,
		      sizeof (struct mom_jsonentry_st) * jcount);
	      if (jent != tinyent)
		MOM_GC_FREE (jent);
	      jent = newjent;
	    }
	  if (namv.ptr != NULL)
	    {
	      jent[jcount].je_name = namv;
	      jent[jcount].je_attr = valv;
	      jcount++;
	    }
	  namv = MOM_NULLV;
	  valv = MOM_NULLV;
	  while (jp->jsonp_c > 0 && isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  if (jp->jsonp_c == ',')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
	      while (jp->jsonp_c > 0 && isspace (jp->jsonp_c))
		jp->jsonp_c = getc (jp->jsonp_file);
	      continue;
	    }
	}
      while (jp->jsonp_c >= 0);
      return (momval_t) mom_make_json_object
	(MOMJSOB_COUNTED_ENTRIES (jcount, jent), NULL);
    }
  else
    JSONPARSE_ERROR (jp, "unexpected char %c at offset %ld",
		     jp->jsonp_c, ftell (jp->jsonp_file));
  return MOM_NULLV;
}

static int
jsonentry_cmp (const void *l, const void *r)
{
  const struct mom_jsonentry_st *le = l;
  const struct mom_jsonentry_st *re = r;
  assert (le->je_name.ptr != NULL);
  assert (re->je_name.ptr != NULL);
  return mom_json_cmp (le->je_name, re->je_name);
}

const momjsonobject_t *
mom_make_json_object (int firstdir, ...)
{
  va_list args;
  int dir = 0;
  unsigned count = 0;
  //////
  // first argument scan, to count the number of entries
  dir = firstdir;
  va_start (args, firstdir);
  while (dir != MOMJSON__END)
    {
      switch (dir)
	{
	case MOMJSONDIR__ENTRY:
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  dir = va_arg (args, int);
	  count++;
	  break;
	case MOMJSONDIR__STRING:
	  (void) va_arg (args, const char *);
	  (void) va_arg (args, momval_t);
	  dir = va_arg (args, int);
	  count++;
	  break;
	case MOMJSONDIR__COUNTED_ENTRIES:
	  {
	    unsigned nbent = va_arg (args, unsigned);
	    (void) va_arg (args, struct mom_jsonentry_st *);
	    dir = va_arg (args, int);
	    count += nbent;
	  }
	  break;
	default:
	  MOM_FATAPRINTF ("unexpected JSON directive %d", dir);
	}
    }
  va_end (args);
  unsigned size = count + 1;
  struct momjsonobject_st *jsob
    = MOM_GC_ALLOC ("parsed json object", sizeof (struct momjsonobject_st)
		    + size * sizeof (struct mom_jsonentry_st));
  /////
  // second argument scan, to fill the entries
  count = 0;
  dir = firstdir;
  dir = firstdir;
  va_start (args, firstdir);
  while (dir != MOMJSON__END)
    {
      switch (dir)
	{
	case MOMJSONDIR__ENTRY:
	  {
	    momval_t namv = va_arg (args, momval_t);
	    momval_t attv = va_arg (args, momval_t);
	    dir = va_arg (args, int);
	    if (namv.ptr
		&& (*namv.ptype == momty_string
		    || (*namv.ptype == momty_item)))
	      {
		jsob->jobjtab[count].je_name = namv;
		jsob->jobjtab[count].je_attr = attv;
		count++;
	      }
	  }
	  break;
	case MOMJSONDIR__STRING:
	  {
	    const char *namstr = va_arg (args, const char *);
	    momval_t attv = va_arg (args, momval_t);
	    dir = va_arg (args, int);
	    if (namstr && namstr[0] && mom_is_jsonable (attv))
	      {
		momval_t namv = MOM_NULLV;
		momitem_t *namitm =
		  mom_get_item_of_name_or_ident_cstr (namstr);
		if (namitm)
		  namv = (momval_t) namitm;
		if (!namv.ptr)
		  namv = (momval_t) mom_make_string (namstr);
		jsob->jobjtab[count].je_name = namv;
		jsob->jobjtab[count].je_attr = attv;
		count++;
	      }
	  }
	  break;
	case MOMJSONDIR__COUNTED_ENTRIES:
	  {
	    unsigned nbent = va_arg (args, unsigned);
	    struct mom_jsonentry_st *jentab =
	      va_arg (args, struct mom_jsonentry_st *);
	    dir = va_arg (args, int);
	    if (jentab && nbent > 0)
	      {
		for (unsigned ix = 0; ix < nbent; ix++)
		  {
		    momval_t namv = jentab[ix].je_name;
		    momval_t attv = jentab[ix].je_attr;
		    if (namv.ptr
			&& (*namv.ptype == momty_string
			    || *namv.ptype == momty_item)
			&& mom_is_jsonable (attv))
		      {
			jsob->jobjtab[count].je_name = namv;
			jsob->jobjtab[count].je_attr = attv;
			count++;
		      }
		  }
	      }
	  }
	  break;
	default:
	  MOM_FATAPRINTF ("unexpected JSON directive %d", dir);
	}
    }
  va_end (args);
#ifndef NDEBUG
  for (unsigned ix = 0; ix < count; ix++)
    assert (jsob->jobjtab[ix].je_name.ptr != NULL);
#endif
  // sort the entries and remove the unlikely duplicates
  qsort (jsob->jobjtab, count,
	 sizeof (struct mom_jsonentry_st), jsonentry_cmp);
  bool shrink = false;
  for (unsigned ix = 0; ix + 1 < count; ix++)
    {
      int cmpj = jsonentry_cmp (jsob->jobjtab + ix,
				jsob->jobjtab + ix + 1);
      if (MOM_UNLIKELY (cmpj == 0))
	{
	  shrink = true;
	  for (unsigned j = ix; j + 1 < count; j++)
	    jsob->jobjtab[j] = jsob->jobjtab[j + 1];
	  jsob->jobjtab[count].je_name = MOM_NULLV;
	  jsob->jobjtab[count].je_attr = MOM_NULLV;
	  count--;
	}
      else
	assert (cmpj < 0);
    }
  // compute the hash
  momhash_t h1 = 17, h2 = count, h = 0;
  for (unsigned ix = 0; ix < count; ix++)
    {
      h1 =
	((ix & 0xf) + 1) * h1 +
	((45077 *
	  mom_value_hash ((const momval_t) jsob->jobjtab[count].
			  je_name)) ^ h2);
      h2 =
	(75041 * h2) ^ (7589 *
			mom_value_hash ((const momval_t) jsob->jobjtab[count].
					je_attr));
    }
  h = h1 ^ h2;
  if (MOM_UNLIKELY (!h))
    {
      h = h1;
      if (MOM_UNLIKELY (!h))
	h = h2;
      if (MOM_UNLIKELY (!h))
	h = (count & 0xfff) + 11;
    }
  if (MOM_UNLIKELY (shrink))
    {
      struct momjsonobject_st *newjsob
	= MOM_GC_ALLOC ("shrink JSON object", sizeof (struct momjsonobject_st)
			+ count * sizeof (struct mom_jsonentry_st));
      memcpy (newjsob, jsob,
	      sizeof (struct momjsonobject_st) +
	      count * sizeof (struct mom_jsonentry_st));
      GC_FREE (jsob);
      jsob = newjsob;
    }
  jsob->hash = h;
  jsob->slen = count;
  jsob->typnum = momty_jsonobject;
  return jsob;
}

static void
compute_json_array_hash (momjsonarray_t *jarr)
{
  if (!jarr || jarr->hash != 0)
    return;
  if (jarr->typnum && jarr->typnum != momty_jsonarray)
    return;
  momhash_t h = jarr->slen + 1;
  for (unsigned ix = 0; ix < jarr->slen; ix++)
    h = (15061 * h) ^ (1511 * mom_value_hash (jarr->jarrtab[ix]) + ix * 17);
  if (!h)
    h = (jarr->slen & 0xffff) + 2531;
  jarr->hash = h;
  jarr->typnum = momty_jsonarray;
}

const momjsonarray_t *
mon_make_json_array (unsigned nbelem, ...)
{
  momjsonarray_t *jarr = MOM_GC_ALLOC ("newly made json array",
				       sizeof (momjsonarray_t) +
				       nbelem * sizeof (momval_t));
  va_list args;
  va_start (args, nbelem);
  for (unsigned ix = 0; ix < nbelem; ix++)
    {
      momval_t comp = va_arg (args, momval_t);
      if (!mom_is_jsonable (comp))
	continue;
      jarr->jarrtab[ix] = comp;
    }
  va_end (args);
  jarr->typnum = momty_jsonarray;
  jarr->slen = nbelem;
  compute_json_array_hash (jarr);
  return jarr;
}


const momjsonarray_t *
mom_make_json_array_count (unsigned nbelem, const momval_t *arr)
{
  if (!arr)
    return NULL;
  momjsonarray_t *jarr = MOM_GC_ALLOC ("newly made counted json array",
				       sizeof (momjsonarray_t) +
				       nbelem * sizeof (momval_t));
  for (unsigned ix = 0; ix < nbelem; ix++)
    {
      momval_t comp = arr[ix];
      if (!mom_is_jsonable (comp))
	continue;
      jarr->jarrtab[ix] = comp;
    }
  jarr->typnum = momty_jsonarray;
  jarr->slen = nbelem;
  compute_json_array_hash (jarr);
  return jarr;
}

const momjsonarray_t *
mom_make_json_array_til_nil (momval_t firstv, ...)
{
  unsigned nbelem = 0;
  va_list args;
  va_start (args, firstv);
  if (firstv.ptr)
    {
      momval_t valv = MOM_NULLV;
      do
	{
	  nbelem++;
	  valv = va_arg (args, momval_t);
	}
      while (valv.ptr != NULL);
    }
  va_end (args);
  momjsonarray_t *jarr = MOM_GC_ALLOC ("newly made jsonarray til nil",
				       sizeof (momjsonarray_t) +
				       nbelem * sizeof (momval_t));
  if (nbelem > 0)
    jarr->jarrtab[0] = firstv;
  va_start (args, firstv);
  for (unsigned ix = 1; ix < nbelem; ix++)
    {
      momval_t valv = va_arg (args, momval_t);
      if (!mom_is_jsonable (valv))
	continue;
      jarr->jarrtab[ix] = valv;
    }
  va_end (args);
  jarr->typnum = momty_jsonarray;
  jarr->slen = nbelem;
  compute_json_array_hash (jarr);
  return jarr;
}

int
mom_json_cmp (momval_t l, momval_t r)
{
  if (l.ptr == r.ptr)
    return 0;
  if (!l.ptr)
    return -1;
  if (!r.ptr)
    return 1;
  if (l.ptr == r.ptr)
    return 0;
  return mom_value_cmp (l, r);
}

int
mom_json_cstr_cmp (momval_t jv, const char *str)
{
  assert (str != NULL);
  if (!jv.ptr)
    return -1;
  if (*jv.ptype == momty_item)
    jv = (momval_t) (mom_item_get_name_or_idstr (jv.pitem));
  if (jv.ptr && *jv.ptype == momty_string)
    return strcmp (jv.pstring->cstr, str);
  return -1;
}

momval_t
mom_jsonob_get_def (const momval_t jsobv, const momval_t namev,
		    const momval_t def)
{
  if (!jsobv.ptr || !namev.ptr)
    return def;
  if (*jsobv.ptype != momty_jsonobject)
    return def;
  const struct momjsonobject_st *job = jsobv.pjsonobj;
  if (!job->slen)
    return def;
  unsigned lo = 0, hi = job->slen, md = 0;
  while (lo + 3 < hi)
    {
      md = (lo + hi) / 2;
      const momval_t curnamv = job->jobjtab[md].je_name;
      int cmp = mom_json_cmp (namev, curnamv);
      if (!cmp)
	return job->jobjtab[md].je_attr;
      else if (cmp < 0)
	hi = md;
      else
	lo = md;
    }
  for (md = lo; md < hi; md++)
    {
      const momval_t curnamv = job->jobjtab[md].je_name;
      if (mom_json_cmp (namev, curnamv) == 0)
	return job->jobjtab[md].je_attr;
    }
  return def;
}


momval_t
mom_jsonob_getstr (const momval_t jsobv, const char *namestr)
{
  if (!jsobv.ptr || !namestr)
    return MOM_NULLV;
  if (*jsobv.ptype != momty_jsonobject)
    return MOM_NULLV;
  const struct momjsonobject_st *job = jsobv.pjsonobj;
  if (!job->slen)
    return MOM_NULLV;
  unsigned lo = 0, hi = job->slen, md = 0;
  while (lo + 3 < hi)
    {
      md = (lo + hi) / 2;
      const momval_t curnamv = job->jobjtab[md].je_name;
      int cmp = mom_json_cstr_cmp (curnamv, namestr);
      if (!cmp)
	return job->jobjtab[md].je_attr;
      else if (cmp > 0)
	hi = md;
      else
	lo = md;
    }
  for (md = lo; md < hi; md++)
    {
      const momval_t curnamv = job->jobjtab[md].je_name;
      if (mom_json_cstr_cmp (curnamv, namestr) == 0)
	return job->jobjtab[md].je_attr;
    }
  return MOM_NULLV;
}
