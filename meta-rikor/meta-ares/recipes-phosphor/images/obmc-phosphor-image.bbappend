

# Install entire Phosphor application stack by default

IMAGE_FEATURES += "obmc-bmc-state-mgmt"
# IMAGE_FEATURES += "obmc-host-ctl"
IMAGE_FEATURES += "obmc-host-ipmi"
IMAGE_FEATURES += "obmc-logging-mgmt"
IMAGE_FEATURES += "ssh-server-dropbear"
IMAGE_FEATURES += "obmc-sensors"
IMAGE_FEATURES += "obmc-network-mgmt"
IMAGE_FEATURES += "obmc-settings-mgmt"
IMAGE_FEATURES += "obmc-system-mgmt"
# IMAGE_FEATURES += "obmc-user-mgmt"
# IMAGE_FEATURES += "obmc-debug-collector"
# IMAGE_FEATURES += "obmc-chassis-mgmt"
# IMAGE_FEATURES += "obmc-chassis-state-mgmt"
IMAGE_FEATURES += "obmc-flash-mgmt"
# IMAGE_FEATURES += "obmc-host-state-mgmt"
IMAGE_FEATURES += "obmc-inventory"
# IMAGE_FEATURES += "obmc-remote-logging-mgmt"
IMAGE_FEATURES += "obmc-net-ipmi"
IMAGE_FEATURES += "obmc-software"
IMAGE_FEATURES += "obmc-leds"
# IMAGE_FEATURES += "obmc-fan-control"



IMAGE_FEATURES_append_df-obmc-ubi-fs = " read-only-rootfs"


CORE_IMAGE_EXTRA_INSTALL_append = " bash "
CORE_IMAGE_EXTRA_INSTALL_append = " packagegroup-obmc-apps-extras "
CORE_IMAGE_EXTRA_INSTALL_append = " packagegroup-obmc-apps-extrasdevtools "
CORE_IMAGE_EXTRA_INSTALL_append = " i2c-tools "
CORE_IMAGE_EXTRA_INSTALL_append = " obmc-console "
CORE_IMAGE_EXTRA_INSTALL_append = " pam-plugin-access "
CORE_IMAGE_EXTRA_INSTALL_append = " pam-ipmi "
CORE_IMAGE_EXTRA_INSTALL_append = " ${OBMC_IMAGE_EXTRA_INSTALL} "
CORE_IMAGE_EXTRA_INSTALL_append = " ffdc "
CORE_IMAGE_EXTRA_INSTALL_append = " rsync "
CORE_IMAGE_EXTRA_INSTALL_append = " rng-tools "
CORE_IMAGE_EXTRA_INSTALL_append = " lrzsz "
CORE_IMAGE_EXTRA_INSTALL_append = " postfix "

