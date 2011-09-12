#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

# Setup special Python module:
export PYTHONPATH="$AF_ROOT/bin_pyaf/3.2:$PYTHONPATH"

#source $AF_ROOT/py3k_setup.sh 3.2
#export BLENDER_USER_SCRIPTS="$AF_ROOT/plugins/blender2.5"
#export BLENDER_SYSTEM_SCRIPTS="$AF_ROOT/plugins/blender2.5"

export AF_CMD_PREFIX="./"

export BLENDER_LOCATION="/opt/blender-2.57b-linux-glibc27-x86_64"

# Redefine blender location in "override.sh" file:
[ -f override.sh ] && source override.sh
