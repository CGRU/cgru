#!/bin/bash

echo "Clearing Blender examples in '$PWD'"

for scene in `ls scene.blend*`; do
   [ $scene == "scene.blend" ] && continue
   rm -fv $scene
done
rm -fv untitled.blend*
[ -d render ] && rm -rfv render
