#!/bin/bash

# This script tries to open CGRU Examples folder in some bowsers:

examples=`dirname $0`
cd $examples
examples=$PWD
browsers="krusader gnome-commander nautilus konqueror"

for browser in $browsers; do
   if [ `which $browser` ]; then
      [ $browser == "krusader" ] && browser="$browser --left $examples --right"
      [ $browser == "gnome-commander" ] && browser="$browser -l $examples -r"
      echo $browser $examples
      $browser $examples
      exit 0
   fi
done

echo "No browser founded."
exit 1
