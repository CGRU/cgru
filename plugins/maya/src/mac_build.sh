#!/bin/bash

export UNIXTYPE="MACOSX"
export MXX_RU_CPP_TOOLSET=gcc_darwin

function build_cgru(){
   export MAYA_LOCATION=/Applications/Autodesk/maya$MAYA_VERSION
   echo "Building CGRU for Maya $MAYA_VERSION$MAYA_ARCH ..."
   ruby cgru_plugin.mxw.rb $*
}

# maya 2009:
export MAYA_VERSION=2009
#export MAYA_ARCH=-x64
export MAYA_ARCH=

if [ -f override.sh ]; then
   source override.sh
fi

build_cgru $*
