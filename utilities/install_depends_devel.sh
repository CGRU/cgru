#!/bin/bash

# Detect Linux distrubution:
source ./distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1

# List packages:

packages="yasm p7zip wget inkscape cmake subversion"
packages_noarch=""

# Packages for Debian distributions:
function debianArch(){
	packages="$packages vim"
	packages="$packages g++"
	packages="$packages git-core"
	packages="$packages python3-dev"
	packages="$packages python3-pyside"
	packages="$packages libpq-dev"
	packages="$packages p7zip-full"
	packages="$packages libzip-dev"
	# Qt4:
	packages="$packages qt4-dev-tools"
	packages="$packages qt4-qmake"
	# Qt5:
	packages="$packages qt5-default"
	packages="$packages qtmultimedia5-dev"
	# Qt5-AFermer:
	packages="$packages libboost-filesystem-dev"
	packages="$packages qtdeclarative5-dev"
	# EXRJoin:
	packages="$packages libopenexr-dev"
	# ImageMagick:
	#packages="$packages libjpeg62 libjpeg62-dev"
	#packages="$packages libtiff5 libtiff5-dev"
	#packages="$packages libpng12-0 libpng12-dev"
	#packages="$packages libfreetype6 libfreetype6-dev"
	#packages="$packages libfontconfig1 libfontconfig1-dev"

	pkg_manager_cmd="apt-get install"
	pkg_extension=""
}

# Packages for RedHat distributions:
function redhatArch(){
	packages="$packages vim"
	packages="$packages gcc-c++"
	packages="$packages python-devel"
	packages="$packages postgresql-devel"
	packages="$packages qt-devel"
	packages="$packages rpm-build"
	packages="$packages git"
	packages="$packages PyQt4"
	packages="$packages libzip libzip-devel"

	pkg_manager_cmd="yum install"
	pkg_extension=".$ARCHITECTURE"
}

function fedoraArch(){
	packages="$packages vim"
	packages="$packages gcc-c++"
	packages="$packages postgresql-devel"
	packages="$packages qt-devel"
	packages="$packages rpm-build"
	packages="$packages git"
	packages="$packages PyQt4"
	packages="$packages libzip libzip-devel"
	if [[ "$DISTRIBUTIVE_VERSION" < "24" ]]; then
		packages="$packages python-devel"
	else
		packages="$packages python3-devel"
	fi

	pkg_manager_cmd="dnf install"
	pkg_extension=""
}

# Packages for SUSE distributions:
function suseArch(){
	packages="$packages vim"
	packages="$packages gcc-c++"
	packages="$packages python-devel"
	packages="$packages postgresql-devel"
	packages="$packages libqt4-devel"
	packages="$packages python-qt4"
	packages="$packages libzip libzip-devel git"
	packages="$packages rpm-build"

	pkg_manager_cmd="zypper install"
	pkg_extension=".$ARCHITECTURE"
}

# Packages for AltLinux distributions:
function altArch(){
	packages="$packages vim-console"
	packages="$packages gcc4.5-c++"
	packages="$packages python-dev"
	packages="$packages postgresql-devel"
	packages="$packages qt4-devel"
	packages="$packages PyQt"
	packages="$packages git-core"
	packages="$packages libzip libzip-devel"
	packages="$packages rpm-build"

	pkg_manager_cmd="apt-get install"
	pkg_extension=""
}

# Packages for Mageia distributions:
function mageiaArch(){
	packages="$packages vim"
	packages="$packages gcc-c++"
	packages="$packages python-devel"
	packages="$packages postgresql-devel"
	packages="$packages qt4-devel-private"
	packages="$packages rpm-build"
	packages="$packages git"
	packages="$packages PyQt4"
	packages="$packages libzip libzip-devel"

	pkg_manager_cmd="urpmi"
	pkg_extension=""
}

# Packages for Arch Linux distributions:
function archArch(){
	packages="$packages vim"
	#packages="$packages gcc-c++"
	#packages="$packages python-devel"
	packages="$packages postgresql"
	#packages="$packages qt4-devel-private"
	#packages="$packages rpm-build"
	#packages="$packages git"
	packages="$packages python-pyqt4"
	#packages="$packages libzip libzip-devel"

	pkg_manager_cmd="pacman -S"
	pkg_extension=""
}


# Case distribution:
case ${DISTRIBUTIVE} in
	AltLinux)
		altArch
		;;
	Debian|Ubuntu|Mint)
		debianArch
		;;
	openSUSE)
		suseArch
		;;
	Mageia)
		mageiaArch
		;;
	Fedora)
		fedoraArch
		;;
	Arch|Manjaro)
		archArch
		;;
	*)
		redhatArch
		;;
esac

cmd="$pkg_manager_cmd"
for package in $packages; do
	cmd="$cmd $package$pkg_extension"
done
for package in $packages_noarch; do
	cmd="$cmd $package"
done

echo $cmd
$cmd
