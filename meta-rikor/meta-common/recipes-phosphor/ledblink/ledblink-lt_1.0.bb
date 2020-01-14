#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "LED blink application"
SECTION = "examples"
DEPENDS += ""
RDEPENDS_${PN} += "bash"
LICENSE = "GPL-2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8264535c0c4e9c6c335635c4026a8022"

# This tells bitbake where to find the files we're providing on the local filesystem
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

# Use local tarball
SRC_URI = "file://ledblink-${PV} \
           file://LICENSE "

# Make sure our source directory (for the build) matches the directory structure in the tarball
S = "${WORKDIR}"

do_install() {
	install -d ${D}${bindir}
	install -m 0775 ledblink-${PV} ${D}${bindir}
}

