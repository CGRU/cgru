#!/bin/bash

pyversion=$1

if [ -z "$pyversion" ] ; then
    pyversion="2.7.1"
else
   bin_pyaf=$AF_ROOT/bin_pyaf/$python_ver
   if [ -d "$bin_pyaf" ]; then
      if [ -z "$PYTHONPATH" ]; then
         export PYTHONPATH=$bin_pyaf
      else
         export PYTHONPATH=$bin_pyaf:$PYTHONPATH
      fi
   fi
fi

cgru_python=$CGRU_LOCATION/utilities/python/$pyversion
if [ -d "$cgru_python" ]; then
   export PYTHONHOME=$cgru_python
   export PATH=$cgru_python/bin:$PATH
   echo "Setting CGRU Python: $cgru_python"
fi

#if [ -z "$LD_LIBRARY_PATH" ] ; then
#   export LD_LIBRARY_PATH=$PYTHONHOME/lib
#else
#   export LD_LIBRARY_PATH=$PYTHONHOME/lib:$LD_LIBRARY_PATH
#fi
#
#if [ -z "$PYTHONPATH" ] ; then
#   export PYTHONPATH=$PYTHONHOME/lib/python2.7
#else
#   export PYTHONPATH=$PYTHONHOME/lib/python2.7:$PYTHONPATH
#fi
