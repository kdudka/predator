# Copyright (C) 2009-2010 Kamil Dudka <kdudka@redhat.com>
#
# This file is part of predator.
#
# predator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# predator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with predator.  If not, see <http://www.gnu.org/licenses/>.

GCC45           ?= gcc-4.5.1.tar.bz2#       # released gcc 4.5.1
GCC45_DIR       ?= gcc-4.5.1#               # unpackged release of gcc 4.5.1

GCC_SRC         ?= gcc-src#                 # SVN working copy for gcc src
GCC_BUILD       ?= gcc-build#               # working directory gcc build
GCC_INSTALL     ?= gcc-install#             # where to install gcc from SVN

GCC_LIBS_PREFIX ?= /usr#                    # common prefix for gcc prereqs

GMP_LIB         ?= $(GCC_LIBS_PREFIX)#      # location of -lgmp
MPC_LIB         ?= $(GCC_LIBS_PREFIX)#      # location of -lmpc
MPFR_LIB        ?= $(GCC_LIBS_PREFIX)#      # location of -lmpfr

INVADER         ?= invader.zip
INVADER_DIR     ?= invader-1_1
INVADER_SRC     ?= $(INVADER_DIR)/sources
INVADER_CIL     ?= $(INVADER_SRC)/cil

SPARSE          ?= sparse#                  # local git repo for SPARSE
SSD_GIT         ?= ssd#                     # local git repo for SSD

CURL            ?= curl --location -v#      # URL grabber command-line
GIT             ?= git#                     # use this to override git(1)
SVN             ?= svn#                     # use this to override svn(1)

.PHONY: all check clean distcheck distclean fetch unpack \
	build_gcc build_gcc_svn update_gcc update_gcc_src_only \
	build_inv \
	api_cl api_sl api

all: $(SSD_GIT) include/gcc/
	$(MAKE) -C cl $@
	$(MAKE) -C fwnull $@
	$(MAKE) -C sl $@
	$(MAKE) -C fa_analysis $@

# fetch all, but gcc
fetch: $(INVADER) $(SPARSE) $(SSD_GIT)

# unpack Invader's sources
unpack: $(INVADER_DIR) $(GCC45_DIR)

# wipe out all, but gcc
clean:
	rm -rf $(INVADER) $(SPARSE) $(SSD_GIT) $(GCC45_DIR)
	rm -rf $(INVADER_DIR)
	rm -rf $(GCC_BUILD)
	rm -rf cl_build fwnull_build sl_build fa_analysis_build
	$(MAKE) -C sl clean

# wipe out all
distclean: clean
	rm -rf gcc $(GCC_SRC) $(GCC_INSTALL)
	$(MAKE) -C sl distclean

check: include/gcc/
	$(MAKE) -C cl $@
	$(MAKE) -C fwnull $@
	$(MAKE) -C sl $@
	$(MAKE) -C fa_analysis $@

distcheck: include/gcc/
	$(MAKE) -C cl $@
	$(MAKE) -C fwnull $@
	$(MAKE) -C sl $@
	$(MAKE) -C fa_analysis $@

api_cl:
	$(MAKE) -C cl/api clean
	$(MAKE) -C cl/api

api_sl:
	$(MAKE) -C sl/api clean
	$(MAKE) -C sl/api

api: api_cl api_sl

# unpack released gcc 4.5
$(GCC45_DIR): $(GCC45)
	tar xf $(GCC45)

# initialize a git repo for Invader and apply downstream patches
$(INVADER_DIR): $(INVADER)
	unzip -o $(INVADER)
	cd $(INVADER_DIR) \
		&& $(GIT) init \
		&& $(GIT) add * \
		&& $(GIT) commit -m "initial import of $(INVADER)" \
		&& $(GIT) branch orig \
		&& $(GIT) am ../invader-extras/00*.patch \
		&& $(GIT) checkout -b next
	cd $(INVADER_SRC) && ../../ocaml/mltags

# build Invader from sources
build_inv: $(INVADER_DIR)
	cd $(INVADER_CIL) && ./configure # TODO: --prefix=...
	$(MAKE) -C $(INVADER_CIL) -j1 # oops, we don't support parallel build?
	# TODO: make check ... challenge? :-)
	# TODO: make install

# build gcc from the released tarball, instead of the SVN sources
build_gcc: $(GCC45_DIR)
	ln -fsvT $(GCC45_DIR) $(GCC_SRC)
	$(MAKE) build_gcc_svn

# build gcc from sources
build_gcc_svn: $(GCC_SRC)
	@if test -d $(GCC_BUILD); then \
			echo; \
			echo "--- directory '$(GCC_BUILD)' exists"; \
			echo "--- configure script will NOT be run explicitly"; \
			echo "--- please run 'rm -rf $(GCC_BUILD)' if the build fails"; \
			echo; \
		else \
			set -x \
			&& mkdir $(GCC_BUILD) \
			&& TOP_LEVEL=`pwd` \
			&& cd $(GCC_BUILD) \
			&& $$TOP_LEVEL/$(GCC_SRC)/configure \
				--enable-languages=c++,c \
				--disable-multilib \
				--prefix=$$TOP_LEVEL/$(GCC_INSTALL) \
				--with-gmp=$(GMP_LIB) \
				--with-mpc=$(MPC_LIB) \
				--with-mpfr=$(MPFR_LIB); \
		fi
	cd $(GCC_BUILD) && $(MAKE)
	cd $(GCC_BUILD) && $(MAKE) -j1 install
	$(MAKE) include/gcc

# updated SVN working directory of gcc
update_gcc_src_only: $(GCC_SRC)
	cd $(GCC_SRC) && $(SVN) up

# fetch up2date sources of gcc and rebuild it
update_gcc: update_gcc_src_only
	$(MAKE) build_gcc_svn

# fetch Invader tarball
$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

# fetch released gcc 4.5
$(GCC45):
	$(CURL) -o $@ 'ftp://ftp.lip6.fr/pub/gcc/releases/gcc-4.5.1/gcc-4.5.1.tar.bz2'

# initialize a local git repo for SPARSE
$(SPARSE):
	$(GIT) clone git://git.kernel.org/pub/scm/devel/sparse/chrisl/sparse.git $@

# create SVN working copy for gcc sources
$(GCC_SRC):
	$(SVN) co svn://gcc.gnu.org/svn/gcc/trunk $@

# clone read-only git repo of SSD
$(SSD_GIT):
	$(GIT) clone http://dudka.no-ip.org/git/ssd.git $@

include/gcc: gcc-install/lib/gcc
	cd include && ln -fsvT ../gcc-install/lib/gcc/`ls ../gcc-install/lib/gcc/`/4.[56]*/plugin/include gcc
