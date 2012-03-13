#!/bin/bash

# CGRU location:
cgruRoot="../.."

function usage(){
   if [ -n "$ErrorMessage" ]; then
      echo "ERROR: $ErrorMessage"
   fi
   echo "Usage:"
   echo "`basename $0` AFANASY_BRANCH DISTRVARS_FILE VERSION_NUMBER"
   exit
}

# Afanasy location:
afanasy=$1
if [ -z $afanasy ]; then
   ErrorMessage="Afanasy branch '$afanasy' not specitied."
   usage
fi
afanasy="afanasy/${afanasy}"
if [ ! -d $cgruRoot/$afanasy ]; then
   ErrorMessage="Afanasy directory '$cgruRoot/$afanasy' does not exists."
   usage
fi

# Disrtibutive variables:
varsfile=$2
if [ -z $varsfile ]; then
   ErrorMessage="Disrtibutive variables file not specitied."
   usage
fi
if [ ! -f $varsfile ]; then
   ErrorMessage="Disrtibutive variables file '$varsfile' does not exists."
   usage
fi
source $varsfile

# Packages version number:
packsver=$3
if [ -z $packsver ]; then
   ErrorMessage="Packages version number is not set."
   usage
fi
export VERSION_NUMBER=$packsver

# CGRU general components:
cgruAll="__all__"

# Releases binary files directory:
releases="__releases__"

# Directory to export CGRU Subversion:
cgruExp="cgru_export"

# Debian packages:
debpackages="debpackages"
installdir="/opt/cgru"

# Temporary directory
tmpdir="tmp"
if [ -d $tmpdir ]; then
   echo "Removing old temporary directory '$tmpdir'"
   rm -rf $tmpdir
fi
mkdir -p $tmpdir

# Exporting CGRU:
cgruExp=$tmpdir/$cgruExp
if [ -d $cgruExp ]; then
   echo "Removing old export directory '$cgruExp'"
   rm -rf $cgruExp
fi
echo "Exporting '$cgruRoot' to '$cgruExp'..."
svn export $cgruRoot $cgruExp

# Copying general components (__all__)
if [ -d $cgruAll ]; then
   echo "Copying components from '$cgruAll'"
   cp -rp $cgruAll/* $cgruExp
fi

#
echo "Processing icons:"
#
# Icons directories:
iconsdirssrc="\
utilities/release/icons \
doc/icons utilities/regexp/icons \
utilities/moviemaker/icons \
$afanasy/icons \
$afanasy/doc/icons \
"
for iconsdir in $iconsdirssrc; do
   src="$cgruRoot/$iconsdir"
   if [ ! -d "$src" ]; then
      ErrorMessage="Icons source folder '$src' does not exists."
      usage
   fi
   dest="$cgruExp/$iconsdir"
   if [ ! -d "$dest" ]; then
      ErrorMessage="Icons destination folder '$dest' does not exists."
      usage
   fi
   # Generate png icons from svg if was not:
   if [ ! -d "$src/icons" ]; then
      echo "Generating icons in '$src':"
      tmp=$PWD
      cd $src
      ./make.sh   
      cd $tmp
   fi
   echo "Copying '$iconsdir' icons"
   cp -r "$src/icons" "$dest"
done

#
echo "Extracting Afanasy components from '$afanasy' to 'afanasy'..."
#
cp -rp $cgruExp/$afanasy/* $cgruExp/afanasy
rm -rf $cgruExp/afanasy/branches
rm -rf $cgruExp/afanasy/trunk

# Creating ZIP archives:
if [ -d ${releases} ]; then
   echo "Creating ZIP archives with all CGRU files.."
   releasesnames=`ls "${releases}"`
   for release in $releasesnames; do
      releasedir="${releases}/$release"
      [ -d $releasedir ] || continue
      tmp="$tmpdir/${release}/cgru"
      mkdir -p $tmp
      echo "Creating CGRU archive for ${release}..."
      cp -rp $cgruExp/* $tmp
      cp -rp $releasedir/* $tmp
      tmp=$PWD
      cd $tmpdir/${release}
      zip -qr "../../${releases}/cgru.${VERSION_NUMBER}.${release}.zip" "cgru"
      cd $tmp
   done
fi
#
echo "Creating debian packages..."
#
[ -d $debpackages ] && rm -rf $debpackages
mkdir $debpackages
echo "Copying current afanasy binaries..."
if [ ! -d $cgruRoot/$afanasy/bin ]; then
   ErrorMessage="No afanasy binaries directory founded '$cgruRoot/$afanasy/bin'."
   usage
fi
cp -rp $cgruRoot/$afanasy/bin $cgruExp/afanasy

# Debian packages control:
debianall="$cgruExp/afanasy/debian $cgruExp/utilities/release/debian"
for debian in $debianall; do
   packages=`ls "${debian}"`
   for package in $packages; do
      [ -d "${debian}/${package}" ] || continue
      commands="${debian}/${package}.sh"
      [ -f $commands ] || continue
      # copy package stucture:
      cp -rp "${debian}/${package}" "${tmpdir}"
      # perform specific commands:
      $commands $cgruExp $tmpdir/$package $installdir $cgruRoot
      # count package size:
      for i in `du -sb0 ${tmpdir}/${package}`; do size=$i; break; done
      [ -z $size ] || export SIZE=$size
      # replace variables:
      ./replacevars.sh ${debian}/${package}/DEBIAN/control ${tmpdir}/${package}/DEBIAN/control
      # build package:
      dpkg-deb -b "${tmpdir}/${package}" "${debpackages}/${package}.${VERSION_NUMBER}_${VERSION_NAME}.deb"
      echo "   Size = $size"
   done
done

echo "Completed"; exit 0
