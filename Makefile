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

GCC45           ?= gcc-4.5.2.tar.bz2#       # released gcc 4.5.2
GCC45_DIR       ?= gcc-4.5.2#               # unpackged release of gcc 4.5.2

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

CURL            ?= curl --location -v#      # URL grabber command-line
GIT             ?= git#                     # use this to override git(1)
SVN             ?= svn#                     # use this to override svn(1)

DIRS_BUILD      ?= cl fwnull sl fa
DIRS_INSTALL    ?= cl fwnull sl

.PHONY: all check clean distcheck distclean api cl/api sl/api ChangeLog \
	build_gcc build_gcc_svn update_gcc update_gcc_src_only \
	build_inv

all: include/gcc/
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

check: include/gcc/
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

clean:
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

distclean:
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

distcheck: include/gcc/
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

install: include/gcc/
	$(foreach dir, $(DIRS_INSTALL), $(MAKE) -C $(dir) $@ &&) true

cl/api:
	$(MAKE) -C cl/api clean
	$(MAKE) -C cl/api

sl/api:
	$(MAKE) -C sl/api clean
	$(MAKE) -C sl/api

api: cl/api sl/api

# unpack released gcc 4.5
$(GCC45_DIR): $(GCC45)
	test -d $(GCC45_DIR) || tar xf $(GCC45)

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
$(GCC_SRC):
	@if test -d $(GCC_SRC); then \
			echo "--- keeping '$(GCC_SRC)' as is"; \
		else \
			set -x \
			&& $(MAKE) $(GCC45_DIR) \
			&& ln -fsvT $(GCC45_DIR) $(GCC_SRC) \
			&& readlink -e $(GCC_SRC); \
		fi

# build gcc from sources
build_gcc: $(GCC_SRC)
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
				--disable-bootstrap \
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
update_gcc_src_only:
	test -d $(GCC_SRC)
	test -d $(GCC_SRC)/.svn
	cd $(GCC_SRC) && $(SVN) up

# fetch up2date sources of gcc and rebuild it
update_gcc: update_gcc_src_only
	$(MAKE) build_gcc

# fetch Invader tarball
$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

# fetch released gcc 4.5
$(GCC45):
	$(CURL) -o $@ 'ftp://ftp.lip6.fr/pub/gcc/releases/gcc-4.5.2/gcc-4.5.2.tar.bz2'

# create SVN working copy for gcc sources
build_gcc_svn:
	if test -e "$(GCC_SRC)"; then exit 1; fi
	$(SVN) co svn://gcc.gnu.org/svn/gcc/trunk $(GCC_SRC)
	$(MAKE) build_gcc

include/gcc: gcc-install/lib/gcc
	cd include && ln -fsvT ../gcc-install/lib/gcc/`ls ../gcc-install/lib/gcc/`/4.[56]*/plugin/include gcc

ChangeLog:
	git log --pretty="format:%ad  %an%n%n%w(80,8,8)%B%n" --date=short > $@

gcc-install/lib/gcc:
	@echo "*** 'gcc-install/lib/gcc' does not exist.  If you want to proceed"
	@echo "*** with standalone build of gcc, try 'make build_gcc' first.  If"
	@echo "*** it does not help, consult ./FAQ.  If anything goes wrong,"
	@echo "*** please submit a bug report to <idudka@fit.vutbr.cz>."
	@echo
	@false
