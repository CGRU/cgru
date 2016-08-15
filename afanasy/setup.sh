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
find -maxdepth 1 -type f -name "setup_*.sh" -exec source {} \;

# Override username:
# export AF_USERNAME=$USER

# Override hostname:
# export AF_HOSTNAME=$HOSTNAME
