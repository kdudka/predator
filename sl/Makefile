# Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
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

CMAKE           ?= cmake
CTEST           ?= ctest

.PHONY: all clean configure distclean distcheck fast version.h

all: version.h configure ../cl_build
	# make sure the libcl.so is up2date
	$(MAKE) -C ../cl_build
	mkdir -p ../sl_build
	cd ../sl_build && $(CMAKE) ../sl
	$(MAKE) -C ../sl_build
	@test -d ../.git || sed \
		"s|SL_PLUG=.*$$|SL_PLUG='`readlink -f ../sl_build/libsl.so`'|" -i \
		probe.sh slgcc ../chk-error-label-reachability.sh ../register-paths.sh

../cl_build:
	test -d $@ || $(MAKE) -C ../cl

fast:
	$(MAKE) -sC ../sl_build

check: all
	cd ../sl_build && $(CTEST) --output-on-failure

version.h:
	@if test -d ../.git; then \
		printf "#define SL_GIT_SHA1 \"%s\"\n" \
			`git log -1 | head -1 | cut -f2 -d" "` \
			> $@.tmp \
			&& install -m0644 -C -v $@.tmp $@ \
			&& rm -f $@.tmp; \
		fi

clean:
	if test -d ../.git; then rm -f version.h; fi
	$(MAKE) clean -C data -f Makefile.in
	if test -e ../sl_build/Makefile; then $(MAKE) clean -C ../sl_build; fi

configure:
	@cd data >/dev/null && ./configure

distclean: clean
	rm -rf ../sl_build

distcheck: distclean
	$(MAKE) check CMAKE='cmake -D TEST_ONLY_FAST=OFF -D SL_DEBUG=ON'
	$(MAKE) all CMAKE='cmake -D TEST_ONLY_FAST=ON -D SL_DEBUG=OFF'
