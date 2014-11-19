#!/bin/sh -vx
# file install-git-hooks.sh
for f in *-githook.sh; do
    b=$(basename $f -githook.sh)
    ln -sfv ../../$f .git/hooks/$b
done
	 
