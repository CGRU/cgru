#!/usr/bin/env bash

#echo PWD=$PWD
#echo prog=$0
#script="`dirname $0`/getrevision.py"
script="./getrevision.py"
export CGRU_REVISION=`python3 "$script" $1`

echo CGRU_REVISION $CGRU_REVISION
