#!/bin/bash

for arg in "$@"; do
    if [ $arg == "--skipcheck" ]; then
        check="--exitsuccess"
    fi
done

# Location:
cgruRoot="../.."

function rcopy(){ rsync -rL --exclude '.svn' --exclude '__pycache__' --exclude '*.pyc' $1 $2; }

# Version and revision:
export VERSION_NUMBER=`cat $cgruRoot/version.txt`
pushd $cgruRoot/utilities > /dev/null
popd > /dev/null

# Disrtibutive variables:
source ../distribution.sh
[ -z "${DISTRIBUTIVE}" ] && exit 1
# Disrtibutive depend packages variables:
source ./depends.sh

# Check:
./check.sh $check
if [ "$?" != "0" ]; then
   echo "Some required binaries not founded. Use \"--skipcheck\" argument to skip it."
   exit 1
fi

# Temporary directory
tmpdir="tmp"
if [ -d $tmpdir ]; then
   echo "Removing old temporary directory '$tmpdir'"
   rm -rf $tmpdir
fi
mkdir -p $tmpdir
chmod a+rwx $tmpdir

# Processing icons:
./process_icons.sh

# Exporting CGRU:
cgruExp="cgru_export"
cgruExp=$tmpdir/$cgruExp
if [ -d $cgruExp ]; then
   echo "Removing old export directory '$cgruExp'"
   rm -rf $cgruExp
fi
echo "Exporting '$cgruRoot' to '$cgruExp'..."
./export.sh $cgruExp

#
# Creating Packages:
installdir="/opt/cgru"

if [ -z "$PACKAGE_FORMAT" ]; then
   echo "Packages format is not set (PACKAGE_FORMAT variable is empty)."
   exit 1
elif [ "$PACKAGE_FORMAT" == "DPKG" ]; then
   echo "Creating DEBIAN packages..."
elif [ "$PACKAGE_FORMAT" == "RPM" ]; then
   echo "Creating RPM packages..."
else
   echo "Unknown packages format = '$PACKAGE_FORMAT'"
   exit 1
fi
echo "Packages format = '${PACKAGE_FORMAT}'"

# packages output directoty:
packages_output_dir="output"
[ -d $packages_output_dir ] && rm -rf $packages_output_dir
mkdir $packages_output_dir
chmod a+rwx $packages_output_dir

# Walk in every package folder:
packages_dirs="$cgruRoot/afanasy/package $cgruRoot/utilities/release/package"
for packages_dir in $packages_dirs; do
   packages=`ls "${packages_dir}"`
   for package in $packages; do
      [ -d "${packages_dir}/${package}" ] || continue
      # check copy script:
      copy_script="${packages_dir}/${package}.sh"
      [ -f $copy_script ] || continue
      echo "Creating package '$package':"
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
      if [ "$PACKAGE_FORMAT" == "DPKG" ]; then
         # copy DEBIAN folder:
         rcopy "${packages_dir}/${package}/DEBIAN" "${tmpdir}/${package}"
         # replace variables:
         ./replacevars.sh ${packages_dir}/${package}/DEBIAN/control ${tmpdir}/${package}/DEBIAN/control
         # build package:
         dpkg-deb -b "${tmpdir}/${package}" "${packages_output_dir}/${package}.${VERSION_NUMBER}_${VERSION_NAME}.deb"
      elif [ "$PACKAGE_FORMAT" == "RPM" ]; then
         # copy RPM folder:
         rcopy "${packages_dir}/${package}/RPM" "${tmpdir}/${package}"
         # replace variables:
         ./replacevars.sh  "${packages_dir}/${package}/RPM/SPECS/${package}.spec" "${tmpdir}/${package}/RPM/SPECS/${package}.spec"
         # create required folders:
         mkdir -v "${tmpdir}/${package}/RPM/BUILD"
         mkdir -v "${tmpdir}/${package}/RPM/RPMS"
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
archive_name="cgru.${VERSION_NUMBER}.${VERSION_NAME}.tar.gz"
curdir=$PWD
cd "${packages_output_dir}"
tar -cvzf "${archive_name}" *
mv "${archive_name}" "${curdir}/"
cd "${curdir}"
chmod a+rwx "${archive_name}"

# Copmleted.
chmod -R a+rwx "${tmpdir}"
chmod -R a+rwx "${packages_output_dir}"
echo "Done."; exit 0
