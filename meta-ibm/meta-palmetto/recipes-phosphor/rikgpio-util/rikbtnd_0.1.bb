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

SUMMARY = ""
DESCRIPTION = ""
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8264535c0c4e9c6c335635c4026a8022"

SRC_URI = "file://rikbtnd \
          "

S = "${WORKDIR}/rikbtnd"

DEPENDS += "glibc libgpio update-rc.d-native"

pkgdir = "rikbtnd"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  install -m 755 rikbtnd ${dst}/rikbtnd
  ln -snf ../fbpackages/${pkgdir}/rikbtnd ${bin}/rikbtnd

  install -d ${D}${sysconfdir}/init.d
  install -d ${D}${sysconfdir}/rcS.d
  install -m 755 rikbtnd-setup.sh ${D}${sysconfdir}/init.d/rikbtnd-setup.sh
  update-rc.d -r ${D} rikbtnd-setup.sh start 99 S .
}

FBPACKAGEDIR = "${prefix}/local/fbpackages"

FILES_${PN} = "${FBPACKAGEDIR}/rikbtnd ${prefix}/local/bin"
FILES_${PN} += "${sysconfdir}"

RDEPENDS_${PN} = "glibc libgpio"


