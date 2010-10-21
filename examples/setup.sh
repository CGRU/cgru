#!/bin/bash

# overrides (set custom values there):
[ -f ./override.sh ] && source ./override.sh

source ./setup_cgru.sh
source ./setup_afanasy.sh

cgru_python="$CGRU_LOCATION/utilities/python/2.5.5"

if [ -d $cgru_python ]; then
   export PATH=$cgru_python/bin:$PATH
   echo "Using CGRU Python = '$cgru_python'"
fi
