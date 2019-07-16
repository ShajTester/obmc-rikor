#!/bin/sh

echo 1 > /sys/class/gpio/gpio487/value
sleep 1s
echo out > /sys/class/gpio/gpio487/direction
echo 0 > /sys/class/gpio/gpio487/value
sleep 1s
echo 1 > /sys/class/gpio/gpio487/value 
echo in > /sys/class/gpio/gpio487/direction

logger "server_on.sh"
