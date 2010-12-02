#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pushd .. >> /dev/null
   source ./setup.sh
   popd >> /dev/null
fi

# overrides (set custom values there):
[ -f ./override.sh ] && source ./override.sh
