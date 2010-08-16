#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pwd=$PWD
   cd ../../..
   source setup.sh
   cd $pwd
fi

cd ..

./Renders.sh r0 10
sleep 1
./Renders.sh r1 5
sleep 1
./Renders.sh l0 5
sleep 1

afcmd rpri r1. 200
sleep 1
afcmd rpri r0. 150
sleep 1
afcmd rpri l0. 100
sleep 1

afcmd ruser l00 tima
afcmd ruser l01 john
afcmd ruser l02 bob
afcmd ruser l03 alex
afcmd ruser l04 peter
sleep 1

afcmd rnimby l0.
afcmd rNIMBY l0[0\|2]

export AF_USERNAME=tima
export AF_HOSTNAME=l00
./job.py --name BFT103.moviePAL --labels convert:encode --types generic -n100 -t5
./job.py --name BFT103.comp --labels dpx:key:back --types nuke -n100 -t5
./job.py --name ET305.comp --labels ready:key:back --types nuke -n100 -t10

export AF_USERNAME=john
export AF_HOSTNAME=l01
./job.py --name ET305.train --labels /out/mantra1 --types mantra -n100 -t10
./job.py --name ET305.cars --labels /out/mantra1 --types mantra -n100 -t10

export AF_USERNAME=bob
export AF_HOSTNAME=l02
./job.py --name BFT103.bullet --labels image:shadow --types prman -n100 -t30
./job.py --name BFT103.bridge --labels image:shadow --types prman -n100 -t30

export AF_USERNAME=alex
export AF_HOSTNAME=l03
./job.py --name CAP300.house --labels image:ifd --types mantra:hbatch -n100 -t20
./job.py --name CAP300.cars --labels /out/mantra1 --types mantra -n100 -t10

export AF_USERNAME=peter
export AF_HOSTNAME=l04
./job.py --name CAP300.comp --labels preview:dpx:key:back --types nuke -n100 -t10
./job.py --name CAP300.people --labels /out/mantra1 --types mantra -n100 -t10
