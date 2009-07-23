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

separate.so: separate.o
	$(LD) -o $@ $(LDFLAGS) $<

clean:
	rm -f separate.o separate.so

run: separate.so
	$(GCC_INSTALL)/bin/gcc $(CFLAGS) -fplugin=./separate.so -c separate.c \
		-o tmp.o && rm -f tmp.o
