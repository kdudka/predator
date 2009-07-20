GCC_SRC = ../gcc-src
GCC_BUILD = ../gcc-build
GCC_INSTALL = ../gcc-install

# FIXME: hard-coded for now
GCC_PLUG_INC = \
	$(GCC_INSTALL)/lib/gcc/x86_64-unknown-linux-gnu/4.5.0/plugin/include

CC = gcc
CFLAGS += -I$(GCC_PLUG_INC)

.PHONY: clean

plugin.so: plugin.o
	$(CC) -o $@ -shared $<

clean:
	rm -f plugin.o plugin.so
