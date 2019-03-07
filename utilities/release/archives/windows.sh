#!/bin/bash

# Make release with binaries for MS Windows

dest=$1
[ -d "$dest" ] || exit 1

echo "Clearing bash scripts in '$dest':"

echo "Removing unix shell scripts:"
find "${dest}" -type f -name "*.sh" -exec rm -vf {} \;

echo "Removeing windows service logs:"
find "${dest}/afanasy/service" -type f -name "afservice_*-log-*.txt" -exec rm -vf {} \;

exit 0

