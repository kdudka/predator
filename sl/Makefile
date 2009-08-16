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

CC = gcc
LD = gcc
#CFLAGS += -std=c99 -pedantic -Wall -W -g -O0 -fPIC -I$(GCC_PLUGIN_INC)
CFLAGS += -fPIC -I$(GCC_PLUGIN_INC)
CXXFLAGS += -fPIC
LDFLAGS += -fPIC -shared

GDB ?= gdb

CFLAGS += -g -O0 -Wall $(SPARSE_CFLAGS)
CXXFLAGS += -g -O0 -Wall -Werror -pedantic -std=c++0x \
			-I../ssd/src -DHAVE_ISATTY
SFLAGS += -Wno-decl

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

.PHONY: all check clean debug demo

all: demo

#TODO: dependency scanning
$(CL_OBJS): *.hh code_listener.h

slsparse: slsparse.o $(CL_OBJS) $(SPARSE_LIBS)
	$(CC) $(CFLAGS) $(SPARSE_CFLAGS) -o $@ slsparse.o $(CL_OBJS) \
		$(SPARSE_LIBS) -lboost_iostreams

slplug.so: slplug.o
	$(LD) -o $@ $(LDFLAGS) $<

test-all: LDFLAGS += -lboost_unit_test_framework
test-all: test-all.o dll.o

test-list: list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

check: test-all
	# FIXME: make more generic
	gcc -ansi -Wall -Werror -W -pedantic -c code_listener.h -o /dev/null
	./runtest.sh all --build_info=yes --log_level=test_suite

clean:
	rm -f slplug.o slplug.so
	rm -f slsparse slsparse.o test-list test-all test-all.o dll.o $(CL_OBJS)

run: slplug.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./slplug.so -c slplug.c \
		-o /dev/null

demo: check dll.c slsparse
	./slsparse $(SFLAGS) dll.c

debug: dll.c slsparse slsparse.gdb
	$(GDB) -x slsparse.gdb

chkplug: slplug.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./slplug.so -c slplug.c
	$(MAKE) slplug.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./slplug.so -c slplug.c

dbgplug: slplug.so
	$(GDB) -x slplug.gdb

../sparse/libsparse.a: ../sparse/*.c ../sparse/*.h ../sparse/local.mk
	$(MAKE) -C ../sparse
