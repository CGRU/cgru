#!/bin/bash

name="exrjoin"

if [ -d include ] ; then
   # Custom include:
   CPPFLAGS="-I$PWD/include/OpenEXR"
else
   # System include:
   CPPFLAGS="-I/usr/include/OpenEXR"
fi

if [ -d lib ] ; then
   # Custom libs:
   LDFLAGS="-L/lib64 -L/usr/lib64"
   LDFLAGS="$LDFLAGS -L$PWD/lib"
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

cmd="g++ -s -o ${name} ${name}.o $LDFLAGS"
echo $cmd
$cmd

rm exrjoin.o