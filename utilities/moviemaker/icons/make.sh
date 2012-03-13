#!/bin/bash

iconsdir="icons/hicolor"
svgdir="$iconsdir/scalable/apps"
oldname="mavishky"
newname="cgru$oldname"

if [ -d $iconsdir ]; then
   echo "Rmoving old icons directory '$iconsdir'"
   rm -rf $iconsdir
fi
mkdir -p $svgdir
cp "$oldname.svg" "$svgdir/$newname.svg"

sizes="16 22 24 32 36 48 64 72 96 128 192"
for s in $sizes
do
   folder="${iconsdir}/${s}x${s}/apps"
   mkdir -p $folder
   inkscape -w=${s} -h=${s} --export-png=$folder/$newname.png $svgdir/$newname.svg
done
