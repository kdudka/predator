# shellcheck shell=bash
# common code base for sl_build/sl{gcc,gdb,llvm}

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

find_gcc_host() {
    "$GCC_HOST" --version >/dev/null 2>&1 \
        && return 0

    GCC_HOST="$topdir/gcc-install/bin/gcc"
    "$GCC_HOST" --version >/dev/null \
        && return 0

    die "unable to run gcc: $GCC_HOST --version"
}

find_cc1_host() {
    CC1_HOST="$("$GCC_HOST" -print-prog-name="${GCC_PROG_NAME-cc1}")"
    test -x "$CC1_HOST" && return 0
    die "unable to find ${GCC_PROG_NAME-cc1}: $GCC_HOST -print-prog-name=${GCC_PROG_NAME-cc1}"
}

find_clang_host() {
    "$CLANG_HOST" --version >/dev/null 2>&1 \
        && return 0

    CLANG_HOST="/usr/local/bin/clang"
    "$CLANG_HOST" --version >/dev/null \
        && return 0

    die "unable to run clang: $CLANG_HOST --version"
}

find_opt_host() {
    "$OPT_HOST" --version >/dev/null 2>&1 \
        && return 0

    OPT_HOST="/usr/local/bin/opt"
    "$OPT_HOST" --version >/dev/null \
        && return 0

    die "unable to run opt: $OPT_HOST --version"
}

find_plug() {
    test -r "${!1}" || eval $1="$topdir/${2}_build/lib${2}.so"
    test -r "${!1}" || die "$3 plug-in not found: ${!1}"
}
