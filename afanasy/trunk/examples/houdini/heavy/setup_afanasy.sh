#!/bin/bash

# Setup Afanasy if was not:
if [ -z $AF_ROOT ]; then
   pwd=$PWD
   cd ../../..
   source setup.sh
   cd $pwd
fi

# Launch commands from current directory:
export AF_CMD_PREFIX="./"
