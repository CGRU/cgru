#!/bin/bash

# Setup Python from CGRU, if exists:
# (some systems do not have compatible Python)

cgru_python="$CGRU_LOCATION/utilities/python/2.5.5"

if [ -d $cgru_python ]; then
   export PATH=$cgru_python/bin:$PATH
   echo "Using CGRU Python: $cgru_python"
fi
