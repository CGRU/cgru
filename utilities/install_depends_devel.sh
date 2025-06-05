#!/usr/bin/env bash

# Detect Linux distrubution:
source ./distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1

# List packages:

#packages="yasm p7zip wget inkscape cmake subversion"
packages="inkscape cmake subversion"
packages_noarch=""

# Packages for Debian distributions:
function debianArch(){
	packages="$packages vim"
	packages="$packages g++"
	packages="$packages git-core"
	packages="$packages python3-dev"
	packages="$packages python3-pyqt5"
	packages="$packages libpq-dev"
	packages="$packages make"
	packages="$packages p7zip-full"
	packages="$packages libzip-dev"
	packages="$packages rsync"

	# Qt5:
	packages="$packages qtbase5-dev"
	packages="$packages qtmultimedia5-dev"

	pkg_manager_cmd="apt-get install"
	pkg_extension=""
}

# Packages for RedHat distributions:
function redhatArch(){
	packages="$packages vim-minimal"
	packages="$packages gcc-c++"
	packages="$packages git"
	packages="$packages python3-devel"
	packages="$packages libpq-devel"
	packages="$packages python3-qt5"
	packages="$packages qt5-qtbase-devel"
	packages="$packages qt5-qtmultimedia-devel"
	#packages="$packages libzip libzip-devel"
	packages="$packages rpm-build"

	pkg_manager_cmd="yum install"
	pkg_extension=".$ARCHITECTURE"
}

function fedoraArch(){
	packages="$packages vim"
	packages="$packages gcc-c++"
	packages="$packages libpq-devel"
	packages="$packages qt5-qtmultimedia-devel"
	packages="$packages rpm-build"
	packages="$packages git"
	packages="$packages python3-pyside2"
	packages="$packages libzip libzip-devel"
	packages="$packages python3-devel"
	packages="$packages OpenEXR-devel"

	pkg_manager_cmd="dnf install"
	pkg_extension=""
}

# Packages for SUSE distributions:
function suseArch(){
	packages="$packages vim"
	packages="$packages gcc-c++"
	packages="$packages python3-devel"
	packages="$packages postgresql-devel"
	packages="$packages openexr-devel"
	packages="$packages libqt5-qtbase-devel"
	packages="$packages libqt5-qtmultimedia-devel"
	packages="$packages python3-qt5"
	packages="$packages libzip libzip-devel git"
	packages="$packages rpm-build"

	pkg_manager_cmd="zypper install"
	pkg_extension=""
}

# Packages for AltLinux distributions:
function altArch(){
	packages="$packages vim-console"
	packages="$packages gcc5-c++"
	packages="$packages git-core"
	packages="$packages python3-dev"
	packages="$packages python3-module-PyQt5"
	packages="$packages postgresql-devel"
	packages="$packages qt5-base-devel"
	packages="$packages libEGL-devel"
	packages="$packages qt5-multimedia-devel"
	packages="$packages libzip5 libzip-devel"
	packages="$packages openexr-devel"
	packages="$packages rpm-build"

	pkg_manager_cmd="apt-get install"
	pkg_extension=""
}

# Packages for Mageia distributions:
function mageiaArch(){
	packages="$packages vim-minimal"
	packages="$packages git"
	packages="$packages gcc-c++"
	packages="$packages lib64python3-devel"
	packages="$packages python3-pyside"
	packages="$packages postgresql-devel"
	packages="$packages qt5-devel lib64qt5multimedia-devel"
	packages="$packages rpm-build"
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

# Packages for Arch Linux distributions:
function bsdArch(){
	packages="$packages neovim"
	packages="$packages lang/python"
	packages="$packages py39-cython-devel"
	packages="$packages postgresql-libpqxx"
	packages="$packages devel/qt6"
	packages="$packages py39-pyside6-6.6.2"
	#packages="$packages libsysinfo"

	pkg_manager_cmd="pkg install"
	pkg_extension=""
}

# Packages for MacOSX
function macosxArch(){
	packages="yasm"
	packages="$packages cmake"
	packages="$packages wget"
	packages="$packages p7zip"
	packages="$packages openexr"
	packages="$packages imagemagick"
	packages="$packages postgresql"
	packages="$packages qt5"
	packages="$packages python3"
	packages="$packages pyqt5"

	if hash brew 2>/dev/null; then
		pkg_manager_cmd="brew install"
	else
		pkg_manager_cmd="port install"
	fi

	pkg_extension=""
}


# Case distribution:
case ${DISTRIBUTIVE} in
	AltLinux)
		altArch
		;;
	Debian|Ubuntu|Mint|Astra)
		debianArch
		;;
	openSUSE)
		suseArch
		;;
	Mageia|ROSA)
		mageiaArch
		;;
	Fedora)
		fedoraArch
		;;
	Arch|Manjaro)
		archArch
		;;
	MacOSX)
		macosxArch
		;;
	FreeBSD)
		bsdArch
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
$cmd "$@"
