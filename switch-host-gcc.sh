#!/bin/sh
export SELF="$0"
export LC_ALL=C

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s HOST_GCC_EXECUTABLE\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator against an arbitrary build of host
    GCC.  The host GCC needs to be built with the support for GCC plug-ins.  The
    currently supported version of host GCC is 4.6.2, but feel free to use any
    other version of GCC at your own responsibility.

    HOST_GCC_EXECUTABLE is an exectuable of gcc(1).  It can be given either with
    absolute path (e.g. /home/bob/gcc-4.7.0/bin/gcc) or, if it can be reached
    from \$PATH, only the basename is sufficient (e.g. gcc, or gcc-4.6).

EOF
    exit 1
}

test 1 = "$#" || usage

status_update() {
    printf "\n%s...\n\n" "$*"
    tty >/dev/null && printf "\033]0;%s\a" "$*"
}

# try to run gcc
GCC_HOST="$1"
"$GCC_HOST" --version || die "unable to run gcc: $GCC_HOST --version"

status_update "Nuking working directory"
make distclean \
    || die "'make distclean' has failed"

status_update "Trying to build Code Listener"
make -C cl CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
    || die "failed to build Code Listener"

status_update "Checking whether Code Listener works"
make -C cl check \
    || die "Code Listener does not work"

status_update "Trying to build Predator"
make -C sl CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
    || die "failed to build Predator"

status_update "Checking whether Predator works"
make -C sl check

status_update "Trying to build Forester"
make -C fa CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
    || die "failed to build Forester"

status_update "Checking whether Forester works"
make -C fa check
