#! /bin/sh
## file dump-state.sh
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
##  utility to dump the sqlite3 database state-monimelt.dbsqlite
## in a textual SQL dump state-monimelt.sql
tempdump=$(basename $(tempfile -d . -p _tmp_ -s .sql))
trap 'rm -vf $tempdump' EXIT INT QUIT TERM
export LANG=C LC_ALL=C
logger -s -t dump-monimelt-state $(date +"start dump-monimelt-state %c")
date +'-- state-monimelt dump %Y %b %d' > $tempdump
sqlite3 state-monimelt.dbsqlite .schema >> $tempdump || exit 1
echo '-- state-monimelt tables contents' >> $tempdump 
sqlite3 state-monimelt.dbsqlite >> $tempdump <<EOF
.mode insert t_item
  SELECT * FROM t_item ORDER BY uid;
.mode insert t_name
  SELECT * FROM t_name ORDER BY name;
EOF
echo "-- state-monimelt end dump " >> $tempdump
mv state-monimelt.sql state-monimelt.sql~
mv $tempdump state-monimelt.sql
