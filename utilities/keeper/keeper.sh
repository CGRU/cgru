#!/bin/bash

pushd `dirname $0` > /dev/null
export CGRU_KEEPER=$PWD
cd ../..
[ -f "./setup.sh" ] && source ./setup.sh
popd > /dev/null

pyver="3.2.1"
source $AF_ROOT/py3k_setup.sh $pyver

python=$CGRU_KEEPER/Python
[ -d $python ] || python=$CGRU_LOCATION/utilities/python/$pyver
echo Python=$python

export PATH="$python/bin:$PATH"

python3 $CGRU_KEEPER/keeper.py
