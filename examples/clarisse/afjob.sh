#!/bin/bash

[ -d render ] || mkdir -v -m 777 render

# clarisse_node "$PWD/archive.render" -image "scene/image" -start_frame "1" -end_frame "2" -output "$PWD/render/img.####" -format "jpg" -log_width "0"

$CGRU_PYTHONEXE "${CGRU_LOCATION}/afanasy/python/afjob.py" "$PWD/archive.render" "1" "10" -fpt "1" -node "scene/image" -output "$PWD/render/img.####"
