#!/bin/bash
export SELF="$0"

# this makes 7x speedup in case 'grep' was compiled with multi-byte support
export LC_ALL=C

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

match() {
    grep "$@" >/dev/null
    return $?
}

match_line() {
    printf "%s" "$line" | match "$@"
}

subst_line() {
    printf "%s" "$line" | sed "$@"
}

pass_line() {
    printf "%s\n" "$line" >> "$output"
}

write_desc() {
    printf " *\n * @brief %s\n *\n" "$brief_desc" >> "$output"

    if match '^..*$' "$long_desc"; then 
        sed -e 's|^\(.\)| * \1|'        \
            -e 's|^$| *|'               \
            "$long_desc" >> "$output"
    fi

    printf "%s\n" " * @attention
 * This description is automatically imported from tests/predator-regre/README.
 * Any changes made to this comment will be thrown away on the next import.
 */" >> "$output"
}

merge_test() (
    echo "$current_test - $brief_desc" >&2

    state="SEEK_BEG"
    while read -r; do
        line="$REPLY"
        case "$state" in
            SEEK_BEG)
                pass_line
                match_line '^/\*\*$' && state="SEEK_FILE"
                ;;

            SEEK_FILE)
                pass_line
                if match_line -F " */"; then
                    state=SEEK_BEG
                elif match_line -F " * @file ${current_test}"; then
                    write_desc
                    state=SEEK_END
                fi
                ;;

            SEEK_END)
                match_line -F " */" && state=DONE
                ;;

            DONE)
                pass_line
                ;;
        esac
    done

    if test $state = SEEK_BEG; then
        printf "\n/**\n * @file %s\n" "$current_test" >> "$output"
        write_desc
    fi
)

process_test() {
    test -n "$current_test" || return
    test -r "$current_test" || {
        printf "unable to open %s\n" "$current_test" >&2
        return 1
    }

    output="$(mktemp)"
    test -w "$output" || die "mktemp failed"

    # we intentionally do not use mv(1) because of symlinks etc.
    merge_test < "$current_test" && cat "$output" > "$current_test"
    rm -f "$output"

    current_test=
    >"$long_desc"
}

process_README() {
    long_desc="$(mktemp)"
    test -w "$long_desc" || die "mktemp failed"
    trap "rm -fv $long_desc" RETURN

    current_test=
    brief_desc=
    while read; do
        line="$REPLY"
        if match_line '^ *[^.]*\.c - .*$'; then
            process_test
            current_test="$(subst_line 's|^ *\([^.]*\.c\) - .*$|\1|')"
            brief_desc="$(subst_line 's|^ *[^.]*\.c - \(.*\)$|\1|')"
            continue
        fi

        match_line "^[^ ]\+" && process_test
        test -n "$current_test" || continue
        subst_line \
            -e 's|^                  |  |'   \
            -e 's|^                - |- |'   \
            >> "$long_desc"
        echo >> "$long_desc"
    done

    process_test
}

process_test_dir() (
    cd tests/$1 || die "failed to enter tests/$1"
    test -r README || die "unable to find tests/$1/README"
    process_README < README
)

test -d build-aux || die "this script needs to be run from \$PREDATOR_ROOT"
test -x build-aux/update-comments-in-tests.sh || die "unable to find self"

process_test_dir predator-regre   || die "failed to process predator-regre"
process_test_dir nspr-arena-32bit || die "failed to process nspr-arena-32bit"
