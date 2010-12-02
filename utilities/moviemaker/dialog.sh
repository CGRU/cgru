#!/bin/bash

moviemakerdir=`dirname $0`
pushd $moviemakerdir >> /dev/null
cd ../..
source ./setup.sh
popd >> /dev/null

python $moviemakerdir/dialog.py $*
