#!/bin/bash

ver=2.13.01
folder=nasm-$ver
arch=$folder.tar.gz

[ -d $folder ] && rm -rvf folder

if [ ! -f $arch ]; then
   wget http://www.nasm.us/pub/nasm/releasebuilds/$ver/$arch
fi

tar xvfz $arch
