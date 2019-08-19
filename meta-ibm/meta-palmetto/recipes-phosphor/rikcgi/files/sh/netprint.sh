#!/bin/sh


# Примеры busctl взяты отсюда:
# https://github.com/openbmc/phosphor-dbus-interfaces/tree/master/xyz/openbmc_project/Network
# https://github.com/openbmc/phosphor-networkd/blob/master/docs/Network-Configuration.md#usecases
# https://github.com/openbmc/openbmc/issues/2293
# https://tecadmin.net/check-ip-address-on-linux/
#

# https://gist.github.com/kwilczynski/5d37e1cced7e76c7c9ccfdf875ba6c5b
cidr_to_netmask() {
    value=$(( 0xffffffff ^ ((1 << (32 - $1)) - 1) ))
    echo "$(( (value >> 24) & 0xff )).$(( (value >> 16) & 0xff )).$(( (value >> 8) & 0xff )).$(( value & 0xff ))"
}



DHCPVAL=$(busctl get-property xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.EthernetInterface DHCPEnabled | awk '{gsub(/"/, ""); print $NF;exit}')
if [[ "$DHCPVAL" = "false" ]]; then
	DHCPVAL="no"
else
	DHCPVAL="yes"
fi

JSON="{"
JSON=$JSON"\"hostname\":\""$(busctl get-property xyz.openbmc_project.Network /xyz/openbmc_project/network/config xyz.openbmc_project.Network.SystemConfiguration HostName | awk '{gsub(/"/, ""); print $NF;exit}')"\""
JSON=$JSON",\"addr\":\""$(ip route get 8.8.8.8 | awk '{gsub(/"/, ""); print $NF;exit}')"\""
JSON=$JSON",\"mac\":\""$(busctl get-property xyz.openbmc_project.Network /xyz/openbmc_project/network/eth0 xyz.openbmc_project.Network.MACAddress MACAddress | awk '{gsub(/"/, ""); print $NF;exit}')"\""
JSON=$JSON",\"dhcp\":\""$DHCPVAL"\""
JSON=$JSON",\"gateway\":\""$(busctl get-property xyz.openbmc_project.Network /xyz/openbmc_project/network/config xyz.openbmc_project.Network.SystemConfiguration DefaultGateway | awk '{gsub(/"/, ""); print $NF;exit}')"\""
JSON=$JSON",\"ip6_addr\":\""$(ip -6 addr show dev eth0 | awk '/inet6/{gsub(/"/, ""); print $2}' | awk -F "/" '{print $1}')"\""

NETMASK=$(ip -o -f inet addr show | awk '/scope global/ {print $4}' | awk -F "/" '{print $2}')
logger "IPv4 netmask "$(cidr_to_netmask $NETMASK )

JSON=$JSON",\"mask\":\"$(cidr_to_netmask $NETMASK )\""
JSON=$JSON"}"

echo $JSON

