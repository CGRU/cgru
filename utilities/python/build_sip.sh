#!/bin/bash

pythonver=$1
if [ -z "$pythonver" ]; then
   pythonver="2.7.2"
fi

pythondir=$PWD/$pythonver
if [ ! -d "$pythondir" ]; then
   echo "Error: No python '$pythondir' founded."
   exit 1
fi

export PATH=$pythondir/bin:$PATH

export CFLAGS=-fPIC
export CPPFLAGS=$CFLAGS

cd sip-4.12.4

if [ ! -z "$2" ]; then
   python configure.py -h
else
   python configure.py
   make
   make install
fi
