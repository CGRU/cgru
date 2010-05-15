#!/bin/bash

# Setup Afanasy:
curdir=$PWD
cd ..
source ./setup_afanasy.sh
cd $curdir

# Launch render script:
./render.py
