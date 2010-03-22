# Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
#
# This file is part of sl.
#
# sl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# sl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with sl.  If not, see <http://www.gnu.org/licenses/>.

CMAKE           ?= cmake
CTEST           ?= ctest
DATASET         ?= data/*.c
TIMEOUT         ?= timeout 300

GCC_SRC         ?= ../gcc-src
GCC_BUILD       ?= ../gcc-build
GCC_INSTALL     ?= ../gcc-install

LD              ?= g++
AR              ?= ar
RANLIB          ?= ranlib

PLUG_CFLAGS     ?= -fPIC -std=c99 -pedantic -Wall -W -Werror -I.. -I../gcc -iquote.

.PHONY: all clean distclean \
	distcheck \
	version.h

all: version.h ../ssd
	mkdir -p ../sl_build
	cd ../sl_build && $(CMAKE) ../sl
	$(MAKE) -C ../sl_build

check: all
	cd ../sl_build && $(CTEST) --output-on-failure

version.h:
	printf "#define SL_GIT_SHA1 \"%s\"\n" \
		`git log | head -1 | cut -f2 -d" "` \
		| tee $@.tmp \
		&& install -C -v $@.tmp $@ \
		&& rm -f $@.tmp

CL_OBJS = cl_chain.o \
	cl_dotgen.o \
	cl_factory.o \
	cl_locator.o \
	cl_pp.o \
	cl_symexec.o \
	cl_storage.o \
	cl_typedot.o \
	cld_argsub.o \
	cld_intchk.o \
	cld_opchk.o \
	cld_optrans.o \
	cld_unilabel.o \
	cld_uniregs.o \
	cld_univars.o \
	cld_unswitch.o \
	code_listener.o \
	location.o \
	ssd.o \
	storage.o \
	symcall.o \
	symdump.o \
	symexec.o \
	symheap.o \
	symplot.o \
	symproc.o \
	symstate.o

CL_LIBS = -lboost_iostreams -lboost_system

libcl.a: $(CL_OBJS)
	$(AR) r $@ $(CL_OBJS)
	$(RANLIB) $@

SLSPARSE_OBJS = \
	hash_table.o \
	slsparse.o \
	type_enumerator.o

#TODO: dependency scanning
$(CL_OBJS) $(SLSPARSE_OBJS) slplug.o: *.h *.hh

$(CL_OBJS):     CXXFLAGS += -fPIC -std=c++0x -pedantic -Wall -Wextra
ssd.o:          CXXFLAGS += -DHAVE_ISATTY
slplug.o:         CFLAGS += $(PLUG_CFLAGS)

$(SLSPARSE_OBJS): ../sparse
$(SLSPARSE_OBJS): CFLAGS += -fPIC -Wall -Wextra -I.. 
slsparse: $(SLSPARSE_OBJS) libcl.a ../sparse/libsparse.a
	$(CC) $(CFLAGS) -o $@ $(SLSPARSE_OBJS) \
		-L. -lcl ../sparse/libsparse.a $(CL_LIBS) $(LDFLAGS)

slplug.so: LDFLAGS += -fPIC -shared $(CL_LIBS)
slplug.so: slplug.o libcl.a
	$(LD) -o $@ $(LDFLAGS) slplug.o -lcl -L.

clean:
	rm -f libcl.a $(CL_OBJS)
	rm -f test-list test-all test-all.o data/dll.o
	rm -f slsparse slsparse.o sparse-internal-symbols.dot
	rm -f hash_table.o type_enumerator.o
	rm -f slplug.o slplug.so
	$(MAKE) distclean -C data
	$(MAKE) clean -C ../sl_build || true

distclean: clean
	rm -f *.o *.dot
	rm -rf ../sl_build

distcheck: distclean
	$(MAKE) check

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

../sparse:
	$(MAKE) -C .. sparse

../sparse/libsparse.a: ../sparse
	$(MAKE) -C ../sparse

../ssd:
	$(MAKE) -C .. ssd

../ssd/util/faketty.so: ../ssd/util/faketty.c
	$(MAKE) -C ../ssd/util
