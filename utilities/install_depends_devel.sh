#!/bin/bash

# Detect Linux distrubution:
source ./distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1

# List packages:

packages="$packages gcc yasm"

# Common for Debian distributives:
function debianArch(){
   packages="$packages g++"
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
   packages="$packages libzip libzip-devel"
   # ImageMagick:
   packages="$packages libjpeg libjpeg-devel"
   packages="$packages libtiff libtiff-devel"
   packages="$packages libpng libpng-devel"
   packages="$packages freetype freetype-devel"
   packages="$packages fontconfig fontconfig-devel"

   pkg_manager_cmd="yum install"
   pkg_extension=".$ARCHITECTURE"

   # openSUSE:
   if [ $DISTRIBUTIVE == "openSUSE" ]; then
      pkg_manager_cmd="zypper install"
   fi
}

# Case distribution:
case ${DISTRIBUTIVE} in
   Debian)
      debianArch
      ;;
   Ubuntu)
      debianArch
      ;;
   *)
      redhatArch
      ;;
esac

cmd="$pkg_manager_cmd"
for package in $packages; do
   cmd="$cmd $package$pkg_extension"
done

echo $cmd
$cmd
