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

SUMMARY = "Email notifier"
DESCRIPTION = "Email notifier service by Rikor"
SECTION = "base"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"

SRC_URI = " \
	file://LICENSE \
	file://CMakeLists.txt \
	file://rikmail.service \
	file://src/main.cpp \
	file://src/xyz.openbmc_project.ares.rikmail.xml \
	"
SRCREV = "${AUTOREV}"

DEPENDS += "systemd nlohmann-json"
#DEPENDS += "glibc dbus dbus-glib glib-2.0 glib-2.0-native"
DEPENDS += "glib-2.0 glib-2.0-native"

S = "${WORKDIR}"

inherit cmake systemd pkgconfig

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=MinSizeRel"

# FILES_${PN} += "/etc/rikfan"
SYSTEMD_SERVICE_${PN} += "rikmail.service"



