#!/bin/bash

for folder in *; do
   [ ! -d $folder ] && continue
   [ $folder != "qt" ] || continue
   pushd $folder > /dev/null
   if [ -x build.sh ]; then
      echo "#############################   $folder   #############################"
      ./build.sh
   fi
   popd > /dev/null
done
