#!/usr/bin/env bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Search directory where Houdini installed:
HOUDINI_INSTALL_DIR="/opt"
if [ -z "$HOUDINI_LOCATION" ]; then
	for folder in `ls "$HOUDINI_INSTALL_DIR"`; do
		if [ "`echo $folder | awk '{print match( \$1, "hfs")}'`" == "1" ]; then
			export HOUDINI_LOCATION="${HOUDINI_INSTALL_DIR}/${folder}"
		fi
	done
else
	echo "HOUDINI_LOCATION is already set: ${HOUDINI_LOCATION}"
fi

#Override houdini location based on locate_houdini.sh
locate_houdini="$CGRU_LOCATION/software_setup/locate_houdini.sh"
if [ -f $locate_houdini ]; then
	source $locate_houdini
	HOUDINI_LOCATION="$APP_DIR"
fi

# Check Houdini location:
if [ -z "$HOUDINI_LOCATION" ]; then
	echo "Can't find houdini in '$HOUDINI_INSTALL_DIR'"
	exit 1
fi
echo "Houdini location = '$HOUDINI_LOCATION'"

# Source Houdini setup shell script:
pushd $HOUDINI_LOCATION >> /dev/null
source houdini_setup_bash
popd $pwd >> /dev/null

# Setup CGRU houdini plugins location:
export HOUDINI_CGRU_PATH=$CGRU_LOCATION/plugins/houdini

# Create or add to HOUDINI_PATH to include CGRU houdini plugins:
if [ "$HOUDINI_PATH" != "" ]; then
	export HOUDINI_PATH="${HOUDINI_CGRU_PATH}:${HOUDINI_PATH}:&"
else
	export HOUDINI_PATH="${HOUDINI_CGRU_PATH}:&"
fi

# Set Python path to afanasy submission script:
export PYTHONPATH=$HOUDINI_CGRU_PATH:$PYTHONPATH

export APP_DIR="$HOUDINI_LOCATION"
export APP_EXE="houdini"


