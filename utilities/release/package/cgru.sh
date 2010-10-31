#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
root=$4
[ -z $root ] && exit

# Creating directories:
mkdir -p $pack/$cgru/doc
mkdir -p $pack/$cgru/bin
mkdir -p $pack/$cgru/utilities/regexp/bin

# Moving icons to standart linux location:
cp -r $src/doc/icons/icons $pack/usr/local/share
cp -r $src/utilities/moviemaker/icons/icons $pack/usr/local/share
cp -r $src/utilities/regexp/icons/icons $pack/usr/local/share

files="\
index.html \
plugins \
examples \
doc/rus_doklad \
utilities/doc \
utilities/moviemaker \
utilities/regexp/doc \
"

# Copying files:
for f in $files; do cp -r $src/$f $pack/$cgru/$f; done
cp -r $src/utilities/fixpathes.py $pack/$cgru/utilities
cp -r $root/utilities/regexp/bin/regexp $pack/$cgru/utilities/regexp/bin
cp $root/bin/exrjoin $pack/$cgru/bin/
cp $root/bin/ffmpeg $pack/$cgru/bin/
cp $root/bin/convert $pack/$cgru/bin/
