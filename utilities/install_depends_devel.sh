#!/bin/bash

# Detect Linux distrubution:
source ./distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1

# List packages:

packages="yasm p7zip ruby wget inkscape cmake subversion"
packages_noarch="rubygems"

# Packages for Debian distributives:
function debianArch(){
   packages="$packages vim"
   packages="$packages g++"
   packages="$packages python-dev"
   packages="$packages libpq-dev"
   packages="$packages qt4-dev-tools"
   packages="$packages python-qt4"
   packages="$packages p7zip-full"
   packages="$packages git-core"
   packages="$packages libzip-dev"
   # ImageMagick:
   #packages="$packages libjpeg62 libjpeg62-dev"
   packages="$packages libtiff4 libtiff4-dev"
   packages="$packages libpng12-0 libpng12-dev"
   packages="$packages libfreetype6 libfreetype6-dev"
   packages="$packages libfontconfig1 libfontconfig1-dev"

   pkg_manager_cmd="apt-get install"
   pkg_extension=""
}

# Packages for RedHat distributives:
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
   # ImageMagick:
   packages="$packages libjpeg libjpeg-devel"
   packages="$packages libjpeg-turbo libjpeg-turbo-devel"
   packages="$packages libtiff libtiff-devel"
   packages="$packages libpng libpng-devel"
   packages="$packages freetype freetype-devel"
   packages="$packages fontconfig fontconfig-devel"

   pkg_manager_cmd="yum install"
   pkg_extension=".$ARCHITECTURE"
}

# Packages for SUSE distributives:
function suseArch(){
   packages="$packages vim"
   packages="$packages gcc-c++"
   packages="$packages python-devel"
   packages="$packages postgresql-devel"
   packages="$packages libqt4-devel"
   packages="$packages python-qt4"
   packages="$packages libzip libzip-devel git"
   packages="$packages rpm-build"
   # ImageMagick:
   packages="$packages libjpeg6 libjpeg-devel"
   packages="$packages libtiff3 libtiff-devel"
   packages="$packages libpng12-0 libpng-devel"
   packages="$packages freetype freetype2 freetype2-devel"
   packages="$packages fontconfig fontconfig-devel"

   pkg_manager_cmd="zypper install"
   pkg_extension=".$ARCHITECTURE"
}

# Packages for AltLinux distributives:
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
   # ImageMagick:
   packages="$packages libjpeg libjpeg-devel"
   packages="$packages libtiff libtiff-devel"
   packages="$packages libpng12 libpng-devel"
   packages="$packages libfreetype libfreetype-devel"
   packages="$packages fontconfig fontconfig-devel"

   pkg_manager_cmd="apt-get install"
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
