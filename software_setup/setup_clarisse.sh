#!/bin/bash

#
# Source general for all soft directives:
#
source "$CGRU_LOCATION/software_setup/setup__all.sh"

# Try PySide first:
# https://github.com/mottosso/Qt.py
export QT_PREFERRED_BINDING="PySide2:PySide:PyQt4"

#
# Search for Clarisse installation:
#
if [ "`uname`" == "Darwin" ]; then
    CLARISSE_INSTALL_DIR="/Applications"
else
    CLARISSE_INSTALL_DIR="/opt"
fi

CLARISSE_FOLDERS=`ls "$CLARISSE_INSTALL_DIR"`
CLARISSE_LOCATION=""
CLARISSE_EXEC=""
for CLARISSE_FOLDER in $CLARISSE_FOLDERS ;
do
   if [ "`echo $CLARISSE_FOLDER | awk '{print match( \$1, "Clarisse")}'`" == "1" ]; then
      CLARISSE_LOCATION="${CLARISSE_INSTALL_DIR}/${CLARISSE_FOLDER}"
      if [ "`uname`" == "Darwin" ]; then
        CLARISSE_EXEC="${CLARISSE_FOLDER}.app/${CLARISSE_FOLDER}"
      else
        CLARISSE_EXEC="clarisse"
      fi
   fi
done

export CLARISSE_LOCATION="${CLARISSE_LOCATION}/clarisse"
export CLARISSE_EXEC="${CLARISSE_LOCATION}/${CLARISSE_EXEC}"

#
# For export:
#
export APP_DIR=$CLARISSE_LOCATION
export APP_EXE=$CLARISSE_EXEC

#
# Override clarisse location based on locate_clarisse.sh:
#
locate_clarisse="$CGRU_LOCATION/software_setup/locate_clarisse.sh"
[ -f $locate_clarisse ] && source $locate_clarisse

echo "CLARISSE = '${APP_EXE}'"
