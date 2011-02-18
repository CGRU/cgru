#!/bin/bash

# Location:
cgruRoot="../.."

function rcopy(){ rsync -rL --exclude '.svn' --exclude '*.pyc' $1 $2; }

# Version and revision:
packsver=`cat $cgruRoot/version.txt`
pushd $cgruRoot/utilities > /dev/null
packsrev=`python ./getrevision.py ..`
popd > /dev/null
echo "CGRU $packsver rev$packsrev"

# Afanasy branch:
afanasy="trunk"

# Disrtibutive variables:
source ../distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1

# Function to print usage and exit:
function usage(){
   if [ -n "$ErrorMessage" ]; then
      echo "ERROR: $ErrorMessage"
   fi
   echo "Usage:"
   echo "   `basename $0` afanasy_branch=\"${afanasy}\" [version_number=\"${packsver}\"]"
   echo "Example:"
   echo "   `basename $0` ${afanasy} ${packsver}"
   exit
}

# Afanasy location:
[ ! -z "$1" ] && afanasy=$1
afanasy="afanasy/${afanasy}"
if [ ! -d $cgruRoot/$afanasy ]; then
   ErrorMessage="Afanasy directory '$cgruRoot/$afanasy' does not exists."
   usage
fi

# Packages version number:
[ ! -z "$2" ] && packsver=$2
export VERSION_NUMBER=$packsver

# Temporary directory
tmpdir="tmp"
if [ -d $tmpdir ]; then
   echo "Removing old temporary directory '$tmpdir'"
   rm -rf $tmpdir
fi
mkdir -p $tmpdir
chmod a+rwx $tmpdir

# Exporting CGRU:
cgruExp="cgru_export"
cgruExp=$tmpdir/$cgruExp
if [ -d $cgruExp ]; then
   echo "Removing old export directory '$cgruExp'"
   rm -rf $cgruExp
fi
echo "Exporting '$cgruRoot' to '$cgruExp'..."
#svn export $cgruRoot $cgruExp
./export.sh $cgruExp $afanasy

# Writing version and revision:
#echo "${packsver} rev${packsrev}" > $cgruExp/version.txt

# Copying general components:
cgruAll="__all__"
if [ -d $cgruAll ]; then
   echo "Copying components from '$cgruAll'"
   rsync -rL $cgruAll/* $cgruExp
fi

# Processing icons:
./process_icons.sh $afanasy

#echo "Extracting Afanasy components from '$afanasy' to 'afanasy'..."
#
#cp -rp $cgruExp/$afanasy/* $cgruExp/afanasy
#rm -rf $cgruExp/afanasy/branches
#rm -rf $cgruExp/afanasy/trunk
#rm -rf $cgruExp/afanasy/tags

#
# Creating Packages:
installdir="/opt/cgru"

if [ -z "$PACKAGE_MANAGER" ]; then
   echo "Package manager is not set (PACKAGE_MANAGER variable is empty)."
   exit 1
elif [ "$PACKAGE_MANAGER" == "DPKG" ]; then
   echo "Creating DEBIAN packages..."
elif [ "$PACKAGE_MANAGER" == "RPM" ]; then
   echo "Creating RPM packages..."
else
   echo "Unknown package manager = '$PACKAGE_MANAGER'"
   exit 1
fi

# packages output directoty:
packages_output_dir="output"
[ -d $packages_output_dir ] && rm -rf $packages_output_dir
mkdir $packages_output_dir
chmod a+rwx $packages_output_dir

# copy current afanasy binaries from branch:
#echo "Copying current afanasy binaries..."
#if [ ! -d $cgruRoot/$afanasy/bin ]; then
#   ErrorMessage="No afanasy binaries directory founded '$cgruRoot/$afanasy/bin'."
#   usage
#fi
#cp -rpv $cgruRoot/$afanasy/bin $cgruExp/afanasy
#[ -d $cgruRoot/$afanasy/bin_pyaf ] && cp -rpv $cgruRoot/$afanasy/bin_pyaf $cgruExp/afanasy

# Walk in every package folder:
packages_dirs="$cgruExp/afanasy/package $cgruExp/utilities/release/package"
packages_dirs="$cgruRoot/$afanasy/package $cgruRoot/utilities/release/package"
#packages_dirs="$cgruExp/utilities/release/package"
for packages_dir in $packages_dirs; do
   packages=`ls "${packages_dir}"`
   for package in $packages; do
      [ -d "${packages_dir}/${package}" ] || continue
      # check copy script:
      copy_script="${packages_dir}/${package}.sh"
      [ -f $copy_script ] || continue
      # copy files for package, but not control folders:
      mkdir -p ${tmpdir}/${package}
      for folder in `ls "${packages_dir}/${package}"`; do
         [ "${folder}" == "RPM" ] && continue
         [ "${folder}" == "DEBIAN" ] && continue
         folder="${packages_dir}/${package}/${folder}"
         rcopy "${folder}" "${tmpdir}/${package}"
      done
      # apply copy script, for common files:
      $copy_script $cgruExp $tmpdir/$package $installdir

#continue
      # count package size:
      for i in `du -sb0 ${tmpdir}/${package}`; do size=$i; break; done
      [ -z $size ] || export SIZE=$size

      # perform package manager specific operations:
      if [ "$PACKAGE_MANAGER" == "DPKG" ]; then
         # copy DEBIAN folder:
         rcopy "${packages_dir}/${package}/DEBIAN" "${tmpdir}/${package}"
         # replace variables:
         ./replacevars.sh ${packages_dir}/${package}/DEBIAN/control ${tmpdir}/${package}/DEBIAN/control
         # build package:
         dpkg-deb -b "${tmpdir}/${package}" "${packages_output_dir}/${package}.${VERSION_NUMBER}_${VERSION_NAME}.deb"
      elif [ "$PACKAGE_MANAGER" == "RPM" ]; then
         # copy RPM folder:
         rcopy "${packages_dir}/${package}/RPM" "${tmpdir}/${package}"
         # replace variables:
         ./replacevars.sh  "${packages_dir}/${package}/RPM/SPECS/${package}.spec" "${tmpdir}/${package}/RPM/SPECS/${package}.spec"
         # launch rpm build script:
         curdir=$PWD
         cd ${tmpdir}/${package}
         rpmbuild -bb "RPM/SPECS/${package}.spec" --buildroot "${PWD}/RPM/BUILDROOT"
         cd $curdir
         # move package from RPM build directories structure:
         for folder in `ls ${tmpdir}/${package}/RPM/RPMS`; do
            mv -f ${tmpdir}/${package}/RPM/RPMS/${folder}/* "${packages_output_dir}"
         done
      fi
      echo "   Size = $size"
#break
   done
done

# Create install & uninstall scripts:
./install_create.sh "${packages_output_dir}"

# Create archive:
archive_name="cgru.${packsver}.${VERSION_NAME}.tar.gz"
curdir=$PWD
cd "${packages_output_dir}"
tar -cvzf "${archive_name}" *
mv "${archive_name}" "${curdir}/"
cd "${curdir}"
chmod a+rwx "${archive_name}"

# Creating 7zip releazes archives:
releases="__releases__"
if [ -d ${releases} ]; then
   echo "Creating 7zip archives with all CGRU files..."
   releasesnames=`ls "${releases}"`
   for release in $releasesnames; do
      releasedir="${releases}/$release"
      [ -d $releasedir ] || continue
      tmp="$tmpdir/${release}/cgru"
      mkdir -p $tmp
      echo "Creating CGRU archive for ${release}..."
      cp -rp $cgruExp/* $tmp
      [ ! -z "`ls $releasedir`" ] && rsync -rL --inplace $releasedir/* $tmp
      tmp=$PWD
      cd $tmpdir/${release}
      acrhivename="../../${releases}/cgru.${VERSION_NUMBER}.${release}.7z"
      [ -f $acrhivename ] && rm -fv $acrhivename
      7za a -r -y -t7z "${acrhivename}" "cgru" | grep "/bin/"
      [ $? != 0 ] && echo "Failed!"
      chmod a+rw "${acrhivename}"
      cd $tmp
   done
fi

# Copmleted.
chmod -R a+rwx "${tmpdir}"
chmod -R a+rwx "${packages_output_dir}"
echo "Completed"; exit 0
