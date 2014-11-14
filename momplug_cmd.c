// file momplug_cmd.c

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

/** following is for our Makefile, see rule for momplug_%.so there
   MONIMELTLIBS: -lreadline
**/

#include "monimelt.h"

#include <readline/readline.h>
#include <readline/history.h>

const char mom_plugin_GPL_compatible[] = "GPLv3+";

// the command stack has values and marks
static momval_t *vst_valarr_mom;
static char *vst_markarr_mom;
static unsigned vst_size;
static unsigned vst_top;

static void
cmd_stack_push_mom (momval_t val, char mark)
{
  if (vst_top >= vst_size)
    {
      unsigned newsiz = (4 * vst_size / 3 + 21) | 0x1f;
      momval_t *newvalarr =
	MOM_GC_ALLOC ("valarr", newsiz * sizeof (momval_t));
      char *newmarkarr = MOM_GC_SCALAR_ALLOC ("markarr", newsiz + 1);
      if (vst_top > 0)
	{
	  memcpy (newvalarr, vst_valarr_mom, vst_top * sizeof (momval_t));
	  memcpy (newmarkarr, vst_markarr_mom, vst_top);
	  MOM_GC_FREE (vst_valarr_mom);
	  MOM_GC_FREE (vst_markarr_mom);
	}
      vst_valarr_mom = newvalarr;
      vst_markarr_mom = newmarkarr;
    }
  vst_valarr_mom[vst_top] = val;
  vst_markarr_mom[vst_top] = mark;
  vst_top++;
}

static inline momval_t
cmd_stack_nth_value_mom (int rk)
{
  if (rk < 0)
    rk += vst_top;
  if (rk >= 0 && rk < (int) vst_top)
    return vst_valarr_mom[vst_top - rk];
  return MOM_NULLV;
}

static inline char
cmd_stack_nth_mark_mom (int rk)
{
  if (rk < 0)
    rk += vst_top;
  if (rk >= 0 && rk < (int) vst_top)
    return vst_markarr_mom[vst_top - rk];
  return 0;
}

static void
cmd_stack_pop_mom (unsigned nb)
{
  if (nb < vst_top && nb > 0)
    {
      memset (vst_valarr_mom + vst_top - nb, 0, sizeof (momval_t) * nb);
      memset (vst_markarr_mom + vst_top - nb, 0, sizeof (char) * nb);
      vst_top -= nb;
    }
}

#define COMMANDS(CMD)				\
  CMD(quit,"quit without dumping")		\
  CMD(help,"give help")				\
  CMD(exit,"dump & exit")			\
  CMD(stack,"print the stack")			\
  CMD(top,"print the top of stack")		\
  CMD(dump,"dump state & continue")		\
  CMD(dup,"duplicate top")

// readline alternate completion function
static char **
cmd_completion_mom (const char *text, int start, int end)
{
  char **resarr = NULL;
  MOM_DEBUGPRINTF (run, "cmd_completion text=%s start=%d end=%d", text, start,
		   end);
  if (start == 0 && end > start && isalpha (text[0]))
    {
      momval_t jarr = MOM_NULLV;
      momval_t tup =
	(momval_t) mom_alpha_ordered_tuple_of_named_prefixed_items (text,
								    &jarr);
      unsigned nbent = mom_tuple_length (tup);
      MOM_DEBUGPRINTF (run, "cmd_completion nbent=%u", nbent);
      if (!nbent)
	return NULL;
      resarr = calloc (nbent + 1, sizeof (char *));
      if (MOM_UNLIKELY (!resarr))
	MOM_FATAPRINTF ("cmd_completion failed to calloc resarr nbent=%d",
			nbent);
      for (unsigned ix = 0; ix < nbent; ix++)
	{
	  resarr[ix] =
	    strdup (mom_string_cstr (mom_json_array_nth (jarr, ix)));
	  if (MOM_UNLIKELY (!resarr[ix]))
	    MOM_FATAPRINTF ("cmd_completion failed to strdup ix=%d", ix);
	  MOM_DEBUGPRINTF (run, "cmd_completion resarr[%d]=%s", ix,
			   resarr[ix]);
	}
    }
  return resarr;
}

void
mom_plugin_init (const char *arg)
{
  MOM_DEBUGPRINTF (run, "start of " __FILE__ " arg=%s", arg);
  rl_initialize ();
  rl_readline_name = "monimelt";
  rl_attempted_completion_function = cmd_completion_mom;
}

#define POLL_TIMEOUT 500
void
momplugin_after_load (void)
{
  int cnt = 0;
  MOM_INFORMPRINTF ("momplug_cmd starting after load");
  char *lin = NULL;
  for (;;)
    {
      char prompt[64];
      cnt++;
      snprintf (prompt, sizeof (prompt), "monimelt%03d: ", cnt);
      lin = NULL;
      MOM_DEBUGPRINTF (run, "before readline prompt=%s", prompt);
      lin = readline (prompt);
      MOM_DEBUGPRINTF (run, "after readline lin=%s", lin);
      if (!lin || !strcmp (lin, "quit"))
	break;
    };
  MOM_INFORMPRINTF ("momplug_cmd ending after load");
}
