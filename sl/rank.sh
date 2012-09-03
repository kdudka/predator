#!/bin/bash
export SELF="$0"

export LC_ALL=C
export TIMEOUT="timeout 900"

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s path/to/chk-error-label-reachability.sh \
[file1 [file2 [...]]]\n" "$SELF" >&2
    exit 1
}

match() {
    grep "$@" >/dev/null
    return $?
}

export RUNNER="$1"
test -x "$1" || usage
shift

GRAND_TOTAL=0

rank_files() {
    PLUS_ONE=0
    PLUS_TWO=0
    MINUS_TWO=0
    MINUS_FOUR=0

    for i in "$@"; do
        HAS_BUG=no
        if echo "$i" | match -E "BUG|unsafe"; then
            HAS_BUG=yes
        fi

        printf "%-96s\t" "$i"

        RESULT="$($TIMEOUT $RUNNER $i -m32 2>/dev/null)"
        case "$RESULT" in
            SAFE)
                if test xyes = "x$HAS_BUG"; then
                    printf "SAFE\t[-4]\n"
                    MINUS_FOUR=$(expr 1 + $MINUS_FOUR)
                else
                    printf "SAFE\t[+2]\n"
                    PLUS_TWO=$(expr 1 + $PLUS_TWO)
                fi
                ;;

            UNSAFE)
                if test xyes = "x$HAS_BUG"; then
                    printf "UNSAFE\t[+1 with luck]\n"
                    PLUS_ONE=$(expr 1 + $PLUS_ONE)
                else
                    printf "UNSAFE\t[-2]\n"
                    MINUS_TWO=$(expr 1 + $MINUS_TWO)
                fi
                ;;

            *)
                printf "UNKNOWN\t[+0]\n"
                ;;
        esac
    done

    if test 0 -lt "$PLUS_ONE"; then
        printf -- "--- UNSAFE CORRECT:\t%2d [need to check ptrace!]\n" \
            "$PLUS_ONE"
    fi
    if test 0 -lt "$PLUS_TWO"; then
        printf -- "--- SAFE CORRECT:\t%2d [+2 each]\n" "$PLUS_TWO"
    fi
    if test 0 -lt "$MINUS_TWO"; then
        printf -- "--- UNSAFE INCORRECT:\t%2d [-2 each]\n" "$MINUS_TWO"
    fi
    if test 0 -lt "$MINUS_FOUR"; then
        printf -- "--- SAFE INCORRECT:\t%2d [-4 each]\n" "$MINUS_FOUR"
    fi

    PLUS_TWO_SUM=$(expr 2 \* $PLUS_TWO)
    MINUS_TWO_SUM=$(expr -2 \* $MINUS_TWO)
    MINUS_FOUR_SUM=$(expr -4 \* $MINUS_FOUR)
    TOTAL=$(expr $PLUS_ONE + $PLUS_TWO_SUM + $MINUS_TWO_SUM + $MINUS_FOUR_SUM)
    GRAND_TOTAL=$(expr $GRAND_TOTAL + $TOTAL)

    SUFFIX=
    if test 0 -lt "$PLUS_ONE"; then
        SUFFIX=" [with luck]"
    fi

    printf "<<< TOTAL: %d%s\n\n" "$TOTAL" "$SUFFIX"
}

rank_files "$@"
