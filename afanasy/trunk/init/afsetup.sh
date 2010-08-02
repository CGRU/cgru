#!/bin/bash

echo "Afanasy setup sourced."

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


cd $AF_ROOT/init
setup_files=`ls setup_*`
for setup_file in "$setup_files"; do
   [ -z "$setup_file" ] && continue
   [ -f "$setup_file" ] || continue
   echo "Sourcing custom '$setup_file'."
   source ./$setup_file
done

cd $curdir
