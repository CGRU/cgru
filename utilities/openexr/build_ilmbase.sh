#!/bin/bash

prefix=$PWD

export CPPFLAGS="-fPIC"

cd ilmbase-1.0.2

if [ ! -z $1 ] ; then
   ./configure -h; exit
fi

make clean

./configure --prefix=$prefix --exec-prefix=$prefix --enable-shared=

make

make install
