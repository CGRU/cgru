#!/bin/bash

x264="${PWD}/x264"
export CFLAGS="-I${x264}"
export LDFLAGS="-L${x264}"

#dirs="input output filters"
#for dir in $dirs; do
#   objs=`ls $x264/$dir/*.o`
#   for obj in $objs; do
#      export LDFLAGS="$LDFLAGS $obj"
#   done
#done
#echo $LDFLAGS

#exit

cd ffmpeg

if [ ! -z "$1" ]; then
   ./configure --help
   exit
else
   ./configure --enable-gpl --enable-libx264
   make
fi

cp -v ffmpeg ../../../bin
