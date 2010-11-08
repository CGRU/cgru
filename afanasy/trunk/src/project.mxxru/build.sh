#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

# Set locations:
cgru=$PWD
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`

# Setup Version:
pushd .. > /dev/null
folder=$PWD
cd $cgru
source ./getversion.sh $folder
popd > /dev/null


# Setup Python:
export AF_PYTHON="python"
export AF_PYTHON_MODULE_EXTRA_LIBS=
# Try "-lrt -lz -lgthread-2.0 -lglib-2.0" on CentOS

[ -f override.sh ] && source override.sh

exec ruby afanasy.mxw.rb $*
