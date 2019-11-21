OBMC_IMAGE_EXTRA_INSTALL_append_ares = " mboxd"
IMAGE_INSTALL_append = " bmcweb"
IMAGE_INSTALL_append = " phosphor-webui"
IMAGE_INSTALL_append = " phosphor-hwmon"
# IMAGE_INSTALL_append = " obmc-control-chassis"

IMAGE_INSTALL += "libgpio"
# IMAGE_INSTALL += "liblog"
IMAGE_INSTALL += "lighttpd"
IMAGE_INSTALL += "rikcgi"
IMAGE_INSTALL += "rikgpio"
IMAGE_INSTALL += "rikbtnd"
IMAGE_INSTALL += "rikcgi"
IMAGE_INSTALL += "rikcgi-login"
IMAGE_INSTALL += "rikor-fru"
IMAGE_INSTALL += "rikfans"
IMAGE_INSTALL += "ledblink-lt"


