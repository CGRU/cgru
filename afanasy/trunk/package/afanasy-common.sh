#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
[ -z $cgru ] && exit

# Copying files:
mkdir -p $pack/$cgru/afanasy/bin
files="\
afanasy/setup.sh \
afanasy/python \
afanasy/bin/pyaf.so \
afanasy/config_default.xml \
afanasy/pathmap.txt \
afanasy/farm_default.xml \
afanasy/bin/afcmd \
afanasy/init \
"
for f in $files; do cp -r $src/$f $pack/$cgru/$f; done
