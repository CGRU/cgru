#!/bin/bash

echo "Clearing Maya examples in '$PWD'"

for scene in `ls scene.mb*`; do
   [ $scene == "scene.mb" ] && continue
   rm -fv $scene
done
for image in `ls render`; do
   if [ -d "render/$image" ]; then
      [ "$image" == ".svn" ] && continue
      rm -rvf render/$image
      continue
   fi
   rm -fv render/$image
done
