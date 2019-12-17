FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

NAMES = " apb/bus@1e78a000/i2c-bus@1c0/lm75@49 \
          apb/bus@1e78a000/i2c-bus@1c0/lm75@4b \
          apb/bus@1e78a000/i2c-bus@1c0/lm75@4c \
          apb/bus@1e78a000/i2c-bus@1c0/lm75@4e \
          apb/bus@1e78a000/i2c-bus@1c0/tmp421@4f \
          apb/bus@1e78a000/i2c-bus@300/pmbus@58 \
          apb/bus@1e78a000/i2c-bus@300/pmbus@59 \
"

ITEMSFMT = "ahb/{0}.conf"

ITEMS = "${@compose_list(d, 'ITEMSFMT', 'NAMES')}"
ITEMS += "iio-hwmon.conf"

ENVS = "obmc/hwmon/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN} += " ${@compose_list(d, 'ENVS', 'ITEMS')}"
