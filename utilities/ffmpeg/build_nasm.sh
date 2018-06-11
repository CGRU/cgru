#!/bin/bash

ver=2.13.01
folder=nasm-$ver
arch=$folder.tar.gz
prefix=$PWD/nasm

cd $folder
./configure --prefix=$prefix
make && make install

