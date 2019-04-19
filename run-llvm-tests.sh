#!/bin/bash
export SELF="$0"
export LC_ALL=C
MAKE="make -j9"

if [ `uname` == Darwin ]; then
	TIMEOUT='gtimeout 5'
else
	TIMEOUT='timeout 5'
fi

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s ANALYZER [CLANG_HOST OPT_HOST]\n" "$SELF" >&2
    cat >&2 << EOF

    Use this script for checking plug-ins.

    ANALYZER is analyzer cl Code Listener
                         sl Predator
                         fa Forester

    CLANG_HOST is the absolute path to clang(1). Default is /usr/bin/clang.
    If you have multiple versions of clang/llvm installed on the system, it
    can be something like /usr/bin/clang-3.8. You can also provide a local
    build of clang, e.g. /home/bob/clang+llvm-3.8-x86_64/bin/clang.

    OPT_HOST is the absolute path to opt. Mainly in the same folder as clang.
    Default is /usr/bin/opt.

EOF
    exit 1
}

status_update() {
    printf "\n%s...\n\n" "$*"
    tty >/dev/null && printf "\033]0;%s\a" "$*"
}

if [ "$#" == 3 ]; then

    test cl = "$1" || test sl = "$1" || test fa = "$1" || usage

    HOST="$2"
    OPT_HOST="$3"

    # check the given HOST
    test "/" == "${HOST:0:1}" \
        || die "CLANG_HOST is not an absolute path: $HOST"
    test -x "$HOST" \
        || die "HOST is not an absolute path to an executable file: $HOST"

    # try to run CLANG_HOST
    "$HOST" --version || die "unable to run clang: $HOST --version"

    test "/" == "${OPT_HOST:0:1}" \
        || die "OPT_HOST is not an absolute path: $OPT_HOST"
    test -x "$OPT_HOST" \
        || die "OPT_HOST is not an absolute path to an executable file: $OPT_HOST"

    status_update "Checking whether $1 works"
    $MAKE -C $1 check CMAKE="cmake -D CLANG_HOST='$HOST' -D OPT_HOST='$OPT_HOST' -D ENABLE_LLVM=ON -D GCC_EXEC_PREFIX='${TIMEOUT}'"

else
    test 1 = "$#" || usage

    test cl = "$1" || test sl = "$1" || test fa = "$1" || usage

    status_update "Checking whether $1 works"
    $MAKE -C $1 check CMAKE="cmake -D ENABLE_LLVM=ON -D GCC_EXEC_PREFIX='${TIMEOUT}'"
fi
