CXXFLAGS = -g -Wall -std=c++98
PROG     = symbolic-heap

all: $(PROG)

symbolic-heap: symbolic-heap.cc

run: symbolic-heap
	./symbolic-heap >symbolic-heap.out

code: code.c
	gcc -g -fdump-tree-gimple -c $<

pack: clean
	tar czvf seplog-`date -I`.tar.gz *.cc *.c *.sh Makefile *.txt example/*

git-get:
	cd git; git clone perchta:/home/git/tools
clean:
	rm -f $(PROG) *.o
