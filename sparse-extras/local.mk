CFLAGS += -O0 -DDEBUG -g3 -gdwarf-2
all-wrap: all
sep: all
	$(MAKE) -C ../separate
