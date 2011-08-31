#!/bin/bash

export CGRU_KEEPER=`dirname $0`

pushd $CGRU_KEEPER > /dev/null
cd ../..
[ -f "./setup.sh" ] && source ./setup.sh
popd > /dev/null

python=$CGRU_LOCATION/utilities/python/3.2.1
export PATH="$python/bin:$PATH"
python3 keeper.py
