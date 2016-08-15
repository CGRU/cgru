#!/bin/bash

echo "Clearing Maya examples in '$PWD'"

for scene in scene.mb*; do
   [ $scene == "scene.mb" ] && continue
   rm -fv $scene
done

[ -d render ] && rm -rfv render
