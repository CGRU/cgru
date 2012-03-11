#!/bin/bash

pushd ../.. >> /dev/null
source ./setup.sh
popd

# Launch render script:
python ./render.py
