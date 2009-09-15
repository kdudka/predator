file ../gcc-install/bin/gcc
set args -fplugin=./slplug.so -std=c99 -pedantic -Wall -W -g -O0 -fPIC -c \
    -I.. -I../gcc -iquote. data/slplug.c \
    -fplugin-arg-slplug-verbose=5 \
    -fplugin-arg-slplug-dump-pp
set confirm 0
set follow-fork-mode child
set pagination 0
set print pretty
set unwindonsignal
run
echo \nbt:\n
bt
echo \nframe:\n
f 1
echo \ncode:\n
list
echo \n\n
