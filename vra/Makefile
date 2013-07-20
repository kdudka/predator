# Copyright (C) 2013 Daniela Ďuričeková <xduric00@stud.fit.vutbr.cz>
#
# This file is part of value-range analyzer.
#
# Value-range analyzer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or any later version.
#
# Value-range analyzer is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# the value-range analyzer. If not, see <http://www.gnu.org/licenses/>.

CMAKE           ?= cmake
CTEST           ?= ctest

.PHONY: all check check-unit check-overall clean cppcheck doc \
	distclean distcheck pack version.h

all: version.h
	$(MAKE) -C ../cl_build # make sure the libcl.so is up2date
	mkdir -p ../vra_build
	cd ../vra_build && $(CMAKE) -DCMAKE_BUILD_TYPE=Debug ../vra
	$(MAKE) -C ../vra_build

check-unit:
	$(MAKE) -C tests-unit
	cd tests-unit && ./run-all-unit-tests.sh

check-overall:
	$(MAKE) -C tests-overall
	cd tests-overall && ./tests-run.sh

check: all
	$(MAKE) check-unit check-overall

cppcheck: all
	cppcheck --enable=style,performance,portability,information,missingInclude \
		--template gcc .

doc:
	$(MAKE) -C doc

version.h:
	@if test -e ../.git; then \
		printf "#define VRA_GIT_SHA1 \"%s\"\n" \
			`git log -1 | head -1 | cut -f2 -d" "` \
			| tee $@.tmp \
			&& install -m0644 -C -v $@.tmp $@ \
			&& rm -f $@.tmp; \
		fi

clean:
	if test -e ../.git; then rm -f version.h; fi
	if test -e ../vra_build/Makefile; then \
		$(MAKE) clean -C ../vra_build; \
	fi
	find . -name "a.out" -exec rm '{}' \;
	$(MAKE) clean -C doc
	$(MAKE) clean -C tests-overall
	$(MAKE) clean -C tests-unit

distclean: clean
	rm -rf ../vra_build

distcheck: distclean
	$(MAKE) check

pack:
	$(MAKE) clean
	tar -C ../ --exclude "*.tar.gz" -f vra-`date +"%Y-%m-%d"`.tar.gz -cvz vra
