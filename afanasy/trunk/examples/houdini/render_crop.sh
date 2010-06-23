#!/bin/bash

# Setup CGRU:
source ./setup_cgru.sh

# Setup Afanasy:
source ./setup_afanasy.sh

python ./render_crop.py
