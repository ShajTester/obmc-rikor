HOMEPAGE = " https://github.com/AspeedTech-BMC/u-boot"
SECTION = "bootloaders"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

# We use the revision in order to avoid having to fetch it from the
# repo during parse
SRCREV = "9b682d258b7d599a1d0979d1baacbb9d367b6323"
#SRCREV = " ${AUTOREV}"

UBRANCH = "pilot4"
SRC_URI = "git://git@github.com/AspeedTech-BMC/u-boot.git;branch=${UBRANCH};protocol=https"
#SRC_URI = "git:///home/shivahs/pilot4/lenovo/ourgit/openbmc-rel5/u-boot;branch=${UBRANCH};protocol=file"
S = "${WORKDIR}/git"

#PV = "v2016.07+git${SRCPV}"

