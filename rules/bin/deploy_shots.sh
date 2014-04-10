#!/bin/bash

cgru="/data/cgru"

pushd "${cgru}" > /dev/null
source "./setup.sh" > /dev/null
popd > /dev/null

python "$CGRU_LOCATION/rules/bin/deploy_shots.py" "$@"
