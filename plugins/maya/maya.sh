#!/bin/bash

# CGRU add-ons for Maya location, override it, or simple launch from current folder as an example
export MAYA_CGRU_LOCATION=$PWD

# Set Maya version, to get it default location and setup CGRU scripts path
export MAYA_VERSION=2014

# Set Maya location
export MAYA_LOCATION=/usr/autodesk/maya${MAYA_VERSION}${MAYA_ARCH}
# Set Maya executable
export MAYA_EXEC=${MAYA_LOCATION}/bin/maya${MAYA_VERSION}

# The name of Maya main window menu
export MAYA_CGRU_MENUS_NAME="CGRU"

# path to modelling, animation, rendering ... menu items
# if not set default MAYA_CGRU_LOCATION/mel will be used
#export MAYA_CGRU_MENUS_LOCATION="/cg/tools/maya/settings"

# Temporary directory, by default maya uses '/usr/tmp' which may not exists
export TMPDIR=/tmp

# Overrides (set custom variables values there):
[ -f override.sh ] && source override.sh

# Add Maya binaries to system path:
export PATH=${MAYA_LOCATION}/bin:${PATH}

# Add CGRU icons to Maya:
export XBMLANGPATH=${MAYA_CGRU_LOCATION}/icons/%B

# Add CGRU scripts to Maya scripts path:
export MAYA_SCRIPT_PATH=${MAYA_CGRU_LOCATION}/mel/AETemplates:${MAYA_SCRIPT_PATH}
# Add CGRU plugins to Maya plugins path:
export MAYA_PLUG_IN_PATH=${MAYA_CGRU_LOCATION}/mll/${MAYA_VERSION}:${MAYA_PLUG_IN_PATH}

# Enable Afanasy if it is set:
if [ ! -z "${AF_ROOT}" ]; then
	echo "Afanasy found: '${AF_ROOT}'"
	export MAYA_SCRIPT_PATH="${MAYA_CGRU_LOCATION}/afanasy:${MAYA_SCRIPT_PATH}"
fi

# Run Maya:
${MAYA_EXEC} $*
