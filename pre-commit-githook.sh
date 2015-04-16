#!/bin/bash
# file pre-commit-githook.sh

# This hook is invoked by git commit. It takes no parameter, and is
# invoked before a commit is made.
pwd
make -j 4
