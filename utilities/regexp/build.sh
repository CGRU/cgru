#!/bin/bash

dest="bin"

if [ -d $dest ]; then
   echo "Removing old destination directory '$dest'"
   rm -rf $dest
fi

[ -f override.sh ] && source override.sh

mkdir bin
cd bin
cmake ..
make
cd ..
