#!/bin/bash

pythonver=$1
[ -z "$pythonver" ] && pythonver=2.7.1

pythonsrc=Python-$pythonver
if [ ! -d "$pythonsrc" ]; then
   echo "Error: No python sources '$pythonsrc' founded."
   exit 1
fi

pythondir=$PWD/$pythonver

export CFLAGS=-fPIC
export CPPFLAGS=$CFLAGS

cd $pythonsrc

if [ ! -z "$2" ]; then
   ./configure -h
else
   ./configure --prefix=$pythondir --exec-prefix=$pythondir
   make
   make install
fi
