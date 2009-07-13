INVADER = invader.zip
SPARSE = sparse.tar.gz
LIST = $(INVADER) $(SPARSE)

CURL = curl --location --silent -v

.PHONY: fetch unpack

unpack: $(LIST)
	unzip -o $(INVADER)
	tar fvxz $(SPARSE)

fetch: $(LIST)

$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

$(SPARSE):
	$(CURL) -o $@ 'http://kernel.org/pub/software/devel/sparse/dist/sparse-0.4.1.tar.gz'

