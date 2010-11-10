#!/bin/bash

moviemakerdir=`dirname $0`
pushd $moviemakerdir >> /dev/null
cd ../..
source ./setup.sh .
cd afanasy
[ -d trunk ] && cd trunk
source ./setup.sh
popd >> /dev/null

python=python
cgru_python=../python/2.5.5/bin/python
if [ -e $cgru_python ]; then
   python=$cgru_python
fi

$python $moviemakerdir/dialog.py $*
