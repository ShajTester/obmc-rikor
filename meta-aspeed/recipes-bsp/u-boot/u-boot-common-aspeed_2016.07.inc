HOMEPAGE = "https://github.com/openbmc/u-boot"
SECTION = "bootloaders"
DEPENDS += "flex-native bison-native"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

# We use the revision in order to avoid having to fetch it from the
# repo during parse
#SRCREV = "03d9fa47384e41ecc210081a0db145b5880cc2f8"
SRCREV = "3c38e8c510307e9b5530b3c62ff92fb1bdfda9c1"

UBRANCH = "ast2600_openbmc"
SRC_URI = "git://git@github.com/AspeedTech-BMC/u-boot.git;branch=${UBRANCH};protocol=https"
#SRC_URI = "git:///home/shivahs/AST2600/u-boot;branch=${UBRANCH};protocol=file"

S = "${WORKDIR}/git"

PV = "ast2600+git${SRCPV}"
