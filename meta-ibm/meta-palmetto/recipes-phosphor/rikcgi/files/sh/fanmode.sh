#!/bin/bash

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

SIZE=${#Q[@]}
DEFSIZE=11
echo "$SIZE"
if [[ $SIZE == $DEFSIZE ]]; then
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

    	killall autofan.sh
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN1 0
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN2 1
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN3 2
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN4 3
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN5 4
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN6 5
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN7 6
	#/usr/local/fbpackages/fan_ctrl/set_fan_speed.sh $FAN8 7

	#echo $FAN1 $FAN2 $FAN3 $FAN4 $FAN5 $FAN6 $FAN7 $FAN8

	echo $FAN1V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1
	echo $FAN2V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2
	echo $FAN3V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3
	echo $FAN4V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4
	echo $FAN5V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5
	echo $FAN6V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6
	echo $FAN7V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7
	echo $FAN8V > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8

else
	killall autofan.sh
	#Пока не будем включать автоматическую регулировку
	#./autofan.sh &
	#echo "AutoMODE"
fi

cat fanpost.json
