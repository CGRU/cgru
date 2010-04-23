#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
[ -z $cgru ] && exit

# Creating directories:
mkdir -p $pack/$cgru/doc
mkdir -p $pack/$cgru/bin
mkdir -p $pack/$cgru/lib/python

# Copying files:
cp -r $src/doc/images $pack/$cgru/doc
cp $src/bin/*.py $pack/$cgru/bin
cp $src/lib/python/*.py $pack/$cgru/lib/python
cp $src/config_default.xml $pack/$cgru

# Moving icons to standart linux location:
cp -r $src/utilities/release/icons/icons $pack/usr/local/share

# Copy pages and images from CGRU documentation:
extexsions="html css"
for ext in $extexsions; do
   cp $src/doc/*.$ext $pack/$cgru/doc
done
