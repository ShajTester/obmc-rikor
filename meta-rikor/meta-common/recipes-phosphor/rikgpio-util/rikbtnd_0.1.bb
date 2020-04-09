
SUMMARY = "rikbtnd"
DESCRIPTION = "rikbtnd"
SECTION = "base"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"

SRC_URI = "file://rikbtnd"

S = "${WORKDIR}/rikbtnd"

DEPENDS += "glibc libgpio"
DEPENDS += "glib-2.0 glib-2.0-native"

inherit cmake systemd pkgconfig


# do_install() {
#   bin="${D}/usr/local/bin"
#   install -d $bin
#   install -m 755 rikbtnd ${bin}/rikbtnd
#   install -m 755 rikbtnd-setup.sh ${bin}/rikbtnd-setup.sh
#   install -m 755 rikbtnd-afterpoweron.sh ${bin}/rikbtnd-afterpoweron.sh

#   install -d ${D}${systemd_system_unitdir}
#   install -m 644 rikbtnd.service ${D}${systemd_system_unitdir}/rikbtnd.service

#   install -d ${D}${ROOT_HOME}
#   install -m 644 .profile ${D}${ROOT_HOME}/.profile
# }

FILES_${PN} = "/usr/bin"
FILES_${PN} += "${systemd_system_unitdir}"
FILES_${PN} += "${ROOT_HOME}"

RDEPENDS_${PN} = "glibc libgpio"

REQUIRED_DISTRO_FEATURES= "systemd"

SYSTEMD_SERVICE_${PN} = "rikbtnd.service"

