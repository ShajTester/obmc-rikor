DESCRIPTION = "Linux kernel for Aspeed"
SECTION = "kernel"
LICENSE = "GPLv2"

PROVIDES += "virtual/kernel"

KCONFIG_MODE="--alldefconfig"

KSRC ?= "git://github.com/AspeedTech-BMC/linux-4.9;protocol=git;branch=${KBRANCH}"
SRC_URI = "${KSRC}"
SRC_URI += " file://defconfig"

FILESEXTRAPATHS_prepend_ast2400 := "${THISDIR}/ast2400:"
FILESEXTRAPATHS_prepend_ast2500 := "${THISDIR}/ast2500:"
FILESEXTRAPATHS_prepend_ast2600 := "${THISDIR}/ast2600:"

LINUX_VERSION_EXTENSION ?= "-${SRCREV}"

PV = "${LINUX_VERSION}+git${SRCPV}"

inherit kernel
require recipes-kernel/linux/linux-yocto.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
