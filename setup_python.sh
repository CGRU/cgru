#!/bin/bash

if [ -z "${CGRU_PYTHONEXE}" ]; then
   python="${CGRU_LOCATION}/python"
   if [ -d "${python}" ]; then
      echo "Using CGRU Python: ${python}"
      export PATH="$python/bin:${PATH}"
      export CGRU_PYTHONDIR="${python}"
      export CGRU_PYTHONEXE="${python}/bin/python3"
   else
      export CGRU_PYTHONEXE="python"
   fi
fi
