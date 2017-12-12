#!/bin/bash
set -eo pipefail

while ! ping -c 1 $AF_SERVERNAME 2> /dev/null
do
	echo "Waiting for $AF_SERVERNAME ..."
	sleep 1
done

PROC_COUNT=`nproc --all`
AF_RENDER_DEFAULT_CAPACITY=`expr $PROC_COUNT \* $AF_RENDER_AUTO_CAPACITY_PER_PROC` exec /usr/local/bin/docker-entrypoint.sh "$@"