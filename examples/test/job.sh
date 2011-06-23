#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

cgru_python_setup=$CGRU_LOCATION/utilities/python/setup.sh
[ -f $cgru_python_setup ] && source $cgru_python_setup ""

python job.py "$@"
