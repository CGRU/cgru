#!/bin/bash
# Name=Start Watch...
# Icon=afwatch.png

source "`dirname "$0"`/_setup.sh"
if [ ! -z "$AF_WATCH_CMD" ]; then
   "$AF_WATCH_CMD" "$@"
else
   "$AF_ROOT/bin/afwatch" "$@"
fi
