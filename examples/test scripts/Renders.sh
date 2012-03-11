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

for(( n=0; n<$count; n++))
do
   if [ $count == 1 ]; then
      export AF_HOSTNAME="$1"
   else
      export AF_HOSTNAME="$1$n"
   fi
   echo $AF_HOSTNAME
   if [ "$3" != "no" ]; then
      afrender >& /dev/null &
   fi
done
