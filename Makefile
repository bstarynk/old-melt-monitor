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
PACKAGES=  libcurl jansson #sqlite3 glib-2.0 gmime-2.6
PKGCONFIG= pkg-config
CC=gcc
CCFLAGS=  -std=gnu11 -Wall -Wextra -fdiagnostics-color=auto
CFLAGS= $(CCFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
CXX=g++
CXXFLAGS= -std=c++11 -Wall -pthread  $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent
INDENTFLAGS= -gnu --no-tabs
ASTYLE= astyle --style=gnu
PREPROFLAGS= -I. -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3
TAGS= etags
LIBES= -L/usr/local/lib -lunistring -lgc $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
        -lgccjit -lonion -lpthread -lcrypt -lm -ldl
PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
# modules are generated inside modules/
MODULE_SOURCES= $(sort $(wildcard modules/momg_*.c))
MODULES=  $(patsubst %.c,%.so,$(MODULE_SOURCES))
SOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.c)))
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))
RM= rm -fv
####
####
.PHONY: all tags modules plugins clean tests tests-jit indent restore-state dump-state passwords
.SUFFIXES: .so .i
# to make with tsan: make OPTIMFLAGS='-g3 -fsanitize=thread -fPIE' LINKFLAGS=-pie
all: monimelt modules plugins tags
clean:
	$(RM) *~ *.o *.so */*.so *.log */*~ */*.orig *.i *.orig melt*.cc meltmom*.[ch] meltmom*.o meltmom*.so meltmom*.mk \
	      _tmp_* monimelt core* modules/*.tmp modules/*.bad webdir/*~ *.tmp  _timestamp.* *dbsqlite*-journal *%
	$(RM) modules/*.so modules/*~ modules/*%
	$(RM) -rf _monimelt_*
	$(RM) jit-doc.html
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
	  echo indenting $$f; cp $$f $$f% ; $(INDENT) $(INDENTFLAGS) $$f ; $(INDENT)  $(INDENTFLAGS) $$f; done
	@for f in $(wildcard *.cc) ; do \
          echo formatting $$f; $(ASTYLE) $$f; done

$(OBJECTS): monimelt.h predef-monimelt.h apply-monimelt.h

.indent.pro: monimelt.h
	- sed -n 's/typedef.*\(mom[a-z0-9_]*_t\).*;/-T \1/p' monimelt.h | sort -u > $@-tmp
	- echo '-T FILE' >> $@-tmp
	- echo '-T json_t' >> $@-tmp
	- echo '-T onion_request' >> $@-tmp
	- echo '-T onion_response' >> $@-tmp
	- echo '-T int8_t' >> $@-tmp
	- echo '-T int16_t' >> $@-tmp
	- echo '-T int32_t' >> $@-tmp
	- echo '-T int64_t' >> $@-tmp
	- echo '-T intptr_t' >> $@-tmp
	- echo '-T uint8_t' >> $@-tmp
	- echo '-T uint16_t' >> $@-tmp
	- echo '-T uint32_t' >> $@-tmp
	- echo '-T uint64_t' >> $@-tmp
	- echo '-T uintptr_t' >> $@-tmp
	- echo '-T fd_set' >> $@-tmp
	- echo '-T CURLM' >> $@-tmp
	- echo '-T CURL' >> $@-tmp
	- echo '-T gcc_jit_context' >> $@-tmp
	- echo '-T gcc_jit_object' >> $@-tmp	
	- sort  $@-tmp > $@
	- rm $@-tmp

tags: TAGS

TAGS: $(wildcard *.h *.c */*.c)
	$(TAGS) $(wildcard *.h *.c */*.c)

%.i: %.c
	$(COMPILE.c) -C -E $< | sed s:^#://#: > $@

modules: $(MODULES)

plugins: $(PLUGINS)

## MONIMELT generated code starts with momg_ followed by alphanum or +
## or - or _ characters, conventionally by the name or identstr of the
## module item. see MONIMELT_SHARED_MODULE_PREFIX in monimelt.h
modules/momg_%.so: modules/momg_%.c | monimelt.h predef-monimelt.h
	@-mv -vf $@ $@~
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

tests: tests-jit

## the web passwords file .monpasswd, à la htpasswd(5)
passwords: .mompasswd

.mompasswd:
	-mv -fv .mompasswd .mompasswd~
	@echo Enter web password for master
	@(echo -n master: ; mkpasswd -msha-256) > .mompasswd

tests-jit: ./monimelt $(wildcard xtra-jitest-*.mom)
	[ -d /tmp/newmonimelt/ ] || mkdir /tmp/newmonimelt/
	rm -rf /tmp/newmonimelt/*
	./monimelt -Dgencod -X xtra-jitest-fact.mom \
            --chdir /tmp/newmonimelt/ \
	    --generate-jit-module jitest_fact_module \
            --dump-state /tmp/newmonimelt/
