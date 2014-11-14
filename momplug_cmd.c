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
static unsigned vst_size_mom;
static unsigned vst_top_mom;


static void
cmd_stack_push_mom (momval_t val, char mark)
{
  if (vst_top_mom >= vst_size_mom)
    {
      unsigned newsiz = (4 * vst_size_mom / 3 + 21) | 0x1f;
      momval_t *newvalarr =
	MOM_GC_ALLOC ("valarr", newsiz * sizeof (momval_t));
      char *newmarkarr = MOM_GC_SCALAR_ALLOC ("markarr", newsiz + 1);
      if (vst_top_mom > 0)
	{
	  memcpy (newvalarr, vst_valarr_mom, vst_top_mom * sizeof (momval_t));
	  memcpy (newmarkarr, vst_markarr_mom, vst_top_mom);
	  MOM_GC_FREE (vst_valarr_mom);
	  MOM_GC_FREE (vst_markarr_mom);
	}
      vst_valarr_mom = newvalarr;
      vst_markarr_mom = newmarkarr;
    }
  vst_valarr_mom[vst_top_mom] = val;
  vst_markarr_mom[vst_top_mom] = mark;
  vst_top_mom++;
}

static inline momval_t
cmd_stack_nth_value_mom (int rk)
{
  if (rk < 0)
    rk += vst_top_mom;
  if (rk >= 0 && rk < (int) vst_top_mom)
    return vst_valarr_mom[vst_top_mom - rk];
  return MOM_NULLV;
}

static inline char
cmd_stack_nth_mark_mom (int rk)
{
  if (rk < 0)
    rk += vst_top_mom;
  if (rk >= 0 && rk < (int) vst_top_mom)
    return vst_markarr_mom[vst_top_mom - rk];
  return 0;
}

static void
cmd_stack_pop_mom (unsigned nb)
{
  if (nb < vst_top_mom && nb > 0)
    {
      memset (vst_valarr_mom + vst_top_mom - nb, 0, sizeof (momval_t) * nb);
      memset (vst_markarr_mom + vst_top_mom - nb, 0, sizeof (char) * nb);
      vst_top_mom -= nb;
    }
}

#define COMMANDS(CMD)				\
  CMD(dump,"dump state & continue")		\
  CMD(dup,"duplicate top")			\
  CMD(exit,"dump & exit")			\
  CMD(help,"give help")				\
  CMD(quit,"quit without dumping")		\
  CMD(stack,"print the stack")			\
  CMD(top,"print the top of stack")		\
				/* end of COMMANDS */

#define CMD_DECLARE(N,H) static void cmd_do_##N##_mom (const char*);
COMMANDS (CMD_DECLARE)
#undef CMD_DECLARE
     typedef void cmd_do_fun_t (const char *);
     struct cmd_descr_st
     {
       const char *cmd_name;
       cmd_do_fun_t *const cmd_fun;
       const char *cmd_help;
     };

     static struct cmd_descr_st cmd_array_mom[] = {
#define CMD_DEFINE(N,H) {.cmd_name= #N, .cmd_fun= cmd_do_##N##_mom, .cmd_help= H},
       COMMANDS (CMD_DEFINE)
#undef CMD_DEFINE
       {NULL, NULL, NULL}
     };

#define CMDARRSIZE_MOM (sizeof(cmd_array_mom)/sizeof(cmd_array_mom[0]))

// readline completion entry function
static char *
cmd_completion_entry_mom (const char *text, int state)
{
  static momval_t jarr;
  MOM_DEBUGPRINTF (cmd, "cmd_completion_entry text='%s' state=%d", text,
		   state);
  if (!state)
    {
      jarr = MOM_NULLV;
      if (isalpha (text[0]))
	{
	  jarr = MOM_NULLV;
	  momval_t tup =
	    (momval_t) mom_alpha_ordered_tuple_of_named_prefixed_items (text,
									&jarr);
	  unsigned nbent = mom_tuple_length (tup);
	  MOM_DEBUG (cmd, MOMOUT_LITERAL ("cmd_completion name nbent:"),
		     MOMOUT_DEC_INT ((int) nbent), MOMOUT_SPACE (32),
		     MOMOUT_LITERAL ("jarr="),
		     MOMOUT_VALUE ((const momval_t) jarr), NULL);
	  if (!nbent)
	    return NULL;
	}
      else if (text[0] == ',')
	{
	  jarr = MOM_NULLV;
	  unsigned cmdlen = strlen (text + 1);
	  unsigned nbent = 0;
	  momval_t jvals[CMDARRSIZE_MOM + 1] = { MOM_NULLV };
	  for (int ix = 0; ix < (int) CMDARRSIZE_MOM; ix++)
	    {
	      if (!cmd_array_mom[ix].cmd_name)
		break;
	      if (!strncmp (text + 1, cmd_array_mom[ix].cmd_name, cmdlen))
		{
		  jvals[nbent] =
		    (momval_t)
		    MOM_OUTSTRING (0, MOMOUT_LITERAL (","),
				   MOMOUT_LITERALV ((const char *)
						    cmd_array_mom
						    [ix].cmd_name));
		  nbent++;
		}
	    }
	  MOM_DEBUGPRINTF (cmd, "cmd_completion command nbent=%d", nbent);
	  if (nbent > 0)
	    {
	      jarr = (momval_t) mom_make_json_array_count (nbent, jvals);
	      MOM_DEBUG (cmd,
			 MOMOUT_LITERAL ("cmd_completion command nbent:"),
			 MOMOUT_DEC_INT ((int) nbent), MOMOUT_SPACE (32),
			 MOMOUT_LITERAL ("jarr="),
			 MOMOUT_VALUE ((const momval_t) jarr), NULL);
	    }
	  else
	    return NULL;
	}
      else if (text[0] == '_' && isalnum (text[1]) && isalnum (text[2]))
	{
	  // a line starting with some id-like thing is tab-expanded
	  // to the set of items of id prefixed by the text
	  jarr = MOM_NULLV;
	  momval_t setv =
	    (momval_t) mom_set_of_items_of_ident_prefixed (text);
	  unsigned setcard = mom_set_cardinal (setv);
	  MOM_DEBUG (cmd, MOMOUT_LITERAL ("cmd_completion idprefix:"),
		     MOMOUT_LITERALV ((const char *) text), MOMOUT_SPACE (48),
		     MOMOUT_LITERAL ("set:"), MOMOUT_VALUE (setv));
	  momval_t *strarr =
	    MOM_GC_ALLOC ("strarr", (setcard + 1) * sizeof (momval_t));
	  for (unsigned ix = 0; ix < setcard; ix++)
	    strarr[ix] =
	      (momval_t) mom_identv_of_item (mom_set_nth_item (setv, ix));
	  jarr = (momval_t) mom_make_json_array_count (setcard, strarr);
	  MOM_DEBUG (cmd,
		     MOMOUT_LITERAL ("cmd_completion idprefix card:"),
		     MOMOUT_DEC_INT ((int) setcard), MOMOUT_SPACE (32),
		     MOMOUT_LITERAL ("jarr="),
		     MOMOUT_VALUE ((const momval_t) jarr), NULL);
	}
    }
  if (state >= 0 && jarr.ptr && state < (int) mom_json_array_size (jarr))
    {
      const char *restr = mom_string_cstr (mom_json_array_nth (jarr, state));
      MOM_DEBUGPRINTF (cmd, "cmd_completion state#%d restr=%s", state, restr);
      if (restr)
	return strdup (restr);
    }
  MOM_DEBUGPRINTF (cmd, "cmd_completion fail state#%d", state);
  return NULL;
}


////////////////////////////////////////////////////////////////
static void
cmd_do_dump_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_dump lin=%s", lin);
}


static void
cmd_do_dup_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_dup lin=%s", lin);
}

static void
cmd_do_exit_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_exit lin=%s", lin);
}


static void
cmd_do_help_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_help lin=%s", lin);
  putchar ('\n');
  for (struct cmd_descr_st * cd = cmd_array_mom;
       cd != NULL && cd->cmd_name != NULL; cd++)
    printf (" %s: %s\n", cd->cmd_name, cd->cmd_help);
  putchar ('\n');
  fflush (NULL);
}

static void
cmd_do_quit_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_quit lin=%s", lin);
}

static void
cmd_do_stack_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_stack lin=%s", lin);
}

static void
cmd_do_top_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_top lin=%s", lin);
}


////////////////////////////////////////////////////////////////

void
mom_plugin_init (const char *arg)
{
  MOM_DEBUGPRINTF (cmd, "start of " __FILE__ " arg=%s", arg);
  rl_initialize ();
  rl_readline_name = "monimelt";
  rl_completion_entry_function = cmd_completion_entry_mom;
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
      MOM_DEBUGPRINTF (cmd, "before readline prompt=%s", prompt);
      lin = readline (prompt);
      MOM_DEBUGPRINTF (cmd, "after readline lin=%s", lin);
      if (!lin)
	break;
    };
  MOM_INFORMPRINTF ("momplug_cmd ending after load");
}
