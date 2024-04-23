#!/usr/bin/env bash

echo "Clearing Houdini examples in '$PWD'"

rm -fv *.hip*.hip

rm -fv *.mov

# Remove all folders:

find -mindepth 1 -maxdepth 1 -type d -prune -exec rm -rvf {} \;
