#!/bin/bash

# Setup CGRU:
pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

python ./render.py $*
