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
PACKAGES= sqlite3 glib-2.0
PKGCONFIG= pkg-config
CC=gcc
CFLAGS= -std=gnu11 -Wall $(PREPROFLAGS) $(OPTIMFLAGS)
CXX=g++
CXXFLAGS= -std=c++11 -Wall -pthread  $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent -gnu
PREPROFLAGS= $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g
LIBES= -luuid -lgc $(shell $(PKGCONFIG) --libs $(PACKAGES)) -lpthread -lm -ldl
SOURCES= $(sort $(filter-out $(wildcard mod_*.c), $(wildcard [a-z]*.c)))
MODSOURCES= $(sort $(wildcard mod_*.c))
MODULES= $((patsubst %.c,%.so,$(MODSOURCES))
OBJECTS= $(patsubst %.c,%.o,$(SOURCES))
RM= rm -fv
.PHONY: all modules clean tests indent
all: monimelt make-named modules
clean:
	$(RM) *~ *.o *.so *.orig _tmp_* monimelt
################
monimelt: $(OBJECTS)
	$(LINK.c)  -rdynamic $^ $(LIBES) -o $@

indent: # don't indent monimelt-names.h
	@for f in *.c $(filter-out monimelt-names.h, $(wildcard *.h)); do \
	  echo formatting $$f; $(INDENT) $$f ; done

$(OBJECTS): monimelt.h monimelt-names.h

make-named: u-make-named.cc
	$(LINK.cc)  $<  $(LIBES) -o $@

modules: $(MODULES)
