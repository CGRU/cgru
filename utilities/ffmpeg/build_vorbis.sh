#!/bin/bash

libogg=$PWD/libogg
cd libogg-*
export LDFLAGS="-B/usr/lib/gold-ld/"
if [ ! -z "$1" ]; then
   ./configure --help
else
   ./configure --prefix=$libogg --enable-shared=
   make && make install
fi

cd ..

libvorbis=$PWD/libvorbis
cd libvorbis-*
export CFLAGS="-B/usr/lib/gold-ld/ -I$libogg/include"
export LDFLAGS="-B/usr/lib/gold-ld/ -L$libogg/lib -L$libogg/lib64"
if [ ! -z "$1" ]; then
   ./configure --help
else
   ./configure --prefix=$libvorbis --enable-shared=
   make && make install
fi
