#!/bin/bash

moviemakerdir=`dirname $0`
pushd $moviemakerdir >> /dev/null
cd ../..
source ./setup.sh
popd >> /dev/null

cgru_python=$CGRU_LOCATION/utilities/python/2.7.1

if [ -d $cgru_python ]; then
   export PATH=:$cgru_python/bin:$PATH
fi
