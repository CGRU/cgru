#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

function build_cgru(){
   if [ -z $MAYA_LOCATION ]; then
      export MAYA_LOCATION=/usr/autodesk/maya$MAYA_VERSION$MAYA_ARCH
   fi
   echo "Building CGRU for Maya $MAYA_VERSION$MAYA_ARCH ..."
   ruby cgru_plugin.mxw.rb $*
}

export MAYA_VERSION=2013
export MAYA_ARCH=-x64

[ -f override.sh ] && source override.sh

build_cgru $*
