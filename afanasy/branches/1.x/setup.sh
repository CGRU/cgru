#!/bin/bash

# Set Afanasy root:
export AF_ROOT=$PWD

export PATH="${AF_ROOT}/bin:${PATH}"

# Python module path:
AF_PYTHON="${AF_ROOT}/bin:${AF_ROOT}/python"
if [ "$PYTHONPATH" != "" ]; then
   PYTHONPATH="${AF_PYTHON}:${PYTHONPATH}"
else
   PYTHONPATH="${AF_PYTHON}"
fi
export AF_PYTHON
export PYTHONPATH

# Override hostname:
# export AF_USERNAME=$USER

# Override username:
# export AF_HOSTNAME=$HOSTNAME
