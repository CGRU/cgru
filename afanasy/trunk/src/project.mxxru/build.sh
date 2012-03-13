#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

export AF_PYTHON="python2.6"

if [ -f override.sh ]; then
   source override.sh
fi

exec ruby afanasy.mxw.rb $*
