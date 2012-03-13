#!/bin/bash

# Setup Afanasy if was not
if [ -z $AF_ROOT ]; then
   pwd=$PWD
   cd ../..
   source setup.sh
   cd $pwd
fi

# Set Afanasy scripts version to use:
export NUKE_CGRU_VERSION="2.7"

# Set Afanasy scripts location to use:
export NUKE_CGRU_PATH=$AF_ROOT/plugins/nuke/$NUKE_CGRU_VERSION

# Use custom render launcher
# export NUKE_CGRU_USETMPIMG=1
# Directory to save temporary scene to render, if not set current folder used
# export NUKE_CGRU_TMPSCENEDIR="/compositing/.tmp"

export NUKE_PATH=$NUKE_CGRU_PATH

export NUKE_EXEC="/cg/soft/Nuke5.2/Nuke5.2"

# to launch commands from current directory:
export AF_CMD_PREFIX="./"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh
