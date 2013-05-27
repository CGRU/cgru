#!/bin/bash

yasm="${PWD}/yasm"
faac="${PWD}/faac"
x264="${PWD}/x264"
lame="${PWD}/lame"
ogg="${PWD}/libogg"
vorbis="${PWD}/libvorbis"
export PATH="$yasm/bin:$PATH"
export CFLAGS="-I${x264} -I$lame/include -I$ogg/include -I$vorbis/include -I$faac/include"
export LDFLAGS="-L${x264} -L$lame/lib -L$lame/lib64 -L$ogg/lib -L$ogg/lib64 -L$vorbis/lib -L$vorbis/lib64 -L$faac/lib -L$faac/lib64"
export LDFLAGS="$LDFLAGS -ldl"
export LDFLAGS="$LDFLAGS -B/usr/lib/gold-ld/"

cd ffmpeg

if [ ! -z "$1" ]; then
   ./configure --help
   exit
else
   ./configure --enable-gpl --enable-nonfree --enable-libx264 --enable-libmp3lame --enable-libvorbis --enable-libfaac
   make
fi

cp -v ffmpeg ../../../bin
