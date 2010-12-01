#!/bin/bash

appnames="mavishky scanscan"
iconsdir="icons/hicolor"
svgdir="$iconsdir/scalable/apps"

if [ -d $iconsdir ]; then
   echo "Rmoving old icons directory '$iconsdir'"
   rm -rf $iconsdir
fi
mkdir -p $svgdir

for appname in $appnames
do
   iconname="cgru$appname"

   cp "$appname.svg" "$svgdir/$iconname.svg"

   sizes="16 22 24 32 36 48 64 72 96 128 192"
   for s in $sizes
   do
      folder="${iconsdir}/${s}x${s}/apps"
      mkdir -p $folder
      inkscape -w ${s} -h ${s} --export-png=$folder/$iconname.png $svgdir/$iconname.svg
   done
done
