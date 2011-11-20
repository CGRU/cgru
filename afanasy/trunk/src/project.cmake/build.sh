#!/bin/bash

export AF_ADD_CFLAGS=""
export AF_ADD_LFLAGS="-lutil -lpthread"

# export AF_PYTHON_INCLUDE_PATH=C:\Python31\include
# export AF_PYTHON_LIBRARIES=C:\Python31\libs\libpython31.a

pushd .. > /dev/null
folder=$PWD
cd ../../../utilities
source ./getrevision.sh $folder
popd > /dev/null

[ -f override.sh ] && source override.sh

cmake . && make
