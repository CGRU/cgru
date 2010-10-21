#!/bin/bash

pushd .. >> /dev/null
source setup.sh
popd >> /dev/null

# to launch commands from current directory:
export AF_CMD_PREFIX="./"

# CGRU for Maya add-ons location, override it, or simple launch from current folder as an example
export MAYA_CGRU_LOCATION=$CGRU_LOCATION/plugins/maya

export MAYA_VERSION=2011
export MAYA_ARCH=-x64
# For 32bit Maya uncomment next line
# export MAYA_ARCH=

# The name of Maya main window menu
export MAYA_CGRU_MENUS_NAME="CGRU"

# automatically load plugins located in MAYA_CGRU_LOCATION/mll/MAYA_VERSION directory
export MAYA_CGRU_PLUG_INS_AUTOLOAD=1

# Locate Maya:
export MAYA_LOCATION=/usr/autodesk/maya${MAYA_VERSION}${MAYA_ARCH}

# Define Maya executabe:
export MAYA_EXEC=${MAYA_LOCATION}/bin/maya${MAYA_VERSION}

# Set more standart (to all distributives) temporary directory:
export TMPDIR=/tmp

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

# Add Maya binary to PATH:
export PATH=${MAYA_LOCATION}/bin:${PATH}

# Add CGRU icons to Maya:
export XBMLANGPATH=${MAYA_CGRU_LOCATION}/icons/%B

# Add CGRU scripts to Maya scripts path:
export MAYA_SCRIPT_PATH=${MAYA_CGRU_LOCATION}/mel/AETemplates:${MAYA_CGRU_LOCATION}/mel/_native/${MAYA_VERSION}:${MAYA_SCRIPT_PATH}
# Add CGRU plugins to Maya plugins path:
export MAYA_PLUG_IN_PATH=${MAYA_CGRU_LOCATION}/mll/${MAYA_VERSION}${MAYA_ARCH}:${MAYA_PLUG_IN_PATH}

# Add Afanasy scripts to Maya:
export MAYA_SCRIPT_PATH=${AF_ROOT}/plugins/maya:${MAYA_SCRIPT_PATH}
