#!/bin/bash

if [ -z "$CGRU_LOCATION" ]; then
   pushd `dirname $0` > /dev/null
   source "./setup.sh"
   popd > /dev/null
fi

"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/keeper/keeper.py" "$@"
