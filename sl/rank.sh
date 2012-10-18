#!/bin/bash
export SELF="$0"

export LC_ALL=C
export TIMEOUT="timeout 900"

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s path/to/check-property.sh property \
[file1 [file2 [...]]]\n" "$SELF" >&2
    exit 1
}

match() {
    grep -E "$@" >/dev/null
    return $?
}

export RUNNER="$1"
test -x "$1" || usage
shift

export PROPERTY="$1"
test memory = "$1" || test label = "$1" || usage
shift

GRAND_TOTAL=0

rank_files() {
    CORRECT_FALSE=0
    CORRECT_TRUE=0
    INCORRECT_FALSE=0
    INCORRECT_TRUE=0
    UNKNOWNS=0

    for i in "$@"; do
        HAS_BUG=no
        if echo "$i" | match "BUG|unsafe|false"; then
            HAS_BUG=yes
        fi

        MEMBUG="$(echo $i | grep -E -o "(valid-free)|(valid-deref)|(valid-memtrack)")"

        printf "%-72s\t" "$i"

        RESULT="$($TIMEOUT $RUNNER --verbose --task $PROPERTY -- $i -m32 2>/dev/null)"
        case "$( echo $RESULT | grep -E -o "(TRUE)|(FALSE)" )" in
            TRUE)
                if test xyes = "x$HAS_BUG" ; then
                    printf "\033[1;31m!TRUE!\t[-8]\n  Reason: [$( echo $RESULT | grep -E -o '(error|warning):[^\[]*')]\033[0m\n"
                    INCORRECT_TRUE=$(expr 1 + $INCORRECT_TRUE)
                else
                    printf "\033[1;32mTRUE\t[+2]\033[0m\n"
                    CORRECT_TRUE=$(expr 1 + $CORRECT_TRUE)
                fi
                ;;

            FALSE)
              if test xyes = "x$HAS_BUG" && ( test -z "$MEMBUG" || test "FALSE(p_$MEMBUG)" = "$( echo $RESULT | grep -E -o 'FALSE\(p_[a-z_-]*\)')" )  ; then
                    printf "\033[1;32mFALSE\t[+1 with luck]\033[0m\n"
                    CORRECT_FALSE=$(expr 1 + $CORRECT_FALSE)
                else
                    printf "\033[1;31m!FALSE!\t[-4]\n  Reason: [$( echo $RESULT | grep -E -o '(error|warning):[^\[]*')]\033[0m\n"
                    INCORRECT_FALSE=$(expr 1 + $INCORRECT_FALSE)
                fi
                ;;

            *)  printf "\033[1;34mUNKNOWN\t[+0]\n  Reason: [$( echo $RESULT | grep -E -o '(error|warning):[^\[]*')]\033[0m\n"
                UNKNOWNS=$(expr 1 + $UNKNOWNS)
                ;;
        esac
    done

    if test 0 -lt "$CORRECT_FALSE"; then
        printf -- "--- FALSE CORRECT:\t%2d [need to check ptrace!]\n" \
            "$CORRECT_FALSE"
    fi
    if test 0 -lt "$CORRECT_TRUE"; then
        printf -- "--- TRUE CORRECT:\t%2d [+2 each]\n" "$CORRECT_TRUE"
    fi
    if test 0 -lt "$INCORRECT_FALSE"; then
        printf -- "--- FALSE INCORRECT:\t%2d [-4 each]\n" "$INCORRECT_FALSE"
    fi
    if test 0 -lt "$INCORRECT_TRUE"; then
        printf -- "--- TRUE INCORRECT:\t%2d [-8 each]\n" "$INCORRECT_TRUE"
    fi
    if test 0 -lt "$UNKNOWNS"; then
        printf -- "--- UNKNOWNS:\t\t%2d [0]\n" "$UNKNOWNS"
    fi

    CORRECT_TRUE_SUM=$(expr 2 \* $CORRECT_TRUE)
    INCORRECT_FALSE_SUM=$(expr -4 \* $INCORRECT_FALSE)
    INCORRECT_TRUE_SUM=$(expr -8 \* $INCORRECT_TRUE)
    TOTAL=$(expr $CORRECT_FALSE + $CORRECT_TRUE_SUM + $INCORRECT_FALSE_SUM + $INCORRECT_TRUE_SUM)
    GRAND_TOTAL=$(expr $GRAND_TOTAL + $TOTAL)

    SUFFIX=
    if test 0 -lt "$CORRECT_FALSE"; then
        SUFFIX=" [with luck]"
    fi

    printf "<<< TOTAL: %d%s\n\n" "$TOTAL" "$SUFFIX"
}

rank_files "$@"
