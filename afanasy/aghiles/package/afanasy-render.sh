#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Copying files:
mkdir -p $pack/$installdir/afanasy/bin
cp -r $src/afanasy/bin/afrender $pack/$installdir/afanasy/bin

mkdir -p $pack/$installdir/afanasy/launch
cp -r $src/afanasy/launch/afrender.sh $pack/$installdir/afanasy/launch
