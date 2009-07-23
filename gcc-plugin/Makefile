GCC_SRC = ../gcc-src
GCC_BUILD = ../gcc-build
GCC_INSTALL = ../gcc-install

# FIXME: hard-coded for now
GCC_PLUGIN_INC = \
	$(GCC_INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.5.0/plugin/include

CC = gcc
LD = gcc
CFLAGS += -std=c99 -pedantic -Wall -W -g -O2 -fPIC -I$(GCC_PLUGIN_INC)
LDFLAGS += -fPIC -shared

.PHONY: all check clean run

all: check

separate.so: separate.o
	$(LD) -o $@ $(LDFLAGS) $<

clean:
	rm -f separate.o separate.so

run: separate.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./separate.so -c separate.c \
		-o tmp.o && rm -f tmp.o

check: separate.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./separate.so -c separate.c
	$(MAKE) separate.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./separate.so -c separate.c
