#!/bin/bash
# Name=Dailies...
# Icon=dailies.png
source "`dirname "$0"`/_setup.sh"
"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/moviemaker/dialog.py" "$@"
