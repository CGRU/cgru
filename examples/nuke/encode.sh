#!/bin/bash

cgru=../..

./nuke -ti $cgru/plugins/nuke/encode.py -x $cgru/plugins/nuke/encode.nk -s Cineon -r "Read1,Read2" render/preview.%04d.jpg movie.mov

