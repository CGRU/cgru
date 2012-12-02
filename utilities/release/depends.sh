#!/bin/bash

[ -z "$DISTRIBUTIVE" ] && source ../distribution.sh

echo "Depends for $DISTRIBUTIVE:"

# Case distribution:
case ${DISTRIBUTIVE} in
	Debian | Ubuntu | Mint )
		export DEPENDS_AFANASY="libpq5"
		export DEPENDS_QTGUI="libqt4-core libqt4-gui libqt4-network libqt4-xml"
		export DEPENDS_CGRU="python-qt4 imagemagick"
		;;
	Fedora )
		export DEPENDS_AFANASY="libpqxx"
		export DEPENDS_CGRU="PyQt4 ImageMagick"
		;;
	openSUSE )
		export DEPENDS_AFANASY="libpq5"
		export DEPENDS_CGRU="python-qt4 ImageMagick"
		;;
	AltLinux )
		export DEPENDS_AFANASY="libpq5.4"
		export DEPENDS_CGRU="python-module-PyQt4 ImageMagick"
		;;
	CentOS )
		export DEPENDS_AFANASY="postgresql-libs"
		export DEPENDS_CGRU="PyQt4"
		;;
	*)
		export DEPENDS_AFANASY="libpq5"
		export DEPENDS_CGRU="PyQt4"
		;;
esac

for dep in $DEPENDS_AFANASY; do
	[ -z "$DEPENDS_AFANASY_COMMA" ] || DEPENDS_AFANASY_COMMA="${DEPENDS_AFANASY_COMMA}, "
	export DEPENDS_AFANASY_COMMA="${DEPENDS_AFANASY_COMMA}${dep}"
done

for dep in $DEPENDS_QTGUI; do
	[ -z "$DEPENDS_QTGUI_COMMA" ] || DEPENDS_QTGUI_COMMA="${DEPENDS_QTGUI_COMMA}, "
	export DEPENDS_QTGUI_COMMA="${DEPENDS_QTGUI_COMMA}${dep}"
done

for dep in $DEPENDS_CGRU; do
	[ -z "$DEPENDS_CGRU_COMMA" ] || DEPENDS_CGRU_COMMA="${DEPENDS_CGRU_COMMA}, "
	export DEPENDS_CGRU_COMMA="${DEPENDS_CGRU_COMMA}${dep}"
done

[ -z "$DEPENDS_AFANASY" ] || echo "DEPENDS_AFANASY = '$DEPENDS_AFANASY_COMMA'"
[ -z "$DEPENDS_QTGUI"   ] || echo "DEPENDS_QTGUI   = '$DEPENDS_QTGUI_COMMA'"
[ -z "$DEPENDS_CGRU"    ] || echo "DEPENDS_CGRU    = '$DEPENDS_CGRU_COMMA'"
