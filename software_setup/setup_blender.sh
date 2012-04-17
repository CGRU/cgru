#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Setup default Blender location:
export BLENDER_LOCATION="/usr/bin"

export APP_DIR="${BLENDER_LOCATION}"
export APP_EXE="${BLENDER_LOCATION}/blender"

# Try to link Afanasy script to user addons:
blender_home=$HOME/.blender
if [ -d "$blender_home" ]; then
   for ver in `ls "$blender_home"`; do
      addons="$blender_home/$ver/scripts/addons"
      [ -d "$addons" ] || mkdir -p "$addons"
      script="render_afanasy.py"
      link="$addons/$script"
      [ -f "$link" ] || ln -svf "$AF_ROOT/plugins/blender/$script" "$link"
   done
fi

# Define location:
locate_file="$CGRU_LOCATION/software_setup/locate_blender.sh"
[ -f "${locate_file}" ] && source "${locate_file}"
