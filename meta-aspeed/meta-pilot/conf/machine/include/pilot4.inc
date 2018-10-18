#@TYPE: Machine
#@NAME: Aspeed PILOT4
#@DESCRIPTION: Common machine configuration for the Aspeed PILOT4 Chip

require conf/machine/include/aspeed.inc

#UBOOT_MACHINE ?= "ast_g5_ncsi_config"
UBOOT_ENTRYPOINT ?= "0x81000000"
UBOOT_LOADADDRESS ?= "0x81000000"

#MACHINE_EXTRA_RDEPENDS += "udev-aspeed-vuart"

DEFAULTTUNE ?= "arm7a-novfp"

SERIAL_CONSOLES = "115200;ttyS4"

#SOC_FAMILY = "aspeed-g5"
#include conf/machine/include/soc-family.inc
#MACHINEOVERRIDES .= ":ast2500"

require conf/machine/include/tune-arm7a-novfp.inc

