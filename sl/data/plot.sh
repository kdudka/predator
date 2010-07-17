#!/bin/sh
set -x

for i in test-0???.c; do
    test=`basename $i .c`
    mkdir $test     || exit 1
    cd $test        || exit 1
    cp -v ../$i .   || exit 1
    timeout 8 ../../slgcc $i
    ls *.dot >/dev/null 2>&1 \
        && make -f ../Makefile
    cd ..           || exit 1
done
