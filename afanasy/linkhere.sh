#!/bin/bash

src=$1

# Folders to link:
folders="doc icons plugins webvisor"

# Binaries to link:
apps="cmd render watch server talk monitor"

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

# Linking binaries:
bin="${src}/bin"
if [ -d "${bin}" ] ; then
   curdir=$PWD
   cd "${bin}"
   for app in $apps
   do
      ln -svf "../src/tmp/${app}/af${app}" "af${app}"
   done
   ln -svf "../src/tmp/libpyaf/pyaf.so" "pyaf.so"
   cd $curdir
else
   echo "Warining, directory '$bin' does not exists."
fi
