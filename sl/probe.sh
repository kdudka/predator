#!/bin/sh
export SELF="$0"

export LC_ALL=C
export TIMEOUT="timeout 4"

export CFLAGS="$CFLAGS -c -o /dev/null -O0 -m32"
test -n "$PFLAGS" || PFLAGS="error_label:ERROR"
export PFLAGS

export MSG_INTERNAL_ERROR='internal compiler error'
export MSG_LABEL_FOUND=': error: error label "ERROR" has been reached'
export MSG_LINKING_PROB='linker input file unused because linking not done'
export MSG_MEMLEAK=': warning: memory leak detected'
export MSG_PEAK_MEM_USAGE=': note: peak memory usage: '
export MSG_SIGNALLED=': note: signalled to die'
export MSG_SOME_ERROR='error: .* has detected some errors'
export MSG_SOME_WARNINGS='warning: .* has reported some warnings'
export MSG_TIME_ELAPSED=': note: clEasyRun\(\) took '

# check whether stdout is connected to a terminal
if tty 0>&1 >/dev/null; then
    # initialize coloring escape sequences
    export R="\033[1;31m"
    export G="\033[1;32m"
    export Y="\033[1;33m"
    export B="\033[1;34m"
    export N="\033[0m"
fi

die() {
    printf "%s: %s\n" "$SELF" "$*" >&2
    exit 1
}

usage() {
    printf "Usage: %s test-00.c [test-01.c [...]]\n" "$SELF" 2>&1
    exit 1
}

test -r "$1" || usage

# find gcc
self="`readlink -f "$0"`"
topdir="`dirname "$self"`/.."
GCC="$topdir/gcc-install/bin/gcc"
test -x "$GCC" || die "gcc not found: $GCC"

# try to run gcc
$TIMEOUT "$GCC" --version >&2 \
    || die "unable to run gcc: $TIMEOUT $GCC --version"

# find libsl.so
SL_PLUG="$topdir/sl_build/libsl.so"
test -r "$SL_PLUG" || die "libsl.so not found: $SL_PLUG"

wait_for_pid() {
    while test -d /proc/$1; do
        sleep .0625
        EC=$?
        if test 127 -le $EC; then
            exit $EC
        fi
    done
}

forward_signal() {
    if test -r "$GCC_PID_FILE"; then
        GCC_PID="$(<"$1")"
        test 0 -lt "$GCC_PID" || return
        printf "\nkilling %d by SIG%s ... " "$GCC_PID" "$2" >&2
        kill "-$2" "$GCC_PID" 2>/dev/null

        # wait for the compiler
        wait_for_pid "$GCC_PID"
        printf "done\n" >&2

    else
        # no gcc running?
        printf "\ngot SIG%s while there was no gcc running, time to leave...\n"\
            "$2" >&2
        exit 1
    fi
}

match() {
    grep "$@" >/dev/null
    return $?
}

handle_output() {
    # count errors
    ERRORS="$(grep ': error: ' "$1" | wc -l)"
    if match "$MSG_SOME_ERROR" "$1"; then
        ERRORS=$(expr $ERRORS - 1)
    fi

    # count warnings
    WARNINGS="$(grep ': warning: ' "$1" | wc -l)"
    if match "$MSG_SOME_WARNINGS" "$1"; then
        WARNINGS=$(expr $WARNINGS - 1)
    fi

    # check error label's reachability
    ERR_LABEL_REACHED=no
    if match "$MSG_LABEL_FOUND" "$1"; then
        ERRORS=$(expr $ERRORS - 1)
        WARNINGS=$(expr $WARNINGS - 1)
        ERR_LABEL_REACHED=yes
    fi

    # print basic status
    if match "$MSG_INTERNAL_ERROR" "$1"; then
        printf "${R}internal error${N}              "

    elif test 0 -lt $ERRORS; then
        printf "${R}%d error(s)${N}                 " $ERRORS

    elif match "$MSG_SIGNALLED" "$1"; then
        printf "${R}signalled to die${N}            "

    elif test xyes = "x$ERR_LABEL_REACHED"; then
        printf "${Y}label ERROR has been reached${N}"

    elif test 0 -lt $WARNINGS; then
        NO_LEAK_WARNS="`grep -v "$MSG_SOME_WARNINGS" "$1" | grep -v "$MSG_MEMLEAK" \
            | grep ': warning: ' | wc -l`"
        if test 0 -lt "$NO_LEAK_WARNS"; then
            printf "${Y}%d warning(s)${N}               " $WARNINGS
        else
            printf "${B}leaks memory${N}                "
        fi

    elif test 0 -eq "$2"; then
        if echo "$1" | match -F "BUG|unsafe"; then
            printf "${R}[seems broken]${N} "
        fi
        printf "${G}proven safe${N}                 "

    else
        printf "${R}exit code was${N}               " "$2"
    fi

    # print timing information and memory usage if available
    if match -E "$MSG_TIME_ELAPSED" "$1"; then
        TIME_ELAPSED="`grep -E "$MSG_TIME_ELAPSED" "$1" \
            | sed -r "s|^.*${MSG_TIME_ELAPSED}([0-9.]+ s).*$|\\1|"`"
        printf "\t%12s" "$TIME_ELAPSED"

        if match "$MSG_PEAK_MEM_USAGE" "$1"; then
            MEM_USAGE="`grep -E "$MSG_PEAK_MEM_USAGE" "$1" \
                | sed -r "s|^.*${MSG_PEAK_MEM_USAGE}([0-9.]+ MB).*$|\\1|"`"
            printf "\t%8s" "$MEM_USAGE"
        fi
    fi
}

try_one() {
    SRC="$1"
    BARE_GCC_OUT="$2"
    PREDATOR_OUT="$3"
    GCC_PID_FILE="$1.pid"

    trap "rm -f '$GCC_PID_FILE'; printf '\n'" RETURN
    trap "forward_signal '$GCC_PID_FILE' INT" SIGINT
    trap "forward_signal '$GCC_PID_FILE' QUIT; exit 130" SIGQUIT
    trap "forward_signal '$GCC_PID_FILE' TERM; exit 142" SIGTERM

    CMD="$TIMEOUT $GCC $CFLAGS $SRC"
    if eval "$CMD" >"$BARE_GCC_OUT" 2>&1; then
        if match "$MSG_LINKING_PROB" "$BARE_GCC_OUT"; then
            printf "${R}attempt to run linker${N}"
            return 1
        fi

        # turn on the plug-in
        CMD="$CMD -fplugin=$SL_PLUG -fplugin-arg-libsl-args=$PFLAGS"

        # run in background so that we can kill it easily
        CMD="$CMD -fplugin-arg-libsl-pid-file=$GCC_PID_FILE &"

        # run the job now!
        eval "$CMD" > "$PREDATOR_OUT" 2>&1

        # wait for the subshell
        wait $!
        STATUS=$?

        if test -r "$GCC_PID_FILE"; then
            # wait for the compiler
            GCC_PID="$(<"$GCC_PID_FILE")"
            wait_for_pid "$GCC_PID"
        fi

        handle_output "$PREDATOR_OUT" "$STATUS"

        if test 127 -le $STATUS; then
            printf "\n" >&2
        fi
    else
        printf "${R}compilation failed${N}"
    fi
}

while test -n "$1"; do
    short_name="`basename "$(dirname "$1")"`/`basename "$1"`"
    printf '%s\t%-48s\t' "`date +'%Y-%m-%d %H:%M:%S'`" "$short_name"
    try_one "$1" "$1-bare-gcc.err" "$1-predator.err"
    shift
done
