#!/bin/bash

# Get sources folder:
pushd .. > /dev/null
src=$PWD

# Get distribution variables:
cd ../..
cd utilities
source ./getrevision.sh $src
[ -z "${DISTRIBUTIVE}" ] && source ./distribution.sh > /dev/null

# Go to initial folder:
popd > /dev/null

# Process options:
options=""
sql="REQUIRED"
for arg in "$@"; do
	[ $arg == "--nosql" ] && sql="NO"
done

# Configure SQL:
export AF_POSTGRESQL=$sql

# Configure build flags:
export AF_ADD_CFLAGS=""
export AF_ADD_LFLAGS="-lutil"

echo "Building on '${DISTRIBUTIVE}'"
case ${DISTRIBUTIVE} in
    openSUSE)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
        ;;
    Debian)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        ;;
    Gentoo)
        ;;
    Ubuntu)
        export ADD_CMAKE_MODULE_PATH="$PWD"
#        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
        ;;
    Mint)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        ;;
    Fedora)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
        ;;
    AltLinux)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        ;;
    CentOS)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        ;;
    MacOSX)
        ;;
    *)
        echo "Warning: Untested system: '${DISTRIBUTIVE}'"
        ;;
esac

# Build:
cmake . && make $@
