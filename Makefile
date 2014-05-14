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
## glib-2.0 & gmodule-2.0 are from GTK3, see https://developer.gnome.org/glib/
## gmime is also Gnome related, see https://developer.gnome.org/gmime/
## onion is not packaged, see https://github.com/davidmoreno/onion
## Boehm GC is from http://www.hboehm.info/gc/
PACKAGES= sqlite3 glib-2.0 gmodule-2.0 gmime-2.6  libcurl
PKGCONFIG= pkg-config
CC=gcc
CFLAGS= -std=gnu11 -Wall $(PREPROFLAGS) $(OPTIMFLAGS)
CXX=g++
CXXFLAGS= -std=c++11 -Wall -pthread  $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent -gnu
PREPROFLAGS= $(shell $(PKGCONFIG) --cflags $(PACKAGES)) -Inanohttp/ -I.
OPTIMFLAGS= -Og -g
LIBES= libnanohttp.a -luuid -lgc  $(shell $(PKGCONFIG) --libs $(PACKAGES))  -lm -ldl
SQLITE= sqlite3
SOURCES= $(sort $(filter-out $(wildcard mod_*.c), $(wildcard [a-z]*.c)))
MODSOURCES= $(sort $(wildcard mod_*.c))
MODULES= $(patsubst %.c,%.so,$(MODSOURCES))
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))
NANOHTTP_SOURCES= $(wildcard nanohttp/*.c)
NANOHTTP_OBJECTS= $(patsubst nanohttp/%.c, nanohttp/%.o, $(NANOHTTP_SOURCES))

RM= rm -fv
.PHONY: all modules clean tests indent restore-state dump-state
.SUFFIXES: .so
all: monimelt make-named modules 
clean:
	$(RM) *~ *.o *.so *.orig _tmp_* monimelt core* */*~ *.tmp  lib*.a _timestamp.* \
	     state-monimelt.dbsqlite-journal nanohttp/*.o nanohttp/*.mkd
################
monimelt: $(OBJECTS) _timestamp.o  libnanohttp.a
	$(LINK.c)  -rdynamic $^ $(LIBES) -o $@
	rm _timestamp.*

_timestamp.c:
	@date +'const char monimelt_timestamp[]="%c";' > _timestamp.tmp
	@(echo -n 'const char monimelt_lastgitcommit[]="' ; \
	 git log --format=oneline --abbrev=12 --abbrev-commit -q | head -1 | tr -d '\n\r\f\"' ; \
	 echo '";') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

indent: # don't indent monimelt-names.h
	@for f in *.c $(filter-out monimelt-names.h, $(wildcard *.h)); do \
	  echo indenting $$f; $(INDENT) $$f ;$(INDENT) $$f; done
	@for f in $(wildcard nanohttp/*.[ch]); do \
	  echo indenting $$f; $(INDENT) $$f ;$(INDENT) $$f; done

$(OBJECTS): monimelt.h monimelt-names.h

make-named: u-make-named.cc
	$(LINK.cc)  $<  $(LIBES) -o $@

modules: $(MODULES)

## MONIMELT generated code starts with momg_ followed by alphanum or +
## or - or _ characters. see MONIMELT_SHARED_MODULE_PREFIX in monimelt.h
momg_%.so: momg_%.c | monimelt.h monimelt-names.h
	$(LINK.c) -fPIC $< -shared -o $@
	@logger -t makemonimelt -p user.info -s compiled $< into shared $@ at $$(date +%c)

## extra modules
mod_%.so: mod_%.c  | monimelt.h monimelt-names.h
	$(LINK.c) -fPIC $< -shared -o $@

restore-state: 
	-mv -v state-monimelt.dbsqlite  state-monimelt.dbsqlite~
	$(SQLITE) state-monimelt.dbsqlite < state-monimelt.sql

dump-state:
	./dump-state.sh

libnanohttp.a: $(NANOHTTP_OBJECTS)
	ar rcv $@ $^

nanohttp/%.o: nanohttp/%.c nanohttp/%.h
	$(COMPILE.c) $< -DHAVE_CONFIG_H -MMD -MT "nanohttp/$(*F).o" -MF  "nanohttp/$(*F).mkd" -o $@

-include nanohttp/*.mkd
