#!/bin/bash

export ENCODE_LIBS=$PWD/ffmpeg
export ENCODE_BINS=$PWD/ffmpeg

[ -f override.sh ] && source override.sh

export PATH=$ENCODE_BINS:$PATH

if [ -z "$LD_LIBRARY_PATH" ] ; then
   export LD_LIBRARY_PATH=$ENCODE_LIBS
else
   export LD_LIBRARY_PATH=$ENCODE_LIBS:$LD_LIBRARY_PATH
fi
