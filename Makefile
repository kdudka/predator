GCC_SRC = gcc-src
GCC_BUILD = gcc-build
GCC_INSTALL = gcc-install

INVADER = invader.zip
SPARSE = sparse.tar.gz
SPARSE_GIT = sparse
LIST = $(INVADER) $(SPARSE)

CURL ?= curl --location -v
GIT ?= git
SVN ?= svn

.PHONY: build_gcc fetch plugin unpack update_gcc update_gcc_src_only

fetch: $(LIST) $(SPARSE_GIT) $(GCC_SVN)

unpack: fetch
	unzip -o $(INVADER)
	tar fvxz $(SPARSE)

build_gcc: $(GCC_SRC)
	@if test -d $(GCC_BUILD); then \
			echo; \
			echo "--- directory '$(GCC_BUILD)' exists"; \
			echo "--- configure script will NOT be run explicitly"; \
			echo "--- please run 'rm -rf $(GCC_BUILD)' if the build fails"; \
			echo; \
		else \
			set -x \
			&& mkdir $(GCC_BUILD) \
			&& TOP_LEVEL=`pwd` \
			&& cd $(GCC_BUILD) \
			&& $$TOP_LEVEL/$(GCC_SRC)/configure \
				--enable-languages=c++,c \
				--disable-multilib \
				--prefix=$$TOP_LEVEL/$(GCC_INSTALL); \
		fi
	cd $(GCC_BUILD) && $(MAKE)
	cd $(GCC_BUILD) && $(MAKE) -j1 install

update_gcc_src_only: $(GCC_SRC)
	cd $(GCC_SRC) && $(SVN) up

update_gcc: update_gcc_src_only
	$(MAKE) build_gcc

plugin:
	test -d $(GCC_INSTALL) || $(MAKE) build_gcc
	$(MAKE) run -C gcc-plugin

$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

$(SPARSE):
	$(CURL) -o $@ \
		'http://kernel.org/pub/software/devel/sparse/dist/sparse-0.4.1.tar.gz'

$(SPARSE_GIT):
	$(GIT) clone git://git.kernel.org/pub/scm/devel/sparse/sparse.git $@

$(GCC_SRC):
	$(SVN) co svn://gcc.gnu.org/svn/gcc/trunk $@

