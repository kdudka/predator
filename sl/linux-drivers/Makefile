
# makefile for linux driver analysis (using predator tool)
# prototype version

# default target:
all: cdrom

CC=gcc-4.5
CFLAGS=	\
-Wall					\
-Wundef					\
-Wstrict-prototypes                     \
-Wno-trigraphs				\
-fno-strict-aliasing			\
-fno-common				\
-Wno-format-security			\
-fno-delete-null-pointer-checks		\
-O2					\
-m32					\
-msoft-float				\
-mregparm=3				\
-freg-struct-return			\
-mpreferred-stack-boundary=2		\
-march=i486				\
-mtune=generic				\
-Wa,-mtune=generic32			\
-ffreestanding				\
-Wno-sign-compare			\
-fno-asynchronous-unwind-tables		\
-mno-sse				\
-mno-mmx				\
-mno-sse2				\
-mno-3dnow				\
-Wframe-larger-than=2048		\
-fno-stack-protector			\
-fomit-frame-pointer			\
-Wdeclaration-after-statement		\
-Wno-pointer-sign			\
-fno-strict-overflow			\
-fconserve-stack                        \
# CFLAGS end

OBJFILES = \
invader-cdrom.o   \
invader-class.o   \
invader-ll_rw_blk.o   \
invader-md.o   \
invader-pci-driver.o   \
linux-2.6.35-cdrom.o   \
linux-2.6.35-class.o   \
linux-2.6.35-md.o   \
linux-2.6.35-pci-driver.o

cdrom:
	./slgcc linux-2.6.35-cdrom-main.c

# compile all drivers (check if can be compiled)
compile: $(OBJFILES)

clean:
	rm -f $(OBJFILES)
