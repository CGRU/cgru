#!/bin/bash

v=6.6.7

im=$PWD/ImageMagick-$v

exr=`dirname $PWD`/openexr

prefix=$PWD
export CC="g++"
export OPENEXR_CFLAGS="-I$exr/include/OpenEXR"
export OPENEXR_LIBS="-L$exr/lib -lIlmImf -lz -lImath -lHalf -lIex -lIlmThread -lpthread"

cd $im

configure="configure"
configure_cgru="$configure.cgru"
if [ ! -f "$configure_cgru" ] ; then
   echo "Processing $configure"
   mv -vf $configure $configure_cgru
   sed \
   -e "s:OPENEXR_CFLAGS="":#OPENEXR_CFLAGS="":g" \
   -e "s:OPENEXR_LIBS="":#OPENEXR_LIBS="":g" \
   < "${configure_cgru}" > "${configure}"
fi

if [ -z "$1" ]; then
   make clean
   #with-quantum-depth=32
   ./configure --prefix=$prefix --with-openexr --enable-hdri --enable-shared=
   make
   make install
else
   ./configure -h
fi
