#!/bin/bash

exitStatus=0
exitStatusError=1
for arg in "$@"; do
   if [ $arg == "--exitsuccess" ]; then
      exitStatusError=0
   else
      afanasy=$arg
   fi
done
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
afanasy_bins="afserver afcmd afrender aftalk afwatch"
afanasy_libs="pyaf.so"
#utilities_bins="regexp/bin/regexp"

cgru_bin="$cgru/bin"
afanasy_bin="$cgru/$afanasy/bin"

# Check non-empty folders:
for dir in $dirs_nonempty; do
   dir="$cgru/$dir"
   if [ ! -d $dir ]; then
      echo "Folder '$dir' does not exist."
      exitStatus=$exitStatusError
   fi
   if [ -z "`ls $dir`" ]; then
      echo "Folder '$dir' is empty."
      exitStatus=$exitStatusError
   fi
done

# Check binaries:
bins=""
for bin in $cgru_bins; do bins="$cgru/bin/$bin $bins"; done
for bin in $afanasy_bins; do bins="$cgru/$afanasy/bin/$bin $bins"; done
for lib in $afanasy_libs; do bins="$cgru/$afanasy/lib/$lib $bins"; done
#for bin in $utilities_bins; do bins="$cgru/utilities/$bin $bins"; done
for bin in $bins; do
   if [ ! -x "$bin" ]; then
      echo "No binary '$bin'."
      exitStatus=$exitStatusError
   fi
done

exit $exitStatus
