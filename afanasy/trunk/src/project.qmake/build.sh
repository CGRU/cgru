#!/bin/bash

# Setup Qt:
export QMAKE=qmake

# Setup Python:
export AF_PYTHON_INC="`python-config --cflags` -fPIC"
export AF_PYTHON_LIB="`python-config --ldflags`"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

# Run qmake to make makefiles
$QMAKE

# Make files
make

# Copy python library:
cp -fv libpyaf/libpyaf.so ../../bin/pyaf.so
