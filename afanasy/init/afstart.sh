#!/bin/bash

exefile="$1"
logfile="$2"
pidfile="$3"

if [ -z "$AF_ROOT" ]; then
   pushd "$(dirname "$exefile")"/..
   source ./setup.sh
   popd > /dev/null
fi

echo "AF_ROOT    = '$AF_ROOT'"
[ -z "$AF_HOSTNAME" ] || echo "AF_HOSTNAME= '$AF_HOSTNAME'"
echo "Launchfile = '$exefile'"
echo "Logfile    = '$logfile'"

touch "$logfile"
chmod a+rw "$logfile"
"$exefile" >& "$logfile" &

if [ ! -z "$pidfile" ]; then
   echo "Pidfile    = '$pidfile'"
   echo $! > "$pidfile"
   chmod a+rw "$pidfile"
fi

exit 0
