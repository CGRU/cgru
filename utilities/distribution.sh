#!/usr/bin/env bash

echo "Detecting UNIX distribution..."

distskeys="Debian Ubuntu Rocky CentOS Red Fedora openSUSE Simply Gentoo Mint SUSE Mageia Arch Manjaro ROSA Astra FreeBSD"
knowndists="Debian Ubuntu Rocky CentOS RedHat Fedora openSUSE AltLinux MacOSX Gentoo Mint SUSE Mageia Arch Manjaro ROSA Astra FreeBSD"

# MacOSX
if [ `uname` == "Darwin" ]; then
	export DISTRIBUTIVE="MacOSX"
	export DISTRIBUTIVE_VERSION=$(sw_vers -productVersion)
fi

# UNIX-s os-release file:
osreleasefile="/etc/os-release"
if [ -z "${DISTRIBUTIVE}" ] && [ -f "${osreleasefile}" ]; then
	source "${osreleasefile}"
	for distr in $distskeys; do
		if [ `eval "echo \"${ID}\" | awk '{ print match(\\$0,tolower(\"${distr}\"))}'"` != "0" ]; then
			export DISTRIBUTIVE="${distr}"
			export DISTRIBUTIVE_VERSION="${VERSION_ID}"
			break
		fi
	done
fi

# Linuxes issue file:
if [ -z "${DISTRIBUTIVE}" ]; then
	# Load issue file:
	issuefile="/etc/system-release"
	[ -f "${issuefile}" ] || issuefile="/etc/gentoo-release" ; [ -f "${issuefile}" ] || issuefile="/etc/issue"
	if [ ! -f "${issuefile}" ]; then
		echo "File '${issuefile}' not found. Can't detect distribution."
		exit 1
	fi

	# Search issue file:
	for distr in $distskeys; do
		issue=`cat "${issuefile}" | grep -i "${distr}"`
		[ -z "${issue}" ] && continue
		if [ `eval "echo \"${issue}\" | awk '{ print match(\\$0,\"${distr}\")}'"` != "0" ]; then
			export DISTRIBUTIVE="${distr}"
			break
		fi
	done

	# Search distributive version
	export DISTRIBUTIVE_VERSION=`echo "${issue}" | awk '{match($0,"[0-9.-]+"); print substr($0,RSTART,RLENGTH)}'`
	# Search distribution version for Manjaro
	if [ "${DISTRIBUTIVE}" == "Manjaro" ]; then
		export DISTRIBUTIVE_VERSION=`echo "/etc/lsb-release" | awk '{match($0,"[0-9.-]+"); print substr($0,RSTART,RLENGTH)}'`
	fi

	# No distributive version found
	if [ -z "${DISTRIBUTIVE_VERSION}" ]; then
		echo "Can't detect ${DISTRIBUTIVE} version. You can:"
		echo "export DISTRIBUTIVE_VERSION="
		exit 1
	fi

fi

# Still no distributive found
if [ -z "${DISTRIBUTIVE}" ]; then
	echo "Unsupported distribution:"
	cat "${issuefile}"
	echo "Supported distributions:"
	echo "${knowndists}"
	exit 1
fi

# Check architecture:
export ARCHITECTURE=`uname -m`

# Common for Debian distributives:
function debianArch(){
	if [ "${ARCHITECTURE}" == "x86_64" ]; then
		export ARCHITECTURE="amd64"
	elif [ "${ARCHITECTURE}" == "aarch64" ]; then
		export ARCHITECTURE="arm64"
	else
		export ARCHITECTURE="i386"
	fi
	export PACKAGE_FORMAT="DPKG"
	export PACKAGE_MANAGER="apt-get"
	export PACKAGE_INSTALL="$PACKAGE_MANAGER install"
}

# Common for RedHat distributives:
function redhatArch(){
	export VERSION_NAME="${DISTRIBUTIVE}-${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
	export PACKAGE_FORMAT="RPM"
	export PACKAGE_MANAGER="yum"
	export PACKAGE_INSTALL="$PACKAGE_MANAGER install"
	export RELEASE_NUMBER="0"
}

# BSD:
function bsdArch(){
	export PACKAGE_MANAGER="pkg"
	export PACKAGE_INSTALL="$PACKAGE_MANAGER install"
}

# Case distribution:
case ${DISTRIBUTIVE} in
	MacOSX)
		;;
	Debian)
		debianArch
		export VERSION_NAME="debian${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
		;;
	Ubuntu)
		debianArch
		export VERSION_NAME="ubuntu${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
		;;
	Mint)
		debianArch
		export VERSION_NAME="mint${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
		;;
	Astra)
		debianArch
		export VERSION_NAME="astra${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
		;;
	openSUSE)
		redhatArch
		export PACKAGE_MANAGER="zypper"
		export PACKAGE_INSTALL="$PACKAGE_MANAGER install"
		;;
	Mageia)
		redhatArch
		export PACKAGE_MANAGER="urpm"
		export PACKAGE_INSTALL="${PACKAGE_MANAGER}i"
		;;
	Simply)
		redhatArch
		export DISTRIBUTIVE="AltLinux"
		export PACKAGE_MANAGER="apt-get"
		export PACKAGE_INSTALL="$PACKAGE_MANAGER install"
		export VERSION_NAME="alt${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
		;;
	Fedora)
		redhatArch
		export PACKAGE_MANAGER="dnf"
		export PACKAGE_INSTALL="$PACKAGE_MANAGER install"
		;;
	Red)
		export DISTRIBUTIVE="RedHat"
		redhatArch
		;;
	FreeBSD)
		bsdArch
		;;
	*)
		redhatArch
		;;
esac

echo "${DISTRIBUTIVE} ${DISTRIBUTIVE_VERSION} ${ARCHITECTURE}"
