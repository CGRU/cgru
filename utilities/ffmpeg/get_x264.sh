#!/bin/bash

if [ -d x264 ]; then
   cd x264
   git pull -v
   cd ..
else
   git clone git://git.videolan.org/x264.git
fi
