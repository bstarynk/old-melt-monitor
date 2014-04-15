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
  int json_c;		/* read ahead character */
  pthread_mutex_t json_mtx;
  FILE* json_file;
  void* json_data;
  char* json_error;
  jmp_buf json_jmpbuf;
};


void mom_initialize_json_parser (struct jsonparser_st *jp, FILE* file, void* data)
{
  if (!file || !jp) return;
  memset (jp, 0, sizeof(struct jsonparser_st));
  pthread_mutex_init(&jp->json_mtx, NULL);
  jp->json_c = EOF;
  jp->json_file = file;
  jp->json_data = data;
  jp->json_magic = MOMJSON_MAGIC;
  jp->json_error = NULL;
}

static momval_t parse_json_internal(struct jsonparser_st* jp);

momval_t mom_parse_json(struct jsonparser_st* jp, char**perrmsg)
{
  momval_t res = MONIMELT_NULLV;
  if (perrmsg) *perrmsg = NULL;
  if (!jp || jp->json_magic != MOMJSON_MAGIC) {
    if (perrmsg) {
      *perrmsg = "invalid JSON parser";
      return MONIMELT_NULLV;
    }
  }
  pthread_mutex_lock (&jp->json_mtx);
  if (setjmp(jp->json_jmpbuf)) {
    if (perrmsg) {
      if (jp->json_error) *perrmsg = jp->json_error;
      else *perrmsg = "JSON parsing failed";
    }
    pthread_mutex_unlock (&jp->json_mtx);
    return MONIMELT_NULLV;
  }
  res = parse_json_internal(jp);
  pthread_mutex_unlock (&jp->json_mtx);
  return res;
}


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

static momval_t
parse_json_internal(struct jsonparser_st* jp)
{
 again:
  if (jp->json_c<0)
    jp->json_c = getc(jp->json_file);
  if (jp->json_c<0)
    JSON_ERROR(jp, "end of file at offset %ld", ftell(jp->json_file));
  if (isspace(jp->json_c)) {
    jp->json_c = getc(jp->json_file);
    goto again;
  }
  else if (isdigit(jp->json_c) || jp->json_c == '+' || jp->json_c == '-') {
    static char numbuf[64];
    long off = ftell(jp->json_file);
    memset (numbuf, 0, sizeof(numbuf));
    int ix=0;
    bool isfloat = false;
    do {
      if (ix<sizeof(numbuf)-1) numbuf[ix++] = jp->json_c;
      jp->json_c = getc(jp->json_file);
    } while (isdigit(jp->json_c));
    if (jp->json_c == '.') {
      isfloat = true;
      do {
	if (ix<sizeof(numbuf)-1) numbuf[ix++] = jp->json_c;
	jp->json_c = getc(jp->json_file);
      } while (isdigit(jp->json_c));
      if (jp->json_c == 'E' || jp->json_c == 'e'){
	if (ix<sizeof(numbuf)-1) numbuf[ix++] = jp->json_c;
	jp->json_c = getc(jp->json_file);
      };
      if (jp->json_c == '+' || jp->json_c == '-'){
	if (ix<sizeof(numbuf)-1) numbuf[ix++] = jp->json_c;
	jp->json_c = getc(jp->json_file);
      };
      do {
	if (ix<sizeof(numbuf)-1) numbuf[ix++] = jp->json_c;
	jp->json_c = getc(jp->json_file);
      } while (isdigit(jp->json_c));
    }
    numbuf[sizeof(numbuf)-1] = (char)0;
    char *end=NULL;
    if (isfloat) {
      double x = strtod(numbuf, &end);
      if (end && *end)
	JSON_ERROR(jp, "bad float number %s at offset %ld", numbuf,
		   off);
      return (momval_t)mom_make_double(x);
    }
    else {
      long l = strtol(numbuf, &end, 10);
      if (end && *end)
	JSON_ERROR(jp, "bad number %s at offset %ld", numbuf,
		   off);
      return (momval_t)mom_make_int(l);
    }
  }
    
}

#if 0
#error useless old code
enum jsonstate_en
{
  jse_none,
  jse_startjson,
  jse_parsename,
  jse_parsenumber,
  jse_parsestring,
  jse_parsecomparr,
  jse_parseattrobj,
};

static inline void
push_state (struct jsonparser_st *jp, unsigned state, void *val,
	    unsigned rank)
{
  unsigned jtop = jp->json_top;
  if (MONIMELT_UNLIKELY (jtop + 2 >= jp->json_size))
    {
      unsigned newsize = ((3 * jtop / 2 + 50) | 0x3f) + 1;
      if (newsize > jp->json_size)
	{
	  void **newvalarr = GC_MALLOC (sizeof (void *) * newsize);
	  if (MONIMELT_UNLIKELY (!newvalarr))
	    MONIMELT_FATAL ("failed to grow json state value stack to %u",
			    newsize);
	  memset (newvalarr, 0, sizeof (void *) * newsize);
	  memcpy (newvalarr, jp->json_ptrarr, sizeof (void *) * jtop);
	  struct jsonstatelevel_st *newlevarr =
	    GC_MALLOC_ATOMIC (sizeof (struct jsonstatelevel_st) * newsize);
	  if (MONIMELT_UNLIKELY (!newlevarr))
	    MONIMELT_FATAL ("failed to grow json state level stack to %u",
			    newsize);
	  memset (newlevarr, 0, sizeof (struct jsonstatelevel_st) * newsize);
	  memcpy (newlevarr, jp->json_levarr,
		  sizeof (struct jsonstatelevel_st) * jtop);
	  union jsonnum_un *newnumarr =
	    GC_MALLOC_ATOMIC (sizeof (union jsonnum_un) * newsize);
	  if (MONIMELT_UNLIKELY (!newnumarr))
	    MONIMELT_FATAL ("failed to grow json state number stack to %u",
			    newsize);
	  memcpy (newnumarr, jp->json_numarr,
		  sizeof (union jsonnum_un) * jtop);
	  GC_FREE (jp->json_ptrarr);
	  jp->json_ptrarr = newvalarr;
	  GC_FREE (jp->json_levarr);
	  jp->json_levarr = newlevarr;
	  GC_FREE (jp->json_numarr);
	  jp->json_numarr = newnumarr;
	}
    }
  jp->json_ptrarr[jtop] = val;
  jp->json_levarr[jtop].je_rank = rank;
  jp->json_levarr[jtop].je_state = state;
  jp->json_numarr[jtop].dbl = 0.0;
  jp->json_numarr[jtop].num = 0L;
  jp->json_top = jtop + 1;
}

static inline void
replace_state (struct jsonparser_st *jp, unsigned state, void *val,
	       unsigned rank)
{
  unsigned jtop = jp->json_top;
  if (jtop > 0)
    {
      jp->json_ptrarr[jtop - 1] = val;
      jp->json_levarr[jtop - 1].je_rank = rank;
      jp->json_levarr[jtop - 1].je_state = state;
      jp->json_numarr[jtop - 1].dbl = 0.0;
      jp->json_numarr[jtop - 1].num = 0L;
    }
}

static inline void
pop_state (struct jsonparser_st *jp)
{
  unsigned jtop = jp->json_top;
  if (jtop == 0)
    return;
  jp->json_ptrarr[jtop] = NULL;
  jp->json_levarr[jtop].je_state = 0;
  jp->json_levarr[jtop].je_rank = 0;
  jp->json_numarr[jtop].dbl = 0.0;
  jp->json_numarr[jtop].num = 0L;
  jp->json_top = jtop - 1;
}

static inline void
set_top_pointer (struct jsonparser_st *jp, const void *p)
{
  unsigned jtop = jp->json_top;
  if (jtop == 0)
    return;
  jp->json_ptrarr[jtop] = (void *) p;
}

static inline void
set_top_rank (struct jsonparser_st *jp, unsigned rk)
{
  unsigned jtop = jp->json_top;
  if (jtop == 0)
    return;
  jp->json_levarr[jtop].je_rank = rk;
}

static inline void
set_top_num (struct jsonparser_st *jp, intptr_t num)
{
  unsigned jtop = jp->json_top;
  if (jtop == 0)
    return;
  jp->json_numarr[jtop].num = num;
}

static inline void
set_top_dbl (struct jsonparser_st *jp, double dbl)
{
  unsigned jtop = jp->json_top;
  if (jtop == 0)
    return;
  jp->json_numarr[jtop].dbl = dbl;
}

static inline void
set_top_state (struct jsonparser_st *jp, unsigned state)
{
  unsigned jtop = jp->json_top;
  if (jtop == 0)
    return;
  jp->json_levarr[jtop].je_state = state;
}


void
mom_json_initialize (struct jsonparser_st *jp)
{
  memset (jp, 0, sizeof (*jp));
  const unsigned inisiz = 64;
  jp->json_top = 0;
  jp->json_size = inisiz;
  jp->json_ptrarr = GC_MALLOC (sizeof (void *) * inisiz);
  if (!jp->json_ptrarr)
    MONIMELT_FATAL ("failed to initialize json values sized %u", inisiz);
  memset (jp->json_ptrarr, 0, sizeof (void *) * inisiz);
  jp->json_levarr =
    GC_MALLOC_ATOMIC (sizeof (struct jsonstatelevel_st) * inisiz);
  if (!jp->json_levarr)
    MONIMELT_FATAL ("failed to initialize json states sized %u", inisiz);
  memset (jp->json_levarr, 0, sizeof (struct jsonstatelevel_st) * inisiz);
  jp->json_magic = MOMJSON_MAGIC;
  push_state (jp, jse_startjson, NULL, 0u);
}

// return the number of consumed bytes
int
mom_json_consume (struct jsonparser_st *jp, const char *buf, int len)
{
  if (!jp || jp->json_magic != MOMJSON_MAGIC || !buf)
    return -1;
  if (len < 0)
    len = strlen (buf);
  const char *cp = buf;
  const char *endp = buf + len;
  for (;;)
    {
      unsigned jtop = jp->json_top;
      if (jtop == 0)
	return -1;
      unsigned curstate = jp->json_levarr[jtop - 1].je_state;
      unsigned currank = jp->json_levarr[jtop - 1].je_rank;
      void *curptr = jp->json_ptrarr[jtop - 1];
      switch (curstate)
	{
	case jse_startjson:
	  {
	    while (isspace (*cp) && cp < endp)
	      cp++;
	    if (cp >= endp)
	      return endp - buf;
	    if (isalpha (*cp))
	      {
		const unsigned lenstr = 32;
		char *newstr = GC_MALLOC_ATOMIC (lenstr);
		if (MONIMELT_UNLIKELY (!newstr))
		  MONIMELT_FATAL ("out of space for JSON name of %u", lenstr);
		memset (newstr, 0, lenstr);
		replace_state (jp, jse_parsename, newstr, lenstr);
		continue;
	      }
	    else
	      if (((cp[0] == '-' || cp[0] == '+') && cp + 1 < endp
		   && isdigit (cp[1])) || (isdigit (cp[0]) && cp < endp))
	      {
		const unsigned lenstr = 32;
		char *newstr = GC_MALLOC_ATOMIC (lenstr);
		if (MONIMELT_UNLIKELY (!newstr))
		  MONIMELT_FATAL ("out of space for JSON number of %u",
				  lenstr);
		memset (newstr, 0, lenstr);
		if ((cp[0] == '+' || cp[0] == '-') && isdigit (cp[1]))
		  {
		    newstr[0] = cp[0];
		    newstr[1] = cp[1];
		    cp += 2;
		  }
		else if (isdigit (cp[0]))
		  {
		    newstr[0] = cp[0];
		    cp++;
		  }
		replace_state (jp, jse_parsenumber, newstr, lenstr);
		continue;
	      }
	    else if (cp[0] == '"' && cp + 1 < endp)
	      {
		cp++;
		const char *start = cp;
		while (cp + 1 < endp && *cp != '"' && *cp != '\\'
		       && isprint (*cp))
		  cp++;
		const char *nextquote = memchr (cp, '"', endp - cp);
		unsigned lenstr =
		  nextquote ? (1 + ((nextquote - start + 5) | 0x1f))
		  : (endp > start + 30) ? (1 + ((endp - start) | 0x1f)) : 32;
		char *newstr = GC_MALLOC_ATOMIC (lenstr);
		if (MONIMELT_UNLIKELY (!newstr))
		  MONIMELT_FATAL ("out of space for JSON string of %u",
				  lenstr);
		memset (newstr, 0, lenstr);
		memcpy (newstr, start, cp - start);
		replace_state (jp, jse_parsestring, newstr, lenstr);
		set_top_num (jp, cp - start);
		continue;
	      }
	    else if (*cp == '[' && cp < endp)
	      {
		const int arrsize = 8;
		momval_t *newvalarr = GC_MALLOC (sizeof (momval_t) * arrsize);
		if (MONIMELT_UNLIKELY (!newvalarr))
		  MONIMELT_FATAL ("out of space for initial array of %i",
				  arrsize);
		memset (newvalarr, 0, sizeof (momval_t) * arrsize);
		replace_state (jp, jse_parsecomparr, newvalarr, arrsize);
		push_state (jp, jse_startjson, NULL, 0);
#warning not sure about pushed states....
		continue;
	      }
	    else if (*cp == '{' && cp < endp)
	      {
	      }
	    else if (cp < endp)
	      MONIMELT_FATAL ("bad JSON %.40s", cp);
	    break;
	  }
	case jse_parsename:
	  {
	    const char *begname = cp;
	    while ((isalnum (*cp) || *cp == '_') && cp < endp)
	      cp++;
	    unsigned curnamlen = strlen ((char *) curptr);
	    if (MONIMELT_UNLIKELY (curnamlen + (cp - begname) >= currank))
	      {
		unsigned newlen =
		  ((5 * currank / 4 + 5 + (endp - cp)) | 0xf) + 1;
		char *newstr = GC_MALLOC_ATOMIC (newlen);
		if (MONIMELT_UNLIKELY (!newstr))
		  MONIMELT_FATAL ("out of space for JSON name of %u", newlen);
		memset (newstr, 0, newlen);
		strcpy (newstr, (char *) curptr);
		GC_FREE (jp->json_ptrarr[jtop - 1]);
		currank = jp->json_levarr[jtop - 1].je_rank = newlen;
		curptr = jp->json_ptrarr[jtop - 1] = newstr;
	      };
	    strncpy ((char *) curptr + curnamlen, begname, cp - begname);
	    if (cp >= endp)
	      return endp - buf;
	    if (!strcmp ((char *) curptr, "null"))
	      {
		pop_state (jp);
		set_top_pointer (jp, NULL);
		GC_FREE (curptr);
		continue;
	      }
	    else if (!strcmp ((char *) curptr, "true"))
	      {
		pop_state (jp);
		set_top_pointer (jp, mom_get_item_bool (true));
		GC_FREE (curptr);
		continue;
	      }
	    else if (!strcmp ((char *) curptr, "false"))
	      {
		pop_state (jp);
		set_top_pointer (jp, mom_get_item_bool (false));
		GC_FREE (curptr);
		continue;
	      }
	    else if (!strcmp ((char *) curptr, "nan"))
	      {
		pop_state (jp);
		set_top_pointer (jp, mom_make_double (NAN));
		GC_FREE (curptr);
		continue;
	      }
	    else
	      {
		pop_state (jp);
		set_top_pointer (jp, mom_item_named ((char *) curptr));
		GC_FREE (curptr);
		continue;
	      }
	    break;
	  }
	case jse_parsenumber:
	  {
	    const char *begnum = cp;
	    unsigned curnumlen = strlen ((char *) curptr);
	    while (isdigit (*cp) && cp < endp)
	      cp++;
	    bool hasdot = strchr (curptr, '.') != NULL;
	    bool hasexp = strchr (curptr, 'e') != NULL
	      || strchr (curptr, 'E') != NULL;
	    if (!hasdot && *cp == '.' && cp < endp)
	      {
		cp++;
		hasdot = true;
	      };
	    if (hasdot)
	      {
		while (isdigit (*cp) && cp < endp)
		  cp++;
		if (!hasexp)
		  {
		    if (cp + 2 < endp && (cp[0] == 'e' || cp[0] == 'E')
			&& isdigit (cp[1]))
		      cp += 2;
		    else if (cp + 3 < endp && (cp[0] == 'e' || cp[0] == 'E')
			     && (cp[1] == '+' || cp[1] == '-')
			     && isdigit (cp[2]))
		      cp += 3;
		    hasexp = true;
		  };
		if (hasexp)
		  while (isdigit (*cp) && cp < endp)
		    cp++;
	      }
	    if (MONIMELT_UNLIKELY (curnumlen + (cp - begnum) >= currank))
	      {
		unsigned newlen =
		  ((5 * currank / 4 + 5 + (endp - cp)) | 0xf) + 1;
		char *newstr = GC_MALLOC_ATOMIC (newlen);
		if (MONIMELT_UNLIKELY (!newstr))
		  MONIMELT_FATAL ("out of space for JSON number of %u",
				  newlen);
		memset (newstr, 0, newlen);
		strcpy (newstr, (char *) curptr);
		GC_FREE (jp->json_ptrarr[jtop - 1]);
		currank = jp->json_levarr[jtop - 1].je_rank = newlen;
		curptr = jp->json_ptrarr[jtop - 1] = newstr;
	      };
	    strncpy ((char *) curptr + curnumlen, begnum, cp - begnum);
	    if (cp >= endp)
	      return endp - buf;
	    if (hasdot)
	      {
		double x = strtod ((char *) curptr, NULL);
		pop_state (jp);
		set_top_pointer (jp, mom_make_double (x));
		GC_FREE (curptr);
	      }
	    else
	      {
		long l = strtol ((char *) curptr, NULL, 0);
		pop_state (jp);
		set_top_pointer (jp, mom_make_int (l));
		GC_FREE (curptr);
	      }
	    continue;
	  }
	case jse_parsestring:
	  {
	    const char *start = cp;
	    intptr_t curstrlen = jp->json_numarr[jtop].num;
	    unsigned curstrsize = jp->json_levarr[jtop].je_rank;
	    while (cp < endp && *cp != '"' && *cp != '\\' && isprint (*cp))
	      cp++;
	    unsigned curchklen = start - cp;
	    if (MONIMELT_UNLIKELY (curstrlen + curchklen + 12 > curstrsize))
	      {
		unsigned newsize =
		  ((curstrlen + curchklen + curstrsize / 8 + 40) | 0x1f) + 1;
		char *newstr = GC_MALLOC_ATOMIC (newsize);
		if (MONIMELT_UNLIKELY (!newstr))
		  MONIMELT_FATAL ("failed to grow JSON string to %u",
				  newsize);
		memset (newstr, 0, newsize);
		memcpy (newstr, curptr, curstrlen);
		curstrsize = jp->json_levarr[jtop].je_rank = newsize;
		set_top_pointer (jp, newstr);
		curptr = newstr;
	      }
	    if (cp > start)
	      {
		memcpy (curptr + curstrlen, start, curchklen);
		curstrlen = jp->json_numarr[jtop].num =
		  (curstrlen + curchklen);
	      }
	    if (cp >= endp)
	      break;
	    if (*cp == '\\' && cp + 1 < endp)
	      switch (cp[1])
		{
#define ADD1CHAR(Ch) do { ((char*)curptr)[curstrlen] = (Ch);		\
		  curstrlen = jp->json_numarr[jtop].num = curstrlen+1;	\
		}while(0)
		case 'b':
		  ADD1CHAR ('\b');
		  cp += 2;
		  continue;
		case 'f':
		  ADD1CHAR ('\f');
		  cp += 2;
		  continue;
		case 'n':
		  ADD1CHAR ('\n');
		  cp += 2;
		  continue;
		case 'r':
		  ADD1CHAR ('\r');
		  cp += 2;
		  continue;
		case 't':
		  ADD1CHAR ('\t');
		  cp += 2;
		  continue;
		case '"':
		  ADD1CHAR ('\"');
		  cp += 2;
		  continue;
		case '/':
		  ADD1CHAR ('/');
		  cp += 2;
		  continue;
		case '\\':
		  ADD1CHAR ('\\');
		  cp += 2;
		  continue;
		case 'u':
		  if (cp + 6 < endp
		      && isxdigit (cp[2]) && isxdigit (cp[3])
		      && isxdigit (cp[4]) && isxdigit (cp[5]))
		    {
		      char hexd[8];
		      memset (hexd, 0, sizeof (hexd));
		      hexd[0] = cp[2];
		      hexd[1] = cp[3];
		      hexd[2] = cp[4];
		      hexd[3] = cp[5];
		      uint32_t c = (uint32_t) strtol (hexd, NULL, 16);
		      memset (hexd, 0, sizeof (hexd));
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
		      cp += 6;
		      continue;
		    }
		  break;
		default:
		  ADD1CHAR (cp[1]);
		  cp += 2;
		  continue;
#undef ADD1CHAR
		}
	    else if (*cp == '"' && cp < endp)
	      {
		cp++;
		momstring_t *newstrv
		  = mom_make_string_len (curptr, jp->json_numarr[jtop].num);
		pop_state (jp);
		set_top_pointer (jp, newstrv);
		GC_FREE (curptr);
		continue;
	      }
	    break;
	  }
	default:
	  MONIMELT_FATAL ("unexpected JSON state #%u top %u", curstate, jtop);
	}
    }
}
#endif
