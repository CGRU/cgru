#!/bin/bash

pythonver=$1
if [ -z "$pythonver" ]; then
   echo "Error: No python version specified."
   echo "Usage: $0 [python version]"
   exit 1
fi

pythondir=$PWD/$pythonver
if [ ! -d "$pythondir" ]; then
   echo "Error: No python '$pythondir' founded."
   exit 1
fi

qt=`dirname $PWD`/qt/qt-4.7.1

export PATH=$pythondir/bin:$qt/bin:$PATH

cd PyQt-x11-gpl-4.8.3

if [ ! -z "$2" ]; then
   python configure.py -h
else
   python configure.py -g --confirm-license
   make
   make install
fi
