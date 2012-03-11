#!/bin/bash

ver=1.2.0
folder=yasm-$ver
arch=$folder.tar.gz
prefix=$PWD/yasm

cd $folder
./configure --prefix=$prefix
make && make install
