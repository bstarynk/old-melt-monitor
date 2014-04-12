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

struct headentry {
    std::string name, type, uidstr;
    headentry (std::string n, std::string t, std::string u) : name(n), type(t), uidstr(u) {};
};

std::vector<headentry> headvect;


/// read the header file and populate headvect;
void read_header(const char*argv0)
{
    std::ifstream hf(headerpath);
    int lineno=0;
    for (std::string line; std::getline(hf,line);) {
        lineno++;
	if (line.empty())
	  continue;
        char curname[80];
        char curtype[80];
        char curuuid[40];
        memset (curname, 0, sizeof(curname));
        memset (curtype, 0, sizeof(curtype));
        memset (curuuid, 0, sizeof(curuuid));
        int pos= 0;
        if (sscanf(line.c_str(), " MONIMELT_NAMED ( %78[A-Za-z0-9_], %78[A-Za-z0-9_], \"%39[0-9a-fA-F-]\" )%n",
                   curname, curtype, curuuid, &pos) >= 3 && pos>0) {
            headvect.push_back(headentry(std::string(curname),std::string(curtype),std::string(curuuid)));
            if (!strcmp(curname, itemname.c_str())) {
                fprintf(stderr, "%s:%d: name '%s' in line %s\n",
                        headerpath.c_str(), lineno, curname, line.c_str());
                exit(EXIT_FAILURE);
            }
        }
        else if (line.find("MONIMELT_NAMED") != std::string::npos
                 && line[0] != '#' && line[0] != '/') {
            fprintf(stderr, "%s:%d: bad line %s\n",
                    headerpath.c_str(), lineno, line.c_str());
            exit(EXIT_FAILURE);
        }
    }
    hf.close();
    if (verbose)
        printf("%s: parsed %d names in %s\n", argv0, (int)headvect.size(), headerpath.c_str());
}

int check_cb (void*data, int nbcol, char**colval, char**colname)
{
    char*argv0 = (char*)data;
    fprintf(stderr, "%s: duplicate name in database %s with uid=%s\n",
            argv0, dbpath.c_str(), colval[0]);
    return 1;
}

void check_dbase(const char*argv0)
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
    snprintf(sqlbuf, sizeof(sqlbuf),
             "SELECT nuid FROM t_name WHERE name='%s'", itemname.c_str());
    if (verbose) printf ("%s: sql: %s\n", argv0, sqlbuf);
    if (sqlite3_exec(db, sqlbuf, check_cb, (void*)argv0, &errstr)) {
        fprintf(stderr, "failed item check sql: %s\n %s (%s)\n",
                sqlbuf, errstr, sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    };
}


void add_dbase(const char*argv0)
{
    char sqlbuf[512];
    char* errstr=0;
    if (sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, &errstr)) {
        fprintf(stderr, "failed item insertion BEGIN TRANSACTION\n %s (%s)\n",
                errstr, sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    };
    // insert the item
    snprintf(sqlbuf, sizeof(sqlbuf),
             "INSERT INTO t_item (uid,type,jbuild,jfill) VALUES "
             "('%s','%s','','')", itemstruuid, itemtype.c_str());
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
    if (sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, &errstr)) {
        fprintf(stderr, "failed item insertion END TRANSACTION\n %s (%s)\n",
                errstr, sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    };
}


void update_header(const char*argv0)
{
    headvect.push_back(headentry(itemname,itemtype,std::string(itemstruuid)));
    std::stable_sort(headvect.begin(), headvect.end(),
                     [](const headentry&l, const headentry&r)
    {
        return l.name < r.name;
    });
    rename(headerpath.c_str(), (headerpath+"~").c_str());
    if (verbose)
        printf("%s: generating header %s\n", argv0, headerpath.c_str());
    std::ofstream ohf(headerpath);
    ohf << "// generated header " << headerpath << " for named items of Monimelt"
        << std::endl;
    ohf << "#ifndef MONIMELT_NAMED" << std::endl
        << "#error should have defined MONIMELT_NAMED macro" << std::endl
        << "#endif /*MONIMELT_NAMED*/" << std::endl << std::endl;

    for (auto e : headvect) {
        ohf << " MONIMELT_NAMED (" << e.name << ", "
            << e.type << ", \""
            << e.uidstr << "\")" << std::endl;
    }
    ohf << std::endl << "// end of " << headvect.size() << " named items" << std::endl;
    ohf << "#undef MONIMELT_NAMED" << std::endl;
    ohf.close();
    printf("%s: generating %d items in header %s\n", argv0,
           (int)headvect.size(), headerpath.c_str());
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
            if (optarg) dbpath=optarg;
            break;
        case 'H': /* header */
            if (optarg) headerpath=optarg;
            break;
        case 'n':
            if (optarg) itemname = optarg;
            break;
        case 't':
            if (optarg) itemtype = optarg;
            break;
        case '?':
            usage(argv[0]);
            break;
        default:
            fprintf(stderr, "bad option #%d: %c\n", option_index, c);
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
{
    return isalnum(c) || c=='_';
    })) {
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
{
    return isalnum(c) || c=='_';
    })) {
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
    // check the database
    check_dbase(argv[0]);
    // read and check the header
    read_header(argv[0]);
    // update database
    add_dbase(argv[0]);
    // update header
    update_header(argv[0]);
    //
    if (db) sqlite3_close(db);
    std::clog << argv[0] << " added item named " << itemname << " of type " << itemtype << " and uuid " << itemstruuid << std::endl;
}

