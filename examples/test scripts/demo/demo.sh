#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pushd ../../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi

cd ..

./Renders.sh rf 5
sleep 1


export AF_USERNAME=timurhai
export AF_HOSTNAME=wl00
python ./job.py --name BFT103.moviePAL --labels convert:encode --services movgen -n100 -t5
python ./job.py --name BFT103.comp --labels dpx:key:back --services nuke -n100 -t5
python ./job.py --name ET305.comp --labels ready:key:back --services nuke -n100 -t10
python ./job.py --name ET305.road --labels cam12 --services mentalray -n100 -t10

export AF_USERNAME=john
export AF_HOSTNAME=wl01
python ./job.py --name ET305.train --labels cam12 --services max -n100 -t10
python ./job.py --name ET305.cars --labels cam12 --services maya -n100 -t10
python ./job.py --name ET305.smoke --labels cam12 --services blender -n100 -t10

export AF_USERNAME=bob
export AF_HOSTNAME=wl02
python ./job.py --name BFT103.bullet --labels image:shadow --services prman -n100 -t30
python ./job.py --name BFT103.bridge --labels image:shadow --services xsi -n100 -t30
python ./job.py --name BFT103.trees --labels image:shadow --services c4d -n100 -t30

export AF_USERNAME=alex
export AF_HOSTNAME=wl03
python ./job.py --name CAP300.house --labels image:ifd --services mantra:hbatch -n100 -t20
python ./job.py --name CAP300.cars --labels /out/mantra1 --services mantra -n100 -t10
python ./job.py --name CAP300.tram --labels cam4 --services vray -n100 -t10

export AF_USERNAME=peter
export AF_HOSTNAME=wl04
python ./job.py --name CAP300.comp --labels preview:dpx:key:back --services nuke -n100 -t10
python ./job.py --name CAP300.people --labels 'Comp 1' --services afterfx -n100 -t10
python ./job.py --name CAP300.grass --labels layer1 --services arnold -n100 -t10
