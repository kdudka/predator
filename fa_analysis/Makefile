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

CMAKE           ?= cmake
CTEST           ?= ctest

.PHONY: all clean distclean \
	distcheck

all: ../cl_build/libcl.so
	$(MAKE) -C ../cl_build # make sure the libcl.so is up2date
	mkdir -p ../fa_analysis_build
	cd ../fa_analysis_build && $(CMAKE) ../fa_analysis
	$(MAKE) -C ../fa_analysis_build

check: all
	cd ../fa_analysis_build && $(CTEST) --output-on-failure

clean:
	rm -f version.h
	$(MAKE) clean -C data
	if test -e ../fa_analysis_build/Makefile; then \
		$(MAKE) clean -C ../fa_analysis_build; \
	fi

distclean: clean
	rm -rf ../fa_analysis_build

distcheck: distclean
	$(MAKE) check

../cl_build/libcl.so:
	$(MAKE) -C ../cl
