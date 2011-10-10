#!/bin/bash

if [ -z "${CGRU_PYTHONEXE}" ]; then
   export CGRU_PYTHONEXE="python"
   python="${CGRU_LOCATION}/python"
   if [ -d "${python}" ]; then
      echo "Using CGRU Python: ${python}"
      export PYTHONHOME="${python}"
      export PATH="$python/bin:${PATH}"
      export CGRU_PYTHONDIR="${python}"
      pythonexe="${python}/bin/python3"
      [ -x "${pythonexe}" ] && export CGRU_PYTHONEXE="${python}/bin/python3"
   fi
fi
