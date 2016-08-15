#!/bin/bash
pushd "$(dirname "$0")/../.." > /dev/null
source "setup.sh" > /dev/null
popd > /dev/null

"${CGRU_LOCATION}/rules/bin/walk.py" "$@"
