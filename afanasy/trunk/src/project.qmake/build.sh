#!/bin/bash

# Setup Qt (use system by default):
export QMAKE=

# Setup Python:
export AF_PYTHON_INC="`python-config --cflags` -fPIC"
export AF_PYTHON_LIB="`python-config --ldflags`"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

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
cp -fv libpyaf/libpyaf.so ../../bin/pyaf.so
