#!/bin/sh -x
# file pre-commit-githook.sh

# This hook is invoked by git commit. It takes no parameter, and is
# invoked before a commit is made.
pwd
./monimelt-dump-state.sh
git add state-monimelt.sql
