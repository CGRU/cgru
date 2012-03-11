#!/bin/bash

icons="keeper regexp dailies scanscan afanasy afwatch aftalk"
size="128"

iconslinux="cgrumenuitem"
iconsdir="icons/hicolor"
svgdir="$iconsdir/scalable/apps"
sizes="16 22 24 32 36 48 64 72 96 128 192"

if [ -d $iconsdir ]; then
   echo "Rmoving old icons directory '$iconsdir'"
   rm -rf $iconsdir
fi
mkdir -p $svgdir

for icon in $iconslinux; do
   cp "${icon}.svg" "${svgdir}/${icon}.svg"
done

for s in $sizes; do
   for icon in $iconslinux; do
      folder="${iconsdir}/${s}x${s}/apps"
      mkdir -p $folder
      inkscape -w ${s} -h ${s} --export-png=${folder}/${icon}.png $svgdir/${icon}.svg
   done
done

for icon in $icons; do
   [ -f "${icon}.png" ] && rm -fv "${icon}.png"
   inkscape -w $size -h $size --export-png=${icon}.png ${icon}.svg
done
