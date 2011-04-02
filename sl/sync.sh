#!/bin/sh
TIMEOUT="timeout 10"

LC_ALL=C
export LC_ALL

ALL=data/test-0???.c
test -n "$1" && ALL="$*"
export ALL

echo "ATTENTION: This script is not intended to be run !!!"
echo "           Please do not commit the generated files to prevent" \
                 "a disaster..."

ticks(){
    while sleep .5; do
        printf .
    done
}

gen(){
    for i in $ALL; do
      printf -- "--- "
      printf "%s" "`basename $i .c` "
      test -n "$1" && printf "%s " "$1"
      $TIMEOUT ../gcc-install/bin/gcc \
        -m32 \
        -c $i \
        -o /dev/null \
        -DPREDATOR \
        -I../cl -I../cl/gcc -I../include -I../include/gcc \
        -fplugin=../sl_build/libsl.so $1 \
        -fplugin-arg-libsl-preserve-ec \
        2>&1 \
        | grep -v '\[internal location\]$' \
        | sed 's/ \[enabled by default\]$//' \
        | sed 's|data/||' \
        > data/`basename $i .c`.err$2
      printf "\n"
    done
}

ticks &
gen "" .fast
gen "-fplugin-arg-libsl-args=oom"
kill $!
