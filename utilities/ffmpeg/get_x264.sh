#!/bin/bash

if [ -d x264 ]; then
   cd x264
   git pull -v
   cd ..
else
   git clone https://code.videolan.org/videolan/x264.git
fi
