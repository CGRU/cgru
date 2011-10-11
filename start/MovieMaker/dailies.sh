#!/bin/bash
# Name=Dailies...
source "`dirname "$0"`/_setup.sh"
"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/moviemaker/dialog.py" "$@"
