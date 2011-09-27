#!/bin/bash

cd `dirname 0`
pushd ../.. > /dev/null
source "./setup.sh"
popd > /dev/null

python ./afstarter.py "$@"
