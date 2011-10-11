#!/bin/bash
# Name=Scan Scan...
source "`dirname "$0"`/_setup.sh"
"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/moviemaker/scandialog.py" "$@"
