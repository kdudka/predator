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

.PHONY: all clean distclean \
	distcheck \
	version.h

all: version.h ../cl_build/libcl.so
	$(MAKE) -C ../cl_build # make sure the libcl.so is up2date
	mkdir -p ../sl_build
	cd ../sl_build && $(CMAKE) ../sl
	$(MAKE) -C ../sl_build

check: all
	cd ../sl_build && $(CTEST) --output-on-failure

version.h:
	printf "#define SL_GIT_SHA1 \"%s\"\n" \
		`git log -1 . | head -1 | cut -f2 -d" "` \
		| tee $@.tmp \
		&& install -m0644 -C -v $@.tmp $@ \
		&& rm -f $@.tmp

clean:
	rm -f version.h
	$(MAKE) clean -C data
	if test -e ../sl_build/Makefile; then $(MAKE) clean -C ../sl_build; fi

distclean: clean
	rm -rf ../sl_build
	$(MAKE) distclean -C data

distcheck: distclean
	$(MAKE) check

../cl_build/libcl.so:
	$(MAKE) -C ../cl

#pphtml: ../sl_build/libslplug.so ../ssd/util/faketty.so
#	for i in $(DATASET); do \
#		LC_ALL=C \
#		LD_PRELOAD=../ssd/util/faketty.so FAKE_TTY=1,2 \
#			$(GCC_INSTALL)/bin/gcc $(CFLAGS) $(PLUG_CFLAGS) \
#				-fplugin=./slplug.so \
#				-fplugin-arg-libslplug-dump-pp \
#				-fplugin-arg-libslplug-dump-types \
#				-Wno-error -c $$i \
#				2>&1 \
#			| ../ssd/util/eschtml \
#			| ../ssd/util/tty2html \
#			> $$i.pp.html; \
#	done
#
#../ssd:
#	$(MAKE) -C .. ssd
#
#../ssd/util/faketty.so: ../ssd/util/faketty.c
#	$(MAKE) -C ../ssd/util
