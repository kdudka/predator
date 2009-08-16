# uncomment this to use SPARSE installed on your system
#
# SPARSE_CFLAGS += `pkg-config sparse --cflags`
# SPARSE_LIBS ?= `pkg-config sparse --libs`

SPARSE_CFLAGS = -I..
SPARSE_LIBS = ../sparse/libsparse.a

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

.PHONY: all check clean demo

all: demo

#TODO: dependency scanning
$(CL_OBJS): *.hh code_listener.h

separate: separate.o $(CL_OBJS) $(SPARSE_LIBS)
	$(CC) $(CFLAGS) $(SPARSE_CFLAGS) -o $@ separate.o $(CL_OBJS) \
		$(SPARSE_LIBS) -lboost_iostreams

test-all: LDFLAGS += -lboost_unit_test_framework
test-all: test-all.o dll.o

test-list: list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

check: test-all
	# FIXME: make more generic
	gcc -ansi -Wall -Werror -W -pedantic -c code_listener.h -o /dev/null
	./runtest.sh all --build_info=yes --log_level=test_suite

clean:
	rm -f separate separate.o test-list test-all test-all.o dll.o $(CL_OBJS)

demo: check dll.c separate
	./separate $(SFLAGS) dll.c

debug: dll.c separate separate.gdb
	$(GDB) -x separate.gdb

../sparse/libsparse.a: ../sparse/*.c ../sparse/*.h ../sparse/local.mk
	$(MAKE) -C ../sparse
