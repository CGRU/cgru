#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pushd .. >> /dev/null
   source ./setup.sh
   popd >> /dev/null
fi
