#!/bin/sh
if test "register-paths.sh" = "`basename -- $0`"; then
    printf "%s\n" \
        "It makes no sense to run this script.  It needs to be sourced:

. $0"
    exit 1
fi

# if you are _NOT_ in a git repository, the 'build_gcc' target of the root
# Makefile takes care of setting $GCC_HOST to a valid absolute path to gcc
GCC_HOST="$PWD/gcc-install/bin/gcc"
printf "GCC_HOST = %s\n" "$GCC_HOST"

# if you are _NOT_ in a git repository, sl/Makefile takes care of setting
# $SL_PLUG to a valid absolute path to libsl.so
SL_PLUG="$PWD/sl_build/libsl.so"
printf "SL_PLUG = %s\n\n" "$SL_PLUG"

if "$GCC_HOST" --version >/dev/null 2>&1; then
    if test -r "$SL_PLUG"; then
        test -x "$GCC_HOST" && PATH="`dirname "$GCC_HOST"`:$PATH"
        LD_LIBRARY_PATH="`dirname "$SL_PLUG"`:$LD_LIBRARY_PATH"
        export PATH LD_LIBRARY_PATH

        printf "Paths registered!  Use this to run Predator:\n
%s -fplugin=%s ...

Note that using GCC within this shell to build binaries is NOT recommended.
Especially, do not try to build GCC, Code Listener, or Predator itself from
this shell anymore unless you have restored the original \$PATH!\n" \
            "`basename "$GCC_HOST"`" \
            "`basename "$SL_PLUG"`"
    else
        printf "Predator GCC plug-in not found: %s, paths NOT registered!\n" \
            "$SL_PLUG" >&2
    fi
else
    printf "unable to run gcc: %s --version, paths NOT registered!\n" \
        "$GCC_HOST" >&2
fi
