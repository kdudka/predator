INVADER = invader.zip
SPARSE = sparse.tar.gz
SPARSE_GIT = sparse
LIST = $(INVADER) $(SPARSE)

CURL ?= curl --location -v
GIT ?= git

.PHONY: fetch unpack

fetch: $(LIST) $(SPARSE_GIT)

unpack: fetch
	unzip -o $(INVADER)
	tar fvxz $(SPARSE)

$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

$(SPARSE):
	$(CURL) -o $@ 'http://kernel.org/pub/software/devel/sparse/dist/sparse-0.4.1.tar.gz'

$(SPARSE_GIT):
	test -d $(SPARSE_GIT) \
		|| $(GIT) clone git://git.kernel.org/pub/scm/devel/sparse/sparse.git \
		$(SPARSE_GIT)
