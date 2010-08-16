#!/bin/bash

# Setup:
pushd .. >> /dev/null
source ./setup_nuke.sh
popd >> /dev/null

# Launch render script:
./render.py
