#!/bin/bash

cd "$(dirname "$0")"

[ -d render ] || mkdir -v -m 777 render

exec ../../software_setup/start_blender.sh scene.blend
