FILESEXTRAPATHS_prepend_ares := "${THISDIR}/${PN}:"
SRC_URI += "file://ares.cfg"
SRC_URI += "file://aspeed-bmc-opp-ares.dts.patch"
