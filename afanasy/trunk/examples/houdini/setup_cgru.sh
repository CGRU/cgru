#!/bin/bash

# Setup Afanasy if was not:
if [ -z $CGRU_LOCATION ]; then
   pushd ../../../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi
