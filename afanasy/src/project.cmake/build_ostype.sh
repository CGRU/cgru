#!/bin/bash

export AF_OSTYPE=$OSTYPE

[ -z "$AF_OSTYPE" ] && export AF_OSTYPE=`uname -s`
[ -z "$AF_OSTYPE" ] && export AF_OSTYPE="someos"

echo "OSTYPE='$AF_OSTYPE'"

./build.sh
