#!/bin/bash

# This script clears CGRU Examples, delete rendered images and temporary scenes:

examples=`dirname $0`
cd $examples
examples=$PWD
folders=`ls`
script="clear.sh"

for folder in $folders; do
   [ -d "$folder" ] || continue
   cd $folder
   if [ -x "$script" ]; then
      echo "Clearing '$folder':"
      ./$script
      echo "Done."
   fi
   cd ..
done
