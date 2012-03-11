#!/bin/bash

script="`dirname $0`/getrevision.py"
export CGRU_REVISION=`python "$script" $1`

echo CGRU_REVISION $CGRU_REVISION
