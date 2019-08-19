#!/bin/sh


# Команда '/usr/bin/rikcgi-net --indata'
# проверяет ключ сессии и преобразует
# входной JSON в последовательность значений
LOGCGI=$(/usr/bin/rikcgi-net --indata)
logger $LOGCGI
read -a Q <<< $LOGCGI
logger ${Q[1]}

# Q[0] - hostname
# Q[1] - dhcp
# Q[2] - ipv4
# Q[3] - mask
# Q[4] - gate


### busctl call  xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.IP.Create IP ssys "xyz.openbmc_project.Network.IP.Protocol.IPv4" "10.10.0.222" 24 "10.10.0.1"


QSIZE=${#Q[@]}
if [[ $QSIZE -gt 1 ]]; then
	busctl set-property xyz.openbmc_project.Network /xyz/openbmc_project/network/config xyz.openbmc_project.Network.SystemConfiguration HostName s "${Q[0]}"
	if [[ "${Q[1]}" == "yes" ]]; then
		busctl set-property xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.EthernetInterface DHCPEnabled b 1
	else
		busctl set-property xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.EthernetInterface DHCPEnabled b 0
		busctl call  xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.IP.Create IP ssys \
			"xyz.openbmc_project.Network.IP.Protocol.IPv4" \
			"${Q[2]}" \
			${Q[3]} \
			"${Q[4]}" \
			|| logger "Error add IPv4"
	fi
	sleep 10
	systemctl restart systemd-networkd.service
fi
