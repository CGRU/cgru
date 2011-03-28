#!/bin/bash

# Detect Linux distrubution:
source ./distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1

# List packages:

packages="gcc yasm p7zip ruby wget git inkscape"
packages_noarch="rubygems"

# Common for Debian distributives:
function debianArch(){
   packages="$packages g++"
   packages="$packages p7zip-full"
   packages="$packages libzip1 libzip-dev"
   # ImageMagick:
   packages="$packages libjpeg62 libjpeg62-dev"
   packages="$packages libtiff4 libtiff4-dev"
   packages="$packages libpng12-0 libpng12-dev"
   packages="$packages libfreetype6 libfreetype6-dev"
   packages="$packages libfontconfig1 libfontconfig1-dev"

   pkg_manager_cmd="apt-get install"
   pkg_extension=""
}

# Common for RedHat distributives:
function redhatArch(){
   packages="$packages gcc-c++"
   packages="$packages rpm-build"
   packages="$packages libzip libzip-devel"
   # ImageMagick:
   packages="$packages libjpeg libjpeg-devel"
   packages="$packages libtiff libtiff-devel"
   packages="$packages libpng libpng-devel"
   packages="$packages freetype freetype-devel"
   packages="$packages fontconfig fontconfig-devel"

   pkg_manager_cmd="yum install"
   pkg_extension=".$ARCHITECTURE"
}

# Common for SUSE distributives:
function suseArch(){
   packages="$packages gcc-c++"
   packages="$packages libzip1 libzip-devel"
   # ImageMagick:
   packages="$packages libjpeg6 libjpeg-devel"
   packages="$packages libtiff3 libtiff-devel"
   packages="$packages libpng12-0 libpng-devel"
   packages="$packages freetype freetype2 freetype2-devel"
   packages="$packages fontconfig fontconfig-devel"

   pkg_manager_cmd="zypper install"
   pkg_extension=".$ARCHITECTURE"
}

# Case distribution:
case ${DISTRIBUTIVE} in
   Debian)
      debianArch
      ;;
   Ubuntu)
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
