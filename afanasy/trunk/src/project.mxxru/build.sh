#!/bin/bash

export UNIXTYPE="LINUX"
export MXX_RU_CPP_TOOLSET=gcc_linux

export AF_PYTHON="python"
export AF_PYTHON_MODULE_EXTRA_LIBS=
# Try "-lrt -lz -lgthread-2.0 -lglib-2.0" on CentOS

[ -f override.sh ] && source override.sh

exec ruby afanasy.mxw.rb $*
