#!/bin/bash
# Name=Terminal
# Icon=terminal.png

source "`dirname "$0"`/_setup.sh"

terminals="gnome-terminal mate-terminal konsole /Applications/Utilities/Terminal.app/Contents/MacOS/Terminal xterm"

[ -d "$CGRU_LOCATION" ] && cd "$CGRU_LOCATION"

for terminal in $terminals; do
   if [ `which $terminal` ]; then
      echo "Starting '$terminal' in '$PWD'"
      $terminal
      exit 0
   fi
done
