#!/bin/bash

browsers="caja thunar nautilus konqueror gnome-commander krusader"

pushd `dirname $0` > /dev/null
cd ..
cgru="$PWD"
popd > /dev/null

folder=$1
[ -z "$folder" ] && folder=$cgru
[ -d "$folder" ] || folder="$cgru/$folder"

if [ ! -d "$folder" ]; then
   echo "Folder '$folder' not founded."
   exit 1
fi

if [ `uname` == "Darwin" ]; then
   open "$folder"
   exit 0
fi

for browser in $browsers; do
    which $browser >& /dev/null
    [ "$?" != "0" ] && continue
    [ $browser == "krusader" ] && browser="$browser --left "$folder" --right"
    [ $browser == "gnome-commander" ] && browser="$browser -l "$folder" -r"
    echo $browser "\"$folder\""
    $browser "$folder"
    exit 0
done

echo "No browser founded."
exit 1
