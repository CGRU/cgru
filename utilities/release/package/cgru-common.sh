#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Crearte folders:
mkdir -p $pack/$installdir

# Copy all files:
cp -r $src/* $pack/$installdir

# Remove Afanasy binaries:
rm -rf $pack/$installdir/afanasy/bin
