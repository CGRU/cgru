#!/usr/bin/env bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# If job/Keeper already pinned BLENDER_LOCATION, keep it (do not run locate_*).
BLENDER_FROM_ENV=0
if [ -n "$BLENDER_LOCATION" ]; then
	BLENDER_FROM_ENV=1
	echo "BLENDER_LOCATION from environment: ${BLENDER_LOCATION}"
fi

# Setup default Blender location:
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

# Keeper locate_* only when env did not pin a build
if [ "$BLENDER_FROM_ENV" = "0" ]; then
	locate_file="$CGRU_LOCATION/software_setup/locate_blender.sh"
	[ -f "${locate_file}" ] && source "${locate_file}"
fi
