#!/bin/bash

NATRON="/opt/Natron"

CMD="${NATRON}/NatronRenderer"

#$CMD --help && exit

CMD="${CMD} -w w_jpg 1-10 $PWD/render/commmand.####.jpg $PWD/scene.ntp"

echo $CMD

[ -d render ] || mkdir -v -m 777 render

$CMD

