#!/bin/sh

cat /sys/bus/i2c/devices/6-004e/hwmon/hwmon*/temp1_input
cat /sys/bus/i2c/devices/6-004f/hwmon/hwmon*/temp1_input
echo
cat /sys/bus/i2c/devices/6-004b/hwmon/hwmon*/temp1_input
cat /sys/bus/i2c/devices/6-004c/hwmon/hwmon*/temp1_input
echo
cat /sys/bus/i2c/devices/6-0049/hwmon/hwmon*/temp1_input
cat /sys/bus/peci/devices/0-30/peci-cputemp.0/hwmon/hwmon*/temp1_input
cat /sys/bus/peci/devices/0-31/peci-cputemp.1/hwmon/hwmon*/temp1_input

