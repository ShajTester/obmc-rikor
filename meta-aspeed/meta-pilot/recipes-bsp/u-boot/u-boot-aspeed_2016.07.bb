require u-boot-common-aspeed_${PV}.inc
# ./poky/meta/recipes-bsp/u-boot/u-boot.inc
require recipes-bsp/u-boot/u-boot.inc

PROVIDES += "u-boot"
DEPENDS += "dtc-native"

