#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Search directory where Houdini installed:
HOUDINI_INSTALL_DIR="/opt"
for folder in `ls "$HOUDINI_INSTALL_DIR"`; do
	if [ "`echo $folder | awk '{print match( \$1, "hfs")}'`" == "1" ]; then
		export HOUDINI_LOCATION="${HOUDINI_INSTALL_DIR}/${folder}"
	fi
done

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
echo "Houdni location = '$HOUDINI_LOCATION'"

# Source Houdini setup shell script:
pushd $HOUDINI_LOCATION >> /dev/null
source houdini_setup_bash
popd $pwd >> /dev/null

# Setup CGRU houdini scripts location:
export HOUDINI_CGRU_PATH=$CGRU_LOCATION/plugins/houdini

# Set Python path to afanasy submission script:
export PYTHONPATH=$HOUDINI_CGRU_PATH:$PYTHONPATH

# Define OTL scan path:
HOUDINI_CGRU_OTLSCAN_PATH=$HIH/otls:$HOUDINI_CGRU_PATH:$HH/otls

# Create or add to exist OTL scan path:
if [ "$HOUDINI_OTLSCAN_PATH" != "" ]; then
	export HOUDINI_OTLSCAN_PATH="${HOUDINI_CGRU_OTLSCAN_PATH}:${HOUDINI_OTLSCAN_PATH}"
else
	export HOUDINI_OTLSCAN_PATH=$HOUDINI_CGRU_OTLSCAN_PATH
fi

export APP_DIR="$HOUDINI_LOCATION"
export APP_EXE="houdini"


