#!/bin/bash

icons="keeper regexp dailies scanscan"
size="128"

iconslinux="dir"
prefix="cgru"
iconsdir="icons/hicolor"
svgdir="$iconsdir/scalable/apps"
sizes="16 22 24 32 36 48 64 72 96 128 192"

if [ -d $iconsdir ]; then
   echo "Rmoving old icons directory '$iconsdir'"
   rm -rf $iconsdir
fi
mkdir -p $svgdir

for icon in $iconslinux; do
   cp "${icon}.svg" "${svgdir}/${prefix}${icon}.svg"
done

for s in $sizes; do
   for icon in $iconslinux; do
      folder="${iconsdir}/${s}x${s}/apps"
      mkdir -p $folder
      inkscape -w ${s} -h ${s} --export-png=${folder}/${prefix}${icon}.png $svgdir/${prefix}${icon}.svg
   done
done

rm *.png
for icon in $icons; do
   inkscape -w $size -h $size --export-png=${icon}.png ${icon}.svg
done
