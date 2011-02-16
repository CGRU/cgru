#!/bin/bash

for folder in `ls`; do
   [ ! -d $folder ] && continue
   [ $folder == "qt" ] && continue 
   cd $folder
   if [ -x build.sh ]; then
      echo "#############################   $folder   #############################"
      ./build.sh
   fi
   cd ..
done
