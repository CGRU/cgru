#!/bin/bash

ver=1.4.10
location="http://rubyforge.org/frs/download.php/53915"
filename="Mxx_ru-$ver.gem"
link="$location/$filename"

if [ -f $filename ]; then
   echo "File $filename already exists."
   exit
fi

wget $link
