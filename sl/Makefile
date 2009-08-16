# SPARSE_CFLAGS += `pkg-config sparse --cflags`
# SPARSE_LIBS ?= `pkg-config sparse --libs`

SPARSE_CFLAGS = -I..
SPARSE_LIBS = ../sparse/libsparse.a

GCC_SRC = ../gcc-src
GCC_BUILD = ../gcc-build
GCC_INSTALL = ../gcc-install

# FIXME: hard-coded for now
GCC_PLUGIN_INC = \
	$(GCC_INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.5.0/plugin/include

# debug flags
DBGFLAGS ?= -g -O0
CFLAGS += $(DBGFLAGS)
CXXFLAGS += $(DBGFLAGS)

CC = gcc
LD = gcc
AR ?= ar
RANLIB ?= ranlib
GDB ?= gdb

.PHONY: all check clean debug \
	chkplug chksparse chkunit chkhead \
	dbgplug dbgsparse

all: slsparse slplug.so

CL_OBJS = cl_chain.o \
	cl_factory.o \
	cl_locator.o \
	cl_pp.o \
	cld_argsub.o \
	cld_intchk.o \
	cld_unilabel.o \
	cld_uniregs.o \
	code_listener.o \
	ssd.o

libcl.a: $(CL_OBJS)
	$(AR) r $@ $(CL_OBJS)
	$(RANLIB) $@

#TODO: dependency scanning
$(CL_OBJS): code_listener.h *.hh

$(CL_OBJS): CXXFLAGS += -fPIC -std=c++0x -pedantic -Wall -Werror
ssd.o: CXXFLAGS += -I../ssd/src -DHAVE_ISATTY

slsparse.o: code_listener.h
slsparse.o: CFLAGS += -fPIC -Wall $(SPARSE_CFLAGS)

slsparse: slsparse.o libcl.a $(SPARSE_LIBS)
	$(CC) $(CFLAGS) -o $@ slsparse.o -L. -lcl $(SPARSE_LIBS) -lboost_iostreams

PLUG_CFLAGS = -fPIC -std=c99 -pedantic -Wall -W -Werror -pedantic \
	-I$(GCC_PLUGIN_INC)

slplug.o: code_listener.h
slplug.o: CFLAGS += $(PLUG_CFLAGS)

slplug.so: LDFLAGS += -fPIC -shared
slplug.so: slplug.o libcl.a
	$(LD) -o $@ $(LDFLAGS) slplug.o -lcl -L.

test-all: LDFLAGS += -lboost_unit_test_framework
test-all: test-all.o dll.o

test-list: list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

chkunit: test-all
	./runtest.sh all --build_info=yes --log_level=test_suite

chkplug: slplug.so
	$(GCC_INSTALL)/bin/gcc $(PLUG_CFLAGS) -fplugin=./slplug.so -c slplug.c
	$(MAKE) slplug.so
	$(GCC_INSTALL)/bin/gcc $(PLUG_CFLAGS) -fplugin=./slplug.so -c slplug.c

SFLAGS += -Wno-decl
chksparse: dll.c slsparse
	./slsparse $(SFLAGS) dll.c

check: chkplug chksparse chkunit chkhead

chkhead:
	$(MAKE) -f Makefile.chk
	gcc -ansi -Wall -Werror -W -pedantic -c code_listener.h -o /dev/null

clean:
	rm -f libcl.a $(CL_OBJS)
	rm -f test-list test-all test-all.o dll.o
	rm -f slsparse slsparse.o
	rm -f slplug.o slplug.so

dbgsparse: slsparse
	$(GDB) -x slsparse.gdb

dbgplug: slplug.so
	$(GDB) -x slplug.gdb

../sparse/libsparse.a: ../sparse/*.c ../sparse/*.h ../sparse/local.mk
	$(MAKE) -C ../sparse
