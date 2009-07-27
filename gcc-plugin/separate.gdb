file ../gcc-install/bin/gcc
set args -fplugin=./separate.so -std=c99 -pedantic -Wall -W -g -O0 -c \
    ../separate/list.c
set confirm 0
set follow-fork-mode child
set pagination 0
set print pretty
run
echo \nbt:\n
bt
echo \nframe:\n
f 1
echo \ncode:\n
list
echo \n\n
