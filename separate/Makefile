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

test-list: list.c
	$(CC) $(CFLAGS) -DSELF_TEST -o $@ $<

check: test-list
	./runtest.sh list

clean:
	rm -f separate test-list

demo: check list.c separate
	./separate $(SFLAGS) list.c
