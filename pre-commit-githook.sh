#!/bin/sh -x
# file pre-commit-githook.sh

# This hook is invoked by git commit. It takes no parameter, and is
# invoked after a commit is made.
./dump-state.sh
