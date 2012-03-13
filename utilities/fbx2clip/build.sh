#!/bin/bash

dest="bin"

if [ -d $dest ]; then
   echo "Removing old destination directory '$dest'"
   rm -rf $dest
fi

export FBXSDK_PATH="$PWD/fbxsdk"

[ -f override.sh ] && source override.sh

echo "FBXSDK_PATH='${FBXSDK_PATH}'"
if [ ! -d $FBXSDK_PATH ]; then
   echo 'ERROR: FBXSDK not founded!'
   exit 1
fi

mkdir bin
cd bin
cmake ..
make
cd ..
