SUMMARY = "Phosphor LED Group Management for Rikor Ares board"
PR = "r1"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://led.yaml;md5=b82775f6a6e410826a22ce58aee576a2"

inherit native

PROVIDES += "virtual/phosphor-led-manager-config-native"

SRC_URI += "file://led.yaml"
S = "${WORKDIR}"

# Copies example led layout yaml file
do_install() {
    SRC=${S}
    DEST=${D}${datadir}/phosphor-led-manager
    install -D ${SRC}/led.yaml ${DEST}/led.yaml
}