#!/bin/bash

# Setup afanasy, if was not:

if [ -z $AF_ROOT ]; then
   pushd "afanasy" >> /dev/null
   [ -d "trunk" ] && cd "trunk"
   source "./setup.sh"
   popd >> /dev/null
fi
