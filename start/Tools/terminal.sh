#!/bin/bash
# Name=Terminal
# Icon=terminal.png

source "`dirname "$0"`/_setup.sh"

[ -d "$CGRU_LOCATION" ] && cd "$CGRU_LOCATION"

openterminal
