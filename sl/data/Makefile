.PHONY: all chkinv clean distclean *.c *.dot

BASENAME ?= basename
DOT      ?= dot
FORMAT   ?= svg
SED      ?= sed
TIMEOUT  ?= timeout 300
CILLY    ?= ../../invader-1_1/sources/cil/bin/cilly

all: *.dot
*.dot:
	$(TIMEOUT) \
		$(DOT) $@ -T$(FORMAT) \
		| $(SED) 's/"green"/"#00FF00"/g' \
		> `$(BASENAME) $@ .dot`.$(FORMAT) 

chkinv: *.c
inv-bug-??.c: $(CILLY)
	gcc -std=c99 -g -pedantic -Wall -Wextra -Werror -o test-`$(BASENAME) $@ .c` $@
	./runtest.sh `$(BASENAME) $@ .c`
	$(CILLY) > /dev/null
	if $(CILLY) $@; then \
		exit 1; else \
		exit 0; fi

inv-ign-??.c: $(CILLY)
	gcc -std=c99 -g -pedantic -Wall -Wextra -Werror -o test-`$(BASENAME) $@ .c` $@
	$(CILLY) > /dev/null
	if ./runtest.sh `$(BASENAME) $@ .c`; then \
		exit 1; else \
		exit 0; fi

clean:
	rm -fv *.$(FORMAT) test-inv-???-??

distclean: clean
	rm -fv *.dot *.pp.html

$(CILLY):
	$(MAKE) -C ../.. build_inv

