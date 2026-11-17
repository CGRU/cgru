#!/usr/bin/env bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"


# Locate C4D:
C4D_EXEC="CINEMA 4D"
C4D_USER_FOLDER=""
C4D_USER_FOLDER_LOCATION="${HOME}/Library/Preferences/MAXON"

# If job/Keeper already pinned C4D_LOCATION, keep it (do not run locate_*).
C4D_FROM_ENV=0
if [ -n "$C4D_LOCATION" ]; then
    C4D_FROM_ENV=1
    echo "C4D_LOCATION from environment: ${C4D_LOCATION}"
else
    C4D_LOCATION=""
fi

# Find the most recent C4D-Version-User-Folder
STDOUT=$(ls -1 "$C4D_USER_FOLDER_LOCATION" 2>/dev/null)
while read line; do
    if [ "`echo $line | awk '{print match( \$1, "CINEMA")}'`" == "1" ]; then
        C4D_USER_FOLDER="${C4D_USER_FOLDER_LOCATION}/${line}"
    fi
done <<< "$STDOUT"

# Here additional folders can get specified where plugins should get copied from. Should get specified outside that file.
# This gets read by the python_init.py
#export C4D_ADDITIONAL_PLUGIN_FOLDERS="PATH1;PATH2"

export C4D_LOCATION
export C4D_USER_FOLDER

# CGRU for C4D add-ons location, override it, or simple launch from current folder as an example
export C4D_CGRU_LOCATION=$CGRU_LOCATION/plugins/c4d
export C4D_CGRU_SCRIPTS_LOCATION=$C4D_CGRU_LOCATION/scripts

# Local ones on client
export C4D_PLUGIN_LOCATION=${C4D_USER_FOLDER}/plugins
export C4D_PREFERENCES_LOCATION=${C4D_USER_FOLDER}/prefs

C4D_INSTALL_DIR="/Applications/MAXON"

# Find the most recent C4D-Version (only when not pinned)
if [ "$C4D_FROM_ENV" = "0" ]; then
    STDOUT=$(ls -1 "$C4D_INSTALL_DIR" 2>/dev/null)
    while read line; do
        if [ "`echo $line | awk '{print match( \$1, "CINEMA")}'`" == "1" ]; then
            C4D_LOCATION="${C4D_INSTALL_DIR}/${line}/CINEMA 4D.app/Contents/MacOS"
        fi
    done <<< "$STDOUT"

    locate_c4d="$CGRU_LOCATION/software_setup/locate_c4d.sh"
    [ -f "$locate_c4d" ] && source "$locate_c4d"
fi

export C4D_LOCATION

# Define that the c4d-render-script can get found. Gets used by "c4drender"
export C4D_RENDER_SCRIPT="${C4D_CGRU_LOCATION}/render.py"


# Copy the python_init.py file to the user
if [ -d "${C4D_PREFERENCES_LOCATION}/python" ]; then
    cp -fr "${C4D_CGRU_LOCATION}/python_init.py" "${C4D_PREFERENCES_LOCATION}/python/"
    echo "python_init.py got copied"
else
    echo "ERROR: The User-Directory did not exist so the current python_init.py could NOT get copied!!!"
    echo "       Please set C4D_USER_FOLDER to point to the User-Directory!"
fi

# Relative default exec name → full path under C4D_LOCATION
case "$C4D_EXEC" in
    /*) ;;
    *) export C4D_EXEC="${C4D_LOCATION}/${C4D_EXEC}" ;;
esac
export APP_DIR="${C4D_LOCATION}"
export APP_EXE="${C4D_EXEC}"

echo "C4D = '${C4D_EXEC}'"
