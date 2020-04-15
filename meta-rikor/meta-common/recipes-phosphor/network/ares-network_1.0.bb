
DESCRIPTION = "Network config for rikor ares project"
SECTION = "base"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=838c366f69b72c5df05c96dff79b35f2"


# Use local tarball
SRC_URI = "file://00-bmc-eth0.network \
           file://LICENSE "

# Make sure our source directory (for the build) matches the directory structure in the tarball
S = "${WORKDIR}"

do_install() {
	install -d ${D}/etc/systemd/network
	install -m 0644 00-bmc-eth0.network ${D}/etc/systemd/network
}

