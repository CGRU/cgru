#!/bin/bash

for folder in `ls`; do
   [ ! -d $folder ] && continue
   [ $folder == "qt" ] && continue 
   cd $folder
   if [ -x get.sh ]; then
      echo "#############################   $folder   #############################"
      ./get.sh
   fi
   cd ..
done
