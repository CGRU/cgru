#!/bin/bash

v=6.6.8

im=ImageMagick
imv=${im}-${v}

if [ -d $im ]; then
   cd $im
   svn up
   cd ..
else
   url=https://www.imagemagick.org/subversion/ImageMagick/branches/$imv
   svn co $url $im
fi

ln -s $imv $im
