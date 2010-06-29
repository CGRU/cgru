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

#utilities/regexp/icons \
files="\
index.html \
plugins \
doc/rus_doklad \
utilities/doc \
utilities/moviemaker \
utilities/regexp/doc \
"
for f in $files; do cp -r $src/$f $pack/$cgru/$f; done

# Copying files:
cp -r $root/utilities/moviemaker/ffmpeg $pack/$cgru/utilities/moviemaker
cp -r $root/utilities/regexp/bin/regexp $pack/$cgru/utilities/regexp/bin
cp $root/bin/exrjoin $pack/$cgru/bin/
