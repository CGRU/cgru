#!/bin/bash

prefix=$PWD

export CPPFLAGS="-fPIC"

cd ilmbase*

if [ ! -z $1 ] ; then
   ./configure -h; exit
fi

make clean

./configure --prefix=$prefix --exec-prefix=$prefix --enable-shared=

make

make install
