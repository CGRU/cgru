#!/bin/bash

export AF_PYTHON_INC=`python-config --includes`
export AF_PYTHON_LIB=`python-config --libs`

qmake

make

cp -fv libpyaf/libpyaf.so ../../bin/pyaf.so
