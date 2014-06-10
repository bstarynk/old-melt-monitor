##
## file Makefile
##   Copyright (C)  2014 Free Software Foundation, Inc.
##  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
##  This file is part of GCC.
##
##  GCC is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 3, or (at your option)
##  any later version.
##
##  GCC is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##  You should have received a copy of the GNU General Public License
##  along with GCC; see the file COPYING3.   If not see
##  <http://www.gnu.org/licenses/>.
################################################################
## sqlite3 is commonly available, see http://sqlite.org
## glib-2.0 are from GTK3, see https://developer.gnome.org/glib/
## gmime is also Gnome related, see https://developer.gnome.org/gmime/
## onion is not packaged, see https://github.com/davidmoreno/onion
## Boehm GC is from http://www.hboehm.info/gc/
PACKAGES= sqlite3 glib-2.0 #gmime-2.6 libcurl
PKGCONFIG= pkg-config
CC=gcc
CFLAGS= -std=gnu11 -Wall -Wextra $(PREPROFLAGS) $(OPTIMFLAGS)
CXX=g++
CXXFLAGS= -std=c++11 -Wall -pthread  $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent -gnu
PREPROFLAGS= -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g
LIBES= -L/usr/local/lib -lunistring -lgc  $(shell $(PKGCONFIG) --libs $(PACKAGES)) -lonion_handlers -lonion -lpthread -lm -ldl
SQLITE= sqlite3
# modules are monimelt generated code
MODULE_SOURCES= $(sort $(wildcard modules/momg_*.c))
MODULES= $(patsubst %.c,%.so,$(MODULE_SOURCES))
# plugins are extra code
PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
SOURCES= $(sort $(filter-out $(PLUGIN_SOURCES) $(MODULE_SOURCES), $(wildcard [a-z]*.c)))
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))
RM= rm -fv
.PHONY: all modules plugins clean tests indent restore-state dump-state
.SUFFIXES: .so .i
all: monimelt modules plugins
clean:
	$(RM) *~ *.o *.so *.i *.orig _tmp_* monimelt core* webdir/*~ *.tmp  _timestamp.* *dbsqlite*-journal *%
	$(RM) modules/*.so modules/*~
	$(RM) -r _monimelt_termdump*
################
monimelt: $(OBJECTS) _timestamp.o
	@if [ -f $@ ]; then echo -n backup old executable: ' ' ; mv -v $@ $@~ ; fi
	$(LINK.c)  -rdynamic $^ $(LIBES) -o $@
	rm _timestamp.*

_timestamp.c:
	@date +'const char monimelt_timestamp[]="%c";' > _timestamp.tmp
	@(echo -n 'const char monimelt_lastgitcommit[]="' ; \
	    git log --format=oneline --abbrev=12 --abbrev-commit -q | head -1 | tr -d '\n\r\f\"' ; \
	    echo '";') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

indent: .indent.pro # don't indent predef-monimelt.h
	@for f in *.c $(filter-out predef-monimelt.h, $(wildcard *.h)) $(MODULE_SOURCES); do \
	  echo indenting $$f; $(INDENT) $$f ;$(INDENT) $$f; done

$(OBJECTS): monimelt.h predef-monimelt.h

.indent.pro: monimelt.h
	sed -n 's/typedef.*\(mom[a-z0-9_]*_t\);/-T \1/p' monimelt.h | sort -u > $@

%.i: %.c
	$(COMPILE.c) -C -E $< | sed s:^#://#: > $@

modules: $(MODULES)

plugins: $(PLUGINS)

## MONIMELT generated code starts with momg_ followed by alphanum or +
## or - or _ characters, conventionally by the name or identstr of the
## module item. see MONIMELT_SHARED_MODULE_PREFIX in monimelt.h
modules/momg_%.so: modules/momg_%.c | monimelt.h predef-monimelt.h
	$(LINK.c) -DMONIMELT_CURRENT_MODULE=\"$(patsubst momg_%.so,%,$(*F))\" -fPIC $< -shared -o $@
	@logger -t makemonimelt -p user.info -s compiled $< into \
	        shared module $@ named $(patsubst momg_%.so,%,$(*F)) \
	        at $$(date +%c)

## extra plugins
momplug_%.so: momplug_%.c  | monimelt.h predef-monimelt.h
	$(LINK.c) -fPIC $< -shared -o $@

restore-state: 
	-mv -v state-monimelt.dbsqlite  state-monimelt.dbsqlite~
	$(SQLITE) state-monimelt.dbsqlite < state-monimelt.sql

dump-state:
	./monimelt-dump-state.sh
