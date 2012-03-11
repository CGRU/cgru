#!/bin/bash

if [ -z "$CGRU_LOCATION" ]; then
   if [ -L "$0" ]; then
      link=`readlink "$0"`
      pushd `dirname "$link"` > /dev/null
   else
      pushd `dirname "$0"` > /dev/null
   fi
   source "./setup.sh"
   popd > /dev/null
fi

"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/keeper/keeper.py" "$@"
