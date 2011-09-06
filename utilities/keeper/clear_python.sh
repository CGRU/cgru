#!/bin/bash

python=`dirname $0`/Python

[ -d $python ] || exit

rm -vf $python/bin/python3.2
rm -vf $python/bin/python3.2mu
rm -rvf $python/share
find $python -name "test" -exec rm -rvf {} \;
find $python -name "*.a" -exec rm -vf {} \;
find $python -name "*.pyc" -exec rm -vf {} \;
find $python -name "*.pyo" -exec rm -vf {} \;
find $python -name "__pycache__" -exec rm -rvf {} \;
