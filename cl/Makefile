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

.PHONY: all check clean cppcheck distclean distcheck fast version_cl.h

all: version_cl.h
	mkdir -p ../cl_build
	cd ../cl_build && $(CMAKE) ../cl
	$(MAKE) -C ../cl_build

fast:
	$(MAKE) -sC ../cl_build

check: all
	cd ../cl_build && $(CTEST) --output-on-failure

cppcheck: all
	cppcheck -j5 --inline-suppr \
		--enable=style,performance,portability,information,missingInclude \
		--template '{file}:{line}: {severity}: {id}: {message}' -i tests .

version_cl.h:
	@if test -e ../.git; then \
		printf "#define CL_GIT_SHA1 \"%s\"\n" \
			`git log -1 | head -1 | cut -f2 -d" "` \
			> $@.tmp \
			&& install -m0644 -C -v $@.tmp $@ \
			&& rm -f $@.tmp; \
		fi

clean:
	if test -e ../.git; then rm -f version_cl.h; fi
	if test -e ../cl_build/Makefile; then $(MAKE) clean -C ../cl_build; fi

distclean: clean
	rm -rf ../cl_build

distcheck: distclean
	$(MAKE) check CMAKE='cmake -D CL_DEBUG=ON'
	$(MAKE) check CMAKE='cmake -D CL_DEBUG=OFF'
