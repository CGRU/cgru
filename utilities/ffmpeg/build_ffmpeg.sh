#!/bin/bash

x264="${PWD}/x264"
export CFLAGS="-I${x264}"
export LDFLAGS="-L${x264}"
export LDFLAGS="$LDFLAGS -B/usr/lib/gold-ld/"

cd ffmpeg

if [ ! -z "$1" ]; then
   ./configure --help
   exit
else
   ./configure --enable-gpl --enable-libx264
   make
fi

cp -v ffmpeg ../../../bin
