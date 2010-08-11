#!/bin/bash

# Setup Afanasy if was not:
if [ -z $AF_ROOT ]; then
   pushd ../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi

# Launch commands from current directory:
export AF_CMD_PREFIX="./"
