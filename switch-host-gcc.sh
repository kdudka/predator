#!/bin/bash
export SELF="$0"
export LC_ALL=C
MAKE="make -j5"

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s GCC_HOST\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script to (re)build Predator and/or Forester against an arbitrary
    build of host GCC.  The host GCC needs to be built with the support for GCC
    plug-ins.  The recommended version of host GCC is 6.5.0 but Predator can be
    loaded also into older versions of GCC (plug-ins are supported since 4.5.0).
    For other versions of host GCC, please use the compatibility patches in the
    build-aux directory.

    GCC_HOST is a gcc(1) executable file that is built with the support for
    GCC plug-ins.  The most common location of the system GCC is /usr/bin/gcc.
    If you have multiple versions of gcc installed on the system, it can be
    something like /usr/bin/gcc-6.5.0.  You can also provide a local build of
    GCC, e.g.  /home/bob/gcc-6.5.0/bin/gcc.  Please avoid setting GCC_HOST to
    a ccache, distcc, or another GCC wrapper.  Such setups are not supported
    yet.

    On some Linux distributions you need to install an optional package (e.g.
    gcc-plugin-devel on Fedora) in order to be able to build GCC plug-ins.

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

# check the given GCC_HOST
GCC_HOST="$1"
if test "/" != "${GCC_HOST:0:1}"; then
    if echo "$GCC_HOST" | grep / >/dev/null; then
        # assume a relative path to GCC_HOST
        GCC_HOST="$(readlink -f "$GCC_HOST")"
    else
        # assume an executable in $PATH
        GCC_HOST="$(command -v "$GCC_HOST")"
    fi
fi
test -x "$GCC_HOST" || die "GCC_HOST is not an executable file: $1"

# try to run GCC_HOST
"$GCC_HOST" --version || die "unable to run gcc: $GCC_HOST --version"

status_update "Nuking working directory"
$MAKE distclean \
    || die "'$MAKE distclean' has failed"

status_update "Trying to build Code Listener"
$MAKE -C cl CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
    || die "failed to build Code Listener"

status_update "Checking whether Code Listener works"
$MAKE -C cl check \
    || die "Code Listener does not work"

build_analyzer() {
    test -d $1 || return 0

    status_update "Trying to build $2"
    $MAKE -C $1 CMAKE="cmake -D GCC_HOST='$GCC_HOST'" \
        || return $?

    status_update "Checking whether $2 works"
    $MAKE -C $1 check CTEST="ctest -j${NCPU}"
}

build_analyzer sl Predator                          || exit $?
