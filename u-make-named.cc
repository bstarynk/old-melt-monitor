// file u-make-named.c

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
      ----------

      a self-contained utility program to make new predefined named
**/

#include <cstdlib>
#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <cstring>
#include <regex.h>
#include <sqlite3.h>
#include <uuid/uuid.h>
#include <getopt.h>

bool verbose=0;

static struct option long_options[] = {
  {"database", required_argument, 0, 'b'},
  {"verbose", no_argument, 0, 'v'},
  {"header", required_argument, 0, 'H'},
  {"name", required_argument, 0, 'n'},
  {"type", required_argument, 0, 't'},
  {0,0,0,0}
};

std::string dbpath = "state-monimelt.dbsqlite";
std::string headerpath = "monimelt-names.h";
std::string itemname;
std::string itemtype;
uuid_t itemuuid;
char itemstruuid[48];
sqlite3* db=nullptr;

void add_dbase(const char*argv0)
{
  int err = sqlite3_open(dbpath.c_str(),&db);
  if (err) {
    fprintf(stderr, "failed to open sqlite %s - %s\n",
	    dbpath.c_str(), sqlite3_errstr(err));
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("%s: opened database %s\n", argv0, dbpath.c_str());
  char sqlbuf[512];
  char* errstr=0;
  // insert the item
  snprintf(sqlbuf, sizeof(sqlbuf),
	   "INSERT INTO t_item (uid,type) VALUES "
	   "('%s','%s')", itemstruuid, itemtype.c_str());
  if (verbose) printf ("%s: sql: %s\n", argv0, sqlbuf);
  if (sqlite3_exec(db, sqlbuf, nullptr, nullptr, &errstr)) {
    fprintf(stderr, "failed item insertion sql: %s\n %s (%s)\n",
	    sqlbuf, errstr, sqlite3_errmsg(db));
    exit(EXIT_FAILURE);
  };
  // insert the name
  snprintf(sqlbuf, sizeof(sqlbuf),
	   "INSERT INTO t_name(name,nuid) VALUES"
	   "('%s','%s')", itemname.c_str(), itemstruuid);
  if (verbose) printf ("%s: sql: %s\n", argv0, sqlbuf);
  if (sqlite3_exec(db, sqlbuf, nullptr, nullptr, &errstr)) {
    fprintf(stderr, "failed item insertion sql: %s\n %s (%s)\n",
	    sqlbuf, errstr, sqlite3_errmsg(db));
    exit(EXIT_FAILURE);
  };
}


void update_header(const char*argv0)
{
  if (verbose)
    printf("%s: parsing header %s\n", argv0, headerpath.c_str());
  std::ifstream hf(headerpath);
  typedef std::tuple<std::string,std::string,std::string> tup3str_t;
  std::vector<tup3str_t> vec;
  int lineno=0;
  for (std::string line; std::getline(hf,line);) {
    lineno++;
    char curname[80];
    char curtype[80];
    char curuuid[40];
    memset (curname, 0, sizeof(curname));
    memset (curtype, 0, sizeof(curtype));
    memset (curuuid, 0, sizeof(curuuid));
    int pos= 0;
    if (sscanf(line.c_str(), " MONIMELT_NAMED ( %78s, %78s, \"%39[0-9a-fA-F-]\" )%n",
	       curname, curtype, curuuid, &pos) >= 3 && pos>0) {
      vec.push_back(std::make_tuple(std::string(curname),std::string(curtype),std::string(curuuid)));
    }
    else if (line.find("MONIMELT_NAMED")>=0
	     && line[0] != '#' && line[0] != '/') {
      fprintf(stderr, "%s:%d: bad line %s\n",
	      headerpath.c_str(), lineno, line.c_str());
      exit(EXIT_FAILURE);
    }
  }
  hf.close();
  if (verbose)
    printf("%s: parsed %d names in %s\n", argv0, (int)vec.size(), headerpath.c_str());
  vec.push_back(std::make_tuple(itemname,itemtype,std::string(itemstruuid)));
  std::stable_sort(vec.begin(), vec.end(),
		   [](const tup3str_t&l, const tup3str_t&r)
		   { return std::get<0>(l) < std::get<0>(r); });
  rename(headerpath.c_str(), (headerpath+"~").c_str());
  if (verbose)
    printf("%s: generating header %s\n", argv0, headerpath.c_str());
  std::ofstream ohf(headerpath);
  ohf << "// generated header " << headerpath << " for named items of Monimelt"
      << std::endl;
  ohf << "#ifndef MONIMELT_NAMED" << std::endl
      << "#error should have defined MONIMELT_NAMED macro" << std::endl
      << "#endif /*MONIMELT_NAMED*/" << std::endl << std::endl;

  for (auto tu : vec) {
    ohf << " MONIMELT_NAMED (" << std::get<0>(tu) << ", "
	<< std::get<1>(tu) << ", \""
	<< std::get<2>(tu) << "\")" << std::endl;
  }
  ohf << std::endl << "// end of " << vec.size() << " named items" << std::endl;
  ohf << "#undef MONIMELT_NAMED" << std::endl;
  ohf.close();
  printf("%s: generating %d items in header %s\n", argv0,
	 (int)vec.size(), headerpath.c_str());
}

void usage(char*argv0)
{
  fprintf(stderr, "usage: %s\n"
	  "\t [ -v ] #verbose\n"
	  "\t [ -b | --database <db> ] #default %s\n",
	  argv0, dbpath.c_str());
  fprintf (stderr, "\t [ -H | --header <header> ] #default %s\n",
	   headerpath.c_str());
  fprintf (stderr, "\t  -n | --name <item-name>\n");
  fprintf (stderr, "\t  -t | --type <item-type>\n");
  exit (EXIT_FAILURE);
}

int main (int argc, char**argv)
{
  /* getopt_long stores the option index here. */
  int option_index = 0;
  int c=0;
  for (;;) {
    c = getopt_long (argc, argv, "b:H:n:t:v",
		     long_options, &option_index);
    if (c<0) break;
    switch (c) {
    case 'v':
      verbose = true;
      break;
    case 'b': /* database*/
      if (optarg) dbpath=optarg; break;
    case 'H': /* header */
      if (optarg) headerpath=optarg; break;
    case 'n': if (optarg) itemname = optarg; break;
    case 't': if (optarg) itemtype = optarg; break;
    case '?': usage(argv[0]); break;
    default: fprintf(stderr, "bad option #%d: %c\n", option_index, c);
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  // check item name
  if (itemname.empty()) {
    fprintf(stderr, "missing item name : %s -n <item-name> ...\n", argv[0]);
    exit (EXIT_FAILURE);
  };
  if (!isalpha(itemname[0])
      ||!std::all_of(itemname.begin(),itemname.end(),[](char c)
		     { return isalnum(c) || c=='_'; })) {
    fprintf(stderr, "%s: bad item name %s\n", argv[0], itemname.c_str());
    exit (EXIT_FAILURE);
  }
  if (verbose)
    printf("%s: good item name %s\n", argv[0], itemname.c_str());
  // check item type
  if (itemtype.empty()) {
    fprintf(stderr, "missing item type :  %s -t <item-type> ...\n", argv[0]);
    exit (EXIT_FAILURE);
  };
  if (!isalpha(itemtype[0])
      ||!std::all_of(itemtype.begin(),itemtype.end(),[](char c)
		     { return isalnum(c) || c=='_'; })) {
    fprintf(stderr, "%s: bad item type %s\n", argv[0], itemtype.c_str());
    exit (EXIT_FAILURE);
  }
  if (verbose)
    printf("%s: good item type %s\n", argv[0], itemtype.c_str());
  // grep type in source code
  {
    char grepcmd[256];
    snprintf (grepcmd, sizeof(grepcmd), "/bin/fgrep -q 'momit_%s_t' *.c", itemtype.c_str());
    if (verbose)
      printf ("%s: running %s\n", argv[0], grepcmd);
    fflush(NULL);
    if (system(grepcmd)) {
      fprintf(stderr, "%s: unknown item type, failed command %s\n", argv[0], grepcmd);
      exit(EXIT_FAILURE);
    }
  }
  // generate uuid
  uuid_generate_random(itemuuid);
  uuid_unparse_lower(itemuuid, itemstruuid);
  if (verbose)
    printf ("%s: generated uuid %s for %s\n", argv[0], itemstruuid, itemname.c_str());
  // update database
  add_dbase(argv[0]);
  // update header
  update_header(argv[0]);
  //
  if (db) sqlite3_close(db);
  std::clog << argv[0] << " added item named " << itemname << " of type " << itemtype << " and uuid " << itemstruuid << std::endl;
}
