#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Creating directories:
mkdir -p $pack/$installdir/doc
mkdir -p $pack/$installdir/bin
mkdir -p $pack/$installdir/lib/python

# Copying files:
cp $src/setup*.sh $pack/$installdir
cp $src/version.txt $pack/$installdir
cp -r $src/doc/images $pack/$installdir/doc
cp $src/bin/*.py $pack/$installdir/bin
cp $src/bin/deletefiles $pack/$installdir/bin
cp $src/bin/wolsleep $pack/$installdir/bin
cp $src/bin/wolwake $pack/$installdir/bin
cp $src/lib/python/*.py $pack/$installdir/lib/python
cp $src/config_default.xml $pack/$installdir

# Moving icons to standart linux location:
cp -r $src/utilities/release/icons/icons $pack/usr/local/share

# Copy pages and images from CGRU documentation:
extexsions="html css"
for ext in $extexsions; do
   cp $src/doc/*.$ext $pack/$installdir/doc
done
