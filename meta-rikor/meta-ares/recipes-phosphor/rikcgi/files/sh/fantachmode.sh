#!/bin/bash

#Функция чтения текущих rpm
function get_speed
{
  num=$1
  PWM_DIR=/sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0
  set -e

  RETVAL="$(cat $PWM_DIR/fan${num}_input 2> /dev/null)"
  exit_status=$?
  if [[ $exit_status -ne 0 ]]; then
  	echo "absent"
  else
  	echo "$RETVAL"
  fi
}

#Собственно основная фнкция
#Обработка DATA для входного запроса POST
echo -n "" > /www/pages/fanpost.json

if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ "$CONTENT_LENGTH" -gt 0 ]; then
        while read -n $CONTENT_LENGTH POST_DATA <&0
        do
            echo "${POST_DATA}" >> /www/pages/fanpost.json
         done
    fi
fi

read -a Q <<< `cat fanpost.json|sed -E 's/[{}\"]//g;s/,/ /g'`

AUTODEF=on
AUTOMODE=`echo "${Q[8]}" | cut -d : -f 2`

KEY=`echo "${Q[9]}" | cut -d : -f 2`
LTIME=`echo "${Q[10]}" | cut -d : -f 2`

if [[ $AUTODEF == $AUTOMODE ]]; then
	#./autofan.sh &
	TACH1=`get_speed 1` || exit 2
	TACH2=`get_speed 2` || exit 2
	TACH3=`get_speed 3` || exit 2
	TACH4=`get_speed 4` || exit 2
	TACH5=`get_speed 5` || exit 2
	TACH6=`get_speed 6` || exit 2
	TACH7=`get_speed 7` || exit 2
	TACH8=`get_speed 8` || exit 2

	FAN1=UNKNOWN
	FAN2=UNKNOWN
	FAN3=UNKNOWN
	FAN4=UNKNOWN
	FAN5=UNKNOWN
	FAN6=UNKNOWN
	FAN7=UNKNOWN
	FAN8=UNKNOWN
else
	killall autofan.sh
	
	FAN1=`echo "${Q[0]}" | cut -d : -f 2`
	FAN2=`echo "${Q[1]}" | cut -d : -f 2`
	FAN3=`echo "${Q[2]}" | cut -d : -f 2`
	FAN4=`echo "${Q[3]}" | cut -d : -f 2`
	FAN5=`echo "${Q[4]}" | cut -d : -f 2`
	FAN6=`echo "${Q[5]}" | cut -d : -f 2`
	FAN7=`echo "${Q[6]}" | cut -d : -f 2`
	FAN8=`echo "${Q[7]}" | cut -d : -f 2`
	
	FAN1V=$(( (255 * $FAN1) / 100 ))
	FAN2V=$(( (255 * $FAN2) / 100 ))
	FAN3V=$(( (255 * $FAN3) / 100 ))
	FAN4V=$(( (255 * $FAN4) / 100 ))
	FAN5V=$(( (255 * $FAN5) / 100 ))
	FAN6V=$(( (255 * $FAN6) / 100 ))
	FAN7V=$(( (255 * $FAN7) / 100 ))
	FAN8V=$(( (255 * $FAN8) / 100 ))

	echo $FAN1V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1
	echo $FAN2V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2
	echo $FAN3V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3
	echo $FAN4V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4
	echo $FAN5V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5
	echo $FAN6V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6
	echo $FAN7V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7
	echo $FAN8V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8

	TACH1=`get_speed 1` || exit 2
	TACH2=`get_speed 2` || exit 2
	TACH3=`get_speed 3` || exit 2
	TACH4=`get_speed 4` || exit 2
	TACH5=`get_speed 5` || exit 2
	TACH6=`get_speed 6` || exit 2
	TACH7=`get_speed 7` || exit 2
	TACH8=`get_speed 8` || exit 2
fi

STR="\"fantach\":[[\"Fan1\",\"$TACH1\",\"$FAN1\"],[\"Fan2\",\"$TACH2\",\"$FAN2\"],[\"Fan3\",\"$TACH3\",\"$FAN3\"],[\"Fan4\",\"$TACH4\",\"$FAN4\"],[\"Fan5\",\"$TACH5\",\"$FAN5\"],[\"Fan6\",\"$TACH6\",\"$FAN6\"],[\"Fan7\",\"$TACH7\",\"$FAN7\"],[\"Fan8\",\"$TACH8\",\"$FAN8\"]],\"fanauto\":\"$AUTOMODE\",\"key\":\"$KEY\",\"lifetime\":\"$LTIME\""

#echo $STR

echo "${STR}" >> /www/pages/STR.json
