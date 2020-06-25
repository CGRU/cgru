#!/bin/bash

nasm_ver=2.13.01
nasm_folder=$PWD/nasm-$nasm_ver
if [ -d $nasm_folder ]; then
   echo "Using $nasm_folder"
   export PATH="$nasm_folder:$PATH"
fi

prefix=$PWD

cd x264

if [ ! -z "$1" ]; then
   ./configure -h
   exit
fi

./configure --enable-static --prefix=$prefix --exec-prefix=$prefix
make $@
#make install
