#!/bin/bash

packages="ilmbase-1.0.3 openexr-1.7.1"
location="http://github.com/downloads/openexr/openexr"

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
