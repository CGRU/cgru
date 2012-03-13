#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pwd=$PWD
   cd ../..
   source setup.sh
   cd $pwd
fi

exec python job.py $*
