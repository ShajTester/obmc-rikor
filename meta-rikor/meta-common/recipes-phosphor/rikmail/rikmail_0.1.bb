
SUMMARY = "Email notifier"
DESCRIPTION = "Email notifier service by Rikor"
SECTION = "base"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=45522b330639b6c750b573e6685ec898"

# SRC_URI = "git://github.com/ShajTester/rikmail.git;branch=master;protocol=http"
# SRCREV = "${AUTOREV}"
# PV = "1.0+git${SRCPV}"

SRC_URI = "git://github.com/ShajTester/rikmail.git;protocol=http"
SRCREV = "f1f9d0eda3cc7589a1dd63c05c118ad8a237c0a0"
# PV = "1.0+git${SRCPV}"

DEPENDS += "systemd nlohmann-json"
# DEPENDS += "glibc dbus dbus-glib glib-2.0 glib-2.0-native"
DEPENDS += "glib-2.0 glib-2.0-native"

S = "${WORKDIR}/git"

inherit cmake systemd pkgconfig

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=MinSizeRel"

FILES_${PN} = "/usr/bin"
FILES_${PN} += "${systemd_system_unitdir}"
# FILES_${PN} += "${ROOT_HOME}"
# FILES_${PN} += "/etc/rikmail"

SYSTEMD_SERVICE_${PN} += "rikmail.service"
SYSTEMD_SERVICE_${PN} += "send-report.service"
SYSTEMD_SERVICE_${PN} += "send-report.timer"

