#!/bin/bash

pushd .. > /dev/null
folder=$PWD
cd ../../../utilities
source ./getrevision.sh $folder
[ -z "${DISTRIBUTIVE}" ] && source ./distribution.sh > /dev/null
popd > /dev/null

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

# export AF_PYTHON_INCLUDE_PATH=C:\Python31\include
# export AF_PYTHON_LIBRARIES=C:\Python31\libs\libpython31.a

[ -f override.sh ] && source override.sh

cmake . && make
