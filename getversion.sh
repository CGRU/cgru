#!/bin/bash

export CGRU_VERSION=`python getrevision.py $1`

[ -f 'getcustomversion.sh' ] && source 'getcustomversion.sh'

echo CGRU_VERSION $CGRU_VERSION
