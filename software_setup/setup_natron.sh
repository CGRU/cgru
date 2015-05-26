#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Setup CGRU common scripts if location provided
if [ ! -z "${CGRU_LOCATION}" ]; then
	export NATRON_CGRU_PATH="${CGRU_LOCATION}/plugins/natron"
	if [ -z "${NATRON_PATH}" ]; then
		export NATRON_PATH="${NATRON_CGRU_PATH}"
	else
		export NATRON_PATH="${NATRON_PATH}:${NATRON_CGRU_PATH}"
	fi
fi

# Nuke render launcher:
export NATRON_AF_RENDER="natron -b"

# Path to save 'Untitled' scene to render, if not set 'tmp' name in current folder will be used
# export NATRON_AF_TMPSCENE="compositing/tmp"

if [ "`uname`" == "Darwin" ]; then
	NATRON_INSTALL_DIR="/Applications"
else
	NATRON_INSTALL_DIR="/opt"
fi

NATRON_LOCATION="${NATRON_INSTALL_DIR}/Natron"
NATRON_EXEC="Natron"

export NATRON_EXEC="${NATRON_LOCATION}/${NATRON_EXEC}"

export APP_DIR=$NATRON_LOCATION
export APP_EXE=$NATRON_EXEC

#override natron location based on locate_natron.sh
locate_natron="$CGRU_LOCATION/software_setup/locate_natron.sh"
[ -f $locate_natron ] && source $locate_natron

echo "NATRON = '${APP_EXE}'"
