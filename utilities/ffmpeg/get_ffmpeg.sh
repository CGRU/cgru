#!/bin/bash

if [ -d ffmpeg ]; then
   cd ffmpeg
   git pull
   cd ..
else
	git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
fi

