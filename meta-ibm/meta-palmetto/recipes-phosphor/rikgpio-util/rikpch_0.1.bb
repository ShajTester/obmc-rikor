
SUMMARY = ""
DESCRIPTION = ""
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8264535c0c4e9c6c335635c4026a8022"

SRC_URI = "file://rikpch"

S = "${WORKDIR}/rikpch"

DEPENDS+="i2c-tools"

pkgdir = "rikpch"

do_install() {
  dst="${D}/usr/local/fbpackages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  install -m 755 rikpch ${dst}/rikpch
  ln -snf ../fbpackages/${pkgdir}/rikpch ${bin}/rikpch
}

FBPACKAGEDIR = "${prefix}/local/fbpackages"

FILES_${PN} = "${FBPACKAGEDIR}/rikpch ${prefix}/local/bin"

#RDEPENDS_${PN} = "glibc"


