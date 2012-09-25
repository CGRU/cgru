#!/bin/bash

output="$1"
install_cgru="install_cgru.sh"
install_afserver="install_afserver.sh"
install_afrender="install_afrender.sh"
uninstall="uninstall.sh"
template="install_template"

packages_cgru="cgru-common afanasy-common afanasy-render afanasy-qtgui cgru"
packages_afrender="cgru-common afanasy-common afanasy-render"
packages_afserver="cgru-common afanasy-common afanasy-server"
packages_uninstall="cgru afanasy-qtgui afanasy-render afanasy-server afanasy-common cgru-common"

[ -z "${PACKAGE_FORMAT}" ] && source ../distribution.sh
[ -z "${PACKAGE_FORMAT}" ] && exit 1

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
cp -f "${template}" "${output}/${install_afrender}"
cp -f "${template}" "${output}/${uninstall}"

if [ -z "$PACKAGE_FORMAT" ]; then
	echo "Package manager is not set (PACKAGE_FORMAT variable is empty)."
	exit 1
elif [ "$PACKAGE_FORMAT" == "DPKG" ]; then
	extension=".deb"
	install_cmd="dpkg --install"
	uninstall_cmd="dpkg --remove"
elif [ "$PACKAGE_FORMAT" == "RPM" ]; then
	extension=".rpm"
	install_cmd="rpm --install"
	uninstall_cmd="rpm --erase --nodeps"
else
	echo "Unknown package manager = '$PACKAGE_FORMAT'"
	exit 1
fi

curdir=$PWD
cd "${output}"

function writeInstallDependsAFANASY(){
	if [ ! -z "${DEPENDS_AFANASY}" ]; then
		echo "echo Installing AFANASY dependences:" >> $afile
		echo "${PACKAGE_MANAGER} install ${DEPENDS_AFANASY}" >> $afile
		echo "" >> $afile
	fi
}

function writeInstallDependsQTGUI(){
	if [ ! -z "${DEPENDS_QTGUI}" ]; then
		echo "echo Installing QTGUI dependences:" >> $afile
		echo "${PACKAGE_MANAGER} install ${DEPENDS_QTGUI}" >> $afile
		echo "" >> $afile
	fi
}

function writeInstallDependsCGRU(){
	if [ ! -z "${DEPENDS_CGRU}" ]; then
		echo "echo Installing CGRU dependences:" >> $afile
		echo "${PACKAGE_MANAGER} install ${DEPENDS_CGRU}" >> $afile
		echo "" >> $afile
	fi
}

function writeInstallPackages(){
	for package in $*; do
		package_file=`bash -c "ls ${package}*${extension}"`
		for package_file in $package_file; do break; done
		if [ -z "${package_file}" ]; then
			echo "Error: Package '${package}' does not exists."
			exit 1
		fi
		echo "echo Installing ${package_file}" >> $afile
		echo "${install_cmd} ${package_file}" >> $afile
	done
}

afile=${install_cgru}
writeInstallDependsAFANASY
writeInstallDependsQTGUI
writeInstallDependsCGRU
writeInstallPackages ${packages_cgru}

afile=${install_afrender}
writeInstallDependsAFANASY
writeInstallPackages ${packages_afrender}

afile=${install_afserver}
writeInstallDependsAFANASY
writeInstallPackages ${packages_afserver}

for package in ${packages_uninstall}; do
	echo "${uninstall_cmd} ${package}" >> "${uninstall}"
done

cd $curdir

chmod a+rwx "${output}/${install_cgru}"
chmod a+rwx "${output}/${install_afrender}"
chmod a+rwx "${output}/${install_afserver}"
chmod a+rwx "${output}/${uninstall}"
