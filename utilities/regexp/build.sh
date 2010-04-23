#!/bin/bash

# Setup Qt:
export QMAKE=qmake

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

cd src

# Run qmake to make makefiles
$QMAKE

# Make files
make

cd ..
