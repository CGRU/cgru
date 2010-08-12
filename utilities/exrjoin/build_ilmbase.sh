#!/bin/bash

prefix=$PWD

cd ilmbase-1.0.1

if [ ! -z $1 ] ; then
   ./configure -h; exit
fi

./configure --prefix=$prefix --exec-prefix=$prefix --enable-shared=

make

make install
