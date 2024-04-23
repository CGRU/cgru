#!/usr/bin/env bash

pushd ../.. >> /dev/null
source ./setup.sh
popd

[ -d render ] || mkdir -v -m 777 render

# Launch Blender render:
blender -b scene.blend -s 1 -e 3 -j 1 -a
