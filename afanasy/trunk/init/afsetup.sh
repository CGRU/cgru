#!/bin/bash

echo "Afanasy setup sourced."

curdir=$PWD

pushd $AF_ROOT/init >> /dev/null
setup_files=`ls setup_*`
if [ ! -z "$setup_files" ] ; then
   for setup_file in $setup_files; do
      [ -z "$setup_file" ] && continue
      [ -f "$setup_file" ] || continue
      echo "Sourcing custom '$setup_file'."
      source ./$setup_file
   done
fi

popd /dev/null
