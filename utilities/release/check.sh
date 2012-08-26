#!/bin/bash

exitStatus=0
exitStatusError=1
for arg in "$@"; do
   if [ "$arg" == "--exitsuccess" ]; then
      exitStatusError=0
   fi
done

cgru=`dirname $0`/../..
pushd $cgru > /dev/null
cgru=$PWD
popd > /dev/null

dirs_nonempty="plugins/maya/mll"
cgru_bins="exrjoin"
afanasy_bins="afserver afcmd afrender aftalk afwatch"
[ -z "$DISTRIBUTIVE" ] && source "$cgru/utilities/distribution.sh"
if [ ! -z "$DISTRIBUTIVE" ]; then
    case ${DISTRIBUTIVE} in
        Fedora | openSUSE | Mint | Ubuntu | Debian | AltLinux )
            cgru_bins="$cgru_bins ffmpeg"
            ;;
        *)
            cgru_bins="$cgru_bins ffmpeg convert"
            ;;
    esac
fi

cgru_bin="$cgru/bin"
afanasy_bin="$cgru/afanasy/bin"

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
for bin in $afanasy_bins; do bins="$cgru/afanasy/bin/$bin $bins"; done
for bin in $bins; do
   if [ ! -x "$bin" ]; then
      echo "No binary '$bin'."
      exitStatus=$exitStatusError
   fi
done

exit $exitStatus
