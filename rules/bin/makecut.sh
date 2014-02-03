#!/bin/bash

cgru="/data/cgru"

pushd "${cgru}" > /dev/null
source "./setup.sh" > /dev/null
popd > /dev/null

python "$CGRU_LOCATION/utilities/moviemaker/makecut.py" "$@"

