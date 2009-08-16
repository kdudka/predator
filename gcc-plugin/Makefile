GCC_SRC = ../gcc-src
GCC_BUILD = ../gcc-build
GCC_INSTALL = ../gcc-install

# FIXME: hard-coded for now
GCC_PLUGIN_INC = \
	$(GCC_INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.5.0/plugin/include

CC = gcc
LD = gcc
CFLAGS += -std=c99 -pedantic -Wall -W -g -O0 -fPIC -I$(GCC_PLUGIN_INC)
LDFLAGS += -fPIC -shared

GDB ?= gdb

.PHONY: all check clean debug run

all: check

slplug.so: slplug.o
	$(LD) -o $@ $(LDFLAGS) $<

clean:
	rm -f slplug.o slplug.so

run: slplug.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./slplug.so -c slplug.c \
		-o tmp.o && rm -f tmp.o

check: slplug.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./slplug.so -c slplug.c
	$(MAKE) slplug.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./slplug.so -c slplug.c

debug: slplug.so
	$(GDB) -x slplug.gdb
