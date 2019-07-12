#!/bin/bash

FWINFO=$(cat /etc/issue.net | awk 'BEGIN{RS=""; FS=" "} {print "Firmware " $3}' | sed 's/yosemite/rikor/')
FWINFO=$FWINFO" build "$(cat /etc/version)

# HWINFO="Rikor R-BD-ESR-V4-16EA v.5 / BIOS 2.8 / PCIE 3.0 / CPU: 2 x Intel Xeon E5 2630 v3 / RAM:256 GB"
HWINFO="Rikor R-BD-ESR-V4-16EA v.5"

JSON="{"
JSON=$JSON"\"sysinfo\":{\"fwinfo\":\"$FWINFO\",\"hwinfo\":\"$HWINFO\"}"
JSON=$JSON",\"voltage\":"`./voltage.sh`
JSON=$JSON",\"fantach\":"`./fantach.sh`
JSON=$JSON",\"sensors\":"`./sensors.sh`
JSON=$JSON",\"netconfig\":"`/usr/bin/rikcgi-net --get`
JSON=$JSON"}"

echo $JSON

#DONE:
#sensors
#fantach
#voltage

#JSON="{"
#for key in "sysinfo" "power" "sensors" "devices" "fantach" "voltage" "netconfig" "fanmode" "datetime"
#do
     # Запятая ставится перед всеми элементами кроме первого
#     if [[ "$JSON" != "{" ]]; then JSON=$JSON","; fi

     #Добавляем очередной "ключ"=объект-значение, которое возвращает одноименный скрипт без параметров
     #Возвращаемый объект-значение не нужно оборачивать в кавычки
     #Имена скриптов нужно сделать такими же как и ключи
     #Эти скрипты годятся и для частных запросов, т.к. в частных запросах требуется чистый объект-значение
     #В каждом частном скрипте, в самом начале должна быть проверка на наличие аргументов, если оргументов нет, то вернуть всё
#     JSON=$JSON'"'$key'":'`$key`
#done
#JSON=$JSON"}"

# отдаем результат
#echo $JSON

