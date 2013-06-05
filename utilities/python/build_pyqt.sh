#!/bin/bash

pyqt="PyQt*"
sip="$PWD/sip"
export PYTHONPATH="$sip"
pybin="$PWD/3.3.2/bin"
export PATH="$sip:$PATH"
[ -d $pybin ] && export PATH="$pybin:$PATH"

if [ "$1" == "-h" ]; then
   cd $pyqt
   python configure.py -h
   exit 0
fi

flags="-g --confirm-license --no-sip-files"

# Qt:
qtver=$1
if [ -z "$qtver" ]; then
   qtver="4.8.3"
fi
qt=`dirname $PWD`/qt/$qtver
if [ ! -d "$qt" ]; then
   qtver="4.8.4"
   qt=`dirname $PWD`/qt/$qtver
fi
if [ ! -d "$qt" ]; then
   echo "Error: No Qt '$qt' founded."
   exit 1
fi
export PATH=$qt/bin:$PATH

# Python:
python="python"
if [ ! -z `which python3` ]; then
   echo "Using Python 3."
   python="python3"
fi
pythonver=$2
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
   dir="$PWD/pyqt"
   flags="$flags --bindir=$dir"
   flags="$flags --destdir=$dir"
   flags="$flags --plugin-destdir=$dir"
fi

cd $pyqt

flags="$flags -e QtCore -e QtGui -e QtNetwork"

$python configure.py $flags
make
make install
