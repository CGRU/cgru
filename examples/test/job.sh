#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

exec python job.py "$@"
