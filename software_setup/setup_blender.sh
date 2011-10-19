#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Setup special Python module:
export PYTHONPATH="$AF_ROOT/bin_pyaf/3.2:$PYTHONPATH"

export BLENDER_LOCATION="/opt/blender-2.57b-linux-glibc27-x86_64"
export BLENDER_LOCATION="/usr/bin"

export APP_DIR="${BLENDER_LOCATION}"
export APP_EXE="${BLENDER_LOCATION}/blender"

# Define location:
locate_file="$CGRU_LOCATION/software_setup/locate_blender.sh"
[ -f "${locate_file}" ] && source "${locate_file}"
