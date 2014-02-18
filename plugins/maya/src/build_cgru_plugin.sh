#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

function build_cgru(){
	if [ -z $MAYA_LOCATION ]; then
		export MAYA_LOCATION=/usr/autodesk/maya${MAYA_VERSION}-x64
	fi
	echo "Building CGRU for Maya $MAYA_VERSION ..."
	ruby cgru_plugin.mxw.rb $*
}

export MAYA_VERSION=2014

[ -f override.sh ] && source override.sh

build_cgru $*
