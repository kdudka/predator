.PHONY: all clean distclean *.dot

BASENAME ?= basename
DOT ?= dot
FORMAT ?= svg
SED ?= sed
TIMEOUT ?= timeout 300

all: *.dot
*.dot:
	$(TIMEOUT) \
		$(DOT) $@ -T$(FORMAT) \
		| $(SED) 's/"green"/"#00FF00"/g' \
		> `$(BASENAME) $@ .dot`.$(FORMAT) 

clean:
	rm -fv *.$(FORMAT)

distclean: clean
	rm -fv *.dot
