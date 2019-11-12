


SUMMARY = "CGI scripts"
DESCRIPTION = "Login CGI scripts"
SECTION = "base"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=cc4734e8a01f740811ac41130a40cf90"

DEPENDS += "rikor-fru"
RDEPENDS_${PN} += "libstdc++ rikor-fru"

SRCREV = "${AUTOREV}"
SRC_URI = "file://cpp"

inherit cmake
EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_SHARED_LIBS=ON"


S = "${WORKDIR}/cpp"

FBPACKAGEDIR = "/www/pages"

# INSANE_SKIP_${PN} += "dev-deps"

