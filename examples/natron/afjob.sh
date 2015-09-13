#!/bin/bash

CMD="python ${CGRU_LOCATION}/afanasy/python/afjob.py"

CMD="${CMD} $PWD/scene.ntp 1 20 -node w_jpg -name Natron-afjob.py -V"

echo $CMD

[ -d render ] || mkdir -v -m 777 render

$CMD

