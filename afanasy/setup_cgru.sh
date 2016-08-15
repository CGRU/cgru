#!/bin/bash

# This script tries to setup CGRU, if was not.
# It searches "setup.sh" in upper folders and sources it.

I=1

if [ -z "$CGRU_LOCATION" ]; then
   pushd "$AF_ROOT"/..  > /dev/null
   while [ "$PWD" != "/" ]; do
      if [ -f ./setup.sh ]; then
         echo "Sourcing CGRU setup from '$PWD'."
         source ./setup.sh
         break
      fi
      let I=I+1
      pushd ..  > /dev/null
   done
   while [ $I -ne 0 ]; do
     let I=I-1
     popd > /dev/null
  done
fi
