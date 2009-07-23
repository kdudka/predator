GCC_SRC = ../gcc-src
GCC_BUILD = ../gcc-build
GCC_INSTALL = ../gcc-install

# FIXME: hard-coded for now
GCC_PLUGIN_INC = \
	$(GCC_INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.5.0/plugin/include

CC = gcc
LD = gcc
CFLAGS += -O2 -fPIC -I$(GCC_PLUGIN_INC)
LDFLAGS += -fPIC -shared

.PHONY: all clean run

all: run

plugin.so: plugin.o
	$(LD) -o $@ $(LDFLAGS) $<

clean:
	rm -f plugin.o plugin.so

run: plugin.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./plugin.so -c plugin.c \
		-o tmp.o && rm -f tmp.o
