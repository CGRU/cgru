#!/bin/bash

# Setup Afanasy if was not:
if [ -z "${AF_ROOT}" ]; then
   pushd ../..
   source ./setup.sh
   popd
fi

# Launch commands from current directory:
export AF_CMD_PREFIX="./"

