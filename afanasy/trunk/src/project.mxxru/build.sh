#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

export AF_PYTHON="python2.6"

[ -f override.sh ] && source override.sh

exec ruby afanasy.mxw.rb $*
