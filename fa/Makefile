# Copyright (C) 2010 Jiri Simacek
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

CMAKE ?= cmake
CTEST ?= ctest

.PHONY: all clean distclean \
	distcheck

all:
	$(MAKE) -C ../cl_build # make sure the libcl.so is up2date
	mkdir -p ../fa_build
	cd ../fa_build && $(CMAKE) ../fa
	$(MAKE) -C ../fa_build

check: all
	cd ../fa_build && $(CTEST) --output-on-failure

clean:
	if test -e ../fa_build/Makefile; then \
		$(MAKE) clean -C ../fa_build; \
	fi

distclean: clean
	rm -rf ../fa_build

distcheck: distclean
	$(MAKE) check; true
