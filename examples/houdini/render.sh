#!/bin/bash

pushd ../.. >> /dev/null
source ./setup.sh
popd

[ -d render ] || mkdir -v -m 777 render

# Launch render script:
python ./render.py
