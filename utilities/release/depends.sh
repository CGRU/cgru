#!/bin/bash

[ -z "$DISTRIBUTIVE" ] && source ../distribution.sh

echo "Depends for $DISTRIBUTIVE:"

# Case distribution:
case ${DISTRIBUTIVE} in
   Debian | Ubuntu )
      export DEPENDS_AFANASY="libqt4-core, libqt4-gui, libqt4-network, libqt4-xml, libqt4-sql, libqt4-sql-psql"
      export DEPENDS_CGRU="python-qt4"
      ;;
   openSUSE )
      export DEPENDS_CGRU="python-qt4"
      ;;
   AltLinux )
      export DEPENDS_CGRU="python-module-PyQt4"
      ;;
    *)
      export DEPENDS_CGRU="PyQt4"
      ;;
esac

[ -z "$DEPENDS_AFANASY" ] || echo "DEPENDS_AFANASY = '$DEPENDS_AFANASY'"
[ -z "$DEPENDS_CGRU"    ] || echo "DEPENDS_CGRU = '$DEPENDS_CGRU'"
