#!/bin/bash
export SELF="$0"
export LC_ALL=C

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s LLVM_DIR\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator against an arbitrary build of host
    Clang/LLVM. The host Clang/LLVM needs library and header files for develop
    native programs that use the LLVM infrastructure. The currently supported
    version of host Clang/LLVM is 10.0.0 but Predator can be loaded also into
    older versions of LLVM (3.5+, but correct output is guaranteed form 4).

    LLVM_DIR is the absolute path to the folder where LLVMConfig.cmake is found.
    Can be found using the command 'llvm-config –cmakedir'. The most common
    location is /usr/lib/cmake/llvm or /usr/local/share/llvm/cmake.
    If you have multiple versions of Clang/LLVM installed on the system, it can
    be something like /usr/lib/cmake/llvm-10.0.0. You can also provide a local build,
    e.g. <INSTALL_PREFIX>/lib<LIB_SUFFIX>/cmake/llvm.

    On some Linux distributions you need to install an optional package (e.g.
    llvm-devel on Fedora) in order to be able to build LLVM pass.

EOF
    exit 1
}

test 1 = "$#" || usage

status_update() {
    printf "\n%s...\n\n" "$*"
    tty >/dev/null && printf "\033]0;%s\a" "$*"
}

# number of processor units
NCPU="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"
MAKE="make -j${NCPU}"

# check the given LLVM_DIR
LLVM_DIR="$1"
if test "/" != "${LLVM_DIR:0:1}"; then
    if echo "$LLVM_DIR" | grep / >/dev/null; then
        # assume a relative path to LLVM_DIR
        LLVM_DIR="$(realpath "$LLVM_DIR")"
    else
        # assume an executable in $PATH
        LLVM_DIR="$(command -v "$LLVM_DIR")"
    fi
fi
test -r "$LLVM_DIR/LLVMConfig.cmake" \
    || die "LLVM_DIR is not an absolute path to an readable file: $LLVM_DIR/LLVMConfig.cmake"

status_update "Trying to build Code Listener"
$MAKE -C cl CMAKE="cmake -D LLVM_DIR='$LLVM_DIR' -D ENABLE_LLVM=ON" \
    || die "failed to build Code Listener"

status_update "Checking whether Code Listener works"
$MAKE -C cl check CMAKE="cmake -D ENABLE_LLVM=ON" \
    || die "Code Listener does not work"
status_update "System hopes yes"

status_update "Traying to build LLVM passes"
$MAKE build_passes CMAKE="cmake -D LLVM_DIR='$LLVM_DIR'" \
    || die "failed to build LLVM passes"

build_analyzer() {
    test -d $1 || return 0

    status_update "Nuking working directory"
    $MAKE -C $1 distclean || die "'$MAKE -C $1 distclean' has failed"

    status_update "Trying to build $2"
    $MAKE -C $1 CMAKE="cmake -D LLVM_DIR='$LLVM_DIR' -D ENABLE_LLVM=ON" \
        || return $?

    status_update "Checking whether $2 works"
    $MAKE -C $1 check CMAKE="cmake -D ENABLE_LLVM=ON" CTEST="ctest -j${NCPU}  --progress --output-on-failure"
}

# apply patch
patch -p1 < build-aux/llvm.patch

build_analyzer sl Predator                          || exit $?
