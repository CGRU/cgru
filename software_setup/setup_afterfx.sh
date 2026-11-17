#!/usr/bin/env bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# If job/Keeper already pinned AE_LOCATION, keep it (do not run locate_*).
AE_FROM_ENV=0
if [ -n "$AE_LOCATION" ]; then
   AE_FROM_ENV=1
   echo "AE_LOCATION from environment: ${AE_LOCATION}"
else
   export AE_LOCATION="/Applications/Adobe After Effects CC 2014"
fi

export APP_DIR=$AE_LOCATION

# One is for the AfterEffects itself and the other one for the renderer
export APP_EXE_RENDER="$AE_LOCATION/aerender"
export APP_EXE="$AE_LOCATION/Adobe After Effects CC 2014.app/Contents/MacOS/After Effects"
if [ -x "$AE_LOCATION/After Effects" ]; then
   export APP_EXE="$AE_LOCATION/After Effects"
elif [ -x "$AE_LOCATION/AfterFX" ]; then
   export APP_EXE="$AE_LOCATION/AfterFX"
fi

# Keeper locate_* only when env did not pin a build
if [ "$AE_FROM_ENV" = "0" ]; then
   locate_file="$CGRU_LOCATION/software_setup/locate_afterfx.sh"
   if [ -f "${locate_file}" ]; then
      source "${locate_file}"
   fi
fi

export AE_LOCATION=$APP_DIR
