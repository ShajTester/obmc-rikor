


PR = "r1"

DESCRIPTION = "FRU base board"
SECTION = "base"

LICENSE = "GPL-2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8264535c0c4e9c6c335635c4026a8022"

DEPENDS = "glibc"
# DEPENDS_${PN}-dev = "glibc"
RDEPENDS_${PN} += "glibc"

SRCREV = "${AUTOREV}"
SRC_URI = "file://rikor-fru"

S = "${WORKDIR}/rikor-fru"

inherit cmake
EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_SHARED_LIBS=ON"



INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN}-dev += "dev-elf"
FILES_SOLIBSDEV = ""
FILES_${PN} += "${libdir}"
FILES_${PN} += "${libdir}/*.so"


do_install_append() {
  install -d ${D}${systemd_system_unitdir}
  install -m 755 rikor-fru.service ${D}${systemd_system_unitdir}/rikor-fru.service
}

FILES_${PN} += "${systemd_system_unitdir}"

inherit systemd

SYSTEMD_SERVICE_${PN} = "rikor-fru.service"

REQUIRED_DISTRO_FEATURES= "systemd"
