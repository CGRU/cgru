#!/usr/bin/env bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# CGRU for Maya add-ons location, override it, or simple launch from current folder as an example
export MAYA_CGRU_LOCATION="$CGRU_LOCATION/plugins/maya"
export PYTHONPATH="${MAYA_CGRU_LOCATION}:${PYTHONPATH}"

# Locate Maya:
MAYA_EXEC=""

if [ -z "$MAYA_LOCATION" ]; then
  MAYA_INSTALL_DIR="/usr/autodesk"
  MAYA_FOLDERS=`ls "$MAYA_INSTALL_DIR"`
  MAYA_LOCATION=""
  MAYA_VERSION=""
  for MAYA_FOLDER in $MAYA_FOLDERS ;
  do
    if [ "`echo $MAYA_FOLDER | gawk '{print match( \$1, /maya[0-9]+.*/)}'`" == "1" ]; then
      MAYA_LOCATION="${MAYA_INSTALL_DIR}/${MAYA_FOLDER}"
      MAYA_VERSION="`echo $MAYA_FOLDER | gawk '{print substr( \$1, 5, 4)}'`"
    fi
  done
else
  echo "MAYA_LOCATION is already set: ${MAYA_LOCATION}"
fi

export MAYA_EXEC="${MAYA_LOCATION}/bin/maya${MAYA_VERSION}"
echo "MAYA: ${MAYA_EXEC}"
echo "MAYA_VERSION: ${MAYA_VERSION}"
export MAYA_LOCATION
export MAYA_EXEC
export MAYA_VERSION

# The name of Maya main window menu
export MAYA_CGRU_MENUS_NAME="CGRU"

# Set more standard (to all distributives) temporary directory:
export TMPDIR=/tmp

# overrides (set custom values there):
[ -f override.sh ] && source override.sh
[ -f override_maya.sh ] && source override_maya.sh

# Add Maya binary to PATH:
export PATH="${MAYA_LOCATION}/bin:${PATH}"

# Add CGRU icons to Maya:
export XBMLANGPATH="${MAYA_CGRU_LOCATION}/icons/%B:${XBMLANGPATH}"

# Add CGRU scripts to Maya scripts path:
export MAYA_SCRIPT_PATH="${MAYA_CGRU_LOCATION}/mel/AETemplates:${MAYA_SCRIPT_PATH}"
# Add CGRU plugins to Maya plugins path:
export MAYA_PLUG_IN_PATH="${MAYA_CGRU_LOCATION}/mll/${MAYA_VERSION}:${MAYA_PLUG_IN_PATH}"

# Add Afanasy scripts to Maya:
export MAYA_SCRIPT_PATH="${MAYA_CGRU_LOCATION}/afanasy:${MAYA_SCRIPT_PATH}"

# Disable the Customer Improvement Program
export MAYA_DISABLE_CIP=1
# https://discourse.techart.online/t/maya-reducing-maya-shutdown-time-by-disabling-autodesk-cip/4951
# Disable the Customer Error Reporting
export MAYA_DISABLE_CER=1

export APP_DIR="${MAYA_LOCATION}"
export APP_EXE="${MAYA_EXEC}"
