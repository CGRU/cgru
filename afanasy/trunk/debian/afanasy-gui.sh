#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
[ -z $cgru ] && exit

# Moving icons to standart linux location:
cp -r $src/afanasy/icons/icons $pack/usr/local/share

# Copying files:
mkdir -p $pack/$cgru/afanasy/bin
files="\
afanasy/icons \
afanasy/bin/afwatch \
afanasy/bin/aftalk \
afanasy/bin/afmonitor \
"
for f in $files; do cp -r $src/$f $pack/$cgru/$f; done
