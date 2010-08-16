#!/bin/bash

# Setup Afanasy:
pushd ../.. >> /dev/null
source ./setup_afanasy.sh
popd >> /dev/null

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

NUKE_INSTALL_DIR="/usr/local"
NUKE_FOLDERS=`ls "$NUKE_INSTALL_DIR"`
NUKE_LOCATION=""
NUKE_EXEC=""
for NUKE_FOLDER in $NUKE_FOLDERS ;
do
   if [ "`echo $NUKE_FOLDER | gawk '{print match( \$1, "Nuke")}'`" == "1" ]; then
      NUKE_LOCATION="${NUKE_INSTALL_DIR}/${NUKE_FOLDER}"
      NUKE_EXEC="`echo $NUKE_FOLDER | gawk '{print substr( \$1, 1, -1+match( \$1, "v.*"))}'`"
   fi
done
export NUKE_EXEC="${NUKE_LOCATION}/${NUKE_EXEC}"
echo "NUKE = '${NUKE_EXEC}'"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh
