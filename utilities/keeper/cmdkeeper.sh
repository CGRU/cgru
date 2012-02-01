#!/bin/bash

pushd `dirname $0` > /dev/null
if [ -z "$CGRU_LOCATION" ]; then
   cd ../..
   [ -f "./setup.sh" ] && source ./setup.sh
fi
popd > /dev/null

"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/keeper/cmdkeeper.py" "$@"

exit 0
