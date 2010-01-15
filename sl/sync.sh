#!/bin/sh
TIMEOUT="timeout 16"

echo "ATTENTION: This script is not supposed to be run !!!"
echo "           Please do not commit generated files to prevent a disaster..."
for i in data/test-00??.c; do
  printf -- "--- "
  basename $i .c
  $TIMEOUT ../gcc-install/bin/gcc \
    -m32 \
    -c `readlink -f $i` \
    -fplugin=../sl_build/libslplug.so \
    2>&1 \
    | grep -v '\[internal location\]$' \
    | sed s/`readlink -f data | sed 's/\\//\\\\\\//g'`\\/// \
    | sed s/`readlink -f . | sed 's/\\//\\\\\\//g'`\\/// \
    > data/`basename $i .c`.err
done
