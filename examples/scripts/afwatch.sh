#!/bin/bash

pushd .. >> /dev/null
source ./setup.sh
popd >> /dev/null

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

afwatch $*
