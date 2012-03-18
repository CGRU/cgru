#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux
if [ `uname` == "Darwin" ]; then
   echo "Building on Mac OS X:"
   export UNIXTYPE="MACOSX"
   export MXX_RU_CPP_TOOLSET=gcc_darwin
fi

# Set locations:
cgru=$PWD
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`

# Setup Version:
pushd .. > /dev/null
cd $cgru/utilities
source ./getrevision.sh $cgru
popd > /dev/null


# Setup Python:
export AF_PYTHON="python"
export AF_PYTHON_MODULE_EXTRA_LIBS=
# Try "-lrt -lz -lgthread-2.0 -lglib-2.0" on CentOS

[ -f override.sh ] && source override.sh

ruby afanasy.mxw.rb $*

libdir="../../lib"
[ -d $libdir ] || mkdir -pv $libdir
binpyaf="../../bin/pyaf.so"
[ -f $binpyaf ] && mv -fv $binpyaf $libdir/pyaf.so
