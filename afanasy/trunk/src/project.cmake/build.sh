#!/bin/bash

# export AF_PYTHON_INCLUDE_PATH=C:\Python31\include
# export AF_PYTHON_LIBRARIES=C:\Python31\libs\libpython31.a

[ -f override.sh ] && source override.sh

cmake . && make
