#!/bin/bash

# Source general for all soft directives:
source "$CGRU_LOCATION/software_setup/setup__all.sh"


# Locate C4D:
C4F_LOCATION="/Applications/MAXON/CINEMA 4D R13/CINEMA 4D.app/Contents/MacOS"
C4D_EXEC="CINEMA 4D"
C4D_USER_FOLDER="${HOME}/Library/Preferences/MAXON/CINEMA 4D R13_DA28FA2D"

# Here additional folders can get specified where plugins should get copied from. Should get specified outside that file.
# This gets read by the python_init.py
#export C4D_ADDITIONAL_PLUGIN_FOLDERS="PATH1;PATH2"


#override c4d location based on locate_nuke.sh
locate_c4d="$CGRU_LOCATION/software_setup/locate_c4d.sh"
[ -f $locate_c4d ] && source $locate_c4d

export C4D_EXEC="${C4F_LOCATION}/${C4D_EXEC}"
export C4D_LOCATION
export C4D_USER_FOLDER

# CGRU for C4D add-ons location, override it, or simple launch from current folder as an example
export C4D_CGRU_LOCATION=$CGRU_LOCATION/plugins/c4d
export C4D_CGRU_SCRIPTS_LOCATION=$C4D_CGRU_LOCATION/scripts

export C4D_AF_SCRIPTS_LOCATION=$AF_ROOT/plugins/c4d

# Local ones on client
export C4D_PLUGIN_LOCATION=${C4D_USER_FOLDER}/plugins
export C4D_PREFERENCES_LOCATION=${C4D_USER_FOLDER}/prefs

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

export APP_DIR="${C4F_LOCATION}"
export APP_EXE="${C4D_EXEC}"


echo "C4D = '${C4D_EXEC}'"