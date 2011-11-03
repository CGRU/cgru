#!/bin/bash

pushd ../.. >> /dev/null
source ./setup.sh
popd

# Launch Blender render:
blender -b scene.blend -s 3 -e 3 -j 1 -a -o "render/my_image-####" -F JPEG -e BLENDER_RENDER
