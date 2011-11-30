#!/bin/bash

x264="${PWD}/x264"
lame="${PWD}/lame"
export CFLAGS="-I${x264} -I$lame/include"
export LDFLAGS="-L${x264} -L$lame/lib"
export LDFLAGS="$LDFLAGS -B/usr/lib/gold-ld/"

cd ffmpeg

if [ ! -z "$1" ]; then
   ./configure --help
   exit
else
   ./configure --enable-gpl --enable-libx264 --enable-libmp3lame
   make
fi

cp -v ffmpeg ../../../bin
