#!/bin/bash
export SELF="$0"
export LC_ALL=C
export CCACHE_DISABLE=1

# this is an emergency fall-back, which is known to break in certain cases
topdir="`dirname "$(readlink -f "$SELF")"`"

# if you are _NOT_ in a git repository, the 'build_gcc' target of the root
# Makefile takes care of setting $GCC_HOST to a valid absolute path to gcc
export GCC_HOST="${topdir}/gcc-install/bin/gcc"

# if you are _NOT_ in a git repository, sl/Makefile takes care of setting
# $SL_PLUG to a valid absolute path to libsl.so
export SL_PLUG="${topdir}/sl_build/libsl.so"

export MSG_INFLOOP=': warning: end of function .*() has not been reached'
export MSG_LABEL_FOUND=': error: error label "ERROR" has been reached'
export MSG_MEMLEAK=': warning: memory leak detected'
export MSG_OUR_WARNINGS=': warning: .*\[-fplugin=libsl.so\]$'
export MSG_TIME_ELAPSED=': note: clEasyRun() took '
export MSG_UNHANDLED_CALL=': warning: ignoring call of undefined function: '

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s path/to/test-case.c [-m32|-m64] [CFLAGS]\n\n" "$SELF" >&2
    cat >&2 << EOF
    The verification result (SAFE, UNSAFE, or UNKNOWN) will be printed to
    standard output.  All other information will be printed to standard error
    output.  There is no timeout or ulimit set by this script.  If these
    constraints are violated, it should be treated as UNKNOWN result.  Do not
    forget to use the -m32 option when compiling 32bit preprocessed code on a
    64bit OS.

EOF
    exit 1
}

test -r "$1" || usage

# initial checks
"$GCC_HOST" --version >/dev/null || die "unable to run gcc: $GCC_HOST --version"
test -r "$SL_PLUG"  || die "Predator GCC plug-in not found: ${SL_PLUG}"

match() {
    line="$1"
    shift
    printf "%s" "$line" | grep "$@" >/dev/null
    return $?
}

fail() {
    # exit now, it makes no sense to continue at this point
    echo UNKNOWN
    exit 1
}

report_unsafe() {
    # drop the remainder of the output
    cat > /dev/null
    echo UNSAFE
    exit 0
}

parse_gcc_output() {
    ERROR_DETECTED=no
    ENDED_GRACEFULLY=no

    while read line; do
        if match "$line" "$MSG_UNHANDLED_CALL"; then
            # call of an external function we have no model for, we have to fail
            fail

        elif match "$line" "$MSG_LABEL_FOUND"; then
            # an ERROR label has been reached
            report_unsafe

        elif match "$line" ": error: "; then
            # errors already reported, better to fail now
            fail

        elif match "$line" -E "$MSG_INFLOOP|$MSG_MEMLEAK"; then
            # memory leakage and infinite loop do not mean UNSAFE, ignore them
            continue

        elif match "$line" "$MSG_OUR_WARNINGS"; then
            # all other warnings treat as errors
            ERROR_DETECTED=yes

        elif match "$line" "$MSG_TIME_ELAPSED"; then
            # we ended up without a crash, yay!
            ENDED_GRACEFULLY=yes
        fi
    done

    if test xyes = "x$ERROR_DETECTED"; then
        fail
    elif test xyes = "x$ENDED_GRACEFULLY"; then
        echo SAFE
    else
        fail
    fi
}

"$GCC_HOST"                                             \
    -fplugin="${SL_PLUG}"                               \
    -fplugin-arg-libsl-args="error_label:ERROR"         \
    -o /dev/null -O0 -c "$@" 2>&1                       \
    | tee /dev/fd/2                                     \
    | parse_gcc_output
