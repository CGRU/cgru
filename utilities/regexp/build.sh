#!/bin/bash

# Setup Qt (use system by default):
export QMAKE=

# Try CGRU Qt:
cgru_qt="`dirname $PWD`/qt"
if [ -d "$cgru_qt" ]; then
   for folder in `ls $cgru_qt`; do
      if [ "`echo $folder | gawk '{print match( \$1, "qt-4.")}'`" == "1" ]; then
         QMAKE="${cgru_qt}/${folder}/bin/qmake"
         echo "Founded CGRU Qt in '${folder}'"
      fi
   done
fi

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
cd src
$QMAKE

# Make files
make

cd ..
