#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pwd=$PWD
   cd ../..
   source setup.sh
   cd $pwd
fi

source $CGRU_LOCATION/utilities/python/setup.sh

cd ..

export AF_USERNAME=чёткий
export AF_HOSTNAME=тачка
afrender &>/dev/null &
python ./job.py --name Кею! --labels кей --services shake -n100 -t10 -c 500

export AF_USERNAME=кузьмич
export AF_HOSTNAME=копейка
afrender &>/dev/null  &
python ./job.py --name гон --labels трёп --services blender -n100 -t10 -c 500

export AF_USERNAME=михалыч
export AF_HOSTNAME=прибор
afrender &>/dev/null  &
python ./job.py --name работа --labels задачка --services generic -n100 -t10 -c 500

export AF_USERNAME=петрович
export AF_HOSTNAME=хлам
afrender &>/dev/null &
python ./job.py --name капаю --labels картошка --services prman -n100 -t10 -c 500

export AF_USERNAME=нащальника
export AF_HOSTNAME=ноут
afrender &>/dev/null &
python ./job.py --name пипец --labels всем --services xsi -n100 -t10 -c 500

export AF_USERNAME=герла
export AF_HOSTNAME=кампутер
afrender &>/dev/null &
python ./job.py --name ржунимагу --labels прикол --services maya -n100 -t10 -c 500

export AF_USERNAME=равшан
export AF_HOSTNAME=виндовоз
afrender &>/dev/null &
python ./job.py --name щитаю --labels кадры --services movgen -n100 -t10 -c 500

export AF_USERNAME=джамшут
export AF_HOSTNAME=виснет
afrender &>/dev/null &
python ./job.py --name секас --labels раскривушка --services max -n100 -t10 -c 500

sleep 1

export AF_USERNAME=петрович
export AF_HOSTNAME=линух

python ./job.py --name Мощное3D --labels БЬЮТИсцукоПАСС:ТЕНИнах --services hbatch_mantra -n100 -t10 -c 500
python ./job.py --name Крутой_Композ --labels жпеги_показать:чёртов_кей:грёбаые_красотульки:задник --services nuke -n100 -t10 -c 500
