# Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

.PHONY: all check clean cppcheck distclean distcheck version.h

all: version.h
	$(MAKE) -C ../cl_build # make sure that libcl.a is up2date
	mkdir -p ../fwnull_build
	cd ../fwnull_build && $(CMAKE) ../fwnull
	$(MAKE) -C ../fwnull_build

check: all
	cd ../fwnull_build && $(CTEST) --output-on-failure

cppcheck: all
	cppcheck --enable=style,performance,portability,information,missingInclude \
		--template gcc .

version.h:
	@if test -e ../.git; then \
		printf "#define FWNULL_GIT_SHA1 \"%s\"\n" \
			`git log -1 | head -1 | cut -f2 -d" "` \
			| tee $@.tmp \
			&& install -m0644 -C -v $@.tmp $@ \
			&& rm -f $@.tmp; \
		fi

clean:
	if test -e ../.git; then rm -f version.h; fi
	if test -e ../fwnull_build/Makefile; then \
		$(MAKE) clean -C ../fwnull_build; \
	fi

distclean: clean
	rm -rf ../fwnull_build

distcheck: distclean
	$(MAKE) check CMAKE='cmake -D FWNULL_DEBUG=ON'
	$(MAKE) check CMAKE='cmake -D FWNULL_DEBUG=OFF'
