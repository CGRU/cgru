#!/usr/bin/env bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Setup default Blender location:
# Do not override if BLENDER_LOCATION is already set
if [ -z "$BLENDER_LOCATION" ]; then
	if [ -x "/snap/bin/blender" ]; then
		export BLENDER_LOCATION="/snap/bin"
	else
		export BLENDER_LOCATION="/usr/bin"
	fi
	if [ -x "/snap/blender/current" ]; then
		export BLENDER_LOCATION="/snap/bin"
	fi
fi

export BLENDER_CGRU_PATH="${CGRU_LOCATION}/plugins/blender"

# Do not override if BLENDER_USER_SCRIPTS is already defined
if [ -z "$BLENDER_USER_SCRIPTS" ]; then
	export BLENDER_USER_SCRIPTS="${BLENDER_CGRU_PATH}"
fi

export APP_DIR="${BLENDER_LOCATION}"
export APP_EXE="${BLENDER_LOCATION}/blender"

# Define location:
locate_file="$CGRU_LOCATION/software_setup/locate_blender.sh"
[ -f "${locate_file}" ] && source "${locate_file}"
