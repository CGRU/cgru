#!/bin/bash

# This script tries to setup CGRU, if was not.
# It searches "setup.sh" in upper folders and sources it.

curdir=$PWD

if [ -z $CGRU_LOCATION ]; then
   cd $AF_ROOT
   cd ..
   while [ $PWD != "/" ]; do
      if [ -f ./setup.sh ]; then
         echo "Sourcing CGRU setup from '$PWD'."
         source ./setup.sh
         break
      fi
      cd ..
   done
fi

cd $curdir
