#!/bin/bash

export UNIXTYPE="MACOSX"
#export QTDIR="/opt/local/libexec/qt4-mac"

export MXX_RU_CPP_TOOLSET=gcc_darwin
export AF_ARCH=

if [ -f override.sh ]; then
   source override.sh
fi

export PKG_CONFIG_PATH=$QTDIR/lib/pkgconfig
#export PATH=$QTDIR/bin:$PATH

ruby afanasy.mxw.rb $*
