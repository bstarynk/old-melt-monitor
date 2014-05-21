#! /bin/sh
## file monimelt-dump-state.sh (should be in your $PATH)
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
statefile=$1
if [ -z "$statefile" ]; then
    statefile=state-monimelt.dbsqlite
fi
sqlfile=$2
if [ -z "$sqlfile" ]; then
    sqlfile=$(basename $statefile .dbsqlite).sql
fi
if [ ! -e "$statefile" ]; then
    echo "$0:" no state file to dump $statefile > /dev/stderr
    exit 1
fi

if file "$statefile" | grep -qi SQLite ; then
    echo "$0:" dumping state file $statefile
else
    echo "$0:" bad state file $statefile > /dev/stderr
    exit 1
fi

tempdump=$(basename $(tempfile -d . -p _tmp_ -s .sql))
trap 'rm -vf $tempdump' EXIT INT QUIT TERM
export LANG=C LC_ALL=C
logger -s -t dump-monimelt-state $(date +"start dump-monimelt-state %c") $statefile
date +'-- state-monimelt dump %Y %b %d' > $tempdump
echo >> $tempdump
date +' --   Copyright (C) %Y Free Software Foundation, Inc.' >> $tempdump
echo ' --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/' >> $tempdump
echo " --  This sqlite3 dump file $sqlfile is part of GCC." >> $tempdump
echo ' --' >> $tempdump
echo ' --  GCC is free software; you can redistribute it and/or modify' >> $tempdump
echo ' --  it under the terms of the GNU General Public License as published by' >> $tempdump
echo ' --  the Free Software Foundation; either version 3, or (at your option)' >> $tempdump
echo ' --  any later version.' >> $tempdump
echo ' --' >> $tempdump
echo ' --  GCC is distributed in the hope that it will be useful,' >> $tempdump
echo ' --  but WITHOUT ANY WARRANTY; without even the implied warranty of' >> $tempdump
echo ' --  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the' >> $tempdump
echo ' --  GNU General Public License for more details.' >> $tempdump
echo ' --  You should have received a copy of the GNU General Public License' >> $tempdump
echo ' --  along with GCC; see the file COPYING3.   If not see' >> $tempdump
echo ' --  <http://www.gnu.org/licenses/>.' >> $tempdump
echo >> $tempdump
echo 'BEGIN TRANSACTION;' >> $tempdump
sqlite3 state-monimelt.dbsqlite .schema >> $tempdump || exit 1
echo '-- state-monimelt tables contents' >> $tempdump 
sqlite3 state-monimelt.dbsqlite >> $tempdump <<EOF
.mode insert t_params
  SELECT * FROM t_params ORDER BY parname;
.mode insert t_modules
  SELECT * FROM t_modules ORDER BY modname;
.mode insert t_items
  SELECT * FROM t_items ORDER BY itm_idstr;
.mode insert t_names
  SELECT * FROM t_names ORDER BY name;
EOF
echo 'COMMIT;' >> $tempdump
echo "-- state-monimelt end dump " >> $tempdump
if [ -e "$sqlfile" ]; then
    echo -n "backup Sqlite3 dump:" 
    mv -v "$sqlfile" "$sqlfile~"
fi
mv $tempdump "$sqlfile"
ls -l "$sqlfile"
