#!/bin/bash

pushd .. >> /dev/null
source setup.sh
popd >> /dev/null

# to launch commands from current directory:
export AF_CMD_PREFIX="./"

export MAYA_VERSION=2011
export MAYA_ARCH=-x64
# For 32bit Maya uncomment next line
# export MAYA_ARCH=

export MAYA_LOCATION=/usr/autodesk/maya${MAYA_VERSION}${MAYA_ARCH}

export MAYA_EXEC=${MAYA_LOCATION}/bin/maya${MAYA_VERSION}

export TMPDIR=/tmp

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

export PATH=${MAYA_LOCATION}/bin:${PATH}

export MAYA_SCRIPT_PATH=${AF_ROOT}/plugins/maya:${MAYA_SCRIPT_PATH}
