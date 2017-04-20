#!/bin/bash

# Make release with binaries for MS Windows

dest=$1
[ -d "$dest" ] || exit 1

echo "Clearing bash scripts in '$dest':"
find "${dest}" -type f -name "*.sh" -exec rm -vf {} \;
exit 0

