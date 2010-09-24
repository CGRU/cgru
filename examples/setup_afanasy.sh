#!/bin/bash

if [ -z $AF_ROOT ]; then
   if [ -d ../afanasy/trunk ]; then
      pushd ../afanasy/trunk >> /dev/null
   else
      pushd ../afanasy >> /dev/null
   fi
   source ./setup.sh
   popd >> /dev/null
fi

# Launch commands from current directory:
export AF_CMD_PREFIX="./"
