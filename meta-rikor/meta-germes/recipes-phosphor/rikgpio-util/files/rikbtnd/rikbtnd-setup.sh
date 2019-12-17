#!/bin/sh

#For 2U servers
# echo "Initializing PWM value ..."
# echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1	#0
# echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2	#1
# echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4	#3
# echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3	#2
# echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5	#4
# echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6	#5
# echo 120 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7	#6	#CPU0
# echo 120 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8	#7	#CPU1


/usr/bin/rikcgi-fan --init

#For 1U servers
#echo "Initializing PWM value ..."
#echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1	#0
#echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2	#1
#echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4	#3
#echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3	#2
#echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5	#4
#echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6	#5
#echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7	#6	#CPU0
#echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8	#7	#CPU1

# ip a add 10.10.0.222/24 broadcast 10.10.0.255 dev eth0
# route add default gw 10.10.0.1 eth0

echo "Starting rikbtnd ..."
exec /usr/local/bin/rikbtnd

