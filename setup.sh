#!/bin/bash

# Set CGRU root:
export CGRU_LOCATION=$PWD

# Add CGRU bin to path:
export PATH="${CGRU_LOCATION}/bin:${PATH}"

# Add software to PATH:
export PATH="${CGRU_LOCATION}/software_setup/bin:${PATH}"

# Python module path:
export CGRU_PYTHON="${CGRU_LOCATION}/lib/python"
if [ "$PYTHONPATH" != "" ]; then
   export PYTHONPATH="${CGRU_PYTHON}:${PYTHONPATH}"
else
   export PYTHONPATH="${CGRU_PYTHON}"
fi

# Get CGRU version:
export CGRU_VERSION=`cat version.txt`
echo "CGRU_VERSION $CGRU_VERSION : '$CGRU_LOCATION'"

# Source custom setup if any exists:
setup_files=`ls setup_*.sh`
if [ ! -z "$setup_files" ] ; then
   for setup_file in $setup_files; do
      [ -z "$setup_file" ] && continue
      [ -f "$setup_file" ] || continue
      source ./$setup_file ""
   done
fi
