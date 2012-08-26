#!/bin/bash

prefix=$PWD

CPPFLAGS="-fPIC -I$prefix/include -include string.h"

LDFLAGS="-L/lib64 -L/usr/lib64 -L$prefix/lib -L$prefix/lib64 -lpthread"

export CPPFLAGS
export LDFLAGS

cd openexr*

if [ ! -z $1 ] ; then
   ./configure -h; exit
fi

make clean

./configure --disable-ilmbasetest --prefix=$prefix --exec-prefix=$prefix --enable-shared=

make

make install
