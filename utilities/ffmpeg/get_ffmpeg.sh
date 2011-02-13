#!/bin/bash

if [ -d ffmpeg ]; then
   cd ffmpeg
   svn up
   cd ..
else
   svn checkout svn://svn.ffmpeg.org/ffmpeg/trunk ffmpeg
fi
