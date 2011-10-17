#!/bin/bash

prefix=$PWD

cd x264

if [ ! -z "$1" ]; then
   ./configure -h
   exit
fi

./configure --enable-static --prefix=$prefix --exec-prefix=$prefix
make
#make install
