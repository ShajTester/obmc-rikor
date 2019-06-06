HOMEPAGE = "https://github.com//AspeedTech-BMC/u-boot-2017.01"
SECTION = "bootloaders"
DEPENDS += "flex-native bison-native"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

# We use the revision in order to avoid having to fetch it from the
# repo during parse
SRCREV = "f77e606785978c9be7f3553e0e6564c5a586cdf0"

UBRANCH = "aspeed-dev-v2017.01"
SRC_URI = "git://git@github.com/AspeedTech-BMC/u-boot.git;branch=${UBRANCH};protocol=https"
#SRC_URI = "git:///home/shivahs/AST2600/uboot_201701/new/u-boot;branch=${UBRANCH};protocol=file"

S = "${WORKDIR}/git"

PV = "v2017.01+git${SRCPV}"
