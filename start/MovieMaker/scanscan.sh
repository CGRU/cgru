#!/bin/bash
# Name=Scan Scan...
# Icon=scanscan.png
source "`dirname "$0"`/_setup.sh"
"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/moviemaker/scandialog.py" "$@"
