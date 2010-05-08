#!/bin/bash

# Setup Afanasy if was not
if [ -z "${AF_ROOT}" ]; then
   pwd=$PWD
   cd ../..
   source setup.sh
   cd $pwd
fi

# Setup CGRU common scripts if location provided
if [ ! -z "${CGRU_LOCATION}" ]; then
   export NUKE_CGRU_PATH="${CGRU_LOCATION}/plugins/nuke"
   if [ -z "${NUKE_PATH}" ]; then
      export NUKE_PATH="${NUKE_CGRU_PATH}"
   else
      export NUKE_PATH="${NUKE_PATH}:${NUKE_CGRU_PATH}"
   fi
   pwd=$PWD
   cd "${CGRU_LOCATION}"
   source setup.sh
   cd $pwd
fi

# Set Afanasy scripts version to use
export NUKE_AF_VERSION="python"
# Set Afanasy scripts location to use
export NUKE_AF_PATH=$AF_ROOT/plugins/nuke/$NUKE_AF_VERSION
# Add Afanasy scripts to Nuke path
if [ -z "${NUKE_PATH}" ]; then
   export NUKE_PATH="${NUKE_AF_PATH}"
else
   export NUKE_PATH="${NUKE_PATH}:${NUKE_AF_PATH}"
fi

# Default number of threads for rendering:
export NUKE_AF_RENDERTHREADS=2

# Nuke render launcher:
export NUKE_AF_RENDER="nuke -i -m %(threads)s"
if [ ! -z "${CGRU_LOCATION}" ]; then
   export NUKE_AF_RENDER="${NUKE_AF_RENDER} -t ${CGRU_LOCATION}/plugins/nuke/render.py"
fi

# Path to save 'Untitled' scene to render, if not set 'tmp' name in current folder will be used
# export NUKE_AF_TMPSCENE="compositing/tmp"

NUKE_LOCATION="/cg/soft/Nuke6.0"
export NUKE_EXEC="${NUKE_LOCATION}/Nuke6.0"

# to launch commands from current directory:
export AF_CMD_PREFIX="./"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh
