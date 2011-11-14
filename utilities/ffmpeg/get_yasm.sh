#!/bin/bash

ver=1.2.0
folder=yasm-$ver
arch=$folder.tar.gz

[ -d $folder ] && rm -rvf folder

if [ ! -f $arch ]; then
   wget http://www.tortall.net/projects/yasm/releases/$arch
fi

tar xvfz $arch
