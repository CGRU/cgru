#!/bin/bash

echo "Detecting linux distribution..."

knowndistrs="Debian Ubuntu CentOS Fedora openSUSE"

# Load issue file:
issue="/etc/issue"
if [ ! -f "${issue}" ]; then
   echo "File '${issue}' not founded. Can't detect distribution."
   exit 1
fi
issue=`cat "${issue}"`

# Search issue file:
for distr in $knowndistrs; do
   if [ `eval "echo \"${issue}\" | awk '{ print match(\\$0,\"${distr}\")}'"` != "0" ]; then
      export DISTRIBUTIVE="${distr}"
      break
   fi
done

# No distribution founded:
if [ -z "${DISTRIBUTIVE}" ]; then
   echo "Unsupported distribution:"
   echo "${issue}"
   echo "Supported distributions:"
   echo "${knowndistrs}"
   exit
fi

# Search distribution version:
export DISTRIBUTIVE_VERSION=`echo "${issue}" | awk '{match($0,"[0-9]+.[0-9]+"); print substr($0,RSTART,RLENGTH)}'`

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
   export QT_VERSION=">= 4"
   export PYTHON_VERSION=">= 2"
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
      case $DISTRIBUTIVE_VERSION in
         5.0)
            export VERSION_NAME="debian.lenny_${ARCHITECTURE}"
            export QT_VERSION=">= 4.4"
            export PYTHON_VERSION=">= 2.5"
         ;;
      esac
      ;;
   Ubuntu)
      debianArch
      export VERSION_NAME="ubuntu${DISTRIBUTIVE_VERSION}_${ARCHITECTURE}"
      case $DISTRIBUTIVE_VERSION in
         9.10)
            export QT_VERSION=">= 4.5"
            export PYTHON_VERSION=">= 2.5"
            ;;
         10.04)
            export QT_VERSION=">= 4.6"
            export PYTHON_VERSION=">= 2.6"
            ;;
      esac
      ;;
   *)
      redhatArch
      ;;
esac

echo "${DISTRIBUTIVE} ${DISTRIBUTIVE_VERSION} ${ARCHITECTURE}"
echo "Package manager = '${PACKAGE_MANAGER}'"
[ ! -z "${QT_VERSION}" ] && echo "Qt version ${QT_VERSION}"
[ ! -z "${QT_VERSION}" ] && echo "Python version ${PYTHON_VERSION}"
