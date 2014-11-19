#!/bin/bash
# file pre-commit-githook.sh

# This hook is invoked by git commit. It takes no parameter, and is
# invoked before a commit is made.
pwd
# dump the state database and add it
./monimelt-dump-state.sh
git add state-monimelt.sql
# find the set of newly added and newly removed modules
tmpsql=$(mktemp -t monimelt$$-tempmodules-XXXXXXXX.sql)
tmpout=$(mktemp -t _monimelt$$-tempmodules-XXXXXXXX.out-sh)
trap "rm -v $tmpsql $tmpout" SIGINT SIGTERM EXIT
cat > $tmpsql << EOF
 CREATE TEMPORARY TABLE IF NOT EXISTS _t_filemodules (filmodname VARCHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE);
EOF
for b in modules/momg_*.c ; do
    if [ -f "$b" ]; then
	id=$(echo $(basename $b .c) | sed "s/momg_\([a-z0-9_]*\)/\1/")
	echo INSERT INTO _t_filemodules "VALUES('$id');" >> $tmpsql
    fi
done
cat >> $tmpsql <<EOF 
 SELECT printf("monimelt_remove_modules %s",group_concat(filmodname,' ')) 
 FROM _t_filemodules 
 WHERE NOT EXISTS (SELECT modname 
                   FROM t_modules WHERE modname=_t_filemodules.filmodname);
 SELECT printf("monimelt_add_modules %s",group_concat(modname,' ')) 
 FROM t_modules
 WHERE NOT EXISTS (SELECT filmodname
                   FROM _t_filemodules WHERE filmodname=t_modules.modname);
EOF
echo "DROP TABLE _t_filemodules;" >> $tmpsql
printf "\n\n***** generated SQL temporary file %s *****\n" $tmpsql
cat $tmpsql
printf "################# end of %s ##############\n\n" $tmpsql
###
function monimelt_add_modules() {
    for m in $* ; do
	git add modules/momg_$m.c
    done
}
function monimelt_remove_modules() {
    for m in $* ; do
	git rm -f modules/momg_$m.c
    done
}   
sqlite3 state-monimelt.dbsqlite < $tmpsql > $tmpout
printf "\n\n***** generated temporary commands %s *****\n" $tmpout
cat $tmpout
printf "################# end of %s ##############\n\n" $tmpout
source $tmpout
