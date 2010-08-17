#!/bin/bash

output="$1"
install_cgru="install_cgru.sh"
install_afserver="install_afserver.sh"
uninstall="uninstall.sh"
template="install_template"

packages_cgru="afanasy-common cgru-common afanasy-plugins afanasy-render afanasy-doc afanasy-gui cgru"
packages_afserver="afanasy-common afanasy-server"
packages_uninstall="cgru afanasy-gui afanasy-doc afanasy-render afanasy-plugins afanasy-server cgru-common afanasy-common"

[ -z "${PACKAGE_MANAGER}" ] && source ./distribution.sh
[ -z "${PACKAGE_MANAGER}" ] && exit 1

if [ -z "${output}" ]; then
   echo "Error: Output folder not specified."
   echo "Usage: `basename $0` [folder]"
   exit 1
fi

if [ ! -f "${template}" ]; then
   echo "Error: Template file '${template}' not founded."
   exit 1
fi

cp -f "${template}" "${output}/${install_cgru}"
cp -f "${template}" "${output}/${install_afserver}"
cp -f "${template}" "${output}/${uninstall}"

if [ -z "$PACKAGE_MANAGER" ]; then
   echo "Package manager is not set (PACKAGE_MANAGER variable is empty)."
   exit 1
elif [ "$PACKAGE_MANAGER" == "DPKG" ]; then
   extension=".deb"
   install_cmd="dpkg --install"
   uninstall_cmd="dpkg --remove"
elif [ "$PACKAGE_MANAGER" == "RPM" ]; then
   extension=".rpm"
   install_cmd="rpm --install"
   uninstall_cmd="rpm --erase --nodeps"
else
   echo "Unknown package manager = '$PACKAGE_MANAGER'"
   exit 1
fi

curdir=$PWD
cd "${output}"

function writeCommands(){
   for package in $*; do
      package_file=`bash -c "ls ${package}*${extension}"`
      for package_file in $package_file; do break; done
      if [ -z "${package_file}" ]; then
         echo "Error: Package '${package}' does not exists."
         exit 1
      fi
      echo "${install_cmd} ${package_file}" >> $afile
   done
}

afile=${install_cgru}
writeCommands ${packages_cgru}

afile=${install_afserver}
writeCommands ${packages_afserver}

for package in ${packages_uninstall}; do
   echo "${uninstall_cmd} ${package}" >> "${uninstall}"
done

cd $curdir

chmod a+rwx "${output}/${install_cgru}"
chmod a+rwx "${output}/${install_afserver}"
chmod a+rwx "${output}/${uninstall}"
