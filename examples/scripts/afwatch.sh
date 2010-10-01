#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

py=/cg/soft/python/2.5.5
export PYTHONHOME=$py

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

afwatch $*
