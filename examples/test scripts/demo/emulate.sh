#!/bin/bash

script="$PWD/emulate.py"

cd ..

# Source CGRU setup:
if [ -z $CGRU_LOCATION ]; then
    pushd ../.. >> /dev/null
    source ./setup.sh
    popd >> /dev/null
fi

python "$script" "$@"

