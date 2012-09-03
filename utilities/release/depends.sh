#!/bin/bash

[ -z "$DISTRIBUTIVE" ] && source ../distribution.sh

echo "Depends for $DISTRIBUTIVE:"

# Case distribution:
case ${DISTRIBUTIVE} in
   Debian | Ubuntu | Mint )
      export DEPENDS_AFANASY="libqt4-core libqt4-gui libqt4-network libqt4-xml libpq5"
      export DEPENDS_CGRU="python-qt4 imagemagick"
      ;;
   Fedora )
      export DEPENDS_CGRU="PyQt4 ImageMagick"
      ;;
   openSUSE )
      export DEPENDS_CGRU="python-qt4 ImageMagick libpq5"
      ;;
   AltLinux )
      export DEPENDS_CGRU="python-module-PyQt4 libpq5.4 ImageMagick"
      ;;
    *)
      export DEPENDS_CGRU="PyQt4"
      ;;
esac

for dep in $DEPENDS_AFANASY; do
    [ -z "$DEPENDS_AFANASY_COMMA" ] || DEPENDS_AFANASY_COMMA="${DEPENDS_AFANASY_COMMA}, "
    export DEPENDS_AFANASY_COMMA="${DEPENDS_AFANASY_COMMA}${dep}"
done

for dep in $DEPENDS_CGRU; do
    [ -z "$DEPENDS_CGRU_COMMA" ] || DEPENDS_CGRU_COMMA="${DEPENDS_CGRU_COMMA}, "
    export DEPENDS_CGRU_COMMA="${DEPENDS_CGRU_COMMA}${dep}"
done

[ -z "$DEPENDS_AFANASY" ] || echo "DEPENDS_AFANASY = '$DEPENDS_AFANASY_COMMA'"
[ -z "$DEPENDS_CGRU"    ] || echo "DEPENDS_CGRU    = '$DEPENDS_CGRU_COMMA'"
