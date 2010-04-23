#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
[ -z $cgru ] && exit

# Moving icons to standart linux location:
cp -r $src/afanasy/doc/icons/icons $pack/usr/local/share

# Copying files:
mkdir -p $pack/$cgru/afanasy
files="\
afanasy/doc \
"
for f in $files; do cp -r $src/$f $pack/$cgru/$f; done
