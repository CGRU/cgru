#!/usr/bin/env bash

cgru="/data/cgru"

pushd "${cgru}" > /dev/null
source "./setup.sh" > /dev/null
popd > /dev/null

pushd "${cgru}/afanasy" > /dev/null
source "./setup.sh" > /dev/null
popd > /dev/null

"${CGRU_PYTHONEXE}" "$CGRU_LOCATION/utilities/moviemaker/makecut.py" "$@"

