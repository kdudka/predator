
# makefile for linux driver analysis (using predator tool)
# prototype version

# default target:
all: cdrom

CC=gcc-4.5
CFLAGS=-m32
OBJFILES = \
invader-cdrom.o   \
invader-class.o   \
invader-ll_rw_blk.o   \
invader-md.o   \
invader-pci-driver.o   \
linux-2.6.35-cdrom.o   \
linux-2.6.35-class.o   \
linux-2.6.35-md.o   \
linux-2.6.35-pci-driver.o

cdrom:
	../slgcc linux-2.6.35-cdrom-main.c

# compile all drivers (check if can be compiled)
compile: $(OBJFILES)

clean:
	rm -f $(OBJFILES)
