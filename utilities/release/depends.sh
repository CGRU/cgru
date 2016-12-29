#!/bin/bash

[ -z "$DISTRIBUTIVE" ] && source ../distribution.sh

echo "Depends for $DISTRIBUTIVE:"

# Case distribution:
case ${DISTRIBUTIVE} in
	Ubuntu | Mint )
		export DEPENDS_AFANASY="libpq5"
		export DEPENDS_CGRU="python3-pyside imagemagick"
		export DEPENDS_QTGUI="libqt5network5 libqt5widgets5 libqt5multimedia5"
		if [[ "$DISTRIBUTIVE_VERSION" < "16" ]]; then
			export DEPENDS_QTGUI="libqt4-core libqt4-gui libqt4-network"
		fi
		;;
	Debian )
		export DEPENDS_AFANASY="libpq5"
		export DEPENDS_CGRU="python3-pyside imagemagick"
		export DEPENDS_QTGUI="libqtcore4 libqtgui4 libqt4-network"
		if [[ "$DISTRIBUTIVE_VERSION" < "9" ]]; then
			export DEPENDS_QTGUI="libqt4-core libqt4-gui libqt4-network"
		else
			export DEPENDS_AFANASY="${DEPENDS_AFANASY} libpython2.7"
		fi
		;;
	Fedora )
		export DEPENDS_AFANASY="libpqxx"
		export DEPENDS_CGRU="python3-PyQt4 ImageMagick"
		;;
	openSUSE )
		export DEPENDS_AFANASY="libpq5"
		export DEPENDS_CGRU="python3-qt4 ImageMagick"
		;;
	CentOS )
		export DEPENDS_AFANASY="postgresql-libs"
		export DEPENDS_CGRU="PyQt4 ImageMagick"
		;;
	AltLinux )
		export DEPENDS_AFANASY="libpq5.4"
		export DEPENDS_CGRU="python-module-PyQt4 openexr ImageMagick ffmpeg"
		;;
	Mageia )
		export DEPENDS_AFANASY="libpqxx"
		export DEPENDS_CGRU="python3-pyside imagemagick ffmpeg"
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
