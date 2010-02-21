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

CILLY   ?= ../invader-1_1/sources/cil/bin/cilly
DATASET ?= data/dll.c data/slplug.c data/inv-*.c data/off.c data/hash_table.c \
		   data/test-0010.c
TIMEOUT ?= timeout 300

# SPARSE_CFLAGS += `pkg-config sparse --cflags`
# SPARSE_LIBS ?= `pkg-config sparse --libs`

SPARSE_CFLAGS = -I..
SPARSE_LIBS = ../sparse/libsparse.a

GCC_SRC = ../gcc-src
GCC_BUILD = ../gcc-build
GCC_INSTALL = ../gcc-install

# debug flags
DBGFLAGS ?= -g -O0
CFLAGS += $(DBGFLAGS)
CXXFLAGS += $(DBGFLAGS)

LD = g++
AR ?= ar
RANLIB ?= ranlib
GDB ?= gdb

.PHONY: all check clean distclean \
	chkplug chksparse chkunit chkhead \
	dbgplug dbgsparse \
	dotplug dotsparse \
	pphtml invpphtml \
	distcheck wizard

all: slplug.so

CL_OBJS = cl_chain.o \
	cl_dotgen.o \
	cl_factory.o \
	cl_locator.o \
	cl_pp.o \
	cl_seplog.o \
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
	symdump.o \
	symexec.o \
	symheap.o \
	symproc.o \
	symstate.o

libcl.a: $(CL_OBJS)
	$(AR) r $@ $(CL_OBJS)
	$(RANLIB) $@

#TODO: dependency scanning
$(CL_OBJS): code_listener.h config.h *.hh
cl_seplog.o: ../seplog/symbolic-heap.cc

#$(CL_OBJS): CXXFLAGS += -fPIC -std=c++0x -pedantic -Wall -Werror
$(CL_OBJS): CXXFLAGS += -fPIC -std=c++0x -pedantic -Wall
ssd.o: CXXFLAGS += -I../ssd/src -DHAVE_ISATTY

CL_LIBS = -lboost_iostreams -lboost_system

slsparse.o: code_listener.h type_enumerator.h
slsparse.o: CFLAGS += -fPIC -Wall $(SPARSE_CFLAGS)

hash_table.o: hash_table.h
type_enumerator.o: type_enumerator.h config.h
hash_table.o type_enumerator.o: CFLAGS += -std=c99 -pedantic -fPIC -Wall -Wextra -Werror

slsparse: hash_table.o slsparse.o type_enumerator.o libcl.a $(SPARSE_LIBS)
	$(CC) $(CFLAGS) -o $@ slsparse.o hash_table.o type_enumerator.o \
		-L. -lcl $(SPARSE_LIBS) $(CL_LIBS) $(LDFLAGS)
	../sparse/sparse -I.. slsparse.c hash_table.c type_enumerator.c

PLUG_CFLAGS = -fPIC -std=c99 -pedantic -Wall -W -Werror -I.. -I../gcc -iquote.

slplug.o: code_listener.h
slplug.o: CFLAGS += $(PLUG_CFLAGS)

slplug.so: LDFLAGS += -fPIC -shared $(CL_LIBS)
slplug.so: slplug.o libcl.a
	$(LD) -o $@ $(LDFLAGS) slplug.o -lcl -L.

test-dll: LDFLAGS += -lboost_unit_test_framework
test-dll: test-dll.o data/dll.o

test-list: data/list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

chkunit: test-dll
	./runtest.sh dll --build_info=yes --log_level=test_suite

chkplug: slplug.so
	TMP=`mktemp` \
		&& ($(GCC_INSTALL)/bin/gcc $(CFLAGS) $(PLUG_CFLAGS) \
		-fplugin=./slplug.so \
		-fplugin-arg-slplug-bypass-seplog \
		-c slplug.c >"$$TMP" 2>&1 \
		|| (cat "$$TMP"; exit 1)) \
		&& rm -fv "$$TMP"
	$(MAKE) slplug.so
	for i in $(DATASET); do \
		$(GCC_INSTALL)/bin/gcc $(CFLAGS) $(PLUG_CFLAGS) \
			-fplugin=./slplug.so \
			-fplugin-arg-slplug-verbose=1 \
			-fplugin-arg-slplug-dump-pp \
			-fplugin-arg-slplug-gen-dot=$$i-cg.dot \
			-fplugin-arg-slplug-type-dot=$$i-types.dot \
			-Wno-error -c $$i; \
	done

SFLAGS += -Wno-decl
chksparse: slsparse
	./slsparse $(CFLAGS) $(SFLAGS) $(PLUG_CFLAGS) -I. data/slplug.c

check: chkplug chksparse chkunit chkhead
	$(MAKE) dotplug
	$(MAKE) pphtml

chkhead:
	$(MAKE) -f Makefile.chk
	gcc -ansi -Wall -Werror -Wextra -pedantic -c code_listener.h -o /dev/null
	gcc -ansi -Wall -Werror -Wextra -pedantic -c hash_table.h -o /dev/null

clean:
	rm -f libcl.a $(CL_OBJS)
	rm -f test-list test-all test-all.o data/dll.o
	rm -f slsparse slsparse.o sparse-internal-symbols.dot
	rm -f hash_table.o type_enumerator.o
	rm -f slplug.o slplug.so
	$(MAKE) distclean -C data

distclean: clean
	rm -f *.o

dbgsparse: slsparse
	$(GDB) -q -x slsparse.gdb

dbgplug: slplug.so
	$(GDB) -q -x slplug.gdb

dotsparse:
	$(MAKE) -C data distclean
	$(MAKE) chksparse
	$(MAKE) -C data

dotplug:
	$(MAKE) -C data distclean
	$(MAKE) chkplug
	$(MAKE) -C data

pphtml: slplug.so ../ssd/util/faketty.so
	for i in $(DATASET); do \
		LC_ALL=C \
		LD_PRELOAD=../ssd/util/faketty.so FAKE_TTY=1,2 \
			$(GCC_INSTALL)/bin/gcc $(CFLAGS) $(PLUG_CFLAGS) \
				-fplugin=./slplug.so \
				-fplugin-arg-slplug-dump-pp \
				-fplugin-arg-slplug-dump-types \
				-Wno-error -c $$i \
				2>&1 \
			| ../ssd/util/eschtml \
			| ../ssd/util/tty2html \
			> $$i.pp.html; \
	done

invpphtml: ../ssd/util/eschtml ../ssd/util/tty2html $(CILLY)
	for i in $(DATASET); do \
		echo "--- $$i"; \
		$(TIMEOUT) \
			$(CILLY) "$$i" 2>&1 \
			| ../ssd/util/eschtml \
			| ../ssd/util/tty2html \
			> $$i-inv.pp.html; \
	done

../sparse/libsparse.a: ../sparse/*.c ../sparse/*.h ../sparse/local.mk
	$(MAKE) -C ../sparse

../ssd/util/faketty.so: ../ssd/util/faketty.c
	$(MAKE) -C ../ssd/util

$(CILLY):
	$(MAKE) -C .. build_inv

distcheck:
	rm -rfv ../sl_build
	mkdir ../sl_build
	cd ../sl_build && cmake -DCFLAGS=$(CFLAGS) ../sl
	$(MAKE) -C ../sl_build
	cd ../sl_build && ctest --output-on-failure $(MAKEOPTS)

wizard:
	mkdir -p ../sl_build
	cd ../sl_build && ccmake ../sl
	$(MAKE) -C ../sl_build
	cd ../sl_build && ctest --output-on-failure $(MAKEOPTS)
