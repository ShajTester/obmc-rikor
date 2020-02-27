#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "Rikfan manager for DBus"
SECTION = "examples"
DEPENDS += "glibc dbus dbus-glib glib-2.0 glib-2.0-native"
# DEPENDS += "glib-2.0 glib-2.0-native"
RDEPENDS_${PN} += ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"

# This tells bitbake where to find the files we're providing on the local filesystem
#FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

# Use local tarball
SRC_URI =  "file://client.c \
            file://com.rikor.ares.rikfan.xml \
            file://com.rikors.ares.xml \
            file://Makefile \
            file://README.md \
            file://server.c \
            file://LICENSE "


#inherit systemd
inherit pkgconfig

# Make sure our source directory (for the build) matches the directory structure in the tarball
S = "${WORKDIR}"

do_compile() {
	make gen
	make
}

do_install() {
	install -d ${D}${bindir}/rfm-srv
	install -m 0755 rfm-srv ${D}${bindir}/rfm-srv
	install -m 0755 client ${D}${bindir}/rfm-srv
}

FILES_${PN} += "${bindir}/rfm-srv"
