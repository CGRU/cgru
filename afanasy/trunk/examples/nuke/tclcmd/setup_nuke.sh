#!/bin/bash

# Setup Afanasy if was not
if [ -z "${AF_ROOT}" ]; then
   pwd=$PWD
   cd ../../..
   source setup.sh
   cd $pwd
fi

# to launch commands from current directory:
export AF_CMD_PREFIX="./"

# Set Afanasy scripts version to use
export NUKE_AF_VERSION="tclcmd"
# Set Afanasy scripts location to use
export NUKE_AF_PATH=$AF_ROOT/plugins/nuke/$NUKE_AF_VERSION
# Add Afanasy scripts to Nuke path
if [ -z "${NUKE_PATH}" ]; then
   export NUKE_PATH="${NUKE_AF_PATH}"
else
   export NUKE_PATH="${NUKE_PATH}:${NUKE_AF_PATH}"
fi

#export NUKE_CGRU_VERSION="tclcmd"
#export NUKE_CGRU_PATH=$AF_ROOT/plugins/nuke/$NUKE_CGRU_VERSION

export NUKE_EXEC="/cg/soft/Nuke6.0/Nuke6.0"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh
