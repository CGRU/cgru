#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Copying files:
mkdir -p $pack/$installdir/afanasy/bin
cp -r $src/afanasy/bin/afserver $pack/$installdir/afanasy/bin
