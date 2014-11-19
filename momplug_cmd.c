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

#include <regex.h>
#include <readline/readline.h>
#include <readline/history.h>

const char mom_plugin_GPL_compatible[] = "GPLv3+";

#define COMMANDS(CMD)                                   \
  CMD(clear,"clear the stack",non,NULL)                 \
  CMD(debugf,"[flags]; set debugflags",non,NULL)        \
  CMD(dump,"[dir]; dump state & continue",non,NULL)     \
  CMD(dup,"duplicate top",num,"%")                      \
  CMD(echo,"echo the line",non,NULL)                    \
  CMD(exit,"dump & exit",non,NULL)                      \
  CMD(forget,"forget named item",itm,NULL)              \
  CMD(gencmod,"generate C module",itm,NULL)             \
  CMD(getat,"[attr]; get attribute",itm,":=")           \
  CMD(help,"give this help",non,"?")                    \
  CMD(mark,"push mark",non,"(")                         \
  CMD(named,"[Regexp]",non,NULL)			\
  CMD(node,"[Connective]; make node to mark",itm,"*")   \
  CMD(pop,"pop [N] elements",num,"^")                   \
  CMD(predef,"NAME; create a predefined item",non,"^")	\
  CMD(putat,"[attr]; put attribute",itm,":+")           \
  CMD(quit,"quit without dumping",non,NULL)             \
  CMD(remat,"[attr]; remove attribute",itm,":-")        \
  CMD(set,"make set to mark",non,"}")                   \
  CMD(shell,"[command]; run a command",non,"#!")        \
  CMD(stack,"print the stack",non,"!")                  \
  CMD(status,"print status info",non,NULL)              \
  CMD(top,"print the top of stack",non,"=")             \
  CMD(tuple,"make tuple to mark",non,"]")               \
  CMD(xplode,"explode top aggregate",non,"&")           \
				/* end of COMMANDS */

// the command stack has values and marks
static momval_t *vst_valarr_mom;
static unsigned vst_size_mom;
static unsigned vst_top_mom;

static momval_t vst_letterval_mom[30];

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
      vst_size_mom = newsiz;
      MOM_DEBUGPRINTF (cmd, "grow push size=%d valarr@%p",
		       vst_size_mom, vst_valarr_mom);
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
      vst_size_mom = newsiz;
      MOM_DEBUGPRINTF (cmd, "grow pushmark size=%d valarr@%p",
		       vst_size_mom, vst_valarr_mom);
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
      momval_t v = vst_valarr_mom[rk];
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
      momval_t *v = vst_valarr_mom + rk;
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
    return vst_valarr_mom[rk].ptr == MOM_EMPTY;
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


typedef void cmd_do_fun_non_t (const char *);
typedef void cmd_do_fun_num_t (const char *, bool, long);
typedef void cmd_do_fun_itm_t (const char *, bool, momitem_t *);

#define CMD_DECLARE(N,H,T,A) static cmd_do_fun_##T##_t cmd_do_##N##_mom;
COMMANDS (CMD_DECLARE);
#undef CMD_DECLARE

enum cmd_type_en
{
  cmdt__none,
  cmdt_non,
  cmdt_num,
  cmdt_itm
};

struct cmd_descr_st
{
  const char *cmd_name;
  enum cmd_type_en cmd_type;
  union
  {
    void *cmd_funptr;
    cmd_do_fun_non_t *cmd_fun_non;
    cmd_do_fun_num_t *cmd_fun_num;
    cmd_do_fun_itm_t *cmd_fun_itm;
  };
  const char *cmd_help;
  const char *cmd_alias;
};

static const struct cmd_descr_st cmd_array_mom[] = {
#define CMD_DEFINE(N,H,T,A) {.cmd_name= #N, .cmd_type= cmdt_##T, \
			     .cmd_fun_##T= cmd_do_##N##_mom, .cmd_help= H, .cmd_alias= A},
  COMMANDS (CMD_DEFINE)
#undef CMD_DEFINE
  {.cmd_name = NULL,.cmd_type = cmdt__none,.cmd_funptr = NULL,.cmd_help =
   NULL,.cmd_alias = NULL}
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
  char cmdbuf[120];
  char dirbuf[90];
  int pos = -1;
  memset (cmdbuf, 0, sizeof (cmdbuf));
  memset (dirbuf, 0, sizeof (dirbuf));
  MOM_DEBUGPRINTF (cmd, "start do_dump lin=%s", lin);
  if (sscanf (lin, " %75[a-zA-Z0-9_/+.-] %n", dirbuf, &pos) <= 0)
    memset (dirbuf, 0, sizeof (dirbuf));
  MOM_DEBUGPRINTF (cmd, "do_dump dirbuf=%s", dirbuf);
  if (dirbuf[0])
    {
      snprintf (cmdbuf, sizeof (cmdbuf), "command dump into %s", dirbuf);
      printf ("\n" ANSI_BOLD "**dumping per command into %s**" ANSI_NORMAL
	      "\n", dirbuf);
      mom_full_dump (cmdbuf, dirbuf, NULL);
      snprintf (cmdbuf, sizeof (cmdbuf), ",.dump %s", dirbuf);
    }
  else
    {
      printf ("\n" ANSI_BOLD "**dumping per command**" ANSI_NORMAL "\n");
      strcpy (cmdbuf, "command dump");
      mom_full_dump (cmdbuf, ".", NULL);
      strcpy (cmdbuf, ",dump");
    }
  add_history (cmdbuf);
}


static void
cmd_do_dup_mom (const char *lin, bool pres, long num)
{
  MOM_DEBUGPRINTF (cmd, "start do_dup lin=%s num=%ld", lin, num);
  if (!pres)
    num = 1;
  momval_t valv = cmd_stack_nth_value_mom (-num);
  if (valv.ptr)
    {
      char cmdbuf[32];
      memset (cmdbuf, 0, sizeof (cmdbuf));
      cmd_stack_push_mom (valv);
      snprintf (cmdbuf, sizeof (cmdbuf), ",dup %ld", num);
      add_history (cmdbuf);
      printf (ANSI_BOLD "duplicated #%ld" ANSI_NORMAL "/%d\n", num,
	      vst_top_mom);
    }
  else
    printf ("no duplicated value\n");
}

static void
cmd_do_exit_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_exit lin=%s", lin);
  char cmdbuf[80];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  if (lin[0])
    snprintf (cmdbuf, sizeof (cmdbuf), "command exit dump %s", lin);
  else
    strcpy (cmdbuf, "command dump");
  printf ("\n" ANSI_BOLD "**dumping then exiting per command**" ANSI_NORMAL
	  "\n");
  mom_full_dump (cmdbuf, ".", NULL);
  MOM_INFORMPRINTF ("exiting per command after dump %s", lin);
  exit (EXIT_SUCCESS);
}

static void
cmd_do_echo_mom (const char *lin)
{
  char cmdbuf[120];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  MOM_DEBUGPRINTF (cmd, "start do_echo lin=%s", lin);
  printf (ANSI_BOLD "%s" ANSI_NORMAL "\n", lin);
  snprintf (cmdbuf, sizeof (cmdbuf), ",echo %s", lin);
  add_history (cmdbuf);
}

static void
cmd_do_named_mom (const char *lin)
{
  char cmdbuf[120];
  regex_t rx;
  int err = 0;
  momval_t tupv = MOM_NULLV;
  momval_t jarrv = MOM_NULLV;
  memset (cmdbuf, 0, sizeof (cmdbuf));
  memset (&rx, 0, sizeof (rx));
  MOM_DEBUGPRINTF (cmd, "start do_named lin=%s", lin);
  while (isspace (*lin))
    lin++;
  if ((err = regcomp (&rx, lin, REG_EXTENDED | REG_NOSUB)) != 0)
    {
      char errbuf[100];
      memset (errbuf, 0, sizeof (errbuf));
      regerror (err, &rx, errbuf, sizeof (errbuf));
      printf (ANSI_BOLD "bad name regexp %s" ANSI_NORMAL ": %s\n",
	      lin, errbuf);
      regfree (&rx);
      return;
    }
  tupv = (momval_t) mom_alpha_ordered_tuple_of_named_items (&jarrv);
  unsigned nbnamed = mom_tuple_length (tupv);
  unsigned nbmatch = 0;
  for (unsigned nix = 0; nix < nbnamed; nix++)
    {
      momitem_t *curitm = mom_tuple_nth_item (tupv, nix);
      momval_t curnamv = mom_json_array_nth (jarrv, nix);
      assert (curitm && curitm->i_typnum == momty_item);
      assert (mom_is_string (curnamv));
      if (!regexec (&rx, mom_string_cstr (curnamv), 0, NULL, 0))
	{
	  MOM_OUT (mom_stdout, MOMOUT_SPACE (40),
		   MOMOUT_ITEM ((const momitem_t *) curitm));
	  nbmatch++;
	}
    }
  if (nbmatch == 0)
    printf (ANSI_BOLD "no named items (of %d) matching" ANSI_NORMAL " %s\n",
	    nbnamed, lin);
  else
    MOM_OUT (mom_stdout, MOMOUT_SPACE (24),
	     MOMOUT_LITERAL (" " ANSI_BOLD "/ "),
	     MOMOUT_DEC_INT ((int) nbmatch),
	     MOMOUT_LITERAL (" matching items" ANSI_NORMAL " of "),
	     MOMOUT_DEC_INT ((int) nbnamed), MOMOUT_LITERAL ("."),
	     MOMOUT_NEWLINE ());
  regfree (&rx);
  snprintf (cmdbuf, sizeof (cmdbuf), ",named %s", lin);
  add_history (cmdbuf);
}


static void
cmd_do_predef_mom (const char *lin)
{
  char cmdbuf[120];
  char nambuf[80];
  int pos = -1;
  momitem_t *itm = NULL;
  memset (cmdbuf, 0, sizeof (cmdbuf));
  memset (nambuf, 0, sizeof (nambuf));
  MOM_DEBUGPRINTF (cmd, "start do_predef lin=%s", lin);
  if (sscanf (lin, " %72[A-Za-z0-9_] %n", nambuf, &pos) > 0
      && pos > 0 && isalpha (nambuf[0]))
    {
      itm = mom_get_item_of_name (nambuf);
      if (itm)
	{
	  mom_item_set_space ((momitem_t *) itm, momspa_predefined);
	  MOM_INFORM (MOMOUT_LITERAL ("item:"),
		      MOMOUT_ITEM ((const momitem_t *) itm),
		      MOMOUT_LITERAL (" of id "),
		      MOMOUT_LITERALV ((const char *)
				       mom_string_cstr ((momval_t)
							mom_item_get_idstr
							(itm))),
		      MOMOUT_LITERAL (" becomes predefined."));
	}
      else
	{
	  printf (ANSI_BOLD "enter comment of new predefined item %s"
		  ANSI_NORMAL "\n (or empty or a dot to interrupt):\n",
		  nambuf);
	  char *commentline = readline ("comment: ");
	  if (commentline && isprint (commentline[0])
	      && commentline[0] != '.')
	    {
	      itm = mom_make_item ();
	      mom_item_put_attribute ((momitem_t *) itm,
				      mom_named__comment,
				      (momval_t)
				      mom_make_string (commentline));
	      mom_register_item_named_cstr ((momitem_t *) itm, nambuf);
	      mom_item_set_space ((momitem_t *) itm, momspa_predefined);
	      MOM_INFORM (MOMOUT_LITERAL ("made predefined item:"),
			  MOMOUT_ITEM ((const momitem_t *) itm),
			  MOMOUT_LITERAL (" of id "),
			  MOMOUT_LITERALV ((const char *)
					   mom_string_cstr ((momval_t)
							    mom_item_get_idstr
							    (itm))), NULL);
	    }
	  else
	    {
	      printf ("no predefined item created\n");
	      return;
	    }
	  printf (ANSI_BOLD "made predefined:" ANSI_NORMAL "%s" "\n", nambuf);
	}
      snprintf (cmdbuf, sizeof (cmdbuf), ",predef %s", nambuf);
      add_history (cmdbuf);
    }
  else
    {
      printf (ANSI_BOLD "invalid predefined name:" ANSI_NORMAL "%s" "\n",
	      nambuf);
      return;
    };
  snprintf (cmdbuf, sizeof (cmdbuf), ",predef %s", nambuf);
  add_history (cmdbuf);
}


static const char *cmdtyp_name_mom[] = {
  [cmdt_non] = "non",
  [cmdt_num] = "num",
  [cmdt_itm] = "itm",
  NULL, NULL
};

static void
cmd_do_help_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_help lin=%s", lin);
  putchar ('\n');
  if (isalpha (lin[0]))
    {
      int nbh = 0;
      int linlen = strlen (lin);
      for (const struct cmd_descr_st * cd = cmd_array_mom;
	   cd != NULL && cd->cmd_name != NULL; cd++)
	{
	  if (strncmp (cd->cmd_name, lin, linlen))
	    continue;
	  nbh++;
	  printf (" " ANSI_BOLD ",%-10s" ANSI_NORMAL "<%s>" "  %-40s",
		  cd->cmd_name, cmdtyp_name_mom[cd->cmd_type], cd->cmd_help);
	  if (cd->cmd_alias)
	    printf ("  alias " ANSI_BOLD "%s" ANSI_NORMAL "\n",
		    cd->cmd_alias);
	  else
	    putchar ('\n');
	}
      if (nbh > 0)
	{
	  puts ("...");
	  return;
	}
    }
  for (const struct cmd_descr_st * cd = cmd_array_mom;
       cd != NULL && cd->cmd_name != NULL; cd++)
    {
      printf (" " ANSI_BOLD ",%-10s" ANSI_NORMAL "<%s>" "  %-40s",
	      cd->cmd_name, cmdtyp_name_mom[cd->cmd_type], cd->cmd_help);
      if (cd->cmd_alias)
	printf ("  alias " ANSI_BOLD "%s" ANSI_NORMAL "\n", cd->cmd_alias);
      else
	putchar ('\n');
    }
  printf ("\n## other syntax:\n");
  printf ("    \" starts a constant literal string\n");
  printf ("    __ or _. pushes a nil value\n");
  printf ("    names or identifiers pushes their corresponding items\n");
  printf ("    _ or an unknown name proposes to create an item\n");
  printf ("    numbers like 1.2 or -23 are pushed\n");
  printf ("    $a ... $z pushes the letter value.\n");
  printf ("    $=a ... $=z sets the letter value.\n");
  printf ("    $:a ... $:z swaps the letter value with top-of-stack.\n");

  putchar ('\n');
  fflush (NULL);
}

static void
cmd_do_debugf_mom (const char *lin)
{
  char cmdbuf[120];
  snprintf (cmdbuf, sizeof (cmdbuf), ",debugf %s", lin);
  mom_set_debugging (lin);
  add_history (cmdbuf);
}

static void
cmd_do_gencmod_mom (const char *lin, bool pres, momitem_t *moditm)
{
  char nambuf[80];
  char cmdbuf[sizeof (nambuf) + 20];
  char *errmsg = NULL;
  memset (nambuf, 0, sizeof (nambuf));
  memset (cmdbuf, 0, sizeof (cmdbuf));
  MOM_DEBUGPRINTF (cmd, "start do_gencmod lin=%s", lin);
  if (pres && moditm)
    {
      snprintf (cmdbuf, sizeof (cmdbuf),
		",gencmod %s",
		mom_string_cstr ((momval_t)
				 mom_item_get_name_or_idstr (moditm)));
      add_history (cmdbuf);
    }
  else
    {
      moditm = mom_value_to_item (cmd_stack_nth_value_mom (-1));
      if (!moditm)
	{
	  printf ("No module item at top of stack\n");
	  return;
	}
      else
	{
	  add_history (",gencmod");
	}
    };
  MOM_INFORM (MOMOUT_LITERAL ("before compilation to C of module:"),
	      MOMOUT_ITEM ((const momitem_t *) moditm));
  int errcod = mom_generate_c_module (moditm, NULL, &errmsg);
  if (errcod)
    {
      MOM_WARNING (MOMOUT_LITERAL ("failed compilation of module item:"),
		   MOMOUT_ITEM ((const momitem_t *) moditm),
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERALV ((const char *) errmsg),
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERAL (". Error code#"), MOMOUT_DEC_INT (errcod));
      printf (ANSI_BOLD "** module C code generation failed**" ANSI_NORMAL
	      "\n");
      return;
    }
  MOM_INFORM (MOMOUT_LITERAL ("successful generation of C for module:"),
	      MOMOUT_ITEM ((const momitem_t *) moditm));
  snprintf (cmdbuf, sizeof (cmdbuf),
	    "make " MOM_SHARED_MODULE_DIRECTORY "/" MOM_SHARED_MODULE_PREFIX
	    "%s.so", mom_ident_cstr_of_item (moditm));
  printf (ANSI_BOLD "running:" ANSI_NORMAL " %s\n", cmdbuf);
  fflush (NULL);
  errcod = system (cmdbuf);
  if (errcod)
    {
      MOM_WARNING (MOMOUT_LITERAL ("failed command:"),
		   MOMOUT_LITERALV ((const char *) cmdbuf),
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERAL ("with error code:"),
		   MOMOUT_DEC_INT (errcod));
      return;
    }
  if (!mom_load_module (NULL, mom_ident_cstr_of_item (moditm)))
    {
      MOM_WARNING (MOMOUT_LITERAL ("failed to load generated module:"),
		   MOMOUT_ITEM ((const momitem_t *) moditm));
      return;
    }
  MOM_INFORM (MOMOUT_LITERAL ("successful loading of generated module:"),
	      MOMOUT_ITEM ((const momitem_t *) moditm));
  printf (ANSI_BOLD "completed generation and loading of module %s"
	  ANSI_NORMAL "\n", mom_ident_cstr_of_item (moditm));
}


static void
cmd_do_quit_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_quit lin=%s", lin);
  char timbuf[80];
  memset (timbuf, 0, sizeof (timbuf));
  MOM_INFORMPRINTF
    ("command initiated quit (elapsed %.3f, cpu %.3f sec.)",
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
cmd_do_forget_mom (const char *lin, bool pres, momitem_t *itm)
{
  MOM_DEBUG (cmd, MOMOUT_LITERAL ("start do_forget lin:"),
	     MOMOUT_LITERALV (lin),
	     MOMOUT_SPACE (48),
	     MOMOUT_LITERAL ("item:"), MOMOUT_ITEM ((const momitem_t *) itm));
  if (pres && itm)
    {
      // don't add to history
      mom_forget_item (itm);
      if (itm->i_space == momspa_predefined)
	{
	  itm->i_space = momspa_root;
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (ANSI_BOLD "forgot predefined item:"
				   ANSI_NORMAL),
		   MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
	}
      else
	MOM_OUT (mom_stdout,
		 MOMOUT_LITERAL (ANSI_BOLD "forgot item:" ANSI_NORMAL),
		 MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
    }
  else
    printf ("failed to forget\n");
}

static void
cmd_do_xplode_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_xplode lin=%s", lin);
  momval_t topv = cmd_stack_nth_value_mom (-1);
  MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_xplode topv="),
	     MOMOUT_VALUE ((const momval_t) topv), NULL);
  if (mom_is_tuple (topv))
    {
      unsigned tuplen = mom_tuple_length (topv);
      cmd_stack_pop_mom (1);
      cmd_stack_push_mark_mom ();
      for (unsigned ix = 0; ix < tuplen; ix++)
	cmd_stack_push_mom ((momval_t) mom_tuple_nth_item (topv, ix));
      printf (ANSI_BOLD "pushed mark then %d tuple items" ANSI_NORMAL "\n",
	      (int) tuplen);
      add_history (",xplode");
    }
  else if (mom_is_set (topv))
    {
      unsigned setlen = mom_set_cardinal (topv);
      cmd_stack_pop_mom (1);
      cmd_stack_push_mark_mom ();
      for (unsigned ix = 0; ix < setlen; ix++)
	cmd_stack_push_mom ((momval_t) mom_set_nth_item (topv, ix));
      printf (ANSI_BOLD "pushed mark then %d set items" ANSI_NORMAL "\n",
	      (int) setlen);
      add_history (",xplode");
    }
  else if (mom_is_node (topv))
    {
      unsigned nodlen = mom_node_arity (topv);
      cmd_stack_pop_mom (1);
      cmd_stack_push_mark_mom ();
      for (unsigned ix = 0; ix < nodlen; ix++)
	cmd_stack_push_mom (mom_node_nth (topv, ix));
      const momitem_t *connitm = mom_node_conn (topv);
      cmd_stack_push_mom ((momval_t) connitm);
      printf (ANSI_BOLD "pushed mark then %d node sons + connective:"
	      ANSI_NORMAL "%s" "\n", (int) nodlen,
	      mom_string_cstr ((momval_t)
			       mom_item_get_name_or_idstr ((momitem_t *)
							   connitm)));
      add_history (",xplode");
    }
  else
    {
      printf ("invalid top-of-stack to explode\n");
    }
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
  else if (markdepth >= 0)
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
      MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_set empty setv:"),
		 MOMOUT_VALUE ((const momval_t) setv));
      add_history (",set 0");
      if (markdepth == 0)
	cmd_stack_pop_mom (1);
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
      MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_set setv:"),
		 MOMOUT_VALUE ((const momval_t) setv));
      MOM_GC_FREE (setarr);
      if ((int) nbelem == markdepth)
	add_history (",set");
      else
	{
	  char cmdbuf[32];
	  snprintf (cmdbuf, sizeof (cmdbuf), ",set %d", nbelem);
	  add_history (cmdbuf);
	}
      if (markdepth >= 0)
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
    }
  cmd_stack_push_mom (setv);
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
  else if (markdepth >= 0)
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
      MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_tuple tuplev:"),
		 MOMOUT_VALUE ((const momval_t) tuplev));
      if ((int) nbelem == markdepth)
	add_history (",tuple");
      else
	{
	  char cmdbuf[32];
	  snprintf (cmdbuf, sizeof (cmdbuf), ",tuple %d", nbelem);
	  add_history (cmdbuf);
	}
      if (markdepth >= 0)
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
    }
  cmd_stack_push_mom (tuplev);
}

static void
cmd_do_node_mom (const char *lin, bool pres, momitem_t *itm)
{
  momitem_t *connitm = NULL;
  momval_t nodv = MOM_NULLV;
  char nambuf[80];
  char cmdbuf[128];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  memset (nambuf, 0, sizeof (nambuf));
  int markdepth = cmd_stack_mark_depth_mom ();
  MOM_DEBUG (cmd, MOMOUT_LITERAL ("start do_node it lin="),
	     MOMOUT_LITERALV ((const char *) lin), MOMOUT_SPACE (48),
	     MOMOUT_LITERAL ("markdepth="), MOMOUT_DEC_INT (markdepth),
	     MOMOUT_SPACE (32), MOMOUT_LITERAL ("itm="),
	     MOMOUT_ITEM ((const momitem_t *) itm));
  if (pres && itm && markdepth >= 0)
    {
      connitm = itm;
      MOM_DEBUGPRINTF (cmd, "do_node it markdepth=%d top=%d", markdepth,
		       vst_top_mom);
      momval_t *varr = NULL;
      if (markdepth == 0)
	varr = NULL;
      else if (markdepth == 1)
	varr = cmd_stack_nth_ptr_mom (-1);
      else
	varr = cmd_stack_nth_ptr_mom (-markdepth);
      if (varr)
	{
	  MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node it varr[0]:"),
		     MOMOUT_VALUE (varr[0]));
	  if (markdepth > 1)
	    MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node it varr["),
		       MOMOUT_DEC_INT (markdepth - 1), MOMOUT_LITERAL ("]:"),
		       MOMOUT_VALUE (varr[markdepth - 1]));
	}
      else
	MOM_DEBUGPRINTF (cmd, "do_node it no varr");
      MOM_DEBUGPRINTF (cmd, "connitm@%p varr@%p", connitm, varr);
      nodv = (momval_t) mom_make_node_from_array (connitm, markdepth, varr);
      MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node nodv="),
		 MOMOUT_VALUE ((const momval_t) nodv));
      if (nodv.ptr)
	{
	  cmd_stack_pop_mom (markdepth + 1);	/* also pop the mark */
	  snprintf (cmdbuf, sizeof (cmdbuf),
		    ",node %s",
		    mom_string_cstr ((momval_t)
				     mom_item_get_name_or_idstr (connitm)));
	  add_history (cmdbuf);
	}
    }
  else if (!pres && !itm && !lin[0] && markdepth >= 1)
    {
      MOM_DEBUGPRINTF (cmd, "do_node plain markdepth=%d top=%d", markdepth,
		       vst_top_mom);
      connitm = mom_value_to_item (cmd_stack_nth_value_mom (-1));
      MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node plain connitm:"),
		 MOMOUT_ITEM ((const momitem_t *) connitm));
      if (connitm)
	cmd_stack_pop_mom (1);
      momval_t *varr = NULL;
      if (markdepth > 1)
	varr = cmd_stack_nth_ptr_mom (-(markdepth - 1));
      if (varr)
	{
	  MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node plain varr[0]:"),
		     MOMOUT_VALUE (varr[0]));
	  if (markdepth > 1)
	    MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node plain varr["),
		       MOMOUT_DEC_INT (markdepth - 1), MOMOUT_LITERAL ("]:"),
		       MOMOUT_VALUE (varr[markdepth - 1]));
	}
      else
	MOM_DEBUGPRINTF (cmd, "do_node plain no varr");
      nodv =
	(momval_t) mom_make_node_from_array (connitm, markdepth - 1, varr);
      MOM_DEBUG (cmd, MOMOUT_LITERAL ("do_node plain nodv="),
		 MOMOUT_VALUE ((const momval_t) nodv));
      if (nodv.ptr)
	{
	  cmd_stack_pop_mom (markdepth + 1);	/* also pop the mark */
	  snprintf (cmdbuf,
		    sizeof (cmdbuf),
		    ",node %s",
		    mom_string_cstr ((momval_t)
				     mom_item_get_name_or_idstr (connitm)));
	  add_history (cmdbuf);
	}
    };
  MOM_DEBUG (cmd,
	     MOMOUT_LITERAL ("do_node nodv="),
	     MOMOUT_VALUE ((const momval_t) nodv));
  if (nodv.ptr)
    {
      MOM_OUT
	(mom_stdout,
	 MOMOUT_LITERAL (ANSI_BOLD "made node" ANSI_NORMAL " of connective: "
			 ANSI_BOLD),
	 MOMOUT_ITEM ((const momitem_t *) connitm),
	 MOMOUT_LITERAL (ANSI_NORMAL ", arity " ANSI_BOLD),
	 MOMOUT_DEC_INT ((int) mom_node_arity (nodv)),
	 MOMOUT_LITERAL (ANSI_NORMAL), MOMOUT_NEWLINE ());
      cmd_stack_push_mom (nodv);
    }
  else
    printf (ANSI_BOLD "**failed to make node**" ANSI_NORMAL "\n");
}


static void
cmd_do_getat_mom (const char *lin, bool pres, momitem_t *atitm)
{
  char cmdbuf[80];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  MOM_DEBUGPRINTF (cmd, "do_getat lin=%s atitm@%p", lin, atitm);
  momitem_t *itm = mom_value_to_item (cmd_stack_nth_value_mom (-1));
  if (pres && itm && atitm)
    {
      momval_t val = MOM_NULLV;
      mom_lock_item (itm);
      val = mom_item_get_attribute (itm, atitm);
      mom_unlock_item (itm);
      if (val.ptr)
	{
	  cmd_stack_push_mom (val);
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (ANSI_BOLD "got attribute" ANSI_NORMAL " "),
		   MOMOUT_ITEM ((const momitem_t *) atitm),
		   MOMOUT_LITERAL (" " ANSI_BOLD "from" ANSI_NORMAL " "),
		   MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
	  if (snprintf (cmdbuf, sizeof (cmdbuf),
			",getat %s",
			mom_string_cstr ((momval_t)
					 mom_item_get_name_or_idstr (atitm)))
	      < (int) sizeof (cmdbuf) - 1)
	    add_history (cmdbuf);
	}
      else
	{
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (ANSI_BOLD "missing attribute" ANSI_NORMAL
				   " "),
		   MOMOUT_ITEM ((const momitem_t *) atitm),
		   MOMOUT_LITERAL (" " ANSI_BOLD "in" ANSI_NORMAL " "),
		   MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
	}
    }
  else if (!atitm)
    printf (ANSI_BOLD "no attribute" ANSI_NORMAL "\n");
  else if (!itm)
    printf (ANSI_BOLD "no item" ANSI_NORMAL "\n");
}


static void
cmd_do_putat_mom (const char *lin, bool pres, momitem_t *atitm)
{
  char cmdbuf[80];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  MOM_DEBUGPRINTF (cmd, "do_getat lin=%s atitm@%p", lin, atitm);
  momitem_t *itm = mom_value_to_item (cmd_stack_nth_value_mom (-2));
  momval_t atval = cmd_stack_nth_value_mom (-1);
  if (pres && itm && atitm && atval.ptr)
    {
      mom_lock_item (itm);
      mom_item_put_attribute (itm, atitm, atval);
      mom_unlock_item (itm);
      MOM_OUT (mom_stdout,
	       MOMOUT_LITERAL (ANSI_BOLD "put attribute" ANSI_NORMAL " "),
	       MOMOUT_ITEM ((const momitem_t *) atitm),
	       MOMOUT_LITERAL (" " ANSI_BOLD "in" ANSI_NORMAL " "),
	       MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
      if (snprintf (cmdbuf, sizeof (cmdbuf),
		    ",putat %s",
		    mom_string_cstr ((momval_t)
				     mom_item_get_name_or_idstr (atitm)))
	  < (int) sizeof (cmdbuf) - 1)
	add_history (cmdbuf);
      cmd_stack_pop_mom (1);
    }
  else if (!atval.ptr)
    printf (ANSI_BOLD "no value" ANSI_NORMAL "\n");
  else if (!atitm)
    printf (ANSI_BOLD "no attribute" ANSI_NORMAL "\n");
  else if (!itm)
    printf (ANSI_BOLD "no item" ANSI_NORMAL "\n");
}

static void
cmd_do_remat_mom (const char *lin, bool pres, momitem_t *atitm)
{
  char cmdbuf[80];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  MOM_DEBUGPRINTF (cmd, "do_getat lin=%s atitm@%p", lin, atitm);
  momitem_t *itm = mom_value_to_item (cmd_stack_nth_value_mom (-1));
  if (pres && itm && atitm)
    {
      momval_t oldvalv = MOM_NULLV;
      mom_lock_item (itm);
      oldvalv = mom_item_get_attribute (itm, atitm);
      mom_item_remove_attribute (itm, atitm);
      mom_unlock_item (itm);
      if (oldvalv.ptr)
	{
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (ANSI_BOLD "removed attribute" ANSI_NORMAL
				   " "),
		   MOMOUT_ITEM ((const momitem_t *) atitm),
		   MOMOUT_LITERAL (" " ANSI_BOLD "from" ANSI_NORMAL " "),
		   MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
	  if (snprintf
	      (cmdbuf, sizeof (cmdbuf), ",remat %s",
	       mom_string_cstr ((momval_t)
				mom_item_get_name_or_idstr (atitm))) <
	      (int) sizeof (cmdbuf) - 1)
	    add_history (cmdbuf);
	}
      else
	{
	  MOM_OUT (mom_stdout,
		   MOMOUT_LITERAL (ANSI_BOLD "missing attribute" ANSI_NORMAL
				   " "),
		   MOMOUT_ITEM ((const momitem_t *) atitm),
		   MOMOUT_LITERAL (" " ANSI_BOLD "in" ANSI_NORMAL " "),
		   MOMOUT_ITEM ((const momitem_t *) itm), MOMOUT_NEWLINE ());
	}
    }
  else if (!atitm)
    printf (ANSI_BOLD "no attribute" ANSI_NORMAL "\n");
  else if (!itm)
    printf (ANSI_BOLD "no item" ANSI_NORMAL "\n");
}

static void
cmd_do_shell_mom (const char *lin)
{
  int res = 0;
  char cmdbuf[200];
  memset (cmdbuf, 0, sizeof (cmdbuf));
  MOM_DEBUGPRINTF (cmd, "start do_shell lin=%s top=%d", lin, vst_top_mom);
  MOM_INFORMPRINTF ("Before running command: %s", lin);
  fflush (NULL);
  res = system (lin);
  MOM_INFORMPRINTF ("Command %s result=%d", lin, res);
  snprintf (cmdbuf, sizeof (cmdbuf), ",shell %s", lin);
  add_history (cmdbuf);
}

static void
cmd_do_stack_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_stack lin=%s top=%d", lin, vst_top_mom);
  int maxdepth = atoi (lin);
  if (vst_top_mom == 0)
    {
      printf ("\n" ANSI_BOLD "== empty stack ==" ANSI_NORMAL "\n");
    }
  else
    {
      printf ("\n"
	      ANSI_BOLD
	      "== stack of %d levels ==" ANSI_NORMAL "\n", vst_top_mom);
      if (maxdepth <= 0)
	maxdepth = vst_top_mom + 1;
      int minix = vst_top_mom - maxdepth;
      if (minix < 0)
	minix = 0;
      MOM_DEBUGPRINTF (cmd, "do_stack minix=%d top=%d", minix, vst_top_mom);
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
		    MOM_OUT
		      (mom_stdout,
		       MOMOUT_SPACE
		       (48),
		       MOMOUT_LITERAL
		       (" " ANSI_BOLD
			"//: "
			ANSI_NORMAL),
		       MOMOUT_LITERALV (mom_string_cstr (commentv)), NULL);
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
  printf ("\n" ANSI_BOLD "status at %s" ANSI_NORMAL
	  " (elapsed %.3f, cpu %.3f sec.):\n"
	  " %ld created, %ld destroyed, %ld items, %ld named\n",
	  mom_strftime_centi (timbuf, sizeof (timbuf),
			      "%H:%M:%S.__",
			      mom_clock_time (CLOCK_REALTIME)),
	  mom_elapsed_real_time (),
	  mom_clock_time (CLOCK_PROCESS_CPUTIME_ID),
	  (long) nbcreat, (long) nbdestr, (long) nbitems, (long) nbnamed);
  printf ("MOMIMELT build time %s version %s\n", monimelt_timestamp,
	  monimelt_lastgitcommit);
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
      MOM_DEBUGPRINTF (cmd,
		       "do_top vst_top_mom=%d, vst_valarr_mom[%d].ptr=@%p",
		       vst_top_mom, vst_top_mom - 1,
		       (void *) vst_valarr_mom[vst_top_mom - 1].ptr);
      if (vst_valarr_mom[vst_top_mom - 1].ptr == MOM_EMPTY)
	printf ("\n" ANSI_BOLD "** top mark **" ANSI_NORMAL "\n");
      else
	{
	  int markdepth = cmd_stack_mark_depth_mom ();
	  const momval_t curval = vst_valarr_mom[vst_top_mom - 1];
	  MOM_OUT
	    (mom_stdout,
	     MOMOUT_LITERAL
	     (ANSI_BOLD "top/"), MOMOUT_DEC_INT ((int) vst_top_mom), NULL);
	  if (markdepth > 0)
	    MOM_OUT (mom_stdout, MOMOUT_LITERAL ("("),
		     MOMOUT_DEC_INT ((int) markdepth),
		     MOMOUT_LITERAL (")"), NULL);
	  MOM_OUT (mom_stdout, MOMOUT_LITERAL (":" ANSI_NORMAL " "),
		   MOMOUT_VALUE (curval));
	  if (mom_is_item (curval) && !mom_item_get_name (curval.pitem))
	    {
	      momval_t commentv;
	      mom_lock_item (curval.pitem);
	      commentv =
		mom_item_get_attribute (curval.pitem, mom_named__comment);
	      mom_unlock_item (curval.pitem);
	      if (mom_is_string (commentv))
		MOM_OUT
		  (mom_stdout,
		   MOMOUT_SPACE (48),
		   MOMOUT_LITERAL (" " ANSI_BOLD "//: " ANSI_NORMAL),
		   MOMOUT_LITERALV (mom_string_cstr (commentv)), NULL);
	    }
	  if (mom_is_item (curval))
	    {
	      const char *kinds = NULL;
	      unsigned k = mom_item_payload_kind (curval.pitem);
	      switch (k)
		{
		case mompayk_none:
		  kinds = "*no-payload*";
		  break;
		case mompayk_queue:
		  kinds = "queue";
		  break;
		case mompayk_tfunrout:
		  kinds = "tfunrout";
		  break;
		case mompayk_closure:
		  kinds = "closure";
		  break;
		case mompayk_procedure:
		  kinds = "procedure";
		  break;
		case mompayk_tasklet:
		  kinds = "tasklet";
		  break;
		case mompayk_buffer:
		  kinds = "buffer";
		  break;
		case mompayk_vector:
		  kinds = "vector";
		  break;
		case mompayk_hset:
		  kinds = "hset";
		  break;
		case mompayk_assoc:
		  kinds = "assoc";
		  break;
		case mompayk_process:
		  kinds = "process";
		  break;
		case mompayk_webexchange:
		  kinds = "webexchange";
		  break;
		case mompayk_jsonrpcexchange:
		  kinds = "jsonrpcexchange";
		  break;
		default:
		  {
		    char kindbuf[32];
		    snprintf (kindbuf, sizeof (kindbuf), "kind#%d", k);
		    kinds = MOM_GC_STRDUP ("kindbuf", kindbuf);
		  };
		  break;
		}
	      MOM_OUT (mom_stdout, MOMOUT_SPACE (48),
		       MOMOUT_LITERALV ((const char
					 *) ((curval.pitem->i_space ==
					      momspa_predefined) ?
					     " /predef-id=" : " /id=")),
		       MOMOUT_LITERALV (mom_ident_cstr_of_item
					(curval.pitem)), MOMOUT_SPACE (32),
		       MOMOUT_LITERALV ((const char *) kinds),
		       MOMOUT_SPACE (32),
		       MOMOUT_ITEM_ATTRIBUTES ((const momitem_t *)
					       curval.pitem));
	    }
	  MOM_OUT (mom_stdout, MOMOUT_NEWLINE ());
	}
      add_history (",top");
    }
}

static void
cmd_do_clear_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "start do_clear lin=%s", lin);
  if (vst_valarr_mom)
    memset (vst_valarr_mom, 0, vst_size_mom * sizeof (momval_t));
  vst_top_mom = 0;
  add_history (",clear");
  printf (ANSI_BOLD "cleared stack" ANSI_NORMAL "\n");
}


static void
cmd_do_pop_mom (const char *lin, bool pres, long num)
{
  MOM_DEBUGPRINTF (cmd, "start do_pop lin=%s", lin);
  if (vst_top_mom == 0)
    {
      printf (ANSI_BOLD "**stack was empty**" ANSI_NORMAL "\n");
      return;
    }
  long nblev = pres ? num : 1;
  if (nblev >= 1)
    {
      cmd_stack_pop_mom (nblev);
      char cmdbuf[32];
      snprintf (cmdbuf, sizeof (cmdbuf), ",pop %ld", nblev);
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
    printf (ANSI_BOLD
	    "** %d levels remaining on stack**"
	    ANSI_NORMAL "\n", vst_top_mom);
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
  MOM_DEBUG (cmd,
	     MOMOUT_LITERAL
	     ("pushing value:"),
	     MOMOUT_VALUE
	     ((const momval_t) val), MOMOUT_SPACE (48),
	     MOMOUT_LITERAL (" at level "),
	     MOMOUT_DEC_INT ((int) vst_top_mom), NULL);
  cmd_stack_push_mom (val);
  printf ("pushed, stack height %d\n", vst_top_mom);
}

static void
cmd_interpret_mom (const char *lin)
{
  MOM_DEBUGPRINTF (cmd, "cmd_interpret start lin=%s", lin);
  momitem_t *itm = NULL;
  if (!lin || !lin[0])
    return;
  if (isalpha (lin[0]))
    {
      char name[72];
      memset (name, 0, sizeof (name));
      if (sscanf (lin, "%70[a-zA-Z0-9_]", name) > 0 && isalpha (name[0]))
	{
	  MOM_DEBUGPRINTF (cmd, "cmd_interpret name=%s", name);
	  if (name[70 - 1])
	    MOM_WARNPRINTF ("too long name in command %s", lin);
	  itm = mom_get_item_of_name (name);
	  MOM_DEBUG (cmd,
		     MOMOUT_LITERAL
		     ("got item:"),
		     MOMOUT_ITEM
		     ((const momitem_t *) itm),
		     MOMOUT_LITERAL (" of name "),
		     MOMOUT_LITERALV ((const char *) name));
	  if (!itm)
	    {
	      printf ("\n"
		      ANSI_BOLD
		      "Unknown name"
		      ANSI_NORMAL
		      " %s. Creating it (unless comment is empty or .)\n",
		      name);
	      char *commentline = readline ("comment: ");
	      if (commentline && isprint (commentline[0])
		  && commentline[0] != '.')
		{
		  itm = mom_make_item ();
		  mom_item_put_attribute
		    ((momitem_t *)
		     itm,
		     mom_named__comment,
		     (momval_t) mom_make_string (commentline));
		  mom_register_item_named_cstr ((momitem_t *) itm, name);
		  mom_item_set_space ((momitem_t *) itm, momspa_root);
		  MOM_INFORM
		    (MOMOUT_LITERAL
		     ("command created named item:"),
		     MOMOUT_ITEM ((const momitem_t *) itm));
		  cmd_push_value_mom ((momval_t) itm);
		  add_history (lin);
		  return;
		}
	      else
		{
		  printf ("\n"
			  ANSI_BOLD
			  "Ignoring item" ANSI_NORMAL " %s\n", name);
		  return;
		}
	    }
	  else			// itm != NULL
	    {
	      cmd_push_value_mom ((momval_t) itm);
	      MOM_OUT (mom_stdout, MOMOUT_LITERAL ("pushing item:"),
		       MOMOUT_ITEM ((const momitem_t *) itm),
		       MOMOUT_NEWLINE ());
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
	  MOM_OUT (mom_stdout, MOMOUT_LITERAL ("pushing double:"),
		   MOMOUT_VALUE ((const momval_t) dblv), MOMOUT_NEWLINE ());
	  return;
	}
      else if ((endlng > lin && isdigit (endlng[-1])) || !strcmp (lin, "0"))
	{
	  momval_t numv = mom_make_integer (l);
	  cmd_push_value_mom (numv);
	  MOM_OUT (mom_stdout, MOMOUT_LITERAL ("pushing integer:"),
		   MOMOUT_VALUE ((const momval_t) numv), MOMOUT_NEWLINE ());
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
	  printf ("pushing nil\n");
	  add_history ("_.");
	  return;
	}
      else if (!lin[1] || isspace (lin[1]))
	{
	  printf ("\n"
		  ANSI_BOLD
		  "Anonymous item"
		  ANSI_NORMAL
		  ". Creating it (unless comment is empty or .)\n");
	  char *commentline = readline ("comment: ");
	  if (isprint (commentline[0]) && commentline[0] != '.')
	    {
	      itm = mom_make_item ();
	      mom_item_put_attribute
		(itm,
		 mom_named__comment,
		 (momval_t) mom_make_string (commentline));
	      mom_item_set_space (itm, momspa_root);
	      MOM_INFORM
		(MOMOUT_LITERAL
		 ("command created anonymous item:"),
		 MOMOUT_ITEM ((const momitem_t *) itm));
	      cmd_push_value_mom ((momval_t) itm);
	      add_history (lin);
	      return;
	    }
	  else
	    {
	      printf ("\n"
		      ANSI_BOLD "Ignoring anonymous item" ANSI_NORMAL "\n");
	      return;
	    }
	}
      else if (isalnum (lin[1]))
	{
	  char idstr[MOM_IDSTRING_LEN + 1];
	  memset (idstr, 0, sizeof (idstr));
	  if (sscanf
	      (lin,
	       MOM_IDSTRING_FMT,
	       idstr) > 0 && mom_looks_like_random_id_cstr (idstr, NULL))
	    {
	      itm = mom_get_item_of_identcstr (idstr);
	      if (!itm)
		goto bad_command;
	      else
		{
		  cmd_push_value_mom ((momval_t) itm);
		  MOM_OUT (mom_stdout, MOMOUT_LITERAL ("pushing item:"),
			   MOMOUT_ITEM ((const momitem_t *) itm),
			   MOMOUT_NEWLINE ());
		  add_history (lin);
		  return;
		}
	    }
	  else
	    goto bad_command;
	}
    }
  else if (lin[0] == '"')
    {
      momval_t nstrv = (momval_t) mom_make_string (lin + 1);
      cmd_push_value_mom (nstrv);
      add_history (lin);
      printf (ANSI_BOLD "pushed string:" ANSI_NORMAL "%s\n",
	      mom_string_cstr (nstrv));
    }
  else if (lin[0] == '$' && isalpha (lin[1]))
    {
      char c = tolower (lin[1]);
      if (c >= 'a' && c <= 'z')
	{
	  momval_t valv = vst_letterval_mom[c - 'a'];
	  if (valv.ptr)
	    {
	      char cmdbuf[8];
	      printf (ANSI_BOLD "pushed temporary %c" ANSI_NORMAL "\n", c);
	      snprintf (cmdbuf, sizeof (cmdbuf), "$%c", c);
	      add_history (cmdbuf);
	      cmd_stack_push_mom (valv);
	      return;
	    }
	  else
	    printf (ANSI_BOLD "no temporary %c" ANSI_NORMAL "\n", c);
	}
      else
	goto bad_command;
    }
  else if (lin[0] == '$' && isdigit (lin[1]))
    {
      int rk = atoi (lin + 1);
      MOM_DEBUGPRINTF (cmd, "$%d is dup", rk);
      cmd_do_dup_mom ("", true, rk);
    }
  else if (lin[0] == '$' && lin[1] == '=' && isalpha (lin[2]))
    {
      char cmdbuf[8];
      char c = tolower (lin[2]);
      if (c >= 'a' && c <= 'z')
	{
	  if (vst_top_mom == 0)
	    {
	      printf (ANSI_BOLD "empty stack, so untouched temporary %c"
		      ANSI_NORMAL "\n", c);
	      return;
	    }
	  momval_t valv = cmd_stack_nth_value_mom (-1);
	  if (valv.ptr)
	    {
	      printf (ANSI_BOLD "set temporary %c" ANSI_NORMAL "\n", c);
	      snprintf (cmdbuf, sizeof (cmdbuf), "$=%c", c);
	      add_history (cmdbuf);
	    }
	  else
	    {
	      printf (ANSI_BOLD "cleared temporary %c" ANSI_NORMAL "\n", c);
	      snprintf (cmdbuf, sizeof (cmdbuf), "$=%c", c);
	      add_history (cmdbuf);
	    }
	  vst_letterval_mom[c - 'a'] = valv;
	  cmd_stack_pop_mom (1);
	  return;
	}
      else
	goto bad_command;
    }
  else if (lin[0] == '$' && lin[1] == ':' && isalpha (lin[2]))
    {
      char c = tolower (lin[2]);
      if (c >= 'a' && c <= 'z')
	{
	  if (vst_top_mom == 0)
	    {
	      printf (ANSI_BOLD "empty stack, so untouched temporary %c"
		      ANSI_NORMAL "\n", c);
	      return;
	    }
	  momval_t valv = cmd_stack_nth_value_mom (-1);
	  momval_t oldvalv = vst_letterval_mom[c - 'a'];
	  char cmdbuf[8];
	  printf (ANSI_BOLD "swap temporary %c" ANSI_NORMAL "\n", c);
	  snprintf (cmdbuf, sizeof (cmdbuf), "$:%c", c);
	  add_history (cmdbuf);
	  vst_letterval_mom[c - 'a'] = valv;
	  cmd_stack_pop_mom (1);
	  cmd_stack_push_mom (oldvalv);
	  return;
	}
      else
	goto bad_command;
    }
  else if (ispunct (lin[0]))
    {
      int pos = -1;
      char cmdbuf[32];
      int cmdix = -1;
      int alen = 0;
      memset (cmdbuf, 0, sizeof (cmdbuf));
      MOM_DEBUGPRINTF (cmd, "punctuation line %s", lin);
      if (lin[0] == ',')
	sscanf (lin, ",%30[a-z_] %n", cmdbuf, &pos);
      for (cmdix = 0; cmdix < (int) CMDARRSIZE_MOM; cmdix++)
	{
	  const struct cmd_descr_st *cd = cmd_array_mom + cmdix;
	  if (!cd->cmd_name)
	    goto bad_command;
	  if (!strcmp (cmdbuf, cd->cmd_name)
	      || (cd->cmd_alias && (alen = strlen (cd->cmd_alias)) > 0
		  && !strncmp (lin, cd->cmd_alias, alen)
		  && (!lin[alen] || isspace (lin[alen]) || isalnum (lin[alen])
		      || lin[alen] == '_' || !ispunct (lin[alen]))
		  && (pos = alen) > 0))
	    {
	      MOM_DEBUGPRINTF (cmd, "got command %s #%d type%d", cd->cmd_name,
			       cmdix, cd->cmd_type);
	      switch (cd->cmd_type)
		{
		case cmdt_non:
		  cd->cmd_fun_non (lin + pos);
		  return;
		case cmdt_num:
		  {
		    int numpos = -1;
		    long l = 0;
		    if (sscanf (lin + pos, " %ld %n", &l, &numpos) > 0
			&& numpos > 0)
		      {
			MOM_DEBUGPRINTF (cmd, "num command %s l=%ld",
					 cd->cmd_name, l);
			cd->cmd_fun_num (lin + pos + numpos, true, l);
		      }
		    else
		      {
			MOM_DEBUGPRINTF (cmd, "num command %s nonum %s",
					 cd->cmd_name, lin + pos);
			cd->cmd_fun_num (lin + pos, false, 0);
		      }
		  }
		  return;
		case cmdt_itm:
		  {
		    int nampos = -1;
		    momitem_t *itm = NULL;
		    char nambuf[80];
		    memset (nambuf, 0, sizeof (nambuf));
		    if (sscanf
			(lin + pos, " %72[a-zA-Z0-9_] %n", nambuf,
			 &nampos) > 0 && nampos > 0 && (isalpha (nambuf[0])
							|| nambuf[0] == '_'))
		      {
			MOM_DEBUGPRINTF (cmd, "itm command %s nambuf '%s'",
					 cd->cmd_name, nambuf);
			itm = mom_get_item_of_name_or_ident_cstr (nambuf);
			MOM_DEBUG (cmd, MOMOUT_LITERAL ("itm command:"),
				   MOMOUT_LITERALV ((const char *)
						    cd->cmd_name),
				   MOMOUT_LITERAL (" item:"),
				   MOMOUT_ITEM ((const momitem_t *) itm),
				   MOMOUT_LITERAL (" lin:"),
				   MOMOUT_LITERALV ((const char *) lin),
				   MOMOUT_LITERAL (" pos#"),
				   MOMOUT_DEC_INT (pos),
				   MOMOUT_LITERAL (" nambuf'"),
				   MOMOUT_LITERALV ((const char *) nambuf),
				   MOMOUT_LITERAL ("'"), NULL);
		      }
		    else
		      {
			MOM_DEBUGPRINTF (cmd, "itm command %s noname %s",
					 cd->cmd_name, lin + pos);
		      }

		    if (itm)
		      cd->cmd_fun_itm (lin + pos + nampos, true, itm);
		    else
		      cd->cmd_fun_itm (lin + pos, false, NULL);
		  }
		  return;
		default:
		  MOM_FATAPRINTF
		    ("impossible command type for cmdix#%d line %s", cmdix,
		     lin);
		  return;
		}
	    }
	}
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
      MOM_DEBUGPRINTF (cmd, "after readline lin=%s", lin);
      if (!lin)
	break;
      cmd_interpret_mom (lin);
    };
  MOM_INFORMPRINTF ("momplug_cmd ending after load");
}


// eof momplug_cmd.c
