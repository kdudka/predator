# common code base for fa_build/fag{cc,db} and sl_build/slg{cc,db}

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

find_gcc_plug() {
    test -r "$GCC_PLUG" || GCC_PLUG="$topdir/${1}_build/lib${1}.so"
    test -r "$GCC_PLUG" || die "$2 GCC plug-in not found: ${GCC_PLUG}"
}

find_cc1_host() {
    CC1_HOST="$("$GCC_HOST" -print-prog-name=cc1)"
    test -x "$CC1_HOST" && return 0
    die "unable to find cc1: $GCC_HOST -print-prog-name=cc1"
}
