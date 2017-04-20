#!/bin/bash

RENDER="afrender"
export AF_USERNAME="renderer"

count=1
if [ "" != "$2" ]; then
   count=$2
fi

HOST=$HOSTNAME
if [ "" != "$1" ]; then
   HOST=$1
fi

source ./setup.sh

for n in $(seq -f "%04g" 1 $count)
do
	export AF_HOSTNAME="$1$n"
	echo $AF_HOSTNAME

	afrender >& /dev/null &

	sleep .07
done
