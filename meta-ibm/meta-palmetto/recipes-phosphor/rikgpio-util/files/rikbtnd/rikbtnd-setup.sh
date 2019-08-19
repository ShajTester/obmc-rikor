#!/bin/sh

echo "Initializing PWM value ..."
echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm1	#0
echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm2	#1
echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm4	#3
echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm3	#2
echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm5	#4
echo 50 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm6	#5
echo 120 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm7	#6	#CPU0
echo 120 > /sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm8	#7	#CPU1

# ip a add 10.10.0.222/24 broadcast 10.10.0.255 dev eth0
# route add default gw 10.10.0.1 eth0

if [ "$(/usr/bin/rikor-fru-util -b /tmp/rikor-fru-address -g'dhcp1')" = "dhcp" ] ; then

	echo -n "Setup dhclient... "

else

	echo -n "Add static IPv4... "

	read -a Q <<< `/usr/bin/rikor-fru-util -b /tmp/rikor-fru-address -g'ip1 netmask1 gate1'`
	ip a add ${Q[0]}/24 dev eth0
	ip route add default via ${Q[2]} dev eth0

fi


echo "Starting rikbtnd ..."
exec /usr/local/bin/rikbtnd

