HOMEPAGE = " https://github.com/AspeedTech-BMC/u-boot"
SECTION = "bootloaders"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

# We use the revision in order to avoid having to fetch it from the
# repo during parse
SRCREV = "ad1ce74107a6f274db81c157b77e69261858fdbb"

UBRANCH = "pilot4_initial"
SRC_URI = "git://git@github.com/AspeedTech-BMC/u-boot.git;branch=${UBRANCH};protocol=https"

S = "${WORKDIR}/git"

PV = "v2016.07+git${SRCPV}"

