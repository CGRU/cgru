#!/bin/bash

pushd .. >> /dev/null
source ./setup_houdini.sh
popd >> /dev/null

python render.py $*
