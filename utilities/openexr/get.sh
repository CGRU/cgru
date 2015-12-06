#!/bin/bash

packages="ilmbase-2.2.0 openexr-2.2.0"
location="http://download.savannah.nongnu.org/releases/openexr"

for package in $packages; do
   if [ -d $package ] ; then
      echo "Folder $package already exists. Skipping package."
      continue
   fi
   filename="$package.tar.gz"
   if [ -f $filename ] ; then
      echo "File $filename already exists. Skipping download."
   else
      wget "$location/$filename"
   fi
   tar xvzf $filename
done
