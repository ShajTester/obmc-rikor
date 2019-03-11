HOMEPAGE = " https://github.com/AspeedTech-BMC/u-boot"
SECTION = "bootloaders"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

# We use the revision in order to avoid having to fetch it from the
# repo during parse
SRCREV = "a5b289a809aa322e73d2e148b8011a6a58135472"
#SRCREV = "a2b9d593bbbdaded165d8c596840fb742a1d3c42"
#SRCREV = " ${AUTOREV}"

UBRANCH = "thud"
SRC_URI = "git://git@github.com/AspeedTech-BMC/u-boot.git;branch=${UBRANCH};protocol=https"
#SRC_URI = "git:///home/vnigade/pilot4/u-boot;branch=${UBRANCH};protocol=file"
S = "${WORKDIR}/git"

#PV = "v2016.07+git${SRCPV}"

