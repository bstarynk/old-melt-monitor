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

#define MOMJSON_MAGIC 0x48b97a1d	/*json magic 1220114973 */
struct jsonparser_st
{
  uint32_t json_magic;		/* always MOMJSON_MAGIC */
  int json_c;			/* read ahead character */
  pthread_mutex_t json_mtx;
  FILE *json_file;
  void *json_data;
  char *json_error;
  jmp_buf json_jmpbuf;
};


void
mom_initialize_json_parser (struct jsonparser_st *jp, FILE * file, void *data)
{
  if (!file || !jp)
    return;
  memset (jp, 0, sizeof (struct jsonparser_st));
  pthread_mutex_init (&jp->json_mtx, NULL);
  jp->json_c = EOF;
  jp->json_file = file;
  jp->json_data = data;
  jp->json_magic = MOMJSON_MAGIC;
  jp->json_error = NULL;
}

static momval_t parse_json_internal (struct jsonparser_st *jp);

#define JSON_ERROR_AT(Fil,Lin,Jp,Fmt,...) do {	\
    static char buf##Lin[128];			\
    snprintf (buf##Lin, sizeof(buf##Lin),	\
	      "%s:%d: " Fmt,			\
	      Fil, Lin, ##__VA_ARGS__);		\
    jp->json_error = GC_STRDUP(buf##Lin);	\
    longjmp (jp->json_jmpbuf,Lin);		\
  } while(0)

#define JSON_ERROR_REALLY_AT(Fil,Lin,Jp,Fmt,...) \
  JSON_ERROR_AT(Fil,Lin,Jp,Fmt,##__VA_ARGS__)
#define JSON_ERROR(Jp,Fmt,...) \
    JSON_ERROR_REALLY_AT(__FILE__,__LINE__,(Jp),Fmt,##__VA_ARGS__)

momval_t
mom_parse_json (struct jsonparser_st *jp, char **perrmsg)
{
  momval_t res = MONIMELT_NULLV;
  if (perrmsg)
    *perrmsg = NULL;
  if (!jp || jp->json_magic != MOMJSON_MAGIC)
    {
      if (perrmsg)
	{
	  *perrmsg = "invalid JSON parser";
	  return MONIMELT_NULLV;
	}
    }
  pthread_mutex_lock (&jp->json_mtx);
  if (setjmp (jp->json_jmpbuf))
    {
      if (perrmsg)
	{
	  if (jp->json_error)
	    *perrmsg = jp->json_error;
	  else
	    *perrmsg = "JSON parsing failed";
	}
      pthread_mutex_unlock (&jp->json_mtx);
      return MONIMELT_NULLV;
    }
  res = parse_json_internal (jp);
  if (res.ptr == MONIMELT_EMPTY)
    JSON_ERROR (jp, "unexpected terminator %c", jp->json_c);
  pthread_mutex_unlock (&jp->json_mtx);
  return res;
}


static momval_t
json_item_or_string (const char *buf)
{
  if (!buf || !buf[0])
    return MONIMELT_NULLV;
  const momstring_t *str = NULL;
  mom_anyitem_t *itm = mom_item_named_with_string (buf, &str);
  if (itm)
    {
      if (itm->typnum == momty_jsonitem
	  && ((momit_json_name_t *) itm)->ij_namejson
	  && !strcmp (((momit_json_name_t *) itm)->ij_namejson->cstr, buf))
	return (momval_t) itm;
      else
	return (momval_t) (const momstring_t *) str;
    }
  else
    return (momval_t) mom_make_string (buf);
}

// may return MONIMELT_EMPTY if found a terminator like comma,
// right-brace, right-bracket, colon
static momval_t
parse_json_internal (struct jsonparser_st *jp)
{
again:
  if (jp->json_c < 0)
    jp->json_c = getc (jp->json_file);
  if (jp->json_c < 0)
    JSON_ERROR (jp, "end of file at offset %ld", ftell (jp->json_file));
  if (isspace (jp->json_c))
    {
      jp->json_c = getc (jp->json_file);
      goto again;
    }
  else if (jp->json_c == ',' || jp->json_c == ':' || jp->json_c == '}'
	   || jp->json_c == ']')
    // don't consume the terminator! Leave it available to the caller.
    return (momval_t) MONIMELT_EMPTY;
  // extension, admit comments à la C slash slash or à la C++ slash star
  else if (jp->json_c == '/')
    {
      long off = ftell (jp->json_file);
      jp->json_c = getc (jp->json_file);
      if (jp->json_c == '/')
	{
	  do
	    {
	      jp->json_c = getc (jp->json_file);
	    }
	  while (jp->json_c >= 0 && jp->json_c != '\n' && jp->json_c != '\r');
	  goto again;
	}
      else if (jp->json_c == '*')
	{
	  jp->json_c = EOF;
	  int pc = EOF;
	  do
	    {
	      pc = jp->json_c;
	      jp->json_c = getc (jp->json_file);
	    }
	  while (jp->json_c >= 0 && jp->json_c != '/' && pc != '*');
	  goto again;
	}
      else
	JSON_ERROR (jp, "bad slash at offset %ld", off);
    }
  else if (isdigit (jp->json_c) || jp->json_c == '+' || jp->json_c == '-')
    {
      char numbuf[64];
      long off = ftell (jp->json_file);
      memset (numbuf, 0, sizeof (numbuf));
      int ix = 0;
      bool isfloat = false;
      do
	{
	  if (ix < sizeof (numbuf) - 1)
	    numbuf[ix++] = jp->json_c;
	  jp->json_c = getc (jp->json_file);
	}
      while (isdigit (jp->json_c));
      if (jp->json_c == '.')
	{
	  isfloat = true;
	  do
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->json_c;
	      jp->json_c = getc (jp->json_file);
	    }
	  while (isdigit (jp->json_c));
	  if (jp->json_c == 'E' || jp->json_c == 'e')
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->json_c;
	      jp->json_c = getc (jp->json_file);
	    };
	  if (jp->json_c == '+' || jp->json_c == '-')
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->json_c;
	      jp->json_c = getc (jp->json_file);
	    };
	  do
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->json_c;
	      jp->json_c = getc (jp->json_file);
	    }
	  while (isdigit (jp->json_c));
	}
      numbuf[sizeof (numbuf) - 1] = (char) 0;
      char *end = NULL;
      if (isfloat)
	{
	  double x = strtod (numbuf, &end);
	  if (end && *end)
	    JSON_ERROR (jp, "bad float number %s at offset %ld", numbuf, off);
	  return (momval_t) mom_make_double (x);
	}
      else
	{
	  long l = strtol (numbuf, &end, 10);
	  if (end && *end)
	    JSON_ERROR (jp, "bad number %s at offset %ld", numbuf, off);
	  return (momval_t) mom_make_int (l);
	}
    }
  // as an extension, C-identifier names can be read as strings or
  // JSON names
  else if (isalpha (jp->json_c) || jp->json_c == '_')
    {
      char namebuf[64];		// optimize for small stack-allocated names
      memset (namebuf, 0, sizeof (namebuf));
      char *namestr = namebuf;
      unsigned namesize = sizeof (namebuf) - 1;
      unsigned ix = 0;
      do
	{
	  if (ix < namesize)
	    namestr[ix++] = jp->json_c;
	  else
	    {
	      unsigned newsize = ((5 * namesize / 4 + 10) | 0xf) + 1;
	      char *newname = GC_MALLOC_ATOMIC (newsize);
	      if (!newname)
		MONIMELT_FATAL ("no space for JSON name of size %u", newsize);
	      memset (newname, 0, newsize);
	      strcpy (newname, namestr);
	      if (namestr != namebuf)
		GC_FREE (namestr);
	      namestr = newname;
	      namesize = newsize - 1;
	    }
	}
      while (isalnum (jp->json_c) || jp->json_c == '_');
      if (!strcmp (namestr, "null"))
	return MONIMELT_NULLV;
      else if (!strcmp (namestr, "true"))
	return (momval_t) mom_get_item_bool (true);
      else if (!strcmp (namestr, "false"))
	return (momval_t) mom_get_item_bool (false);
      else
	return json_item_or_string (namestr);
    }
  else if (jp->json_c == '[')
    {
      unsigned arrsize = 8;
      unsigned arrlen = 0;
      momval_t *arrptr = GC_MALLOC (arrsize * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!arrptr))
	MONIMELT_FATAL ("cannot allocate initial json array of %u", arrsize);
      memset (arrptr, 0, arrsize * sizeof (momval_t));
      jp->json_c = getc (jp->json_file);
      momval_t comp = MONIMELT_NULLV;
      bool gotcomma = false;
      do
	{
	  comp = parse_json_internal (jp);
	  if (comp.ptr == MONIMELT_EMPTY)
	    {
	      if (jp->json_c == ']')
		{
		  jp->json_c = getc (jp->json_file);
		  break;
		}
	      else if (jp->json_c == ',')
		{
		  jp->json_c = getc (jp->json_file);
		  if (gotcomma)
		    JSON_ERROR (jp,
				"consecutive commas in JSON array at offset %ld",
				ftell (jp->json_file));
		  gotcomma = true;
		  continue;
		}
	      else
		JSON_ERROR (jp, "invalid char %c in JSON array at offset %ld",
			    jp->json_c, ftell (jp->json_file));
	    };
	  gotcomma = false;
	  if (MONIMELT_UNLIKELY (arrlen + 1 >= arrsize))
	    {
	      unsigned newsize = ((5 * arrsize / 4 + 5) | 0xf) + 1;
	      momval_t *newarr = GC_MALLOC (arrsize * sizeof (momval_t));
	      if (MONIMELT_UNLIKELY (!newarr))
		MONIMELT_FATAL ("cannot grow json array to %u", newsize);
	      memset (newarr, 0, arrsize * sizeof (momval_t));
	      memcpy (newarr, arrptr, arrlen * sizeof (momval_t));
	      GC_FREE (arrptr);
	      arrptr = newarr;
	      arrsize = newsize;
	    }
	  arrptr[arrlen++] = comp;
	  comp = MONIMELT_NULLV;
	}
      while (jp->json_c >= 0);
      struct momjsonarray_st *jarr =
	GC_MALLOC (sizeof (struct momjsonarray_st) +
		   arrlen * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!jarr))
	MONIMELT_FATAL ("failed to build JSON array of %d components",
			(int) arrlen);
      memset (jarr, 0,
	      sizeof (struct momjsonarray_st) + arrlen * sizeof (momval_t));
      momhash_t h = 3 * arrlen + 5;
      for (unsigned ix = 0; ix < arrlen; ix++)
	{
	  jarr->jarrtab[ix] = arrptr[ix];
	  h = (6053 * mom_value_hash (arrptr[ix])) ^ (7 * ix + h * 9059);
	}
      if (!h)
	{
	  h = 2 * arrlen + 5;
	  if (!h)
	    h = 113;
	}
      jarr->hash = h;
      jarr->slen = arrlen;
      jarr->typnum = momty_jsonarray;
      GC_FREE (arrptr);
      return (momval_t) jarr;
    }
  else if (jp->json_c == '{')
    {
      unsigned jsize = 4, jcount = 0;
      long off = ftell (jp->json_file);
      jp->json_c = getc (jp->json_file);
      struct mom_jsonentry_st *jent =
	GC_MALLOC (sizeof (struct mom_jsonentry_st) * jsize);
      if (MONIMELT_UNLIKELY (!jent))
	MONIMELT_FATAL ("failed to allocate initial json entry table of %d",
			(int) jsize);
      memset (jent, 0, sizeof (struct mom_jsonentry_st) * jsize);
      do
	{
	  momval_t namv = parse_json_internal (jp);
	  if (namv.ptr == MONIMELT_EMPTY)
	    {
	      namv = MONIMELT_NULLV;
	      if (jp->json_c == '}')
		{
		  jp->json_c = getc (jp->json_file);
		  break;
		}
	      else
		JSON_ERROR (jp,
			    "failed to parse attribute in JSON object at offset %ld",
			    off);
	    }
	  while (isspace (jp->json_c))
	    jp->json_c = getc (jp->json_file);
	  if (jp->json_c == ':')
	    {
	      jp->json_c = getc (jp->json_file);
	    }
	  else
	    JSON_ERROR (jp,
			"missing colon in JSON object after name at offset %ld",
			ftell (jp->json_file));
	  momval_t valv = parse_json_internal (jp);
	  if (valv.ptr == MONIMELT_EMPTY)
	    JSON_ERROR (jp,
			"failed to parse value in JSON object at offset %ld",
			ftell (jp->json_file));
	  while (isspace (jp->json_c))
	    jp->json_c = getc (jp->json_file);
	  if (jp->json_c == ',')
	    {
	      jp->json_c = getc (jp->json_file);
	      continue;
	    }
	  if (MONIMELT_UNLIKELY (jcount >= jsize))
	    {
	      unsigned newjsize = ((5 * jcount / 4 + 5) | 0x7) + 1;
	      struct mom_jsonentry_st *newjent =
		GC_MALLOC (sizeof (struct mom_jsonentry_st) * newjsize);
	      if (MONIMELT_UNLIKELY (!newjent))
		MONIMELT_FATAL ("failed to grow json entry table of %d",
				(int) newjsize);
	      memset (newjent, 0,
		      sizeof (struct mom_jsonentry_st) * newjsize);
	      memcpy (newjent, jent,
		      sizeof (struct mom_jsonentry_st) * jcount);
	      GC_FREE (jent);
	      jent = newjent;
	    }
	  if (namv.ptr != NULL)
	    {
	      jent[jcount].je_name = namv;
	      jent[jcount].je_attr = valv;
	      jcount++;
	    }
	  namv = MONIMELT_NULLV;
	  valv = MONIMELT_NULLV;
	}
      while (jp->json_c >= 0);
      return (momval_t) mom_make_json_object (MOMJSON_COUNTED_ENTRIES, jcount,
					      jent, NULL);
    }
  else if (jp->json_c == '"')
    {
      jp->json_c = getc (jp->json_file);
      unsigned siz = 24, cnt = 0;
      char *str = GC_MALLOC_ATOMIC (siz);
      if (MONIMELT_UNLIKELY (!str))
	MONIMELT_FATAL ("failed to allocate initial string buffer of %d",
			(int) siz);
      memset (str, 0, siz);
      do
	{
	  if (jp->json_c == '"')
	    break;
	  if (MONIMELT_UNLIKELY (cnt + 1 >= siz))
	    {
	      unsigned newsiz = (((5 * cnt / 4) + 12) | 0xf) + 1;
	      char *newstr = GC_MALLOC_ATOMIC (newsiz);
	      if (MONIMELT_UNLIKELY (!str))
		MONIMELT_FATAL ("failed to grow string buffer of %d",
				(int) newsiz);
	      memset (newstr, 0, newsiz);
	      memcpy (newstr, str, cnt);
	    }
	  if (MONIMELT_UNLIKELY (jp->json_c < 0))
	    JSON_ERROR (jp, "unterminated string at offset %ld",
			ftell (jp->json_file));
	  if (jp->json_c == '\\')
	    {
	      jp->json_c = getc (jp->json_file);
#define ADD1CHAR(Ch) do { str[cnt++] = Ch;		\
	  jp->json_c = getc(jp->json_file); } while(0)
	      switch (jp->json_c)
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
		    if (fscanf (jp->json_file, "%04x", &h) > 0)
		      {
			char hexd[8];
			memset (hexd, 0, sizeof (hexd));
			uint32_t c = (uint32_t) h;
			// see http://en.wikipedia.org/wiki/UTF-8
			if (c < 0x80)
			  hexd[0] = c;
			else if (c < 0x800)
			  {
			    hexd[0] = (c >> 6) | 0xC0;
			    hexd[1] = (c & 0x3F) | 0x80;
			  }
			else if (c < 0x10000)
			  {
			    hexd[0] = (c >> 12) | 0xE0;
			    hexd[1] = ((c >> 6) & 0x3F) | 0x80;
			    hexd[2] = (c & 0x3F) | 0x80;
			  }
			else if (c < 0x200000)
			  {
			    hexd[0] = (c >> 18) | 0xF0;
			    hexd[1] = ((c >> 12) & 0x3F) | 0x80;
			    hexd[2] = ((c >> 6) & 0x3F) | 0x80;
			    hexd[3] = (c & 0x3F) | 0x80;
			  }
			else if (c <= 67108863)
			  {
			    hexd[0] = (248 + (c / 16777216));
			    hexd[1] = (128 + ((c / 262144) % 64));
			    hexd[2] = (128 + ((c / 4096) % 64));
			    hexd[3] = (128 + ((c / 64) % 64));
			    hexd[4] = (128 + (c % 64));
			  }
			else if (c <= 2147483647)
			  {
			    hexd[0] = (252 + (c / 1073741824));
			    hexd[1] = (128 + ((c / 16777216) % 64));
			    hexd[2] = (128 + ((c / 262144) % 64));
			    hexd[3] = (128 + ((c / 4096) % 64));
			    hexd[4] = (128 + ((c / 64) % 64));
			    hexd[5] = (128 + (c % 64));
			  }
			ADD1CHAR (hexd[0]);
			if (hexd[1])
			  {
			    ADD1CHAR (hexd[1]);
			    if (hexd[2])
			      {
				ADD1CHAR (hexd[2]);
				if (hexd[3])
				  {
				    ADD1CHAR (hexd[3]);
				    if (hexd[4])
				      {
					ADD1CHAR (hexd[4]);
					if (hexd[5])
					  ADD1CHAR (hexd[5]);
				      }
				  }
			      }
			  }
		      }
		  }
		  break;
		}
	    }
	  else
	    ADD1CHAR (jp->json_c);
#undef ADD1CHAR
	  if (jp->json_c < 0)
	    break;
	}
      while (jp->json_c != '"');
      return (momval_t) mom_make_string_len (str, cnt);
    }
  else
    JSON_ERROR (jp, "unexpected char %c at offset %ld",
		jp->json_c, ftell (jp->json_file));
}

static int
jsonentry_cmp (const void *l, const void *r)
{
  const struct mom_jsonentry_st *le = l;
  const struct mom_jsonentry_st *re = r;
  return mom_json_cmp (le->je_name, re->je_name);
}

momjsonobject_t *
mom_make_json_object (int firstdir, ...)
{
  va_list args;
  int dir = 0;
  unsigned count = 0;
  //////
  // first argument scan, to count the number of entries
  dir = firstdir;
  va_start (args, firstdir);
  while (dir != MOMJSON_END)
    {
      switch (dir)
	{
	case MOMJSON_ENTRY:
	  (void) va_arg (args, momval_t);
	  (void) va_arg (args, momval_t);
	  dir = va_arg (args, int);
	  count++;
	  break;
	case MOMJSON_STRING:
	  (void) va_arg (args, const char *);
	  (void) va_arg (args, momval_t);
	  dir = va_arg (args, int);
	  count++;
	  break;
	case MOMJSON_COUNTED_ENTRIES:
	  {
	    unsigned nbent = va_arg (args, unsigned);
	    (void) va_arg (args, struct mom_jsonentry_st *);
	    dir = va_arg (args, int);
	    count += nbent;
	  }
	  break;
	default:
	  MONIMELT_FATAL ("unexpected JSON directive %d", dir);
	}
    }
  va_end (args);
  unsigned size = count + 1;
  struct momjsonobject_st *jsob
    = GC_MALLOC (sizeof (struct momjsonobject_st)
		 + size * sizeof (struct mom_jsonentry_st));
  if (MONIMELT_UNLIKELY (!jsob))
    MONIMELT_FATAL ("failed to allocate JSON object with %d entries", size);
  memset (jsob, 0, sizeof (struct momjsonobject_st)
	  + size * sizeof (struct mom_jsonentry_st));
  /////
  // second argument scan, to fill the entries
  count = 0;
  dir = firstdir;
  dir = firstdir;
  va_start (args, firstdir);
  while (dir != MOMJSON_END)
    {
      switch (dir)
	{
	case MOMJSON_ENTRY:
	  {
	    momval_t namv = va_arg (args, momval_t);
	    momval_t attv = va_arg (args, momval_t);
	    dir = va_arg (args, int);
	    if (namv.ptr
		&& (*namv.ptype == momty_string
		    || (*namv.ptype == momty_jsonitem
			&& (namv.pjsonitem->ij_namejson))))
	      {
		jsob->jobjtab[count].je_name = namv;
		jsob->jobjtab[count].je_attr = attv;
		count++;
	      }
	  }
	  break;
	case MOMJSON_STRING:
	  {
	    const char *namstr = va_arg (args, const char *);
	    momval_t attv = va_arg (args, momval_t);
	    dir = va_arg (args, int);
	    if (namstr && namstr[0])
	      {
		momval_t namv = MONIMELT_NULLV;
		const momstring_t *namvalstr = NULL;
		mom_anyitem_t *namitm =
		  mom_item_named_with_string (namstr, &namvalstr);
		if (namitm)
		  {
		    if (namitm->typnum == momty_jsonitem
			&& ((momit_json_name_t *) namitm)->ij_namejson
			&&
			!strcmp ((((momit_json_name_t *)
				   namitm)->ij_namejson)->cstr, namstr))
		      namv = (momval_t) namitm;
		    else
		      namv = (momval_t) namvalstr;
		  };
		if (!namv.ptr)
		  namv = (momval_t) mom_make_string (namstr);
		jsob->jobjtab[count].je_name = namv;
		jsob->jobjtab[count].je_attr = attv;
		count++;
	      }
	  }
	  break;
	case MOMJSON_COUNTED_ENTRIES:
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
			    || (*namv.ptype == momty_jsonitem
				&& (namv.pjsonitem->ij_namejson))))
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
	  MONIMELT_FATAL ("unexpected JSON directive %d", dir);
	}
    }
  va_end (args);
  // sort the entries and remove the unlikely duplicates
  qsort (jsob, count, sizeof (struct mom_jsonentry_st), jsonentry_cmp);
  bool shrink = false;
  for (unsigned ix = 0; ix + 1 < count; ix++)
    {
      if (MONIMELT_UNLIKELY (jsonentry_cmp (jsob->jobjtab + ix,
					    jsob->jobjtab + ix + 1) == 0))
	{
	  shrink = true;
	  for (unsigned j = ix; j + 1 < count; j++)
	    jsob->jobjtab[j] = jsob->jobjtab[j + 1];
	  jsob->jobjtab[count].je_name = MONIMELT_NULLV;
	  jsob->jobjtab[count].je_attr = MONIMELT_NULLV;
	  count--;
	}
    }
  // compute the hash
  momhash_t h1 = 17, h2 = count, h = 0;
  for (unsigned ix = 0; ix < count; ix++)
    {
      h1 =
	((ix & 0xf) + 1) * h1 +
	((45077 *
	  mom_value_hash ((const momval_t) jsob->
			  jobjtab[count].je_name)) ^ h2);
      h2 =
	(75041 * h2) ^ (7589 *
			mom_value_hash ((const momval_t) jsob->
					jobjtab[count].je_attr));
    }
  h = h1 ^ h2;
  if (!h)
    {
      h = h1;
      if (!h)
	h = h2;
      if (!h)
	h = (count & 0xfff) + 11;
    }
  if (MONIMELT_UNLIKELY (shrink))
    {
      struct momjsonobject_st *newjsob
	= GC_MALLOC (sizeof (struct momjsonobject_st)
		     + count * sizeof (struct mom_jsonentry_st));
      if (MONIMELT_UNLIKELY (!newjsob))
	MONIMELT_FATAL ("failed to reallocate JSON object with %d entries",
			size);
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

int
mom_json_cmp (momval_t l, momval_t r)
{
  if (l.ptr == r.ptr)
    return 0;
  if (!l.ptr)
    return -1;
  if (!r.ptr)
    return 1;
  if (*l.ptype == momty_jsonitem)
    l = (momval_t) (l.pjsonitem->ij_namejson);
  if (*r.ptype == momty_jsonitem)
    r = (momval_t) (r.pjsonitem->ij_namejson);
  if (l.ptr == r.ptr)
    return 0;
  return mom_value_cmp (l, r);
}

const momval_t
mom_json_get_def (const momval_t jsobv, const momval_t namev,
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
