#!/bin/bash

pyver=$1
[ -z "$pyver" ] && pyver=3.2

# Check CGRU:
if [ -z "$CGRU_LOCATION" ]; then
   echo "ERROR: CGRU_LOCATION is not defined."
else
   # Check Afanasy:
   if [ -z "$AF_ROOT" ]; then
      echo "ERROR: AF_ROOT is not defined."
   else
      # Init Python if exists:
      cgru_python="${CGRU_LOCATION}/utilities/python/${pyver}"
      if [ -d "${cgru_python}" ]; then
         export PATH="${cgru_python}/bin:${PATH}"
      fi
      # Python modules path:
      pyafdir="${AF_ROOT}/bin_pyaf/${pyver}"
      pyaf="${pyafdir}/pyaf.so"
      if [ ! -f "${pyaf}" ]; then
         echo "ERROR: Python module not founded:"
         echo $pyaf
      else
         # Python scripts path:
         afpython3="${AF_ROOT}/python3"
         if [ ! -d "${afpython3}" ]; then
            echo "ERROR: Afanasy Python3 scripts folder not founded:"
            echo $afpython3
         else
            export AF_PYTHON="${pyafdir}:${afpython3}"
            export PYTHONPATH="${AF_PYTHON}:${PYTHONPATH}"
            echo "PYTHONPATH=${PYTHONPATH}"
            echo "Python3 setup sourced."
         fi
      fi
   fi
fi
