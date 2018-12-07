HOMEPAGE = " https://github.com/AspeedTech-BMC/u-boot"
SECTION = "bootloaders"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

# We use the revision in order to avoid having to fetch it from the
# repo during parse
SRCREV = "86f13e23fb320478f9268c6792cac4d994c6dfd3"
#SRCREV = " ${AUTOREV}"

UBRANCH = "pilot4"
SRC_URI = "git://git@github.com/AspeedTech-BMC/u-boot.git;branch=${UBRANCH};protocol=https"
#SRC_URI = "git:///home/shivahs/pilot4/lenovo/ourgit/u-boot;branch=${UBRANCH};protocol=file"
S = "${WORKDIR}/git"

#PV = "v2016.07+git${SRCPV}"

