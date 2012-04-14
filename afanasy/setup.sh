#!/bin/bash

# Set Afanasy root:
export AF_ROOT=$PWD

export PATH="${AF_ROOT}/bin:${PATH}"

# Python module path:
AF_PYTHON="${AF_ROOT}/python"
if [ "$PYTHONPATH" != "" ]; then
   PYTHONPATH="${AF_PYTHON}:${PYTHONPATH}"
else
   PYTHONPATH="${AF_PYTHON}"
fi
export AF_PYTHON
export PYTHONPATH

# Source custom setup if any exists:
setup_files=`ls setup_*.sh`
if [ ! -z "$setup_files" ] ; then
   for setup_file in $setup_files; do
      [ -z "$setup_file" ] && continue
      [ -f "$setup_file" ] || continue
      source ./$setup_file ""
   done
fi

# Override hostname:
# export AF_USERNAME=$USER

# Override username:
# export AF_HOSTNAME=$HOSTNAME
