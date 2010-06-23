#!/bin/bash

# Setup Afanasy if was not:
if [ -z $CGRU_LOCATION ]; then
   pwd=$PWD
   cd ../../../..
   source setup.sh
   cd $pwd
fi
