#!/bin/bash

# Set locations:
cgru=$PWD
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru=`dirname $cgru`
cgru_utils=$cgru/utilities
cgru_qt=$cgru_utils/qt/4.8.4
[ -d $cgru_qt ] || cgru_qt=$cgru_utils/qt/4.8.5
cgru_python=$cgru_utils/python/3.3.2

# Setup Version:
pushd .. > /dev/null
cd $cgru/utilities
source ./getrevision.sh $cgru
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
python="python"
# Use CGRU Python if presets:
if [ -d $cgru_python/bin ]; then
   export PATH=$cgru_python/bin:$PATH
   echo "Using CGRU Python = '$cgru_python'"
fi
if [ ! -z `which python3` ]; then
   echo "Using Python 3."
   python="python3"
fi

export AF_PYTHON_INC="`${python}-config --cflags` -fPIC"
export AF_PYTHON_LIB="`${python}-config --ldflags`"
# Extra libs for linking python module ("libpyaf" project "pyaf.so" target):
if [ `uname` == "Darwin" ]; then
   export AF_PYTHON_MODULE_EXTRA_LIBS=""
   export QMAKESPEC="macx-g++"
   export AF_PYTHON_LIB="-L/Library/Frameworks/Python.framework/Versions/3.2/lib/python3.2/config-3.2m -lpython3.2"
fi
# export AF_PYTHON_MODULE_EXTRA_LIBS="-lrt -lz -lgthread -lglib"
# export AF_PYTHON_MODULE_EXTRA_LIBS="-lrt -lz -lgthread-2.0 -lglib-2.0"

# overrides (set custom values there):
[ -f override.sh ] && source override.sh

# Add all python symbols to let user to use to use dynamic python modules
export AF_PYTHON_LIB="-rdynamic ${AF_PYTHON_LIB}"

echo "AF_PYTHON_INC=$AF_PYTHON_INC"
echo "AF_PYTHON_LIB=$AF_PYTHON_LIB"

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

# Copy binaries from apps on mac:
if [ `uname` == "Darwin" ]; then
   cd ../../bin
   for app in `find . -name "*.app"`; do
      cp -v $app/Contents/MacOS/* .
   done
fi
