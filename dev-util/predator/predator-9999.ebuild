# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=3

SHA1_LONG="57e2659fe995671bc261cc4a6303e376c9efe2ae"
SHA1_SHORT="57e2659"
GCC_HOST_VERSION="4.5.1"

GCC_HOST="gcc-${GCC_HOST_VERSION}"
GCC_HOST_INC="/usr/lib/gcc/${CHOST}/${GCC_HOST_VERSION}/plugin/include"

DESCRIPTION="predator - only libfwnull.so gcc plug-in for now"
HOMEPAGE="http://github.com/kdudka/predator"
SRC_URI="http://github.com/kdudka/${PN}/tarball/${SHA1_LONG}/${PN}-${SHA1_SHORT}.tar.gz"
SSD_HOST="http://www.stud.fit.vutbr.cz"
SSD_SNAP="${SSD_HOST}/~xdudka00/cgi-bin/gitweb.cgi?p=ssd;a=snapshot;h=HEAD;sf=tgz"
RESTRICT="mirror"
FEATURES="test"

LICENSE="GPLv3"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

DEPEND="=sys-devel/${GCC_HOST} dev-libs/boost"
RDEPEND="${DEPEND} dev-util/cmake dev-util/valgrind net-misc/curl"

src_unpack(){
	unpack "${A}"
	cd "kdudka-${PN}-${SHA1_SHORT}" || die "invalid tarball"

	# FIXME: ugly
	einfo "downloading ssd from ${SSD_HOST}..."
	curl -s "${SSD_SNAP}" | tar -vxz || die "unable to download ssd"

	einfo "initializing git SHA1:"
	echo "#define CL_GIT_SHA1 \"${SHA1_LONG}\"" | tee cl/version_cl.h
	echo "#define SL_GIT_SHA1 \"${SHA1_LONG}\"" | tee sl/version.h
	echo "#define FWNULL_GIT_SHA1 \"${SHA1_LONG}\"" | tee fwnull/version.h
}

src_compile(){
	cd "kdudka-${PN}-${SHA1_SHORT}"

	# find host gcc's headers
	einfo "looking for host gcc's headers:"
	ln -sfT "${GCC_HOST_INC}" include/gcc
	readlink -e include/gcc || die "headers of ${GCC_HOST} not found"

	emake "CMAKE=cmake -D GCC_HOST=${GCC_HOST} -D CMAKE_INSTALL_PREFIX=/usr"
}

src_test(){
	einfo "running code listener's test-suite..."
	cd "kdudka-${PN}-${SHA1_SHORT}"
	emake check || die "failure detected by test-suite"
}

src_install(){
	cd "kdudka-${PN}-${SHA1_SHORT}"
	einstall "DESTDIR=${D}"
}
