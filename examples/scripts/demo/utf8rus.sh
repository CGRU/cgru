#!/bin/bash

if [ -z $CGRU_LOCATION ]; then
   pwd=$PWD
   cd ../../..
   source setup.sh
   cd $pwd
fi

cd ..

export AF_USERNAME=чёткий
export AF_HOSTNAME=тачка
afrender &>/dev/null &
./job.py --name Кею! --labels кей --types nuke -n100 -t10 -c 500

export AF_USERNAME=Кузьмич
export AF_HOSTNAME=копейка
afrender &>/dev/null  &
./job.py --name гон --labels трёп --types nuke -n100 -t10 -c 500

export AF_USERNAME=Михалыч
export AF_HOSTNAME=прибор
afrender &>/dev/null  &
./job.py --name работа --labels задачка --types nuke -n100 -t10 -c 500

export AF_USERNAME=Петрович
export AF_HOSTNAME=хлам
afrender &>/dev/null &
./job.py --name капаю --labels картошка --types nuke -n100 -t10 -c 500

export AF_USERNAME=нащальника
export AF_HOSTNAME=ноут
afrender &>/dev/null &
./job.py --name пипец --labels всем --types nuke -n100 -t10 -c 500

export AF_USERNAME=герла
export AF_HOSTNAME=кампутер
afrender &>/dev/null &
./job.py --name ржунимагу --labels прикол --types nuke -n100 -t10 -c 500

export AF_USERNAME=Равшан
export AF_HOSTNAME=виндовоз
afrender &>/dev/null &
./job.py --name щитаю --labels кадры --types nuke -n100 -t10 -c 500

export AF_USERNAME=Джамшут
export AF_HOSTNAME=виснет
afrender &>/dev/null &
./job.py --name секас --labels раскривушка --types nuke -n100 -t10 -c 500

sleep 1

export AF_USERNAME=Петрович
export AF_HOSTNAME=линух

./job.py --name Мощное3D --labels БЬЮТИсцукоПАСС:ТЕНИнах --types hbatch_mantra -n100 -t10 -c 500
./job.py --name Крутой_Композ --labels жпеги_показать:чёртов_кей:грёбаые_красотульки:задник --types nuke -n100 -t10 -c 500
