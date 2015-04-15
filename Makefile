##
## file Makefile
##   Copyright (C)  2015 Free Software Foundation, Inc.
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
## onion is not packaged, see https://github.com/davidmoreno/onion
## Boehm GC is from http://www.hboehm.info/gc/
PACKAGES=  libcurl #sqlite3 glib-2.0 gmime-2.6
PKGCONFIG= pkg-config
CC=gcc
CCFLAGS=  -std=gnu11 -Wall -Wextra -fdiagnostics-color=auto
CFLAGS= $(CCFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
CXX=g++
CXXFLAGS= -std=c++11 -Wall -pthread  $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent -gnu
ASTYLE= astyle --style=gnu  
PREPROFLAGS= -I. -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3
LIBES= -L/usr/local/lib -lunistring -lgc $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
        -lonion -lpthread -lm -ldl
PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
# modules are generated inside global-modules/ & user-modules
MODULE_SOURCES= $(sort $(wildcard global-modules/momg_*.c user-modules/momg_*.c))
MODULES=  $(patsubst %.c,%.so,$(MODULE_SOURCES))
SOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.c)))
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))
RM= rm -fv
####
####
.PHONY: all modules plugins clean tests indent restore-state dump-state
.SUFFIXES: .so .i
# to make with tsan: make OPTIMFLAGS='-g3 -fsanitize=thread -fPIE' LINKFLAGS=-pie
all: monimelt modules plugins
clean:
	$(RM) *~ *.o *.so */*.so */*~ */*.orig *.i *.orig melt*.cc meltmom*.[ch] meltmom*.o meltmom*.so meltmom*.mk \
	      _tmp_* monimelt core* module/*.tmp webdir/*~ *.tmp  _timestamp.* *dbsqlite*-journal *%
	$(RM) global-modules/*.so global-modules/*~ user-modules/*.so user-modules/*~
################
monimelt: $(OBJECTS) _timestamp.o
	@if [ -f $@ ]; then echo -n backup old executable: ' ' ; mv -v $@ $@~ ; fi
	$(LINK.c)  $(LINKFLAGS) -rdynamic $^ $(LIBES) -o $@
	rm _timestamp.*

_timestamp.c:
	@date +'const char monimelt_timestamp[]="%c";' > _timestamp.tmp
	@(echo -n 'const char monimelt_lastgitcommit[]="' ; \
	    git log --format=oneline --abbrev=12 --abbrev-commit -q  \
	      | head -1 | tr -d '\n\r\f\"' ; \
	    echo '";') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

indent: .indent.pro # don't indent predef-monimelt.h or generated modules/momg*.c
	@for f in *.c $(filter-out predef-monimelt.h, $(wildcard *.h)) \
	    ; do \
	  echo indenting $$f; $(INDENT) $$f ;$(INDENT) $$f; done
	@for f in $(wildcard *.cc) ; do \
          echo formatting $$f; $(ASTYLE) $$f; done

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
global-modules/momg_%.so: global-modules/momg_%.c | monimelt.h predef-monimelt.h
	$(LINK.c) -DMONIMELT_CURRENT_MODULE=\"$(patsubst momg_%.so,%,$(*F))\" \
		  -DMONIMELT_MD5_MODULE=\"$(shell md5sum $< | cut '-d ' -f1)\" \
		  -DMONIMELT_LAST_COMMITID=\"$(shell git log -n 1 --abbrev=16 --format=%h)\" \
                  -fPIC $< -shared -o $@
	@logger -t makemonimelt -p user.info -s compiled $< into \
	        shared module $@ named $(patsubst momg_%.so,%,$(*F)) \
	        at $$(date +%c)
user-modules/momg_%.so: user-modules/momg_%.c | monimelt.h predef-monimelt.h
	$(LINK.c) -DMONIMELT_CURRENT_MODULE=\"$(patsubst momg_%.so,%,$(*F))\" \
		  -DMONIMELT_MD5_MODULE=\"$(shell md5sum $< | cut '-d ' -f1)\" \
		  -DMONIMELT_LAST_COMMITID=\"$(shell git log -n 1 --abbrev=16 --format=%h)\" \
                  -fPIC $< -shared -o $@
	@logger -t makemonimelt -p user.info -s compiled $< into \
	        shared module $@ named $(patsubst momg_%.so,%,$(*F)) \
	        at $$(date +%c)

## extra plugins
momplug_%.so: momplug_%.c  | monimelt.h predef-monimelt.h
	$(LINK.c) -fPIC $< -shared -o $@ $(shell grep MONIMELTLIBS: $< | sed s/MONIMELTLIBS://)

###
momjsrpc_client: momjsrpc_client.cc
	$(CXX) $(CXXFLAGS) $(CXXTOOLS_CXXFLAGS) $< -o $@ -lcxxtools-json $(CXXTOOLS_LIBS)
