Predator
========
Predator is a tool for automated formal verification of sequential C programs
operating with pointers and linked lists. The core algorithms of Predator were
originally inspired by works on *separation logic* with higher-order list
predicates, but they are now purely graph-based and significantly extended to
support various forms of low-level memory manipulation used in system-level
code.  Such operations include pointer arithmetic, safe usage of invalid
pointers, block operations with memory, reinterpretation of the memory contents,
address alignment, etc.  The tool can be loaded into *GCC* as a *plug-in*.  This
way you can easily analyse C code sources, using the existing build system,
without manually preprocessing them first.  The analysis itself is, however, not
ready for complex projects yet.  You can find the latest news on the following
project page:

    https://www.fit.vutbr.cz/research/groups/verifit/tools/predator

Building from sources against the system GCC
============================================
This method requires a build of GCC with plug-in support compiled in (not
necessarily the system one).  The currently supported version of GCC is 11.3.0
and the test-suite is guaranteed to fully succeed only against this version
of GCC.  However, the Predator plug-in itself is known to work well also with
other GCC versions ranging from 4.5.0 to 12.1.0.

(1) Install all dependences of Predator
---------------------------------------
   * C/C++ compiler
     - Predator currently supports GCC 4.1.2+ and Clang 3.2+
     - the other analyzers are known to work with GCC 4.6.4+
     - try to install `g++` on Ubuntu, or `gcc-c++` (or `clang`) on Fedora

   * CMake 2.8+
     - available at https://www.cmake.org
     - the executable is called `cmake`
     - usually provided by a package called `cmake`

   * Boost libraries 1.37+
     - available at https://www.boost.org/
     - package is usually called `boost`
     - on binary distros you may need also the `boost-devel` sub-package

(2) Build Predator from sources
-------------------------------
You can use the `switch-host-gcc.sh` script to rebuild Predator against a GCC
build of your choice.  For details, please run `switch-host-gcc.sh` without any
arguments.  The script needs to be run in the same directory as this README is.
If it fails, make sure your environment is sane.  Especially check the PATH and
LD_LIBRARY_PATH environment variables.  On recent Linux distributions, you can
simply use the following command:

    ./switch-host-gcc.sh /usr/bin/gcc

(3) Prepare environment for running Predator transparently [optional]
---------------------------------------------------------------------
To employ the Predator plug-in in a build of an arbitrary project fully
transparently, it is recommended to set certain environment variables.  You can
use the script register-paths.sh to do this automatically.

    . sl_build/register-paths.sh

Then you can simply use the `-fplugin` option of GCC as documented in the gcc(1)
man page:

    gcc -fplugin=libsl.so ...

This will show you the available options of the Predator plug-in itself
(detailed description [here](docs/options.md)):

    echo please help | gcc -fplugin=libsl.so -fplugin-arg-libsl-help -xc -

Building from sources against the system LLVM [experimental]
============================================================
The currently supported version of LLVM is 10.0.0 and the test-suite is
guaranteed to fully succeed only against this version of LLVM after applying
the patch `build-aux/llvm.patch`.

(1) Install all dependences of Predator
---------------------------------------
   * C/C++ compiler, CMake 2.8+ and Boost libraries 1.37+
     - same as for GCC

   * LLVM develop library and header files
     - on Fedora provided by a package called `llvm-devel`

(2) Build Predator from sources
-------------------------------
You can use the `switch-host-llvm.sh` script to rebuild Code Listener and
Predator against a LLVM build of your choice.  For details, please run
`switch-host-llvm.sh` without any arguments.  The script needs to be run in the
same directory as this README is.  If it fails, make sure your environment is
sane.  Especially check the PATH and LD_LIBRARY_PATH environment variables.
On recent Linux distributions, you can simply use the following command:

    make llvm

or

    ./switch-host-llvm.sh /usr/share/llvm/cmake

(3) Usage
---------
Create bitcode file:

    clang -g -S -emit-llvm source.c -o source.bc

Starting analysis (detailed description for opt options [here](docs/options.md)):

    opt source.bc -o /dev/null -lowerswitch -load ./sl_build/libsl.so -sl -help

Or directly (without additional options) [only on Linux]:

    clang -g source.c -Xclang -load -Xclang ./sl_build/libsl.so

Dependencies for Darwin (macOS)
===============================
GCC plug-in has .so suffix and LLVM plug-in has .dylib suffix as usual for
Darwin.  You can install some of these dependencies using Homebrew available at
https://brew.sh/:

    brew install cmake boost boost-build
    brew install gcc@10                  # only for GCC plug-in
    brew install llvm@10                 # only for LLVM plug-in
    brew install coreutils               # only for scripts: gtimeout, gstdbuf

The currently supported version of GCC is 10.2.0.
The currently supported version of LLVM is 10.0.0.

Building GCC from sources [emergency solution]
==============================================
Use this if your system GCC does not support loading (or building) GCC plug-ins.

(1) Install all dependences of GCC
----------------------------------
   * GMP library
     - available at https://gmplib.org/
     - package is usually called `gmp`
     - on binary distros you may need also the `gmp-devel` sub-package

   * MPC library
     - available at https://www.multiprecision.org/
     - package is usually called `mpc` or `libmpc`
     - on binary distros you may need also the `libmpc-devel` sub-package

   * MPFR library
     - available at https://www.mpfr.org/
     - package is usually called `mpfr`
     - on binary distros you may need also the `mpfr-devel` sub-package

(2) Build the GCC compiler from sources
---------------------------------------
This step requires approximately 2 GB of free disk space.  The following command
downloads the sources of a stable GCC release, builds them in a minimalistic way
and install them into a local directory:

    make build_gcc

The above step is the most time-consuming step, which can take from 5 minutes to
several hours, depending on your network bandwidth, machine performance and the
current load of the machine.  The download step can be accelerated by selecting
a closer mirror to download GCC from.  The build step can be accelerated by
giving the option `-jN` to `make`, where N is the number of CPU cores plus one
(a commonly used heuristic, there exist several others).  But first make sure
that your machine has enough resources for building GCC in parallel.
