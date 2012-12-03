#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pushd ../../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi

cd ..

./Renders.sh rf0 10
sleep 1
./Renders.sh rf1 5
sleep 1
./Renders.sh wl0 5
sleep 1

afcmd rpri rf1. 200
sleep 1
afcmd rpri rf0. 150
sleep 1
afcmd rpri wl0. 100
sleep 1

afcmd ruser wl00 timurhai
afcmd ruser wl01 john
afcmd ruser wl02 bob
afcmd ruser wl03 alex
afcmd ruser wl04 peter
sleep 1

afcmd rnimby "wl0."
afcmd rNIMBY "wl0[0\|2]"

export AF_USERNAME=timurhai
export AF_HOSTNAME=wl00
python ./job.py --name BFT103.moviePAL --labels convert:encode --services movgen -n100 -t5
python ./job.py --name BFT103.comp --labels dpx:key:back --services nuke -n100 -t5
python ./job.py --name ET305.comp --labels ready:key:back --services nuke -n100 -t10

export AF_USERNAME=john
export AF_HOSTNAME=wl01
python ./job.py --name ET305.train --labels /out/mantra1 --services mantra -n100 -t10
python ./job.py --name ET305.cars --labels /out/mantra1 --services mantra -n100 -t10

export AF_USERNAME=bob
export AF_HOSTNAME=wl02
python ./job.py --name BFT103.bullet --labels image:shadow --services prman -n100 -t30
python ./job.py --name BFT103.bridge --labels image:shadow --services prman -n100 -t30

export AF_USERNAME=alex
export AF_HOSTNAME=wl03
python ./job.py --name CAP300.house --labels image:ifd --services mantra:hbatch -n100 -t20
python ./job.py --name CAP300.cars --labels /out/mantra1 --services mantra -n100 -t10

export AF_USERNAME=peter
export AF_HOSTNAME=wl04
python ./job.py --name CAP300.comp --labels preview:dpx:key:back --services nuke -n100 -t10
python ./job.py --name CAP300.people --labels /out/mantra1 --services mantra -n100 -t10
