#!/bin/bash

im=$PWD/ImageMagick

exr=`dirname $PWD`/openexr

prefix=$PWD

export OPENEXR_CFLAGS="-I$exr/include/OpenEXR"
export OPENEXR_LIBS="-L$exr/lib -L$exr/lib64 -lIlmImf -lz -lImath -lHalf -lIex -lIlmThread -lpthread"
export LDFLAGS="-B/usr/lib/gold-ld/ -lstdc++"

cd $im

configure="configure"
configure_cgru="$configure.cgru"
if [ ! -f "$configure_cgru" ] ; then
   echo "Processing $configure"
   mv -vf $configure $configure_cgru
   sed \
   -e "s:OPENEXR_CFLAGS=\"\":#OPENEXR_CFLAGS=\"\":g" \
   -e "s:OPENEXR_LIBS=\"\":#OPENEXR_LIBS=\"\":g" \
   < "${configure_cgru}" > "${configure}"
   chmod a+rwx $configure
fi

configure="./configure"
configure="$configure --prefix=$prefix"
configure="$configure --with-openexr"
# configure="$configure --enable-hdri"
configure="$configure --disable-opencl"
configure="$configure --enable-shared="
#with-quantum-depth=32

if [ -z "$1" ]; then
   #make clean
   $configure
   make
   convert='utilities/convert'
   [ -f $convert ] && mv -vf $convert ../../../bin
else
   ./configure -h
fi
