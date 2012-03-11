#!/bin/bash

export UNIXTYPE="MACOSX"
export MXX_RU_CPP_TOOLSET=gcc_darwin

pushd ../../.. >> /dev/null
cgru=$PWD
popd >> /dev/null

#python="$cgru/utilities/python/2.7.1"
qt="$cgru/utilities/qt/4.7.2"

[ -f override.sh ] && source override.sh

export PKG_CONFIG_PATH=$qt/lib/pkgconfig
#export PATH=$qt/bin:$PATH

ruby afanasy.mxw.rb $*
