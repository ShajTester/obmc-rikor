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

SUMMARY = "Identifier"
DESCRIPTION = "Broadcast network identifier"
SECTION = "base"
LICENSE = "CLOSED"
#LIC_FILES_CHKSUM = "file://LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"

SRC_URI[md5sum] = "0fcda301d8ca8c6ab7aa495e27f7e5d6"
SRC_URI = "git://git.rikor.com:3000/yurchenko/identifier;branch=development;protocol=http"
SRCREV = "92d61eec54aa1ea7f966bf058978cb6a6ee8674a"

S = "${WORKDIR}/git"

inherit cmake

