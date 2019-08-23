FILESEXTRAPATHS_prepend := "${THISDIR}/lighttpd:"

SRC_URI += " \
		file://css \
		file://img \
		file://js \
		file://jsnbd \
		"

RDEPENDS_${PN} += " \
		lighttpd-module-cgi \
		"

do_install_append() {

	dst_css="${D}/www/pages/css"
	dst_img="${D}/www/pages/img"
	dst_js="${D}/www/pages/js"
	dst_jsnbd="${D}/www/pages/jsnbd"

	install -d $dst_css
	install -d $dst_img
	install -d $dst_js
	install -d $dst_jsnbd

	install -m 0644 ${WORKDIR}/js/bmc.js ${dst_js}	
	install -m 777 ${WORKDIR}/css/bmc.css ${dst_css}
	install -m 0644 ${WORKDIR}/img/* ${dst_img}
	install -m 0644 ${WORKDIR}/jsnbd/* ${dst_jsnbd}
}

