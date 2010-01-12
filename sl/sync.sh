#!/bin/sh
TIMEOUT="timeout 16"

for i in data/test-00??.c; do
  printf -- "--- "
  basename $i .c
  $TIMEOUT ../gcc-install/bin/gcc \
    -c `readlink -f $i` \
    -fplugin=../sl_build/libslplug.so \
    2>&1 | sed \
    s/`echo /home/xdudka00/git/phd/tools/sl/data/ | sed 's/\\//\\\\\\//g'`// \
    > data/`basename $i .c`.err
done
