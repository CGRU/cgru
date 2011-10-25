#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Copying files:
mkdir -p $pack/$installdir/afanasy
files="\
afanasy/bin \
afanasy/bin_pyaf \
"
for f in $files; do cp -r $src/$f $pack/$installdir/$f; done
