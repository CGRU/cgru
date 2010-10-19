#!/bin/bash

pushd ../.. >> /dev/null
source ./setup.sh
cd afanasy
[ -d trunk ] && cd trunk
source ./setup.sh
popd >> /dev/null

python=python
cgru_python=../python/2.5.5/bin/python
if [ -e $cgru_python ]; then
   python=$cgru_python
fi

export PATH=../../bin:$PATH

$python dialog.py $*
