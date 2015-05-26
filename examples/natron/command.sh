#!/bin/bash

NATRON="/opt/Natron"

CMD="${NATRON}/NatronRenderer"

#$CMD --help && exit

CMD="${CMD} -w w_jpg 1-10 $PWD/scene.ntp"

echo $CMD

$CMD

