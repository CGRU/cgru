#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

# Set locations:
cgru=$PWD
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru_utils=$cgru/utilities
pythondir=$cgru_utils/python

# Setup Version:
pushd .. > /dev/null
folder=$PWD
cd $cgru/utilities
source ./getrevision.sh $folder
popd > /dev/null

export AF_LFLAGS="-lrt -lz -lgthread -lglib"
# on CentOS 5 with Python2.5 try "-lrt -lz -lgthread-2.0 -lglib-2.0"

[ -f override.sh ] && source override.sh

syspath=$PATH
for python in `ls "$pythondir"`; do
   # Skip not folders:
   [ -d "$pythondir/$python" ] || continue
   # Skip folder name does not starts with a digit and a dot:
   [ "`echo $python | awk '{print match( \$1, "[0-9]\\\.")}'`" == "1" ] || continue

   echo "Building for Python${python}"

   export PATH=$pythondir/$python/bin:$syspath
   export AF_PYAFVER=$python
   export AF_PYTHON="python"

   ruby afanasy.mxw.rb $*

   output="../../bin_pyaf/$python"
   [ -d $output ] || mkdir -p $output
   cp -v "tmp/$python/pyaf.so" $output

done
