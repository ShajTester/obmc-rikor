#!/bin/sh

logger afterpoweron

sleep 2

if [ ! -f /sys/bus/peci/devices/peci-0/0-30/peci-cputemp.0/hwmon/hwmon*/temp1_input ]; then
	logger "new peci-client 0x30"
	echo peci-client 0x30 > /sys/bus/peci/devices/peci-0/new_device
else
	logger "exist peci-client 0x30"
fi

if [ ! -f /sys/bus/peci/devices/peci-0/0-31/peci-cputemp.1/hwmon/hwmon*/temp1_input ]; then
	logger "new peci-client 0x31"
	echo peci-client 0x31 > /sys/bus/peci/devices/peci-0/new_device
else
	logger "exist peci-client 0x31"
fi

echo -n "on" > /tmp/rikfan.pipe

exit 0
