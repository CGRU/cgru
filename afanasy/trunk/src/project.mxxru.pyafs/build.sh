#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

pythondir=/cg/soft/python
qt=/cg/tools/qt/qt-4.6.3-static

export AF_LFLAGS="-lrt -lz -lgthread -lglib"

export QTDIR=$qt
export PATH=$qt/bin:$PATH

[ -f override.sh ] && source override.sh

syspath=$PATH
for python in `ls "$pythondir"`; do

   [ -d "$pythondir/$python" ] || continue

   echo "Building for Python${python}"

   export PATH=$pythondir/$python/bin:$syspath
   export AF_PYAFVER=$python
   export AF_PYTHON="python"

   ruby afanasy.mxw.rb $*

   output="../../bin_pyaf/$python"
   [ -d $output ] || mkdir -p $output
   cp -v "tmp/$python/pyaf.so" $output
   
done
