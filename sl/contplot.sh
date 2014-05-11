#!/bin/sh
export SL_OPTS=-fplugin-arg-libsl-args=detect_containers
export SL_PLOT=1

usage() {
    echo Usage: $0 GCC_ARGS
    exit 1
}

seek_input_file() {
    for i in "$@"; do
        test -e "$i" && return 0
    done

    return 1
}

seek_input_file "$@" || usage

self="`readlink -f "$0"`"
self_dir="`dirname "$self"`"
make -j9 fast -C "$self_dir" && "${self_dir}/slgcc" "$@"
