#
# makefile for SepLog experiment
# vim:tw=120
#

CXXFLAGS = -g -Wall -std=c++98
PROG     = symbolic-heap

GITDIR=git/tools/seplog
all: $(PROG)

symbolic-heap: symbolic-heap.cc

run: symbolic-heap
	./symbolic-heap >symbolic-heap.out

code: code.c
	gcc -g -fdump-tree-gimple -c $<

pack: clean
	tar czvf seplog-`date -I`.tar.gz *.cc *.c *.sh Makefile *.txt doxygen.config ##example/*

doc:
	doxygen doxygen.config

git-get:
	cd git && git clone perchta:/home/git/tools

git-push:
	cp Makefile symbolic-heap.cc TODO.txt $(GITDIR)
	cd $(GITDIR) && sed -i 's/  *$$//' *.cc; git commit * && git push

clean:
	rm -f $(PROG) *.o

