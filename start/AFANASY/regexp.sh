#!/bin/bash
# Name=Check RegExp...
# Icon=regexp.png

source "`dirname "$0"`/_setup.sh"

"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/regexp/regexp.py" "$@"
