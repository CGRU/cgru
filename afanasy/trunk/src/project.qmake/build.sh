#!/bin/bash

# Set locations:
cgru=$PWD
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru_utils=$cgru/utilities
cgru_qt=$cgru_utils/qt/qt-4.7.0-static
cgru_python=$cgru_utils/python/2.5.5

# Setup Version:
pushd .. > /dev/null
folder=$PWD
cd $cgru
source ./getversion.sh $folder
popd > /dev/null

# Setup Qt (use system by default):
export QMAKE=
# Use CGRU Qt if presets:
cgru_qmake=$cgru_qt/bin/qmake
if [ -e $cgru_qmake ]; then
	export QMAKE=$cgru_qmake
   echo "Using CGRU Qt = '$cgru_qt'"
fi

# Setup Python:
python=python
# Use CGRU Python if presets:
if [ -e $cgru_python/bin/python ]; then
   export PATH=$cgru_python/bin:$PATH
   echo "Using CGRU Python = '$cgru_python'"
fi

export AF_PYTHON_INC="`python-config --cflags` -fPIC"
export AF_PYTHON_LIB="`python-config --ldflags`"
# Extra libs for linking python module ("libpyaf" project "pyaf.so" target):
export AF_PYTHON_MODULE_EXTRA_LIBS=
# on CentOS 5 with Python2.5 try "-lrt -lz -lgthread-2.0 -lglib-2.0"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

# Add all python symbols to let user to use to use dynamic python modules
export AF_PYTHON_LIB="-rdynamic ${AF_PYTHON_LIB}"

# Setup Qt if qmake not specified (try qt4 first):
if [ -z "$QMAKE" ]; then
   QMAKE=qmake-qt4
   if ! qts=`which $QMAKE`; then
      QMAKE=qmake
      if ! qts=`which $QMAKE`; then
         echo "ERROR: qmake not founded."
         exit 1
      fi
   fi
   for qt in $qts; do QMAKE=$qt; break; done
fi
if [ ! -f "$QMAKE" ]; then
   echo "ERROR: qmake not founded."
   exit 1
fi
echo "qmake = '$QMAKE'"

# Run qmake to make makefiles
$QMAKE

# Make files
make

# Copy python library:
pyaf=libpyaf/libpyaf.so
[ -f $pyaf ] && cp -fv libpyaf/libpyaf.so ../../bin/pyaf.so
