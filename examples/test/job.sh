#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

source $CGRU_LOCATION/utilities/python/setup.sh

exec python job.py "$@"
