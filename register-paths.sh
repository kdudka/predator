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
printf "SL_PLUG = %s\n" "$SL_PLUG"

PATH="`dirname "$GCC_HOST"`:$PATH"
LD_LIBRARY_PATH="`dirname "$SL_PLUG"`:$LD_LIBRARY_PATH"
export PATH LD_LIBRARY_PATH

printf "\nPaths registered!  Use this to run Predator:\n
%s -fplugin=%s ...\n" \
    "`basename "$GCC_HOST"`" \
    "`basename "$SL_PLUG"`"
