################################################################
# the MELT monitor

This is the `README.md` file of the MELT monitor.

## About MELT and its future monitor

[GCC MELT](http://gcc-melt.org/) or simply *MELT* is a high level
domain specific language and plugin for the [GCC](http://gcc.gnu.org/)
compiler.

MELT does not have a satisfactory graphical user interface (it used to
have a *probe* program in GTK).

This repository is the *incomplete, pre-alpha* version of the MELT
monitor (once the MELT monitor would be able to work with the MELT
plugin for GCC, it will be merged into the MELT branch of the GCC
compiler).

This is free software -very experimental-, GPLv3 licensed (and FSF
copyrighted, since it will be merged into MELT). 

## The goals and dreams of the MELT monitor

Even for me [Basile Starynkevitch](http://starynkevitch.net/Basile/)
the overall goals of the MELT monitor are still fuzzy now. However
they include notably:

- providing a web interface to [MELT](http://gcc-melt.org/), in
  particular with the ability to show source code (in C, C++, etc...)
  compiled by a MELT enhanced GCC. This interface should work
  independently of GCC (in particular, should show useful stuff even
  when the `gcc` process ended). Hence, a MELT enhanded compilation by
  GCC is interacting with the MELT monitor (using asynchronous textual
  protocols, probably JSON and/or S-expr based). The web interface is
  expected to be used by few simultanous users (one or a few
  developers in the same team, but certainly not thousands).
  
- store **persistently** information, in particular some kind of GCC
  internal representations and its own information (being able to
  generate the C code of the monitor). Probably, that might mean to
  store some Gimple related representation in databases. So the state
  of the monitor is conceptually persistent and persisted (using
  [JSON](http://json.org/) textual format inside some databases).

- be able to process a quite significant amount of GCC related
  information, at least as large as GCC source code itself. The dream
  is to be able one day to do some data-mining stuff on a large amount
  of GCC compiled code (perhaps a small Linux distribution), perhaps
  finding similarity, using maching learning techniques to learn some
  coding rules, etc etc... (I am not fond of provably *sound* static
  analysis techniques, more interested in heuristics etc...)

- take advantage of multi-processing technologies available on desktop
  machine. This means multi-threading and running on several cores.

- use symbolic, reflexive and heuristic artificial intelligence
  techniques. [Jacques Pitrat's
  ideas](http://bootstrappingartificialintelligence.fr/WordPress3/)
  are very inspirational. So again, a sort of domain specific language
  (which should evolve into a rule based system) translated into C
  code.

- even for the monitor's own development, use a Web interface. So we
  should use wiki-like techniques.

- security is not a major concern. The user is trusted and is not
  expected to behave wrongly.

## Requirements and prerequisites

I only care about Linux (my systems are Debian or related) on x86-64.


### prerequisites

- a very recent [GCC](http://gcc.gnu.org/) compiler on Linux,
  i.e. Debian packages `gcc-4.8`, `g++-4.8`, `gcc-4.8-plugin-dev` and/or
  better (i.e. GCC 4.9)

- a recent HTML5 standards compatible browser, e.g. Firefox 29

- GNU `make`, preferably its 4.0 version
  
- the latest release or snapshot of [MELT](http://gcc-melt.org/)

- the `pkg-config` utility (and Debian package of the same name)

- [Sqlite3](http://sqlite.org), i.e. Debian packages `libsqlite3-dev`
  and `sqlite3`
  
- `glib-2.0` & `gmodule-2.0` from GTK3 see  [Glib](https://developer.gnome.org/glib/)
so `libglib2.0-dev` Debian package

- [gmime](https://developer.gnome.org/gmime/) so `libgmime-2.6-dev`
   Debian package.

- [Boehm's conservative garbage
  collector](http://www.hboehm.info/gc/), i.e. the `libgc-dev` Debian
  package.
 
- [onion HTTP server library](https://github.com/davidmoreno/onion) by
  David Moreno. You want a recent version (may 2014 github snapshot),
  and it may be sadly not packaged by Debian.

- [JQuery2](http://http://jquery.com/) which might be bundled in the monitor

- [CodeMirror](http://codemirror.net/) which might be bundled in the monitor

- [npm](https://www.npmjs.org/) Javascript package manager, i.e. `npm` Debian package

- [browserify](http://browserify.org/) to make JS bundles using `npm`


################################################################
# Technical ideas and terminology

## Processed data

In principle, the system persists most of its state on disk (sqlite3
database, perhaps Mongo database later), perhaps even (conceptually)
the current [continuations](http://en.wikipedia.org/wiki/Continuation)

The monitor is very dynamically typed. It has *immutable values* (like
boxed strings, JSON objects, etc...) and *shared mutable
items*. Values are either immutable values or (shared) references
(i.e. pointers) to items. See `momval_t` type in `monimelt.h`

Values are hashable and comparable.

### immutable values

They include:
1. the nil
2. boxed string
3. boxed integer numbers, and boxed double precision floats
4. set or tuple of items
5. nodes, which have a connector item and a sequence of sons.
6. closures are like nodes, but their connector is a routine and their sons represent
   the closed values

### shared mutable items

Items can be plain items, or box items, vector items, associative hash
items, JSON name items, tasklet items, routine items, queue items,
process items, web request items, etc. Basic blocks and edges as known
to GCC will be represented as items. Each item has

1. A globally unique
[UUID](http://en.wikipedia.org/wiki/Universally_unique_identifier)
which is fixed and is used notably for persistency, hashing, and sorting of items

2. A mutex to ensure that only one thread is accessing or modifying the item

3. Each item has its (dynamically changing) attribute association and
content. Attributes are themselves items, associated to non-nil
values. The content is some arbitrary value. An additional payload is
possible, and specific to the item's type.

### persistency

Values (including items) are persistable. Each item can belong to a
space which can be persisted. An item without a space is transient and
is not persisted on disk.

Items are persisted using their UUID as some key. The space is also a
persistency area. The root space is an Sqlite3 database (whose SQL
dump is kept in the GIT repository).


### dictionnary

We have one global dictionnary of named items. Names are C-like
identifiers.  An item can have at most one name (but many items are
anonymous). Some named item are predefined (see `monimelt-names.h`
file) because they are created before loading the state Sqlite3 file.

## Agenda, tasklets and work threads

The monitor is started with a small and fixed amount of worker threads
(typically 2 to 8 workers) running in parallel. The agenda is a
predefined queue item named `agenda`. It contains a FIFO queue of
tasklet items (and of course attributes and content, which are not
relevant for workers).  Each worker removes one tasklet item in the
front of the queue and steps thru it, and repeats that execution loop
indefinitely (sleeping when no tasklet is queued in the agenda).

A tasklet item has as its payload (conceptually) a call stack of
frames. Each frame has a closure, a state, and local values, numbers
and doubles. The closure has a routine, which gets executed when a
worker steps into the tasklet. It may (and often does) enqueue the
running tasklet -or some other taskets (at the rear of the agenda
queue.

Webrequest items are created by incoming HTTP requests. They are not
fully persistable (only persisted as a boxed item with attributes and
content) and are created not belonging to any space. The
`web_dictionnary` contains names (of HTTP POST requests) associated to
their handling closure, which is used to create a tasklet item created
to handle that web request.

Process items are created to fork processes, and are not fully
persistable neither (also persisted as boxed items). They also
contains a closure used to create a tasklet item created when the
process has ended.

The agenda is persistent. In principle, one could persist the state in
the evening and restart from it later.

# What is working?

Not much, it is pre-alpha software. If lucky you might be able to build it.

This code is free software (GPLv3+) in pre-alpha stage in may 2014.

To test it, (edit `Makefile` if wanted, then) compile monimelt with
    make,
then first build the state-monimelt.dbsqlite sqlite3 database with
    make restore-state
then run
   ./monimelt -W localhost:8086 -J 2   

and try browsing http://localhost:8086/testform.html or
http://localhost:8086/status

You may want to get debugging output, e.g. all debugging with -D _
or only running & web debugging with -D web,run program option.
Look for MOM_DEBUG_LIST_OPTIONS macros in monimelt.h


# Contact information

[Basile Starynkevitch](http://starynkevitch.net/Basile/), from France;
my email is my first (christian) name (i.e. `basile`) followed by the
`@` sign followed by my last (family) name (i.e. `starynkevitch`)
followed by `.net`. Spambots are not supposed understand this, but
humans should. :smile: