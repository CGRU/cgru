#!/bin/bash

output="$1"
install_cgru="install_cgru.sh"
install_afserver="install_afserver.sh"
uninstall="uninstall.sh"
template="install_template"

packages_cgru="afanasy-common cgru-common afanasy-plugins afanasy-render afanasy-examples afanasy-doc afanasy-gui cgru"
packages_afserver="afanasy-common afanasy-server"
packages_uninstall="cgru afanasy-server afanasy-render afanasy-gui afanasy-doc afanasy-examples afanasy-plugins afanasy-common cgru-common"

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
   uninstall_cmd="rpm --erase"
else
   echo "Unknown package manager = '$PACKAGE_MANAGER'"
   exit 1
fi

curdir=$PWD
cd "${output}"

function assemblecmd(){
   cmd="${install_cmd}"
   for package in $*; do
      package_file=`bash -c "ls ${package}*${extension}"`
      for package_file in $package_file; do break; done
      if [ -z "${package_file}" ]; then
         echo "Error: Package '${package}' does not exists."
         exit 1
      fi
      cmd="${cmd} ${package_file}"
   done
}

assemblecmd ${packages_cgru}
echo "${cmd}" >> "${install_cgru}"

assemblecmd ${packages_afserver}
echo "${cmd}" >> "${install_afserver}"

echo "${uninstall_cmd} ${packages_uninstall}" >> "${uninstall}"

cd $curdir

chmod a+rwx "${output}/${install_cgru}"
chmod a+rwx "${output}/${install_afserver}"
chmod a+rwx "${output}/${uninstall}"
