#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Moving icons to standart linux location:
cp -r $src/afanasy/doc/icons/icons $pack/usr/local/share

# Copying files:
mkdir -p $pack/$installdir/afanasy
files="\
afanasy/doc \
"
for f in $files; do cp -r $src/$f $pack/$installdir/$f; done
