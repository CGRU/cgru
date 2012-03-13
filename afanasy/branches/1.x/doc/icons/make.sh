#!/bin/bash

iconsall="doc"
prefix="af"
iconsdir="icons/hicolor"
svgdir="$iconsdir/scalable/apps"
sizes="16 22 24 32 36 48 64 72 96 128 192"

if [ -d $iconsdir ]; then
   echo "Rmoving old icons directory '$iconsdir'"
   rm -rf $iconsdir
fi
mkdir -p $svgdir

for icon in $iconsall; do
   cp "${icon}.svg" "${svgdir}/${prefix}${icon}.svg"
done

for s in $sizes; do
   for icon in $iconsall; do
      folder="${iconsdir}/${s}x${s}/apps"
      mkdir -p $folder
      inkscape -w=${s} -h=${s} --export-png=${folder}/${prefix}${icon}.png $svgdir/${prefix}${icon}.svg
   done
done
