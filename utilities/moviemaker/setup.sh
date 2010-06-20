#!/bin/bash

export ENCODE_LIBS=$PWD/ffmpeg
export ENCODE_BINS=$PWD/ffmpeg

[ -f override.sh ] && source override.sh

export LD_LIBRARY_PATH=$ENCODE_LIBS
export PATH=$ENCODE_BINS:$PATH
