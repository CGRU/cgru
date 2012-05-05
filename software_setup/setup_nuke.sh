#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Setup CGRU common scripts if location provided
if [ ! -z "${CGRU_LOCATION}" ]; then
   export NUKE_CGRU_PATH="${CGRU_LOCATION}/plugins/nuke"
   if [ -z "${NUKE_PATH}" ]; then
      export NUKE_PATH="${NUKE_CGRU_PATH}"
   else
      export NUKE_PATH="${NUKE_PATH}:${NUKE_CGRU_PATH}"
   fi
fi

# Set Afanasy scripts location to use
export NUKE_AF_PATH=$AF_ROOT/plugins/nuke
# Add Afanasy scripts to Nuke path
if [ -z "${NUKE_PATH}" ]; then
   export NUKE_PATH="${NUKE_AF_PATH}"
else
   export NUKE_PATH="${NUKE_PATH}:${NUKE_AF_PATH}"
fi

# Default number of threads for rendering:
export NUKE_AF_RENDERTHREADS=2

# Nuke render launcher:
export NUKE_AF_RENDER="nuke -i"

# Path to save 'Untitled' scene to render, if not set 'tmp' name in current folder will be used
# export NUKE_AF_TMPSCENE="compositing/tmp"

if [ "`uname`" == "Darwin" ]; then
    NUKE_INSTALL_DIR="/Applications"
else
    NUKE_INSTALL_DIR="/usr/local"
fi

NUKE_FOLDERS=`ls "$NUKE_INSTALL_DIR"`
NUKE_LOCATION=""
NUKE_EXEC=""
for NUKE_FOLDER in $NUKE_FOLDERS ;
do
   if [ "`echo $NUKE_FOLDER | awk '{print match( \$1, "Nuke")}'`" == "1" ]; then
      NUKE_LOCATION="${NUKE_INSTALL_DIR}/${NUKE_FOLDER}"
      if [ "`uname`" == "Darwin" ]; then
        NUKE_EXEC="${NUKE_FOLDER}.app/${NUKE_FOLDER}"
      else
        NUKE_EXEC="`echo $NUKE_FOLDER | awk '{print substr( \$1, 1, -1+match( \$1, "v.*"))}'`"
      fi
   fi
done
export NUKE_EXEC="${NUKE_LOCATION}/${NUKE_EXEC}"
#echo "NUKE = '${NUKE_EXEC}'"

LM_LICENSE_FILE="`dirname $NUKE_EXEC`/nuke.lic"
if [ -f $LM_LICENSE_FILE ]; then
   export LM_LICENSE_FILE 
   echo "LM_LICENCS_FILE='$LM_LICENSE_FILE'"
fi

# Setup Dailies:
moviemaker=$CGRU_LOCATION/utilities/moviemaker
export CGRU_DAILIES_TEMPLATE="$moviemaker/templates/dailies_withlogo"
export CGRU_DAILIES_SLATE="$moviemaker/templates/dailies_slate"
export CGRU_DAILIES_CODEC="$moviemaker/codecs/photojpg_best.ffmpeg"
export CGRU_DAILIES_LGSPATH="$moviemaker/logos/logo.png"
export CGRU_DAILIES_LGSSIZE="20"
export CGRU_DAILIES_LGSGRAV="SouthEast"
export CGRU_DAILIES_LGFPATH="$moviemaker/logos/logo.png"
export CGRU_DAILIES_LGFSIZE="10"
export CGRU_DAILIES_LGFGRAV="North"
export CGRU_DAILIES_FORMAT="720x576"
export CGRU_DAILIES_FPS="24"
export CGRU_DAILIES_DRAW169="25"
export CGRU_DAILIES_DRAW235="25"
export CGRU_DAILIES_LINE169="200,200,200"
export CGRU_DAILIES_LINE235="200,200,200"
export CGRU_DAILIES_NAMING="(s)_(v)_(d)"


export APP_DIR=$NUKE_LOCATION
export APP_EXE=$NUKE_EXEC

#override nuke location based on locate_nuke.sh
locate_nuke="$CGRU_LOCATION/software_setup/locate_nuke.sh"
[ -f $locate_nuke ] && source $locate_nuke

echo "NUKE = '${APP_EXE}'"
