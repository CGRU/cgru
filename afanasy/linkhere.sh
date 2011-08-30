#!/bin/bash

src=$1

# Folders to link:
folders="doc icons plugins src webvisor"

# Removing links and exit if no argumets provided:
if [ -z $src ]; then
   echo "Removing links..."
   for folder in $folders
   do
      [ -d "${folder}" ] && rm -v $folder
   done
   echo "Done"
   exit 0
fi

# Linking folders:
for folder in $folders
do
   ln -svf $src/$folder $folder
done
