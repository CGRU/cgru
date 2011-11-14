#!/bin/bash

ver=1.2.0
folder=yasm-$ver
arch=$folder.tar.gz

cd $folder
./configure
make
