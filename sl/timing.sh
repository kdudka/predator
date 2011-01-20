#!/bin/sh
TIMEOUT="timeout 30"

LC_ALL=C
export LC_ALL

ALL=data/test-0???.c
test -n "$1" && ALL="$*"
export ALL

ticks(){
    while sleep .5; do
        printf .
    done
}

gen(){
    for i in $ALL; do
      printf -- "--- "
      printf "%s" "`basename $i .c`"
      $TIMEOUT ../gcc-install/bin/gcc \
        -m32 \
        -c $i \
        -o /dev/null \
        -DPREDATOR \
        -I../cl -I../cl/gcc -I../include -I../include/gcc \
        -fplugin=../sl_build/libsl.so -fplugin-arg-libsl-args=fast \
        -fplugin-arg-libsl-preserve-ec \
        2>&1 \
        | grep 'clEasyRun' \
        > data/`basename $i .c`.timing
      printf "\n"
    done
}

parse(){
    for i in data/*.timing; do
        if grep clEasyRun $i > /dev/null; then
            test="`basename $i .timing`"
            time="`cut -d' ' -f 5 $i`"
            printf "%s %8.3f s\n" "$test" "$time"
        fi
    done
}

LOG="timing-`git stat | head -1 | sed 's/^#.* //'`.txt"
rm -fv data/*.timing $LOG
ticks &
gen
kill $!

parse | sort -nrk2 | tee $LOG
rm -fv data/*.timing
