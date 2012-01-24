#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

export AE_LOCATION="/Applications/Adobe After Effects CS5"

export APP_DIR=$AE_LOCATION

# One is for the AfterEffects itself and the other one for the renderer
export APP_EXE_RENDER="$AE_LOCATION/aerender"
export APP_EXE="$AE_LOCATION/Adobe After Effects CS5.app/Contents/MacOS/After Effects"

# Define location:
export locate_file=$CGRU_LOCATION/software_setup/locate_afterfx.sh

if [ -z "${locate_file}" ]; then
   source "${locate_file}"
fi

export AE_LOCATION=$APP_DIR