#!/bin/sh
TIMEOUT="timeout 8"

echo "ATTENTION: This script is not intended to be run !!!"
echo "           Please do not commit the generated files to prevent" \
                 "a disaster..."

ticks(){
    while sleep .5; do
        printf .
    done
}

gen(){
    for i in data/test-00??.c; do
      printf -- "--- "
      printf "%s" "`basename $i .c` "
      test -n "$1" && printf "%s " "$1"
      $TIMEOUT ../gcc-install/bin/gcc \
        -m32 \
        -c `readlink -f $i` \
        -DPREDATOR \
        -fplugin=../sl_build/libsl.so $1 \
        2>&1 \
        | grep -v '\[internal location\]$' \
        | sed s/`readlink -f data | sed 's/\\//\\\\\\//g'`\\/// \
        | sed s/`readlink -f . | sed 's/\\//\\\\\\//g'`\\/// \
        > data/`basename $i .c`.err$2
      printf "\n"
    done
}

ticks &
gen "-fplugin-arg-libsl-symexec-args=fast" .fast
gen
kill $!
