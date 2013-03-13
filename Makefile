# Copyright (C) 2009-2011 Kamil Dudka <kdudka@redhat.com>
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

GCC_MIRROR      ?= http://ftp.fi.muni.cz/pub/gnu/gnu/gcc

BOOST_STABLE    ?= boost_1_46_1#            # released Boost
BOOST_STABLE_TGZ?= $(BOOST_STABLE).tar.bz2# # tarball of released Boost
BOOST_STABLE_URL?= http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.bz2/download

GCC_STABLE      ?= gcc-4.7.2#               # released gcc
GCC_STABLE_TGZ  ?= $(GCC_STABLE).tar.bz2#   # tarball of released gcc
GCC_STABLE_URL  ?= $(GCC_MIRROR)/$(GCC_STABLE)/$(GCC_STABLE_TGZ)

GCC_SRC         ?= gcc-src#                 # SVN working copy for gcc src
GCC_BUILD       ?= gcc-build#               # working directory gcc build
GCC_INSTALL     ?= gcc-install#             # where to install gcc from SVN

GCC_LIBS_PREFIX ?= /usr#                    # common prefix for gcc prereqs

GMP_LIB         ?= $(GCC_LIBS_PREFIX)#      # location of -lgmp
MPC_LIB         ?= $(GCC_LIBS_PREFIX)#      # location of -lmpc
MPFR_LIB        ?= $(GCC_LIBS_PREFIX)#      # location of -lmpfr

INVADER         ?= invader.zip
INVADER_DIR     ?= invader-1_1

CURL            ?= curl --location -v#      # URL grabber command-line
GIT             ?= git#                     # use this to override git(1)
SVN             ?= svn#                     # use this to override svn(1)

ANALYZERS       ?= fwnull sl fa
DIRS_BUILD      ?= cl $(ANALYZERS)

.PHONY: all check clean distcheck distclean api cl/api sl/api ChangeLog \
	build_boost \
	build_gcc build_gcc_svn update_gcc update_gcc_src_only \
	$(DIRS_BUILD)

all: cl
	$(MAKE) $(ANALYZERS)

$(DIRS_BUILD):
	$(MAKE) -C $@

check: all
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

clean:
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

distclean:
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

distcheck:
	$(foreach dir, $(DIRS_BUILD), $(MAKE) -C $(dir) $@ &&) true

cl/api:
	$(MAKE) -C cl/api clean
	$(MAKE) -C cl/api

sl/api:
	$(MAKE) -C sl/api clean
	$(MAKE) -C sl/api

api: cl/api sl/api

# unpack the release of Boost
$(BOOST_STABLE): $(BOOST_STABLE_TGZ)
	test -d $(BOOST_STABLE) || tar xf $(BOOST_STABLE_TGZ)

# unpack the release of gcc
$(GCC_STABLE): $(GCC_STABLE_TGZ)
	test -d $(GCC_STABLE) || tar xf $(GCC_STABLE_TGZ)

# upack the release of Invader
$(INVADER_DIR): $(INVADER)
	unzip -o $(INVADER)

# build gcc from the released tarball, instead of the SVN sources
$(GCC_SRC):
	@if test -d $(GCC_SRC); then \
			echo "--- keeping '$(GCC_SRC)' as is"; \
		else \
			set -x \
			&& $(MAKE) $(GCC_STABLE) \
			&& ln -fsvT $(GCC_STABLE) $(GCC_SRC) \
			&& readlink -e $(GCC_SRC); \
		fi

# prepare a local instance of Boost libraries
build_boost: $(BOOST_STABLE)
	cd include && ln -fsvT ../$(BOOST_STABLE)/boost boost

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

# fetch a stable release of Boost
$(BOOST_STABLE_TGZ):
	$(CURL) -o $@ '$(BOOST_STABLE_URL)'

# fetch a stable release of gcc
$(GCC_STABLE_TGZ):
	$(CURL) -o $@ '$(GCC_STABLE_URL)'

# create SVN working copy for gcc sources
build_gcc_svn:
	if test -e "$(GCC_SRC)"; then exit 1; fi
	$(SVN) co svn://gcc.gnu.org/svn/gcc/trunk $(GCC_SRC)
	$(MAKE) build_gcc

ChangeLog:
	git log --pretty="format:%ad  %an%n%n%w(80,8,8)%B%n" --date=short -- \
		$(CHLOG_WATCH) > $@
