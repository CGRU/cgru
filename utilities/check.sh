#!/bin/bash

afanasy=$1
[ -z "$afanasy" ] && afanasy="afanasy/trunk"

cgru=`dirname $0`/..
pushd $cgru > /dev/null
cgru=$PWD
popd > /dev/null

if [ ! -d $cgru/$afanasy ]; then
   echo "Can't find Afanasy in '$cgru/$afanasy'"
   exit 1
fi

dirs_nonempty="$afanasy/bin_pyaf plugins/maya/mll"
cgru_bins="convert ffmpeg exrjoin"
afanasy_bins="afserver afcmd afrender aftalk afwatch pyaf.so"
utilities_bins="regexp/bin/regexp"

cgru_bin="$cgru/bin"
afanasy_bin="$cgru/$afanasy/bin"

# Check non-empty folders:
for dir in $dirs_nonempty; do
   dir="$cgru/$dir"
   if [ ! -d $dir ]; then
      echo "Folder '$dir' does not exist."
      exit 1
   fi
   if [ -z "`ls $dir`" ]; then
      echo "Folder '$dir' is empty."
      exit 1
   fi
done

# Check binaries:
bins=""
for bin in $cgru_bins; do bins="$cgru/bin/$bin $bins"; done
for bin in $afanasy_bins; do bins="$cgru/$afanasy/bin/$bin $bins"; done
for bin in $utilities_bins; do bins="$cgru/utilities/$bin $bins"; done
for bin in $bins; do
   if [ ! -x "$bin" ]; then
      echo "No binary '$bin'."
      exit 1
   fi
done

exit 0
