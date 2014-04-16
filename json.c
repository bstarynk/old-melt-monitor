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

void*
mom_json_parser_data(const struct jsonparser_st*jp)
{
  if (!jp) return NULL;
  if (jp->jsonp_magic != MOMJSONP_MAGIC)
    MONIMELT_FATAL("invalid json parser (magic %x, expecting %x)",
		   jp->jsonp_magic, MOMJSONP_MAGIC);
  return jp->jsonp_data;
}

void
mom_end_json_parser (struct jsonparser_st*jp)
{
  if (!jp) return;
  if (jp->jsonp_magic != MOMJSONP_MAGIC)
    MONIMELT_FATAL("invalid json parser (magic %x, expecting %x)",
		   jp->jsonp_magic, MOMJSONP_MAGIC);
  pthread_mutex_destroy(&jp->jsonp_mtx);
  memset (jp, 0, sizeof(struct jsonparser_st));
}

void
mom_close_json_parser (struct jsonparser_st*jp)
{
  if (!jp) return;
  if (jp->jsonp_magic != MOMJSONP_MAGIC)
    MONIMELT_FATAL("invalid json parser (magic %x, expecting %x)",
		   jp->jsonp_magic, MOMJSONP_MAGIC);
  pthread_mutex_destroy(&jp->jsonp_mtx);
  fclose(jp->jsonp_file);
  memset (jp, 0, sizeof(struct jsonparser_st));
}

static momval_t parse_json_internal (struct jsonparser_st *jp);

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

static void compute_json_array_hash (momjsonarray_t * jarr);

momval_t
mom_parse_json (struct jsonparser_st *jp, char **perrmsg)
{
  momval_t res = MONIMELT_NULLV;
  if (perrmsg)
    *perrmsg = NULL;
  if (!jp || jp->jsonp_magic != MOMJSONP_MAGIC)
    {
      if (perrmsg)
	{
	  *perrmsg = "invalid JSON parser";
	  return MONIMELT_NULLV;
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
      return MONIMELT_NULLV;
    }
  res = parse_json_internal (jp);
  if (res.ptr == MONIMELT_EMPTY)
    JSONPARSE_ERROR (jp, "unexpected terminator %c", jp->jsonp_c);
  pthread_mutex_unlock (&jp->jsonp_mtx);
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
  if (jp->jsonp_c < 0)
    jp->jsonp_c = getc (jp->jsonp_file);
  if (jp->jsonp_c < 0)
    JSONPARSE_ERROR (jp, "end of file at offset %ld", ftell (jp->jsonp_file));
  if (isspace (jp->jsonp_c))
    {
      jp->jsonp_c = getc (jp->jsonp_file);
      goto again;
    }
  else if (jp->jsonp_c == ',' || jp->jsonp_c == ':' || jp->jsonp_c == '}'
	   || jp->jsonp_c == ']')
    // don't consume the terminator! Leave it available to the caller.
    return (momval_t) MONIMELT_EMPTY;
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
	  while (jp->jsonp_c >= 0 && jp->jsonp_c != '\n' && jp->jsonp_c != '\r');
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
  else if (isdigit (jp->jsonp_c) || jp->jsonp_c == '+' || jp->jsonp_c == '-')
    {
      char numbuf[64];
      long off = ftell (jp->jsonp_file);
      memset (numbuf, 0, sizeof (numbuf));
      int ix = 0;
      bool isfloat = false;
      do
	{
	  if (ix < sizeof (numbuf) - 1)
	    numbuf[ix++] = jp->jsonp_c;
	  jp->jsonp_c = getc (jp->jsonp_file);
	}
      while (isdigit (jp->jsonp_c));
      if (jp->jsonp_c == '.')
	{
	  isfloat = true;
	  do
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  while (isdigit (jp->jsonp_c));
	  if (jp->jsonp_c == 'E' || jp->jsonp_c == 'e')
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    };
	  if (jp->jsonp_c == '+' || jp->jsonp_c == '-')
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    };
	  do
	    {
	      if (ix < sizeof (numbuf) - 1)
		numbuf[ix++] = jp->jsonp_c;
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  while (isdigit (jp->jsonp_c));
	}
      numbuf[sizeof (numbuf) - 1] = (char) 0;
      char *end = NULL;
      if (isfloat)
	{
	  double x = strtod (numbuf, &end);
	  if (end && *end)
	    JSONPARSE_ERROR (jp, "bad float number %s at offset %ld", numbuf, off);
	  return (momval_t) mom_make_double (x);
	}
      else
	{
	  long l = strtol (numbuf, &end, 10);
	  if (end && *end)
	    JSONPARSE_ERROR (jp, "bad number %s at offset %ld", numbuf, off);
	  return (momval_t) mom_make_int (l);
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
      while (isalnum (jp->jsonp_c) || jp->jsonp_c == '_');
      if (!strcmp (namestr, "null"))
	return MONIMELT_NULLV;
      else if (!strcmp (namestr, "true"))
	return (momval_t) mom_get_item_bool (true);
      else if (!strcmp (namestr, "false"))
	return (momval_t) mom_get_item_bool (false);
      else
	return json_item_or_string (namestr);
    }
  else if (jp->jsonp_c == '[')
    {
      unsigned arrsize = 8;
      unsigned arrlen = 0;
      momval_t *arrptr = GC_MALLOC (arrsize * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!arrptr))
	MONIMELT_FATAL ("cannot allocate initial json array of %u", arrsize);
      memset (arrptr, 0, arrsize * sizeof (momval_t));
      jp->jsonp_c = getc (jp->jsonp_file);
      momval_t comp = MONIMELT_NULLV;
      bool gotcomma = false;
      do
	{
	  comp = parse_json_internal (jp);
	  if (comp.ptr == MONIMELT_EMPTY)
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
		JSONPARSE_ERROR (jp, "invalid char %c in JSON array at offset %ld",
			    jp->jsonp_c, ftell (jp->jsonp_file));
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
      while (jp->jsonp_c >= 0);
      struct momjsonarray_st *jarr =
	GC_MALLOC (sizeof (struct momjsonarray_st) +
		   arrlen * sizeof (momval_t));
      if (MONIMELT_UNLIKELY (!jarr))
	MONIMELT_FATAL ("failed to build JSON array of %d components",
			(int) arrlen);
      memset (jarr, 0,
	      sizeof (struct momjsonarray_st) + arrlen * sizeof (momval_t));
      for (unsigned ix = 0; ix < arrlen; ix++)
	{
	  jarr->jarrtab[ix] = arrptr[ix];
	}
      jarr->slen = arrlen;
      compute_json_array_hash (jarr);
      jarr->typnum = momty_jsonarray;
      GC_FREE (arrptr);
      return (momval_t) jarr;
    }
  else if (jp->jsonp_c == '{')
    {
      unsigned jsize = 4, jcount = 0;
      long off = ftell (jp->jsonp_file);
      jp->jsonp_c = getc (jp->jsonp_file);
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
	      if (jp->jsonp_c == '}')
		{
		  jp->jsonp_c = getc (jp->jsonp_file);
		  break;
		}
	      else
		JSONPARSE_ERROR (jp,
			    "failed to parse attribute in JSON object at offset %ld",
			    off);
	    }
	  while (isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  if (jp->jsonp_c == ':')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
	    }
	  else
	    JSONPARSE_ERROR (jp,
			"missing colon in JSON object after name at offset %ld",
			ftell (jp->jsonp_file));
	  momval_t valv = parse_json_internal (jp);
	  if (valv.ptr == MONIMELT_EMPTY)
	    JSONPARSE_ERROR (jp,
			"failed to parse value in JSON object at offset %ld",
			ftell (jp->jsonp_file));
	  while (isspace (jp->jsonp_c))
	    jp->jsonp_c = getc (jp->jsonp_file);
	  if (jp->jsonp_c == ',')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
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
      while (jp->jsonp_c >= 0);
      return (momval_t) mom_make_json_object (MOMJSON_COUNTED_ENTRIES, jcount,
					      jent, NULL);
    }
  else if (jp->jsonp_c == '"')
    {
      jp->jsonp_c = getc (jp->jsonp_file);
      unsigned siz = 24, cnt = 0;
      char *str = GC_MALLOC_ATOMIC (siz);
      if (MONIMELT_UNLIKELY (!str))
	MONIMELT_FATAL ("failed to allocate initial string buffer of %d",
			(int) siz);
      memset (str, 0, siz);
      do
	{
	  if (jp->jsonp_c == '"')
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
	  if (MONIMELT_UNLIKELY (jp->jsonp_c < 0))
	    JSONPARSE_ERROR (jp, "unterminated string at offset %ld",
			ftell (jp->jsonp_file));
	  if (jp->jsonp_c == '\\')
	    {
	      jp->jsonp_c = getc (jp->jsonp_file);
#define ADD1CHAR(Ch) do { str[cnt++] = Ch;		\
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
		    if (fscanf (jp->jsonp_file, "%04x", &h) > 0)
		      {
			char hexd[8];
			memset (hexd, 0, sizeof (hexd));
			uint32_t c = (uint32_t) h;
			g_unichar_to_utf8((gunichar)c,hexd);
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
	    ADD1CHAR (jp->jsonp_c);
#undef ADD1CHAR
	  if (jp->jsonp_c < 0)
	    break;
	}
      while (jp->jsonp_c != '"');
      return (momval_t) mom_make_string_len (str, cnt);
    }
  else
    JSONPARSE_ERROR (jp, "unexpected char %c at offset %ld",
		jp->jsonp_c, ftell (jp->jsonp_file));
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
			&& (namv.pjsonitem->ij_namejson)))
		&& mom_is_jsonable (attv))
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
	    if (namstr && namstr[0] && mom_is_jsonable (attv))
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
				&& (namv.pjsonitem->ij_namejson)))
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

static void
compute_json_array_hash (momjsonarray_t * jarr)
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
  momjsonarray_t *jarr =
    GC_MALLOC (sizeof (momjsonarray_t) + nbelem * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!jarr))
    MONIMELT_FATAL ("failed to make JSON array of %u elements", nbelem);
  memset (jarr, 0, sizeof (momjsonarray_t) + nbelem * sizeof (momval_t));
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
  compute_json_array_hash (jarr);
  return jarr;
}


const momjsonarray_t *
mom_make_json_array_count (unsigned nbelem, const momval_t * arr)
{
  if (!arr)
    return NULL;
  momjsonarray_t *jarr =
    GC_MALLOC (sizeof (momjsonarray_t) + nbelem * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!jarr))
    MONIMELT_FATAL ("failed to make JSON array of %u elements", nbelem);
  memset (jarr, 0, sizeof (momjsonarray_t) + nbelem * sizeof (momval_t));
  for (unsigned ix = 0; ix < nbelem; ix++)
    {
      momval_t comp = arr[ix];
      if (!mom_is_jsonable (comp))
	continue;
      jarr->jarrtab[ix] = comp;
    }
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
      momval_t valv = MONIMELT_NULLV;
      do
	{
	  nbelem++;
	  valv = va_arg (args, momval_t);
	}
      while (valv.ptr != NULL);
    }
  va_end (args);
  momjsonarray_t *jarr =
    GC_MALLOC (sizeof (momjsonarray_t) + nbelem * sizeof (momval_t));
  if (MONIMELT_UNLIKELY (!jarr))
    MONIMELT_FATAL ("failed to make JSON array of %u elements", nbelem);
  memset (jarr, 0, sizeof (momjsonarray_t) + nbelem * sizeof (momval_t));
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
  if (*l.ptype == momty_jsonitem)
    l = (momval_t) (l.pjsonitem->ij_namejson);
  if (*r.ptype == momty_jsonitem)
    r = (momval_t) (r.pjsonitem->ij_namejson);
  if (l.ptr == r.ptr)
    return 0;
  return mom_value_cmp (l, r);
}

const momval_t
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

#define MOMJSONO_MAGIC 0x5cd05c95 /* json outmagic 1557159061 */
void mom_json_output_initialize (struct jsonoutput_st*jo, FILE*f, void*data, unsigned flags)
{
  memset (jo, 0, sizeof(struct jsonoutput_st));
  jo->jsono_flags = flags;
  pthread_mutex_init(&jo->jsono_mtx, NULL);
  jo->jsono_file=f;
  jo->jsono_data=data;
  jo->jsono_magic = MOMJSONO_MAGIC;
}

void* mom_json_output_data (const struct jsonoutput_st*jo)
{
  if (!jo) return NULL;
  if (jo->jsono_magic != MOMJSONO_MAGIC)
    MONIMELT_FATAL("bad json output magic %x (expected %x)",
		   jo->jsono_magic, MOMJSONO_MAGIC);
  return jo->jsono_data;
}

void mom_json_output_end (struct jsonoutput_st*jo)
{
  if (!jo) return;
  if (jo->jsono_magic != MOMJSONO_MAGIC)
    MONIMELT_FATAL("bad json output magic %x (expected %x)",
		   jo->jsono_magic, MOMJSONO_MAGIC);
  pthread_mutex_destroy(&jo->jsono_mtx);
  if (jo->jsono_flags & jsof_flush)
    fflush(jo->jsono_file);
  memset (jo, 0, sizeof(struct jsonoutput_st));
}

void mom_json_output_close (struct jsonoutput_st*jo)
{
  if (!jo) return;
  if (jo->jsono_magic != MOMJSONO_MAGIC)
    MONIMELT_FATAL("bad json output magic %x (expected %x)",
		   jo->jsono_magic, MOMJSONO_MAGIC);
  pthread_mutex_destroy(&jo->jsono_mtx);
  fclose(jo->jsono_file);
  memset (jo, 0, sizeof(struct jsonoutput_st));
}

static void output_val(struct jsonoutput_st*jo, momval_t val, unsigned depth)
{
  if (!val.ptr) {
    fputs("null",jo->jsono_file);
    return;
  }
  unsigned typ = *val.ptype;
  switch (typ) {
  case momty_int:
    fprintf(jo->jsono_file,"%ld",(long)val.pint->intval);
    return;
  case momty_float:
    {
      double x = val.pfloat->floval;
      if (MONIMELT_UNLIKELY(isnan(x))) {
	if (jo->jsono_flags & jsof_cname)
	  fputs("nan", jo->jsono_file);
	else
	  fputs("null", jo->jsono_file);
      }
      else{
	char numbuf[48];
	memset(numbuf, 0, sizeof(numbuf));
	snprintf(numbuf, sizeof(numbuf), "%.3f", x);
	if (atof(numbuf)==x && strlen(numbuf)<20) {
	  fputs(numbuf,jo->jsono_file);
	  return;
	}
	snprintf(numbuf, sizeof(numbuf), "%.6f", x);
	if (atof(numbuf)==x && strlen(numbuf)<30) {
	  fputs(numbuf,jo->jsono_file);
	  return;
	}
	snprintf(numbuf, sizeof(numbuf), "%.15f", x);
	if (atof(numbuf)==x && strlen(numbuf)<30) {
	  fputs(numbuf,jo->jsono_file);
	  return;
	}
	snprintf(numbuf, sizeof(numbuf), "%#.10g", x);
	if (atof(numbuf)==x && strlen(numbuf)<30) {
	  fputs(numbuf,jo->jsono_file);
	  return;
	}
	snprintf(numbuf, sizeof(numbuf), "%#.15g", x);
	fputs(numbuf,jo->jsono_file);
      }
    }
    break;
  case momty_string:
    {
      putc('"', jo->jsono_file);
      unsigned slen = val.pstring->slen;
      const gchar* cs = (const gchar*)(val.pstring->cstr);
      const gchar*end = NULL;
      if (MONIMELT_UNLIKELY(!g_utf8_validate(cs,-1,&end)))
	MONIMELT_FATAL("invalid UTF-8 string %s", cs);
      for (const gchar*pc=cs; *pc && pc<cs+slen; pc=g_utf8_next_char(pc)) {
	gunichar c=g_utf8_get_char(pc);
	switch (c) {
	case '\\': fputs("\\\\", jo->jsono_file); break;
	case '"' : fputs("\\\"", jo->jsono_file); break;
	case '\b': fputs("\\b", jo->jsono_file); break;
	case '\f': fputs("\\f", jo->jsono_file); break;
	case '\n': fputs("\\n", jo->jsono_file); break;
	case '\r': fputs("\\r", jo->jsono_file); break;
	case '\t': fputs("\\t", jo->jsono_file); break;
	default:
	  if ((unsigned)c<0x7f && (char)c>=' ' && isprint(c)) {
	    putc(c, jo->jsono_file);
	  }
	  else {
	    fprintf(jo->jsono_file, "\\u%04x", (unsigned)c);
	  }
	}
      }
      putc('"', jo->jsono_file);
    }
    break;
  case momty_jsonitem:
    {
      const momstring_t*nstr = val.pjsonitem->ij_namejson;
      if (MONIMELT_UNLIKELY(!nstr || nstr->typnum != momty_string)) {
	char uidstr[40];
	memset (uidstr, 0, sizeof(uidstr));
	uuid_unparse(val.panyitem->i_uuid, uidstr);
	MONIMELT_FATAL("corrupted JSON item of uid %s without string JSON name",
		       uidstr);
      }
      if ((jo->jsono_flags & jsof_cname)
	  && (isalpha(nstr->cstr[0]) ||nstr->cstr[0]=='_')) {
	bool isident=true;
	for (const char*c = nstr->cstr; *c && isident; c++)
	  isident= (isalnum(*c)|| *c=='_');
	if (isident && strcmp(nstr->cstr, "null")
	    && strcmp(nstr->cstr, "nan")
	    && strcmp(nstr->cstr, "true") && strcmp(nstr->cstr, "false"))
	  {
	    fputs(nstr->cstr, jo->jsono_file);
	    return;
	  }
      };
      output_val(jo,(momval_t)nstr,depth);
      return;      
    }
  case momty_boolitem:
    if (val.pboolitem->ib_bool)
      fputs("true",jo->jsono_file);
    else
      fputs("false",jo->jsono_file);
    return;
  case momty_jsonarray:
  case momty_jsonobject:
#warning unimplemented output of JSON array & objects
  default:
    fputs(" null",jo->jsono_file);
    return;
  }
}
