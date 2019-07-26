#!/bin/sh

echo "Initializing PWM value ..."
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7
echo 100 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8


echo "Starting rikbtnd ..."
exec /usr/local/bin/rikbtnd

