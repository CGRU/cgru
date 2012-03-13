#!/bin/bash

# Setup Afanasy if was not
if [ -z $AF_ROOT ]; then
   pwd=$PWD
   cd ../../..
   source setup.sh
   cd $pwd
fi

# to launch commands from current directory:
export AF_CMD_PREFIX="./"

export NUKE_CGRU_VERSION="tclcmd"
export NUKE_CGRU_PATH=$AF_ROOT/plugins/nuke/$NUKE_CGRU_VERSION

export NUKE_EXEC="/cg/soft/Nuke4.7/Nuke4.7"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh
