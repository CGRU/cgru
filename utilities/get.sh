#!/bin/bash

for folder in *; do
   [ ! -d $folder ] && continue
   [ $folder == "qt" ] && continue
   pushd $folder > /dev/null
   if [ -x get.sh ]; then
      echo "#############################   $folder   #############################"
      ./get.sh
   fi
   popd $folder > /dev/null
done
