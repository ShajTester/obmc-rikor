#!/bin/sh

echo "Initializing PWM value ..."
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7
echo 110 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8

# ip a add 10.10.0.222/24 broadcast 10.10.0.255 dev eth0
# route add default gw 10.10.0.1 eth0

echo "Starting rikbtnd ..."
exec /usr/local/bin/rikbtnd

