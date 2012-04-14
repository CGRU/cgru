#!/bin/bash

# Setup afanasy, if was not:

if [ -z $AF_ROOT ]; then
   pushd "afanasy" >> /dev/null
   source "./setup.sh"
   popd >> /dev/null
fi
