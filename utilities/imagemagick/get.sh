#!/bin/bash

v=6.6.7

im=ImageMagick
imv=${im}-${v}

[ -L $im ] && rm -vf $im

if [ -d $imv ]; then
   cd $imv
   svn up
   cd ..
else
   url=https://www.imagemagick.org/subversion/ImageMagick/branches/$imv
   svn co $url $imv
fi

ln -s $imv $im
