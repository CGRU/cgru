#!/bin/bash

# Get sources folder:
pushd .. > /dev/null
src=$PWD

# Get distribution variables:
cd ../..
cgru=$PWD
cd utilities
source ./getrevision.sh $src
[ -z "${DISTRIBUTIVE}" ] && source ./distribution.sh

# Go to initial folder:
popd > /dev/null

# Process options:
options=""
sql="REQUIRED"
gui="YES"
fermer="YES"
for arg in "$@"; do
	[ $arg == "--nosql" ] && sql="NO" && shift
	[ $arg == "--nogui" ] && gui="NO" && shift
	[ $arg == "--nofermer" ] && fermer="NO" && shift
	[ $arg == "--debug" ] && debug="-g" && shift
done

# Configure SQL:
export AF_POSTGRESQL=$sql

# Configure GUI:
export AF_GUI=$gui
export AF_FERMER=$fermer
export AF_QT_VER="4"

# Configure building:
export AF_ADD_CFLAGS="$debug"
export AF_ADD_LFLAGS="-lutil"
export AF_EXTRA_LIBS=""

echo "Building on '${DISTRIBUTIVE}'"
case ${DISTRIBUTIVE} in
    openSUSE)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
		export AF_QT_VER="5"
        ;;
    SUSE)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread -ldl"
		export AF_QT_VER="5"
        ;;
    Debian)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread -lrt"
		if [[ "$DISTRIBUTIVE_VERSION" < "8" ]]; then
			export AF_QT_VER="4"
		else
			export AF_QT_VER="5"
		fi
        ;;
    Gentoo)
        ;;
    Ubuntu)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
		export AF_QT_VER="5"
        ;;
    Mint)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
        ;;
    Fedora)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
		export AF_QT_VER="5"
        ;;
    AltLinux)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
		export AF_QT_VER="4"
        ;;
    CentOS)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
		if [[ "$DISTRIBUTIVE_VERSION" < "7" ]]; then
			export AF_QT_VER="4"
		else
			export AF_QT_VER="5"
		fi
        ;;
    RedHat)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread -ldl"
        ;;
    Mageia)
        export AF_EXTRA_LIBS="pthread"
		export AF_QT_VER="5"
        ;;
    Arch|Manjaro)
        export AF_EXTRA_LIBS="pthread"
        ;;
    MacOSX)
        export AF_QT_VER="5"
        ;;
    *)
        echo "Warning: Untested system: '${DISTRIBUTIVE}'"
        ;;
esac

# Build:
cmake . && make $@
