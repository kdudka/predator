#!/bin/bash
export SELF="$0"
export LC_ALL=C
MAKE="make -j5"

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s CLANG_HOST\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator and/or Forester against an arbitrary
    build of host Clang/LLVM. The host Clang/LLVM needs library and header files
    for develop native programs that use the LLVM infrastructure. The currently 
    supported version of host Clang/LLVM is 3.3, but feel free to use any other 
    version of Clang at your own responsibility.

    CLANG_HOST is the absolute path to clang(1). The most common location of the system 
    Clang is /usr/bin/clang. If you have multiple versions of clang/llvm installed 
    on the system, it can be something like /usr/bin/clang-3.3. You can also 
    provide a local build of clang, e.g. /home/nika/clang+llvm-3.3-x86_64/bin/clang.

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

# check the given HOST
HOST="$1"
test "/" == "${HOST:0:1}" \
    || die "CLANG_HOST is not an absolute path: $HOST"
test -x "$HOST" \
    || die "CLANG_HOST is not an absolute path to an executable file: $HOST"

# try to run CLANG_HOST
"$HOST" --version || die "unable to run clang: $HOST --version"

export CC="$HOST"
export CXX="$HOST"

echo $CXX

status_update "Trying to build Code Listener"
$MAKE -C cl CMAKE="cmake -D CLANG_HOST='$HOST' -D ENABLE_LLVM=ON" \
    || die "failed to build Code Listener"

build_analyzer() {
    test -d $1 || return 0

    status_update "Nuking working directory"
    $MAKE -C $1 distclean || die "'$MAKE -C $1 distclean' has failed"

    status_update "Trying to build $2"
    $MAKE -C $1 CMAKE="cmake -D CLANG_HOST='$HOST' -D ENABLE_LLVM=ON" \
        || return $?

}

build_analyzer sl Predator
build_analyzer fa Forester
