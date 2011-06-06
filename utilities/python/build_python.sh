#!/bin/bash

pythonver=$1
[ -z "$pythonver" ] && pythonver=2.7.1

[ "$2" != "-h" ] && options=$2

pythonsrc=Python-$pythonver$options
if [ ! -d "$pythonsrc" ]; then
   echo "Error: No python sources '$pythonsrc' founded."
   exit 1
fi

pythondir=$PWD/$pythonver$options

export CFLAGS=-fPIC
export CPPFLAGS=$CFLAGS

if [ `uname` == "Darwin" ]; then
   extra="--enable-framework"
   echo "Darwin framework enabled."
fi

cd $pythonsrc

flags="$flags --prefix=$pythondir"
flags="$flags --exec-prefix=$pythondir"
if [ "$options" != "-utf16" ]; then
   if [[ "$pythonver" > "3" ]]; then
      flags="$flags --with-wide-unicode"
   else
      flags="$flags --enable-unicode=ucs4"
   fi
fi
flags="$flags $extra"

if [ "$2" == "-h" ]; then
   ./configure -h
   echo "FLAGS = \"$flags\""
else
   echo "FLAGS = \"$flags\""
   ./configure $flags
   make
   make install
fi
