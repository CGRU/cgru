#!/bin/bash

echo "Clearing Natron examples in '$PWD'"

scenes="scene.ntp"
for scene in $scenes; do
   for tmpscene in `ls ${scene}*`; do
      [ $tmpscene == $scene ] && continue
      rm -fv $tmpscene
   done
done

rm -fv *.mov
[ -d render ] && rm -rfv render
