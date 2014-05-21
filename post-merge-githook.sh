#!/bin/sh -x
# file post-merge-githook.sh

# This hook is invoked by git merge, which happens when a git pull is
# done on a local repository.The hook takes a single parameter, a
# status flag specifying whether or not the merge being done was a
# squash merge.
gitstatus=$1

echo post-merge-githook.sh gitstatus $gitstatus
mv -v -f state-monimelt.dbsqlite state-monimelt.dbsqlite~
sqlite3 -batch -bail state-monimelt.dbsqlite < state-monimelt.sql
