#!/bin/bash

dest="bin"

if [ -d $dest ]; then
   echo "Removing old destination directory '$dest'"
   rm -rf $dest
fi

mkdir bin
cd bin
cmake ..
make
cd ..
