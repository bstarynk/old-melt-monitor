#! /bin/sh
## file make-named.sh
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
script=$0
name=$1
shift
type=$1
shift
comment="$*"
if [ -z "$name" ]; then
    echo $script: missing name 1>&2
    exit 1
fi
if [ -z "$type" ]; then
    echo $script: missing type for $name 1>&2
    exit 1
fi
if grep -w -q "mom_${type}_t" *.c *.h; then
    echo $script: unknown type $type for $name 1>&2
    exit 1
fi
uid=$(uuidgen -r)
sqlite3 state-monimelt.dbsqlite <<EOF
 INSERT INTO t_item(uid,type) VALUES(\'$uid\',\'$type\');
 INSERT INTO t_name(name,nuid) VALUES(\'$name\',\'$uid\');
EOF
