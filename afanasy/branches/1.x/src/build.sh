#!/bin/bash

tmpDir="tmp"

if [ -d $tmpDir ]; then
   rm -rf $tmpDir
fi
mkdir -p $tmpDir

cd tmp

cmake -D CMAKE_INSTALL_PREFIX:PATH=../.. ..

make

make install

cd ..
