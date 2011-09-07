#!/bin/bash

if [ ! -z "$AF_WATCH_CMD" ]; then
   $AF_WATCH_CMD
else
   $AF_ROOT/launch/afwatch.sh
fi
