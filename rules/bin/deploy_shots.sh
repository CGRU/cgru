#!/bin/bash

cgru="/data/cgru"

pushd "${cgru}" > /dev/null
source "./setup.sh" > /dev/null
popd > /dev/null

"${CGRU_PYTHONEXE}" "$CGRU_LOCATION/rules/bin/deploy_shots.py" "$@"

