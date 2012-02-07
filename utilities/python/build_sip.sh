#!/bin/bash

sip="sip-4.13.1"

if [ "$1" == "-h" ]; then
   cd $sip
   python configure.py -h
   exit 0
fi

python="python"
if [ ! -z `which python3` ]; then
   echo "Using Python 3."
   python="python3"
fi

pythonver=$1
if [ ! -z "$pythonver" ]; then
   pythondir=$PWD/$pythonver
   if [ ! -d "$pythondir" ]; then
      echo "Error: No python '$pythondir' founded."
      exit 1
   fi
   export PATH=$pythondir/bin:$PATH
   if [[ "$pythonver" > "3" ]]; then
      python="python3"
   fi
else
   dir="$PWD/sip"
   flags="$flags --bindir=$dir"
   flags="$flags --destdir=$dir"
   flags="$flags --incdir=$dir"
   flags="$flags --sipdir=$dir"
fi

export CFLAGS=-fPIC
export CPPFLAGS=$CFLAGS

cd $sip

$python configure.py $flags
make
make install
