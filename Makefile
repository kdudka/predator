GCC_SRC = gcc-src
GCC_BUILD = gcc-build
GCC_INSTALL = gcc-install
GCC_LIBS_PREFIX ?= /usr

ABDUCT = abduction.tgz
ABDUCT_DIR = abductor_prover
ABDUCT_CIL = $(ABDUCT_DIR)/cil
INVADER = invader.zip
INVADER_DIR = invader-1_1
INVADER_SRC = $(INVADER_DIR)/sources
INVADER_CIL = $(INVADER_SRC)/cil
DNL_LIST = $(ABDUCT) $(INVADER)

SPARSE = sparse
CGT_GIT = cgt
SSD_GIT = ssd

CURL ?= curl --location -v
GIT ?= git
SVN ?= svn

.PHONY: fetch unpack \
	build_abd build_inv \
	build_gcc update_gcc update_gcc_src_only \
	sl

fetch: $(DNL_LIST) $(SPARSE) $(SSD_GIT)
unpack: $(ABDUCT_DIR) $(INVADER_DIR)

$(ABDUCT_DIR): $(ABDUCT) $(INVADER)
	if test -e $(ABDUCT_DIR); then \
			echo; \
			echo "--- not implemented yet"; \
			echo "--- please run 'rm -rf $(ABDUCT_DIR)' to force unpack..."; \
			echo "--- ... or 'touch $(ABDUCT_DIR)' to skip unpack phase"; \
			echo; \
			exit 1; \
	fi
	unzip -d $(ABDUCT_DIR) -o $(INVADER)
	cd $(ABDUCT_DIR) && mv -v $(INVADER_SRC)/* . && rm -rf $(INVADER_DIR)
	tar xf $(ABDUCT)

$(INVADER_DIR): $(INVADER)
	unzip -o $(INVADER)
	cd $(INVADER_DIR) \
		&& $(GIT) init \
		&& $(GIT) add * \
		&& $(GIT) commit -m "initial import of $(INVADER)" \
		&& $(GIT) checkout -b tools \
		&& $(GIT) am ../invader-extras/00*.patch \
		&& $(GIT) checkout -b sl

build_abd: $(ABDUCT_DIR)
	cd $(ABDUCT_CIL) && ./configure
	$(MAKE) -C $(ABDUCT_CIL) -j1

build_inv: $(INVADER_DIR)
	cd $(INVADER_CIL) && ./configure # TODO: --prefix=...
	$(MAKE) -C $(INVADER_CIL) -j1 # oops, we don't support parallel build?
	# TODO: make check ... challenge? :-)
	# TODO: make install

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

$(ABDUCT):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/abduction.tgz'

$(INVADER):
	$(CURL) -o $@ 'http://www.eastlondonmassive.org/invader-1_1.zip'

$(SPARSE):
	$(GIT) clone git://git.kernel.org/pub/scm/devel/sparse/chrisl/sparse.git $@
	cd $@ && $(GIT) checkout -b sl
	cd $@ && $(GIT) am ../sparse-extras/*.patch
	cd $@ && ln -s ../sparse-extras/local.mk

$(GCC_SRC):
	$(SVN) co svn://gcc.gnu.org/svn/gcc/trunk $@

$(CGT_GIT):
	$(GIT) clone http://git.fedorahosted.org/git/cgt.git $@

$(SSD_GIT):
	$(GIT) clone http://dudka.no-ip.org/git/ssd.git $@
