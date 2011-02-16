#!/bin/bash

echo "Detecting linux distribution..."

knowndistrs="Debian Ubuntu CentOS Fedora openSUSE"

# Load issue file:
issuefile="/etc/issue"
if [ ! -f "${issuefile}" ]; then
   echo "File '${issuefile}' not founded. Can't detect distribution."
   exit 1
fi

# Search issue file:
for distr in $knowndistrs; do
   issue=`cat "${issuefile}" | grep "${distr}"`
   [ -z "${issue}" ] && continue
   if [ `eval "echo \"${issue}\" | awk '{ print match(\\$0,\"${distr}\")}'"` != "0" ]; then
      export DISTRIBUTIVE="${distr}"
      break
   fi
done

# No distribution founded:
if [ -z "${DISTRIBUTIVE}" ]; then
   echo "Unsupported distribution:"
   cat "${issuefile}"
   echo "Supported distributions:"
   echo "${knowndistrs}"
   exit 1
fi

# Search distribution version:
export DISTRIBUTIVE_VERSION=`echo "${issue}" | awk '{match($0,"[0-9.-]+"); print substr($0,RSTART,RLENGTH)}'`

# Check architecture:
export ARCHITECTURE=`uname -m`

# Common for Debian distributives:
function debianArch(){
   if [ "${ARCHITECTURE}" == "x86_64" ]; then
      export ARCHITECTURE="amd64"
   else
      export ARCHITECTURE="i386"
   fi
   export PACKAGE_MANAGER="DPKG"
}

# Common for RedHat distributives:
function redhatArch(){
   export VERSION_NAME="${DISTRIBUTIVE}-${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
   export PACKAGE_MANAGER="RPM"
   export RELEASE_NUMBER="0"
}

# Case distribution:
case ${DISTRIBUTIVE} in
   Debian)
      debianArch
      export VERSION_NAME="debian${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
      ;;
   Ubuntu)
      debianArch
      export VERSION_NAME="ubuntu${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
      ;;
   *)
      redhatArch
      ;;
esac

echo "${DISTRIBUTIVE} ${DISTRIBUTIVE_VERSION} ${ARCHITECTURE}"
echo "Package manager = '${PACKAGE_MANAGER}'"
