#!/bin/bash

# Get sources folder:
pushd .. > /dev/null
src=$PWD

# Get distribution variables:
cd ../..
cgru=$PWD
cd utilities
source ./getrevision.sh "$src"
[ -z "${DISTRIBUTIVE}" ] && source ./distribution.sh > /dev/null

# Go to initial folder:
popd > /dev/null

# Process options:
options=""
sql="REQUIRED"
gui="YES"
for arg in "$@"; do
	[ "$arg" == "--nosql" ] && sql="NO" && shift
	[ "$arg" == "--nogui" ] && gui="NO" && shift
done

# Configure SQL:
export AF_POSTGRESQL=$sql

# Configure GUI:
export AF_GUI=$gui
export AF_QT_VER="4"

cgru_python="${cgru}/python"
if [ -d "${cgru_python}" ]; then
	export AF_PYTHON_INCLUDE_PATH="${cgru_python}/include/python3.3m"
	export AF_PYTHON_LIBRARIES="${cgru_python}/lib/libpython3.3m.a"
fi


# Configure building:
export AF_ADD_CFLAGS=""
export AF_ADD_LFLAGS="-lutil"
export AF_EXTRA_LIBS=""

echo "Building on '${DISTRIBUTIVE}'"
case ${DISTRIBUTIVE} in
    openSUSE)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
        ;;
    SUSE)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread -ldl"
        ;;
    Debian)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread -lrt"
        ;;
    Gentoo)
        ;;
    Ubuntu)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
#		if(( $(echo "$DISTRIBUTIVE_VERSION > 14" | bc -l ) )); then
#			export AF_QT_VER="5"
#		fi
        ;;
    Mint)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
        ;;
    Fedora)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread"
        ;;
    AltLinux)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
        ;;
    CentOS)
        export ADD_CMAKE_MODULE_PATH="$PWD"
        export AF_EXTRA_LIBS="pthread"
        ;;
    RedHat)
        export AF_ADD_LFLAGS="$AF_ADD_LFLAGS -lpthread -ldl"
        ;;
    Mageia)
        export AF_EXTRA_LIBS="pthread"
        ;;
    Arch|Manjaro)
        export AF_EXTRA_LIBS="pthread"
        ;;
    MacOSX)
        ;;
    *)
        echo "Warning: Untested system: '${DISTRIBUTIVE}'"
        ;;
esac

# Build:
cmake . && make $@
