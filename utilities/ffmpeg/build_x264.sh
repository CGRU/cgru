#!/bin/bash

yasm_ver=1.2.0
yasm_folder=$PWD/yasm-$yasm_ver
if [ -d $yasm_folder ]; then
   echo "Using $yasm_folder"
   export PATH="$yasm_folder:$PATH"
fi

prefix=$PWD

cd x264

if [ ! -z "$1" ]; then
   ./configure -h
   exit
fi

./configure --enable-static --prefix=$prefix --exec-prefix=$prefix
make
#make install
