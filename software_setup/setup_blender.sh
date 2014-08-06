#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Setup default Blender location:
export BLENDER_LOCATION="/usr/bin"

export BLENDER_CGRU_PATH="${CGRU_LOCATION}/plugins/blender"

export BLENDER_USER_SCRIPTS="${BLENDER_CGRU_PATH}"

export APP_DIR="${BLENDER_LOCATION}"
export APP_EXE="${BLENDER_LOCATION}/blender"

# Define location:
locate_file="$CGRU_LOCATION/software_setup/locate_blender.sh"
[ -f "${locate_file}" ] && source "${locate_file}"
