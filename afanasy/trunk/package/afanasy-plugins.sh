#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Copying files:
mkdir -p $pack/$installdir/afanasy
cp -r $src/afanasy/plugins $pack/$installdir/afanasy

cp -r $src/afanasy/bin_pyaf $pack/$installdir/afanasy
