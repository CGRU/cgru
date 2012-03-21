#!/bin/bash

echo "Clearing Nuke examples in '$PWD'"

scenes="scene.nk scene_stereo.nk"
for scene in $scenes; do
   for tmpscene in `ls ${scene}*`; do
      [ $tmpscene == $scene ] && continue
      rm -fv $tmpscene
   done
done

rm -fv *.mov
[ -d render ] && rm -rfv render
