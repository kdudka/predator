# Makefile for glib tests of singly- and double-linked lists
# headers copied from 64bit configured glib

#LDFLAGS = -m32         # use iff 32bit configured glib

CFLAGS = -I. -I./glib $(LDFLAGS)
DRIVER=../../sl/slgcc
#DRIVER=../slgccv

all-predator:
	$(DRIVER) list.c  $(CFLAGS)
	$(DRIVER) slist.c $(CFLAGS)

PROGS = slist list
all-compile: $(PROGS)
list: list.o
slist: slist.o

clean:
	rm -f *.o $(PROGS)
