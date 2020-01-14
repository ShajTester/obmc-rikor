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

SUMMARY = "Fan control"
DESCRIPTION = "Fan control service by Rikor"
SECTION = "base"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"

SRC_URI = " \
	file://LICENSE \
	file://CMakeLists.txt \
	file://rikfan.service \
	file://src/main.cpp \
	file://src/test.cpp \
	file://src/util.cpp \
	file://src/util.hpp \
	file://src/ec/pid.cpp \
	file://src/ec/pid.hpp \
	file://src/ec/stepwise.cpp \
	file://src/ec/stepwise.hpp \
	"
SRCREV = "${AUTOREV}"

DEPENDS += "systemd nlohmann-json"

S = "${WORKDIR}"

inherit cmake systemd

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=MinSizeRel"

FILES_${PN} += "/etc/rikfan"
SYSTEMD_SERVICE_${PN} += "rikfan.service"



