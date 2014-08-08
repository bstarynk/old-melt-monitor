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
CCFLAGS=  -std=gnu11 -Wall -Wextra
CFLAGS= $(CCFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
CXX=g++
CXXFLAGS= -std=c++11 -Wall -pthread  $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent -gnu
ASTYLE= astyle --style=gnu  
PREPROFLAGS= -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3
LIBES= -L/usr/local/lib -lunistring -lgc  $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
       -lonion_handlers -lonion -lpthread -lm -ldl
## JsonRpc client might use cxxtools http://www.tntnet.org/cxxtools.html
CXXTOOLS_CXXFLAGS:=$(shell cxxtools-config --cxxflags)
CXXTOOLS_LIBS:=$(shell cxxtools-config --libs)
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
MELTGCCFLAGS=  -fplugin=melt -fplugin-arg-melt-init=@melt-default-modules.quicklybuilt -fplugin-arg-melt-workdir=_meltwork
####
#### MONI_MELT variables are for MELT plugin and the monimelt monitor working together
## a temporary suffix
MONI_MELT_TMP:=$(shell mktemp -u -t moni-melt_XXXXXXX)
## job options
MONI_MELT_JOB_FLAGS= --jobs 2
MONI_MELT_RUN_PID=$(MONI_MELT_TMP)_runpid
MONI_MELT_RUN_FLAGS= --write-pid $(MONI_MELT_RUN_PID)
MONI_MELT_SOCKET=$(MONI_MELT_TMP)_socket
MONI_MELT_DEBUG_FLAGS= -D run
MONI_MELT_JSONRPC_FLAGS= --jsonrpc $(MONI_MELT_SOCKET)
####
####
.PHONY: all modules plugins clean tests indent restore-state dump-state \
	melt-process-header melt-process-debug
.SUFFIXES: .so .i
# to make with tsan: make OPTIMFLAGS='-g3 -fsanitize=thread -fPIE' LINKFLAGS=-pie
all: monimelt modules plugins momjsrpc_client
clean:
	$(RM) *~ *.o *.so *.i *.orig melt*.cc meltmom*.[ch] meltmom*.o meltmom*.so meltmom*.mk \
	      _tmp_* monimelt core* webdir/*~ *.tmp  _timestamp.* *dbsqlite*-journal *%
	$(RM) modules/*.so modules/*~
	$(RM) -r _monimelt_termdump*
	$(RM) -r _meltwork
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

indent: .indent.pro # don't indent predef-monimelt.h
	@for f in *.c $(filter-out predef-monimelt.h, $(wildcard *.h)) \
	     $(MODULE_SOURCES); do \
	  echo indenting $$f; $(INDENT) $$f ;$(INDENT) $$f; done
	@for f in *.cc ; do \
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
modules/momg_%.so: modules/momg_%.c | monimelt.h predef-monimelt.h
	$(LINK.c) -DMONIMELT_CURRENT_MODULE=\"$(patsubst momg_%.so,%,$(*F))\" \
                  -fPIC $< -shared -o $@
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

###
momjsrpc_client: momjsrpc_client.cc
	$(CXX) $(CXXFLAGS) $(CXXTOOLS_CXXFLAGS) $< -o $@ -lcxxtools-json $(CXXTOOLS_LIBS)

###
melt-process-header: monimelt.h meltmom-process.quicklybuilt.so | _meltwork monimelt
	@echo MONI_MELT_TMP= $(MONI_MELT_TMP)
	./monimelt --daemon-noclose --chdir $(PWD) $(MONI_MELT_RUN_FLAGS) \
          $(MONI_MELT_JOB_FLAGS) $(MONI_MELT_JSONRPC_FLAGS)
	$(COMPILE.c) -x c $(MELTGCCFLAGS) -DMELTMOM \
	    -fplugin-arg-melt-mode=process_monimelt_header \
	    -fplugin-arg-melt-extra=meltmom-process.quicklybuilt \
	    -fplugin-arg-melt-monimelt-tmp=$(MONI_MELT_TMP) \
	    -c $< -o /dev/null
	kill -TERM $$(cat $(MONI_MELT_RUN_PID))
	ls -l $(MONI_MELT_TMP)*
	$(RM) $(MONI_MELT_TMP)*

melt-process-debug: monimelt.h meltmom-process.quicklybuilt.so | _meltwork monimelt
	@echo MONI_MELT_TMP= $(MONI_MELT_TMP)
	./monimelt $(MONI_MELT_DEBUG_FLAGS) --daemon-noclose --chdir $(PWD) $(MONI_MELT_RUN_FLAGS)  \
          $(MONI_MELT_JOB_FLAGS) $(MONI_MELT_JSONRPC_FLAGS)
	$(COMPILE.c) -x c $(MELTGCCFLAGS) -DMELTMOM  \
	    -fplugin-arg-melt-mode=process_monimelt_header \
	    -fplugin-arg-melt-extra=meltmom-process.quicklybuilt \
	    -fplugin-arg-melt-monimelt-tmp=$(MONI_MELT_TMP) \
	    -fplugin-arg-melt-debugging=mode \
            -c $< -o /dev/null
	kill -TERM $$(cat $(MONI_MELT_RUN_PID))
	ls -l $(MONI_MELT_TMP)*
	$(RM) $(MONI_MELT_TMP)*

_meltwork:
	@ [ -d _meltwork ] || mkdir _meltwork

meltmom-process.quicklybuilt.so: meltmom-process.melt  | _meltwork
	$(COMPILE.c) -x c $(MELTGCCFLAGS) \
	    -fplugin-arg-melt-mode=translatequickly \
	    -fplugin-arg-melt-arg=$< \
	    -x c -c /dev/null -o /dev/null
