# Copyright 2014-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#

SUMMARY = "rikbtnd"
DESCRIPTION = "rikbtnd"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8264535c0c4e9c6c335635c4026a8022"

SRC_URI = "file://rikbtnd"

S = "${WORKDIR}/rikbtnd"

DEPENDS += "glibc libgpio"

do_install() {
  bin="${D}/usr/local/bin"
  install -d $bin
  install -m 755 rikbtnd ${bin}/rikbtnd
  install -m 755 rikbtnd-setup.sh ${bin}/rikbtnd-setup.sh
  install -m 755 rikbtnd-afterpoweron.sh ${bin}/rikbtnd-afterpoweron.sh

  install -d ${D}${systemd_system_unitdir}
  install -m 644 rikbtnd.service ${D}${systemd_system_unitdir}/rikbtnd.service

  install -d ${D}${ROOT_HOME}
  install -m 644 .profile ${D}${ROOT_HOME}/.profile
}

FILES_${PN} = "${prefix}/local/bin"
FILES_${PN} += "${systemd_system_unitdir}"
FILES_${PN} += "${ROOT_HOME}"

RDEPENDS_${PN} = "glibc libgpio"


REQUIRED_DISTRO_FEATURES= "systemd"

inherit systemd
SYSTEMD_SERVICE_${PN} = "rikbtnd.service"

