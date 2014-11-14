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

#define COMMANDS(CMD)				\
  CMD(dump,"dump state & continue")		\
  CMD(dup,"duplicate top.  Alias %")		\
  CMD(exit,"dump & exit")			\
  CMD(help,"give this help")			\
  CMD(pop,"pop [N] elements.  Alias ^")		\
  CMD(mark,"push mark. Alias (")		\
  CMD(quit,"quit without dumping")		\
  CMD(node,"make node to mark.  Alias *")      	\
  CMD(set,"make set to mark.  Alias }")		\
  CMD(tuple,"make tuple to mark.  Alias ]")	\
  CMD(stack,"print the stack.  Alias !")	\
  CMD(status,"print status info")		\
  CMD(xplode,"explode top aggregate. Alias &")	\
  CMD(top,"print the top of stack.  Alias = ")
				/* end of COMMANDS */

// the command stack has values and marks
static momval_t *vst_valarr_mom;
static unsigned vst_size_mom;
static unsigned vst_top_mom;


#define ANSI_BOLD "\e[1m"
#define ANSI_NORMAL "\e[0m"

static void
cmd_stack_push_mom (momval_t val)
{
  if (vst_top_mom >= vst_size_mom)
    {
      unsigned newsiz = (4 * vst_size_mom / 3 + 21) | 0x1f;
      momval_t *newvalarr =
	MOM_GC_ALLOC ("valarr", newsiz * sizeof (momval_t));
      if (vst_top_mom > 0)
	{
	  memcpy (newvalarr, vst_valarr_mom, vst_top_mom * sizeof (momval_t));
	  MOM_GC_FREE (vst_valarr_mom);
	}
      vst_valarr_mom = newvalarr;
    }
  vst_valarr_mom[vst_top_mom] = val;
  vst_top_mom++;
}

static void
cmd_stack_push_mark_mom (void)
{
  if (vst_top_mom >= vst_size_mom)
    {
      unsigned newsiz = (4 * vst_size_mom / 3 + 21) | 0x1f;
      momval_t *newvalarr =
	MOM_GC_ALLOC ("valarr", newsiz * sizeof (momval_t));
      if (vst_top_mom > 0)
	{
	  memcpy (newvalarr, vst_valarr_mom, vst_top_mom * sizeof (momval_t));
	  MOM_GC_FREE (vst_valarr_mom);
	}
      vst_valarr_mom = newvalarr;
    }
  vst_valarr_mom[vst_top_mom].ptr = MOM_EMPTY;
  vst_top_mom++;
}

static inline momval_t
cmd_stack_nth_value_mom (int rk)
{
  if (rk < 0)
    rk += vst_top_mom;
  if (rk >= 0 && rk < (int) vst_top_mom)
    {
      momval_t v = vst_valarr_mom[vst_top_mom - rk];
      if (v.ptr != MOM_EMPTY)
	return v;
    }
  return MOM_NULLV;
}

static inline momval_t *
cmd_stack_nth_ptr_mom (int rk)
{
  if (rk < 0)
    rk += vst_top_mom;
  if (rk >= 0 && rk < (int) vst_top_mom)
    {
      momval_t *v = vst_valarr_mom + vst_top_mom - rk;
      if (v->ptr != MOM_EMPTY)
	return v;
    }
  return NULL;
}

static inline bool
cmd_stack_nth_mark_mom (int rk)
{
  if (rk < 0)
    rk += vst_top_mom;
  if (rk >= 0 && rk < (int) vst_top_mom)
    return vst_valarr_mom[vst_top_mom - rk].ptr == MOM_EMPTY;
  return false;
}

// return the depth of the nearest mark or else -1
static inline int
cmd_stack_mark_depth_mom (void)
{
  for (int ix = (int)vst_top_mom - 1; ix >= 0; ix--)
    if (vst_valarr_mom[ix].ptr == MOM_EMPTY)
      return vst_top_mom - ix - 1;
  return -1;
}

static void
cmd_stack_pop_mom (unsigned nb)
{
  if (nb < vst_top_mom && nb > 0)
    {
      memset (vst_valarr_mom + vst_top_mom - nb, 0, sizeof (momval_t) * nb);
      vst_top_mom -= nb;
    }
  else if (nb >= vst_top_mom)
    {
      if (vst_top_mom > 0)
	memset (vst_valarr_mom, 0, vst_top_mom * sizeof (momval_t));
      vst_top_mom = 0;
    }

}


#define CMD_DECLARE(N,H) static void cmd_do_##N##_mom (const char*);
COMMANDS (CMD_DECLARE);
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
  MOM_WARNPRINTF ("unimplemented command /dump %s", lin);
}


static void
cmd_do_dup_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_dup lin=%s", lin);
  MOM_WARNPRINTF ("unimplemented command /dup %s", lin);
}

static void
cmd_do_exit_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_exit lin=%s", lin);
  MOM_WARNPRINTF ("unimplemented command /exit %s", lin);
}


static void
cmd_do_help_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_help lin=%s", lin);
  putchar ('\n');
  for (struct cmd_descr_st * cd = cmd_array_mom;
       cd != NULL && cd->cmd_name != NULL; cd++)
    printf (" ,%s : %s\n", cd->cmd_name, cd->cmd_help);
  putchar ('\n');
  fflush (NULL);
}

static void
cmd_do_quit_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_quit lin=%s", lin);
  char timbuf[80];
  memset (timbuf, 0, sizeof (timbuf));
  MOM_INFORMPRINTF
    ("command initiated quit at %s (elapsed %.3f, cpu %.3f sec.)\n",
     mom_strftime_centi (timbuf, sizeof (timbuf), "%H:%M:%S.__",
			 mom_clock_time (CLOCK_REALTIME)),
     mom_elapsed_real_time (), mom_clock_time (CLOCK_PROCESS_CPUTIME_ID));
  exit (EXIT_SUCCESS);

}

static void
cmd_do_mark_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_mark lin=%s", lin);
  cmd_stack_push_mark_mom ();
  printf ("marked, stack height %d\n", vst_top_mom);
  add_history (",mark");
}

static void
cmd_do_xplode_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_xplode lin=%s", lin);
}

static void
cmd_do_set_mom (const char *lin)
{
  int markdepth = cmd_stack_mark_depth_mom ();
  int argdepth = atoi (lin);
  int setdepth = -1;
  momval_t setv = MOM_NULLV;
  MOM_DEBUGPRINTF (cmd, "start do_set lin=%s markdepth=%d argdepth=%d", lin,
		   markdepth, argdepth);
  if (argdepth > 0 && argdepth < markdepth)
    setdepth = argdepth;
  else if (*lin == '0' && argdepth == 0)
    setdepth = 0;
  else if (markdepth > 0)
    setdepth = markdepth;
  if (setdepth < 0)
    {
      printf (ANSI_BOLD "*no mark for set*" ANSI_NORMAL "\n");
      return;
    }
  else if (setdepth == 0)
    {
      setv = (momval_t) mom_make_set_sized (0);
      printf (ANSI_BOLD "*empty set*" ANSI_NORMAL "\n");
      add_history (",set 0");
    }
  else if (setdepth > 0)
    {
      momitem_t **setarr =
	MOM_GC_ALLOC ("setarr", (setdepth + 1) * sizeof (momitem_t *));
      unsigned nbelem = 0;
      for (int ix = 0; ix < setdepth; ix++)
	{
	  momitem_t *curelemitm =
	    mom_value_to_item (cmd_stack_nth_value_mom (-(ix + 1)));
	  if (curelemitm)
	    setarr[nbelem++] = curelemitm;
	}
      setv =
	(momval_t) mom_make_set_from_array (nbelem,
					    (const momitem_t **) setarr);
      MOM_GC_FREE (setarr);
      if ((int) nbelem == markdepth)
	add_history (",set");
      else
	{
	  char cmdbuf[32];
	  snprintf (cmdbuf, sizeof (cmdbuf), ",set %d", nbelem);
	  add_history (cmdbuf);
	}
      if (markdepth > 0)
	{
	  cmd_stack_pop_mom (markdepth + 1);
	  printf (ANSI_BOLD "*set %d of %d*" ANSI_NORMAL "\n", nbelem,
		  markdepth);
	}
      else
	{
	  cmd_stack_pop_mom (setdepth);
	  printf (ANSI_BOLD "*set %d*" ANSI_NORMAL "\n", nbelem);
	}
      cmd_stack_push_mom (setv);
    }
}

static void
cmd_do_tuple_mom (const char *lin)
{
  int markdepth = cmd_stack_mark_depth_mom ();
  int argdepth = atoi (lin);
  int tupledepth = -1;
  momval_t tuplev = MOM_NULLV;
  MOM_DEBUGPRINTF (cmd, "start do_tuple lin=%s markdepth=%d argdepth=%d", lin,
		   markdepth, argdepth);
  if (argdepth > 0 && argdepth < markdepth)
    tupledepth = argdepth;
  else if (*lin == '0' && argdepth == 0)
    tupledepth = 0;
  else if (markdepth > 0)
    tupledepth = markdepth;
  if (tupledepth < 0)
    {
      printf (ANSI_BOLD "*no mark for tuple*" ANSI_NORMAL "\n");
      return;
    }
  else if (tupledepth == 0)
    {
      tuplev = (momval_t) mom_make_tuple_sized (0);
      printf (ANSI_BOLD "*empty tuple*" ANSI_NORMAL "\n");
      add_history (",tuple 0");
    }
  else if (tupledepth > 0)
    {
      momitem_t **tuplearr =
	MOM_GC_ALLOC ("tuplearr", (tupledepth + 1) * sizeof (momitem_t *));
      unsigned nbelem = 0;
      for (int ix = 0; ix < tupledepth; ix++)
	{
	  momitem_t *curelemitm =
	    mom_value_to_item (cmd_stack_nth_value_mom (-(ix + 1)));
	  tuplearr[nbelem++] = curelemitm;
	}
      tuplev =
	(momval_t) mom_make_tuple_from_array (nbelem,
					      (const momitem_t **) tuplearr);
      MOM_GC_FREE (tuplearr);
      if ((int) nbelem == markdepth)
	add_history (",tuple");
      else
	{
	  char cmdbuf[32];
	  snprintf (cmdbuf, sizeof (cmdbuf), ",tuple %d", nbelem);
	  add_history (cmdbuf);
	}
      if (markdepth > 0)
	{
	  cmd_stack_pop_mom (markdepth + 1);
	  printf (ANSI_BOLD "*tuple %d of %d*" ANSI_NORMAL "\n", nbelem,
		  markdepth);
	}
      else
	{
	  cmd_stack_pop_mom (tupledepth);
	  printf (ANSI_BOLD "*tuple %d*" ANSI_NORMAL "\n", nbelem);
	}
      cmd_stack_push_mom (tuplev);
    }
}

static void
cmd_do_node_mom (const char *lin)
{
  momitem_t *connitm = NULL;
  momval_t nodv = MOM_NULLV;
  char nambuf[80];
  char cmdbuf[128];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  memset (nambuf, 0, sizeof (nambuf));
  int pos = -1;
  int arity = -1;
  int markdepth = cmd_stack_mark_depth_mom ();
  MOM_DEBUGPRINTF (cmd, "start do_node lin=%s", lin);
  if (sscanf (lin, " %d %n", &arity, &pos) > 0 && pos > 0 && arity >= 0
      && arity + 1 < markdepth && arity < (int) vst_top_mom)
    {
      connitm = mom_value_to_item (cmd_stack_nth_value_mom (1));
      if (connitm)
	cmd_stack_pop_mom (1);
      nodv = (momval_t)
	mom_make_node_from_array (connitm, arity,
				  cmd_stack_nth_ptr_mom (arity + 1));
      if (nodv.ptr)
	{
	  cmd_stack_pop_mom (arity);
	  snprintf (cmdbuf, sizeof (cmdbuf), ",node %d", arity);
	  add_history (cmdbuf);
	}
    }
  else if (sscanf (lin, " %70[a-zA-Z0-9_] %d %n", &nambuf, &arity, &pos) >= 2
	   && (isalpha (nambuf[0]) || nambuf[0] == '_')
	   && arity >= 0 && pos > 0 && arity < (int) vst_top_mom)
    {
      connitm = mom_get_item_of_name_or_ident_cstr (nambuf);
      nodv = (momval_t)
	mom_make_node_from_array (connitm, arity,
				  cmd_stack_nth_ptr_mom (arity + 1));
      if (nodv.ptr)
	{
	  cmd_stack_pop_mom (arity);
	  snprintf (cmdbuf, sizeof (cmdbuf), ",node %s %d",
		    mom_string_cstr ((momval_t)
				     mom_item_get_name_or_idstr (connitm)),
		    arity);
	  add_history (cmdbuf);
	}
    }
  else if (sscanf (lin, " %70[a-zA-Z0-9_] %n", &nambuf, &pos) >= 1
	   && (isalpha (nambuf[0]) || nambuf[0] == '_') && markdepth >= 0)
    {
      connitm = mom_get_item_of_name_or_ident_cstr (nambuf);
      nodv = (momval_t)
	mom_make_node_from_array (connitm, markdepth,
				  cmd_stack_nth_ptr_mom (markdepth + 1));
      if (nodv.ptr)
	{
	  cmd_stack_pop_mom (markdepth + 1);	/* also pop the mark */
	  snprintf (cmdbuf, sizeof (cmdbuf), ",node %s",
		    mom_string_cstr ((momval_t)
				     mom_item_get_name_or_idstr (connitm)));
	  add_history (cmdbuf);
	}
    }
  else if (!lin[0] && markdepth >= 1)
    {
      connitm = mom_value_to_item (cmd_stack_nth_value_mom (1));
      if (connitm)
	cmd_stack_pop_mom (1);
      nodv = (momval_t)
	mom_make_node_from_array (connitm, markdepth,
				  cmd_stack_nth_ptr_mom (markdepth + 1));
      if (nodv.ptr)
	{
	  cmd_stack_pop_mom (markdepth + 1);	/* also pop the mark */
	  snprintf (cmdbuf, sizeof (cmdbuf), ",node %s",
		    mom_string_cstr ((momval_t)
				     mom_item_get_name_or_idstr (connitm)));
	  add_history (cmdbuf);
	}
    };
  if (nodv.ptr)
    {
      MOM_OUT (mom_stdout, MOMOUT_LITERAL
	       (ANSI_BOLD "made node" ANSI_NORMAL " of connective:"),
	       MOMOUT_ITEM ((const momitem_t *) connitm),
	       MOMOUT_LITERAL (" of arity "),
	       MOMOUT_DEC_INT ((int) mom_node_arity (nodv)));
      cmd_stack_push_mom (nodv);
    }
  else
    printf (ANSI_BOLD "**failed to make node**" ANSI_NORMAL "\n");
}

static void
cmd_do_stack_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_stack lin=%s", lin);
  int maxdepth = atoi (lin);
  if (vst_top_mom == 0)
    {
      printf ("\n" ANSI_BOLD "== empty stack ==" ANSI_NORMAL "\n");
    }
  else
    {
      printf ("\n" ANSI_BOLD "== stack of %d levels ==" ANSI_NORMAL "\n",
	      vst_top_mom);
      if (maxdepth <= 0)
	maxdepth = vst_top_mom + 1;
      int minix = vst_top_mom - maxdepth;
      if (minix < 0)
	minix = 0;
      for (int ix = (int)minix; ix < (int) vst_top_mom; ix++)
	{
	  int depth = (int) vst_top_mom - ix;
	  if (ix == 0)
	    printf ("\t" ANSI_BOLD "**bottom**" ANSI_NORMAL "\n");
	  assert (depth > 0 && depth <= (int) vst_top_mom);
	  printf (ANSI_BOLD "[%d]" ANSI_NORMAL " ", depth);
	  if (vst_valarr_mom[ix].ptr == MOM_EMPTY)
	    printf (ANSI_BOLD "***mark***" ANSI_NORMAL "\n");
	  else
	    {
	      const momval_t curval = vst_valarr_mom[ix];
	      MOM_OUT (mom_stdout, MOMOUT_VALUE (curval));
	      if (mom_is_item (curval) && !mom_item_get_name (curval.pitem))
		{
		  momval_t commentv;
		  mom_lock_item (curval.pitem);
		  commentv =
		    mom_item_get_attribute (curval.pitem, mom_named__comment);
		  mom_unlock_item (curval.pitem);
		  if (mom_is_string (commentv))
		    MOM_OUT (mom_stdout, MOMOUT_SPACE (48),
			     MOMOUT_LITERAL (" " ANSI_BOLD "//: "
					     ANSI_NORMAL),
			     MOMOUT_LITERALV (mom_string_cstr (commentv)),
			     NULL);
		}
	      MOM_OUT (mom_stdout, MOMOUT_NEWLINE ());
	    }
	}
      printf ("\t" ANSI_BOLD "**top**" ANSI_NORMAL "\n");
    }
  if (maxdepth < (int) vst_top_mom)
    {
      char buf[32];
      memset (buf, 0, sizeof (buf));
      snprintf (buf, sizeof (buf), ",stack %d", maxdepth);
      add_history (buf);
    }
  else
    add_history (",stack");
}

static void
cmd_do_status_mom (const char *lin)
{
  char timbuf[64];
  memset (timbuf, 0, sizeof (timbuf));
  MOM_DEBUGPRINTF (cmd, "start do_status lin=%s", lin);
  int64_t nbcreat = 0, nbdestr = 0, nbitems = 0, nbnamed = 0;
  mom_item_status (&nbcreat, &nbdestr, &nbitems, &nbnamed);
  printf
    ("\n" ANSI_BOLD "status at %s" ANSI_NORMAL
     " (elapsed %.3f, cpu %.3f sec.):\n"
     " %ld created, %ld destroyed, %ld items, %ld named\n",
     mom_strftime_centi (timbuf, sizeof (timbuf), "%H:%M:%S.__",
			 mom_clock_time (CLOCK_REALTIME)),
     mom_elapsed_real_time (), mom_clock_time (CLOCK_PROCESS_CPUTIME_ID),
     (long) nbcreat, (long) nbdestr, (long) nbitems, (long) nbnamed);
  add_history (",status");
}

static void
cmd_do_top_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_top lin=%s", lin);
  if (vst_top_mom == 0)
    printf ("\n" ANSI_BOLD "== no top because empty ==" ANSI_NORMAL "\n");
  else
    {
      if (vst_valarr_mom[vst_top_mom - 1].ptr == MOM_EMPTY)
	printf ("\n" ANSI_BOLD "** top mark **" ANSI_NORMAL "\n");
      else
	{
	  int markdepth = cmd_stack_mark_depth_mom ();
	  const momval_t curval = vst_valarr_mom[vst_top_mom - 1];
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (ANSI_BOLD "top/"),
		   MOMOUT_DEC_INT ((int) vst_top_mom), NULL);
	  if (markdepth > 0)
	    MOM_OUT (mom_stdout,
		     MOMOUT_LITERAL ("("),
		     MOMOUT_DEC_INT ((int) markdepth),
		     MOMOUT_LITERAL (")"), NULL);
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (":" ANSI_NORMAL " "),
		   MOMOUT_VALUE (curval));
	  if (mom_is_item (curval) && !mom_item_get_name (curval.pitem))
	    {
	      momval_t commentv;
	      mom_lock_item (curval.pitem);
	      commentv =
		mom_item_get_attribute (curval.pitem, mom_named__comment);
	      mom_unlock_item (curval.pitem);
	      if (mom_is_string (commentv))
		MOM_OUT (mom_stdout, MOMOUT_SPACE (48),
			 MOMOUT_LITERAL (" " ANSI_BOLD "//: "
					 ANSI_NORMAL),
			 MOMOUT_LITERALV (mom_string_cstr (commentv)), NULL);
	    }
	  MOM_OUT (mom_stdout, MOMOUT_NEWLINE ());
	}
      add_history (",top");
    }
}


static void
cmd_do_pop_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_pop lin=%s", lin);
  if (vst_top_mom == 0)
    {
      printf (ANSI_BOLD "**stack was empty**" ANSI_NORMAL "\n");
      return;
    }
  int nblev = atoi (lin);
  if (nblev > 1)
    {
      cmd_stack_pop_mom (nblev);
      char cmdbuf[32];
      snprintf (cmdbuf, sizeof (cmdbuf), ",pop %d", nblev);
      add_history (cmdbuf);
    }
  else
    {
      cmd_stack_pop_mom (1);
      add_history (",pop");
    }
  if (vst_top_mom == 0)
    printf (ANSI_BOLD "**stack emptied**" ANSI_NORMAL "\n");
  else
    printf (ANSI_BOLD "** %d levels remaining on stack**" ANSI_NORMAL "\n",
	    vst_top_mom);
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

static void
cmd_push_value_mom (momval_t val)
{
  MOM_DEBUG (cmd, MOMOUT_LITERAL ("pushing value:"),
	     MOMOUT_VALUE ((const momval_t) val),
	     MOMOUT_SPACE (48),
	     MOMOUT_LITERAL (" at level "),
	     MOMOUT_DEC_INT ((int) vst_top_mom), NULL);
  cmd_stack_push_mom (val);
  printf ("pushed, stack height %d\n", vst_top_mom);
}

static void
cmd_interpret_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "interpreting lin=%s", lin);
  momitem_t *itm = NULL;
  if (isalpha (lin[0]))
    {
      char name[72];
      memset (name, 0, sizeof (name));
      if (sscanf (lin, "%70[a-zA-Z0-9_]", name) > 0 && isalpha (name[0]))
	{
	  if (name[70 - 1])
	    MOM_WARNPRINTF ("too long name in command %s", lin);
	  itm = mom_get_item_of_name (name);
	  MOM_DEBUG (cmd, MOMOUT_LITERAL ("got item:"),
		     MOMOUT_ITEM ((const momitem_t *) itm),
		     MOMOUT_LITERAL (" of name "),
		     MOMOUT_LITERALV ((const char *) name));
	  if (!itm)
	    {
	      printf ("\n" ANSI_BOLD "Unknown name" ANSI_NORMAL
		      " %s. Creating it (unless comment is empty or .)\n",
		      name);
	      char *commentline = readline ("comment: ");
	      if (isprint (commentline[0]) && commentline[0] != '.')
		{
		  itm = mom_make_item ();
		  mom_item_put_attribute ((momitem_t *) itm,
					  mom_named__comment,
					  (momval_t)
					  mom_make_string (commentline));
		  mom_register_item_named_cstr ((momitem_t *) itm, name);
		  mom_item_set_space ((momitem_t *) itm, momspa_root);
		  MOM_INFORM (MOMOUT_LITERAL ("command created named item:"),
			      MOMOUT_ITEM ((const momitem_t *) itm));
		  cmd_push_value_mom ((momval_t) itm);
		  add_history (lin);
		  return;
		}
	      else
		{
		  printf ("\n" ANSI_BOLD "Ignoring item" ANSI_NORMAL " %s\n",
			  name);
		  return;
		}
	    }
	  else			// itm != NULL
	    {
	      cmd_push_value_mom ((momval_t) itm);
	      add_history (lin);
	      return;
	    }
	}
      else
	goto bad_command;
    }
  else if (isdigit (lin[0]) || lin[0] == '-' || lin[0] == '+')
    {
      long long l = 0;
      double d = 0;
      char *endlng = NULL;
      char *enddbl = NULL;
      l = strtol (lin, &endlng, 0);
      d = strtod (lin, &enddbl);
      if (enddbl > endlng && endlng > lin)
	{
	  momval_t dblv = mom_make_double (d);
	  cmd_push_value_mom (dblv);
	  add_history (lin);
	  return;
	}
      else if ((endlng > lin && isdigit (endlng[-1])) || !strcmp (lin, "0"))
	{
	  momval_t numv = mom_make_integer (l);
	  cmd_push_value_mom (numv);
	  add_history (lin);
	  return;
	}
      else
	goto bad_command;
    }
  else if (lin[0] == '_')
    {
      // the __ or _. is for nil
      if ((lin[1] == '_' || lin[1] == '.') && (!lin[2] || isspace (lin[2])))
	{
	  cmd_push_value_mom (MOM_NULLV);
	  add_history ("_.");
	  return;
	}
      else if (!lin[1] || isspace (lin[1]))
	{
	  printf ("\n" ANSI_BOLD "Anonymous item" ANSI_NORMAL
		  ". Creating it (unless comment is empty or .)\n");
	  char *commentline = readline ("comment: ");
	  if (isprint (commentline[0]) && commentline[0] != '.')
	    {
	      itm = mom_make_item ();
	      mom_item_put_attribute (itm, mom_named__comment,
				      (momval_t)
				      mom_make_string (commentline));
	      mom_item_set_space (itm, momspa_root);
	      MOM_INFORM (MOMOUT_LITERAL ("command created anonymous item:"),
			  MOMOUT_ITEM ((const momitem_t *) itm));
	      cmd_push_value_mom ((momval_t) itm);
	      add_history (lin);
	    }
	  else
	    {
	      printf ("\n" ANSI_BOLD "Ignoring anonymous item" ANSI_NORMAL
		      "\n");
	      return;
	    }
	}
      else if (isalnum (lin[1]))
	{
	  char idstr[MOM_IDSTRING_LEN + 1];
	  memset (idstr, 0, sizeof (idstr));
	  if (sscanf (lin, MOM_IDSTRING_FMT, idstr) > 0
	      && mom_looks_like_random_id_cstr (idstr, NULL))
	    {
	      itm = mom_get_item_of_identcstr (idstr);
	      if (!itm)
		goto bad_command;
	      else
		{
		  cmd_push_value_mom ((momval_t) itm);
		  add_history (lin);
		  return;
		}
	    }
	  else
	    goto bad_command;
	}
    }
  else if (lin[0] == ',')
    {
      int pos = -1;
      char cmdbuf[32];
      memset (cmdbuf, 0, sizeof (cmdbuf));
      if (sscanf (lin, ",%30[a-z_] %n", cmdbuf, &pos) > 0 && pos > 1)
	{
	  for (int ix = 0; ix < (int) CMDARRSIZE_MOM; ix++)
	    {
	      if (!cmd_array_mom[ix].cmd_name)
		goto bad_command;
	      if (!strcmp (cmdbuf, cmd_array_mom[ix].cmd_name))
		{
		  cmd_array_mom[ix].cmd_fun (lin + pos);
		  return;
		}
	    }
	}
    }
  else if (lin[0] == '%')	/* alias for dup */
    {
      cmd_do_dup_mom (lin + 1);
      return;
    }
  else if (lin[0] == '=')	/* alias for top */
    {
      cmd_do_top_mom (lin + 1);
      return;
    }
  else if (lin[0] == '^')	/* alias for pop */
    {
      cmd_do_pop_mom (lin + 1);
      return;
    }
  else if (lin[0] == '(')	/* alias for mark */
    {
      cmd_do_mark_mom (lin + 1);
      return;
    }
  else if (lin[0] == '}')	/* alias for set */
    {
      cmd_do_set_mom (lin + 1);
      return;
    }
  else if (lin[0] == ']')	/* alias for tuple */
    {
      cmd_do_tuple_mom (lin + 1);
      return;
    }
  else if (lin[0] == '*')	/* alias for node */
    {
      cmd_do_node_mom (lin + 1);
      return;
    }
bad_command:
  MOM_WARNPRINTF ("bad command '%s'", lin);
}

#define POLL_TIMEOUT 500
void
momplugin_after_load (void)
{
  int cnt = 0;
  MOM_INFORMPRINTF ("momplug_cmd starting after load");
  char *lin = NULL;
  printf ("### type ,help to get some help.\n");
  for (;;)
    {
      char prompt[64];
      cnt++;
      snprintf (prompt, sizeof (prompt), "monimelt%03d: ", cnt);
      lin = NULL;
      MOM_DEBUGPRINTF (cmd, "before readline prompt=%s", prompt);
      lin = readline (prompt);
      cmd_interpret_mom (lin);
      MOM_DEBUGPRINTF (cmd, "after readline lin=%s", lin);
      if (!lin)
	break;
    };
  MOM_INFORMPRINTF ("momplug_cmd ending after load");
}


// eof momplug_cmd.c
