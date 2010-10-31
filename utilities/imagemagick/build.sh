#!/bin/bash

im=$PWD/ImageMagick

exr=`dirname $PWD`/openexr

prefix=$PWD
export CC="g++"
export OPENEXR_CFLAGS="-I$exr/include/OpenEXR"
export OPENEXR_LIBS="-L$exr/lib -lIlmImf -lz -lImath -lHalf -lIex -lIlmThread -lpthread"

cd $im

if [ -z "$1" ]; then
   make clean
   #with-quantum-depth=32
   ./configure --prefix=$prefix --enable-hdri --enable-shared=
   make
   make install
else
   ./configure -h
fi
