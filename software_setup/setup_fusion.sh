#!/usr/bin/env bash

#
# Source general for all soft directives:
#
source "$CGRU_LOCATION/software_setup/setup__all.sh"

#
# Search for Fusion installation:
#
if [ "`uname`" == "Darwin" ]; then
    FUSION_INSTALL_DIR="/Applications/BlackmagicDesign"
else
    FUSION_INSTALL_DIR="/opt/BlackmagicDesign"
fi

if [ -z "$FUSION_LOCATION" ]; then
  FUSION_FOLDERS=`ls "$FUSION_INSTALL_DIR"`
  FUSION_LOCATION=""
  FUSION_EXEC=""
  for FUSION_FOLDER in $FUSION_FOLDERS ;
  do
     if [ "`echo $FUSION_FOLDER | awk '{print match( \$1, "Fusion[0-9]+")}'`" == "1" ]; then
        FUSION_LOCATION="${FUSION_INSTALL_DIR}/${FUSION_FOLDER}"
        if [ "`uname`" == "Darwin" ]; then
          FUSION_EXEC="${FUSION_FOLDER}.app/${FUSION_FOLDER}"
        else
          FUSION_EXEC="Fusion"
        fi
     fi
     # use the ``Fusion`` as the render executable by default
     export FUSION_RENDERNODE_LOCATION="${FUSION_LOCATION}"
     FUSION_RENDERNODE_EXEC="${FUSION_EXEC}"
     if [ "`echo $FUSION_FOLDER | awk '{print match( \$1, "FusionRenderNode[0-9]+")}'`" == "1" ]; then
        FUSION_RENDERNODE_LOCATION="${FUSION_INSTALL_DIR}/${FUSION_FOLDER}"
        if [ "`uname`" == "Darwin" ]; then
          FUSION_RENDERNODE_EXEC="${FUSION_FOLDER}.app/${FUSION_FOLDER}"
        else
          FUSION_RENDERNODE_EXEC="FusionRenderNode"
        fi
     fi
  done
  export FUSION_LOCATION
  export FUSION_EXEC="${FUSION_LOCATION}/${FUSION_EXEC}"
  export FUSION_RENDERNODE_EXEC="${FUSION_RENDERNODE_LOCATION}/${FUSION_RENDERNODE_EXEC}"
else
  echo "FUSION_LOCATION is already set: ${FUSION_LOCATION}"
  # FUSION_LOCATION is already set by an external process (ex: Rez)
  # The user also needs to setup the following environment variables for a complete setup
  # $FUSION_EXEC
  # $FUSION_RENDERNODE_LOCATION
  # $FUSION_RENDERNODE_EXEC
fi

#
# For export:
#
export APP_DIR=$FUSION_LOCATION
export APP_EXE=$FUSION_EXEC
export RENDER_DIR=$FUSION_RENDERNODE_LOCATION
export RENDER_EXE=$FUSION_RENDERNODE_EXEC

#
# Override fusion location based on locate_fusion.sh:
#
locate_fusion="$CGRU_LOCATION/software_setup/locate_fusion.sh"
[ -f $locate_fusion ] && source $locate_fusion

echo "FUSION = '${APP_EXE}'"
