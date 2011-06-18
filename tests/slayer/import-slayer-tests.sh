#!/bin/bash
SELF="$0"
SLAY_DIR="$(readlink -f "$1")"
TEST_DIR="$(readlink -f "$2")"

eprintf() {
    printf "$@" 2>&1
}

usage() {
    eprintf "Usage: %s SLAY_DIR TEST_DIR\n" "$SELF"
    exit 1
}

chk_slay_dir() {
    trap "set +x" RETURN
    set -x

    test -d "$SLAY_DIR/test"                || return
    test -r "$SLAY_DIR/test/slayer.h"       || return
    test -d "$SLAY_DIR/test/kmdf/1394"      || return
    test -d "$SLAY_DIR/test/other"          || return
    test -d "$SLAY_DIR/test/sll"            || return

    set +x
    eprintf -- "--- SLAY_DIR seems to be valid\n\n"
}

chk_data_dir() {
    trap "set +x" RETURN
    set -x

    test -d "$TEST_DIR"                     || return
    test -d "$TEST_DIR/nspr-arena-32bit"    || return
    test -d "$TEST_DIR/predator-regre"      || return
    mkdir -p "$TEST_DIR/slayer"             || return

    set +x
    eprintf -- "--- TEST_DIR seems to be valid\n\n"
}

import() {
    # make sure we have enough permissions to read the file
    test -r "$1" || chmod -v "u+r" "$1"

    # copy the file and set its permissions
    abs_path="$TEST_DIR/slayer/$(basename $1)"
    install -vT -m0644 "$1" "$abs_path"

    # create a symlink
    rel_path="../slayer/$(basename $1)"
    ln -sfvT "$rel_path" "$2"

    # convert newlines
    dos2unix "$abs_path"

    # fix include directives
    sed -e 's|"../common.h"|"common.h"|'        \
        -e 's|"../.*harness.h"|"harness.h"|'    \
        -e 's|<harness.h>|"harness.h"|'         \
        -e 's|<slayer.h>|"slayer.h"|'           \
        -i "$abs_path"
}

chk_slay_dir || usage
chk_data_dir || usage

eprintf ">>> importing include files...\n"
find "$SLAY_DIR/test" -name "*.h" | while read file; do
    base="$(basename "$file")"
    eprintf -- "--- %s\n" "$base"
    import "$file" "$TEST_DIR/predator-regre/$base"
    eprintf "\n"
done

N=700
eprintf ">>> importing test-cases...\n"
find "$SLAY_DIR/test" -name "*.c" \
        | LC_ALL=en_US.UTF-8 sort \
        | while read file
do
    base="$(basename "$file")"
    name=$(printf "test-%04d.c" $N)
    dst="$TEST_DIR/predator-regre/$name"
    eprintf -- "--- %s\n" "$base"
    import "$file" "$dst"
    printf "\n/* original file name: %s */\n" "$base" >> "$dst" 
    N=$(($N + 1))
    eprintf "\n"
done

eprintf ">>> patching...\n"
shopt failglob >/dev/null
for file in "$TEST_DIR"/slayer/patches/00??-*.patch; do
    base="$(basename "$file")"
    eprintf -- "--- %s\n" "$base"
    patch -p1 -d "$TEST_DIR/.." < "$file"
    eprintf "\n"
done
