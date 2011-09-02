#!/bin/bash

export CGRU_KEEPER=`dirname $0`

pushd $CGRU_KEEPER > /dev/null
cd ../..
[ -f "./setup.sh" ] && source ./setup.sh
popd > /dev/null

pyver="3.2.1"

source $AF_ROOT/py3k_setup.sh $pyver

python=$CGRU_LOCATION/utilities/python/$pyver
export PATH="$python/bin:$PATH"
python3 keeper.py
