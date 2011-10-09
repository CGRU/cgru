#!/bin/bash

cd `dirname $0`
if [ -z "$CGRU_LOCATION" ]; then
   pushd ../.. > /dev/null
   source "./setup.sh"
   popd > /dev/null
fi

"$CGRU_PYTHONEXE" regexp.py
