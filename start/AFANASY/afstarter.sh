#!/bin/bash
# Name=Submit Job...
source "`dirname "$0"`/_setup.sh"

"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/afstarter/afstarter.py" "$@"
