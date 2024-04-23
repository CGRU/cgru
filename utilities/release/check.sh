#!/usr/bin/env bash

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

dirs_nonempty=""
afanasy_bins="afserver afcmd afrender afwatch"

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
for bin in $afanasy_bins; do bins="$cgru/afanasy/bin/$bin $bins"; done
for bin in $bins; do
	if [ ! -x "$bin" ]; then
		echo "No binary '$bin'."
		exitStatus=$exitStatusError
	fi
done

exit $exitStatus

