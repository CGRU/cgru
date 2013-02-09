#!/bin/bash

# Initialize variables:
src=$1
pack=$2
installdir=$3
[ -z "$installdir" ] && exit

# Moving icons to standart linux location:
cp -r $src/icons/icons $pack/usr/local/share

# chmod /usr:
chmod -R 755 $pack/usr

