#@TYPE: Machine
#@NAME: Aspeed AST2600
#@DESCRIPTION: Common machine configuration for the Aspeed AST2600 Chip

require conf/machine/include/aspeed.inc

UBOOT_MACHINE ?= "ast2600_openbmc_defconfig"
UBOOT_ENTRYPOINT ?= "0x80008000"
UBOOT_LOADADDRESS ?= "0x80008000"

MACHINE_EXTRA_RDEPENDS += "udev-aspeed-vuart"

DEFAULTTUNE ?= "arm7a-novfp"

SERIAL_CONSOLES = "115200;ttyS4"

SOC_FAMILY = "aspeed-g6"
include conf/machine/include/soc-family.inc
MACHINEOVERRIDES .= ":ast2600"

require conf/machine/include/tune-arm7a-novfp.inc
