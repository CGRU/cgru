#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
root=$4
[ -z $root ] && exit

# Creating directories:
mkdir -p $pack/$cgru/doc
mkdir -p $pack/$cgru/utilities/regexp/bin

# Moving icons to standart linux location:
cp -r $src/doc/icons/icons $pack/usr/local/share
cp -r $src/utilities/moviemaker/icons/icons $pack/usr/local/share
cp -r $src/utilities/regexp/icons/icons $pack/usr/local/share

# Copying files:
cp -r $src/utilities/doc $pack/$cgru/utilities
cp -r $src/utilities/moviemaker $pack/$cgru/utilities
cp -r $src/utilities/regexp/doc $pack/$cgru/utilities/regexp
cp -r $root/utilities/regexp/bin/regexp $pack/$cgru/utilities/regexp/bin

files="\
index.html \
plugins \
doc/rus_doklad \
utilities/regexp/icons \
"
for f in $files; do cp -r $src/$f $pack/$cgru/$f; done
