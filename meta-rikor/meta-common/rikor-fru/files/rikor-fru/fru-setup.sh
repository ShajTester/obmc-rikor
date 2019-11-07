#!/bin/sh
#

### BEGIN INIT INFO
# Provides:          sensor-setup
# Required-Start:    power-on
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Power on micro-server
### END INIT INFO



AT24ADDR=$(i2cdetect -y 3 | awk '/5[0-7] / {print $(FNR+1)}')

# echo 24c02 0x50 > /sys/devices/platform/ast-i2c.3/i2c-3/new_device
# echo 24c02 0x52 > /sys/devices/platform/ast-i2c.3/i2c-3/new_device
echo 24c02 0x$AT24ADDR > /sys/bus/i2c/devices/i2c-3/new_device
RIKORFRUPATH="/sys/bus/i2c/devices/i2c-3/3-00$AT24ADDR"
echo $AT24ADDR > /tmp/rikor-fru-address
echo $RIKORFRUPATH > /tmp/rikor-fru-path

