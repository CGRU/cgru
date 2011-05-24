#!/bin/bash

pyver=$1
[ -z "$pyver" ] && pyver=3.2

# Check CGRU & Afanasy:
if [ -z "$CGRU_LOCATION" ]; then
   echo "ERROR: CGRU_LOCATION is not defined."
   exit 1
fi

if [ -z "$AF_ROOT" ]; then
   echo "ERROR: AF_ROOT is not defined."
   exit 1
fi

# Init Python:
AF_PYTHON="${CGRU_LOCATION}/utilities/python/${pyver}"
if [ ! -d "${AF_PYTHON}" ]; then
   echo "ERROR: '${AF_PYTHON}' does not exist."
   exit 1
fi
export PATH=$AF_PYTHON/bin:$PATH

# Python modules path:
AF_PYTHON="${AF_ROOT}/bin_pyaf/${pyver}:${AF_ROOT}/python3"
export PYTHONPATH="${AF_PYTHON}:${PYTHONPATH}"
echo "PYTHONPATH=${PYTHONPATH}"
echo "Python3 setup sourced."
