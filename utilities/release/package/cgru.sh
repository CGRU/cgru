#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Creating directories:
mkdir -p $pack/$installdir/doc
mkdir -p $pack/$installdir/bin
mkdir -p $pack/$installdir/utilities/release
mkdir -p $pack/$installdir/utilities/site

# Moving icons to standart linux location:
cp -r $src/icons/icons $pack/usr/local/share

files="\
index.html \
plugins \
examples \
doc/rus_doklad \
utilities/doc \
utilities/moviemaker \
utilities/regexp \
utilities/release/doc \
utilities/site/doc \
"

# Copying files:
for f in $files; do cp -r $src/$f $pack/$installdir/$f; done
cp -r $src/utilities/fixpathes.py $pack/$installdir/utilities
cp -r $src/utilities/regexp/bin/regexp $pack/$installdir/utilities/regexp/bin
cp $src/bin/exrjoin $pack/$installdir/bin/
cp $src/bin/ffmpeg $pack/$installdir/bin/
cp $src/bin/convert $pack/$installdir/bin/
