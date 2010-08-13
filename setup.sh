#!/bin/bash

# Set CGRU root:
export CGRU_LOCATION=$PWD

# Add CGRU bin to path:
export PATH="${CGRU_LOCATION}/bin:${PATH}"

# Python module path:
export CGRU_PYTHON="${CGRU_LOCATION}/lib/python"
if [ "$PYTHONPATH" != "" ]; then
   export PYTHONPATH="${CGRU_PYTHON}:${PYTHONPATH}"
else
   export PYTHONPATH="${CGRU_PYTHON}"
fi
