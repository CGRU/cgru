#!/bin/bash

name="exrjoin"
output="../../bin/$name"
openexr="../openexr"
openexr_inc="${openexr}/include/OpenEXR"
openexr_lib="${openexr}/lib"

if [ -d $openexr_inc ] ; then
   # Custom include:
   CPPFLAGS="-I$openexr_inc"
else
   # System include:
   CPPFLAGS="-I/usr/include/OpenEXR"
fi

if [ -d $openexr_inc ] ; then
   # Custom libs:
   LDFLAGS="-L/lib64 -L/usr/lib64"
   LDFLAGS="$LDFLAGS -L${openexr_lib}"
   LDFLAGS="$LDFLAGS -L${openexr_lib}64"
   LDFLAGS="$LDFLAGS -Wl,--start-group"
   LDFLAGS="$LDFLAGS -lz"
   LDFLAGS="$LDFLAGS -lpthread"
   LDFLAGS="$LDFLAGS -lIex"
   LDFLAGS="$LDFLAGS -lHalf"
   LDFLAGS="$LDFLAGS -lImath"
   LDFLAGS="$LDFLAGS -lIlmThread"
   LDFLAGS="$LDFLAGS -lIlmImf"
   LDFLAGS="$LDFLAGS -Wl,--end-group"
else
   # System libs:
   LDFLAGS="-lIlmImf"
fi

[ -f override.sh ] && source override.sh

cmd="g++ -c -o ${name}.o -Wall -O2 -DNDEBUG $CPPFLAGS ${name}.cpp"
echo $cmd
$cmd

[ $? != "0" ] && exit 1

cmd="g++ -s -B/usr/lib/gold-ld/ -o ${output} ${name}.o $LDFLAGS"
echo $cmd
$cmd

rm exrjoin.o
