GCC_SRC = gcc-src
GCC_BUILD = gcc-build
GCC_INSTALL = gcc-install
GCC_LIBS_PREFIX ?= /usr

INVADER = invader.zip
LIST = $(INVADER)

SPARSE = sparse
CGT_GIT = cgt
SSD_GIT = ssd

CURL ?= curl --location -v
GIT ?= git
SVN ?= svn

.PHONY: build_gcc fetch sl unpack update_gcc update_gcc_src_only

fetch: $(LIST) $(SPARSE)

unpack: fetch
	unzip -o $(INVADER)

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
				--prefix=$$TOP_LEVEL/$(GCC_INSTALL) \
				--with-gmp=$(GCC_LIBS_PREFIX) \
				--with-mpfr=$(GCC_LIBS_PREFIX); \
		fi
	cd $(GCC_BUILD) && $(MAKE)
	cd $(GCC_BUILD) && $(MAKE) -j1 install
	ln -fsvT gcc-install/lib/gcc/`ls gcc-install/lib/gcc/`/4.5.0/plugin/include gcc

update_gcc_src_only: $(GCC_SRC)
	cd $(GCC_SRC) && $(SVN) up

update_gcc: update_gcc_src_only
	$(MAKE) build_gcc

sl: $(SPARSE) $(SSD_GIT)
	test -d $(GCC_INSTALL) || $(MAKE) build_gcc
	$(MAKE) check -C sl

$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

$(SPARSE):
	$(GIT) clone git://git.kernel.org/pub/scm/devel/sparse/sparse.git $@
	cd $@ && $(GIT) checkout -b sl
	cd $@ && $(GIT) am ../sparse-extras/*.patch
	cd $@ && ln -s ../sparse-extras/local.mk

$(GCC_SRC):
	$(SVN) co svn://gcc.gnu.org/svn/gcc/trunk $@

$(CGT_GIT):
	$(GIT) clone http://git.fedorahosted.org/git/cgt.git $@

$(SSD_GIT):
	$(GIT) clone http://dudka.no-ip.org/git/ssd.git $@
