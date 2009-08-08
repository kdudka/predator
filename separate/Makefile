# uncomment this to use SPARSE installed on your system
#
# SPARSE_CFLAGS += `pkg-config sparse --cflags`
# SPARSE_LIBS ?= `pkg-config sparse --libs`

SPARSE_CFLAGS = -I..
SPARSE_LIBS = ../sparse/libsparse.a

CFLAGS += -g -O0 -Wall
SFLAGS += -Wno-decl

.PHONY: all check clean demo

all: demo

separate: separate.c
	$(CC) $(CFLAGS) $(SPARSE_CFLAGS) -o $@ $< $(SPARSE_LIBS)

test-all: LDFLAGS += -lboost_unit_test_framework
test-all: test-all.o dll.o

test-list: list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

check: test-all
	./runtest.sh all

clean:
	rm -f separate test-list test-all test-all.o dll.o

demo: check dll.c separate
	./separate $(SFLAGS) dll.c
