#!/bin/bash

export AF_OSTYPE=$OSTYPE

[ -z "$AF_OSTYPE" ] && export AF_OSTYPE=${DISTRIBUTIVE}
[ -z "$AF_OSTYPE" ] && export AF_OSTYPE=`uname -s`.`uname -m`

echo "OSTYPE='$AF_OSTYPE'"

./build.sh
