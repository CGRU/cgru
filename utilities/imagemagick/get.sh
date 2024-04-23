#!/usr/bin/env bash

package="ImageMagick"
location="https://download.imagemagick.org/ImageMagick/download"

allfiles=`ls`
for afile in $allfiles; do
   [ -d "$afile" ] || continue
   rm -rvf $afile
done

archive="$package.tar.gz"
if [ -f $archive ] ; then
   echo "Archive $archive already exists. Skipping download."
else
   wget "$location/$archive"
fi
tar xvzf $archive

allfiles=`ls`
for afile in $allfiles; do
   [ -d "$afile" ] || continue
   [ "`echo $afile | awk '{print match( \$1, "ImageMagick")}'`" == "1" ] && ImageMagick=$afile
done

ln -svf $ImageMagick $package
