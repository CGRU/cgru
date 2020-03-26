#!/bin/bash
# Name=Submit Job...
# Icon=afanasy.png

source "`dirname "$0"`/_setup.sh"

"$CGRU_PYTHONEXE" "$CGRU_LOCATION/utilities/afstarter/afstarter.py" "$@"
