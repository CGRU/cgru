#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

pythondir=/cg/soft/python
qt=/cg/data/tools/qt/qt-everywhere-opensource-src-4.6.3-static
export QTDIR=$qt
export PATH=$qt/bin:$PATH

[ -f override.sh ] && source override.sh

syspath=$PATH
for python in `ls "$pythondir"`; do

   [ -d "$pythondir/$python" ] || continue

   export PATH=$pythondir/$python/bin:$syspath
   export AF_PYAFVER=$python
   export AF_PYTHON="python"

   exec ruby afanasy.mxw.rb $*

done
