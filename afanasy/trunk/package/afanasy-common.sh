#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Copying files:
mkdir -p $pack/$installdir/afanasy/bin
mkdir -p $pack/$installdir/afanasy/launch
files="\
afanasy/launch/setup.sh \
afanasy/python \
afanasy/python3 \
afanasy/bin/pyaf.so \
afanasy/config_default.xml \
afanasy/pathmap.txt \
afanasy/farm_example.xml \
afanasy/bin/afcmd \
afanasy/init \
"
for f in $files; do cp -r $src/$f $pack/$installdir/$f; done

cp $src/afanasy/*.sh $pack/$installdir/afanasy
